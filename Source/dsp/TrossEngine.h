// TRoss 606 engine: the drum voices from the Max for Live device (GenVoices.h, generated from its
// gen~ code) plus the pattern memory and sequencer logic from the device's JavaScript.
// No JUCE in here, so it can be tested on its own (tools/render_test.cpp).
#pragma once
#include "GenVoices.h"
#include <atomic>
#include <memory>

namespace tross606
{
enum Voice { BD, SD, LT, HT, CP, CH, OH, CY, kNumVoices };
// sequencer rows: 0 = accent, 1..8 = the voices above (row = voice + 1)
constexpr int kNumRows = 9, kNumSteps = 16, kNumSlots = 12;

// keyboard note -> voice (same map as the device's MIDI input), -1 = not mapped
inline int noteToVoice (int n)
{
    switch (n)
    {
        case 36: case 35:                   return BD;
        case 38: case 40:                   return SD;
        case 45: case 41: case 43:          return LT;
        case 50: case 47: case 48:          return HT;
        case 39:                            return CP;
        case 42: case 44:                   return CH;
        case 46:                            return OH;
        case 49: case 57: case 51: case 52: case 55: return CY;
        default:                            return -1;
    }
}

// Sound parameters, same names and units as the device
struct SoundParams
{
    float master = -6, accent = 50;
    float lvl[kNumVoices]  { 0, 0, 0, 0, 0, 0, 0, 0 };   // dB, -70..6
    float tune[kNumVoices] { 2.2f, 0, 0, 0, 0, 0, 0, 0 }; // semitones
    float decay[kNumVoices] { 27.2f, 80, 80, 80, 80, 70, 70, 70 }; // %
    float bdAttack = 21.3f, sdSnappy = 75, cpNoise = 50, cyTone = 50;
    bool mute[kNumVoices] {}, solo[kNumVoices] {};
};

// 8 slots x 9 rows x 16 steps, one 16-bit mask per row. Edited from the UI thread,
// read and recorded into from the audio thread, so every row is atomic.
class PatternMemory
{
public:
    uint16_t get (int slot, int row) const          { return masks[slot][row].load (std::memory_order_relaxed); }
    void set (int slot, int row, uint16_t m)       { masks[slot][row].store (m, std::memory_order_relaxed); ++version; }
    bool cell (int slot, int row, int step) const  { return (get (slot, row) >> step) & 1; }
    void setCell (int slot, int row, int step, bool on)
    {
        auto m = get (slot, row);
        m = on ? (uint16_t) (m | (1u << step)) : (uint16_t) (m & ~(1u << step));
        set (slot, row, m);
    }
    void clear (int slot) { for (int r = 0; r < kNumRows; ++r) set (slot, r, 0); }
    int getVersion() const { return version.load(); }

private:
    std::atomic<uint16_t> masks[kNumSlots][kNumRows] {};
    std::atomic<int> version { 0 };
};

class Engine
{
public:
    Engine() : g (std::make_unique<GenVoices>()) {}

    void prepare (double sampleRate)
    {
        g = std::make_unique<GenVoices>();
        g->fs_ = sampleRate;
        for (auto& c : trigCount) c = 0;
    }

    void setParams (const SoundParams& p)
    {
        auto& v = *g;
        v.master = p.master;  v.accent = p.accent;
        v.bd_lvl = p.lvl[BD]; v.bd_tune = p.tune[BD]; v.bd_decay = p.decay[BD]; v.bd_attack = p.bdAttack;
        v.sd_lvl = p.lvl[SD]; v.sd_tune = p.tune[SD]; v.sd_decay = p.decay[SD]; v.sd_snappy = p.sdSnappy;
        v.lt_lvl = p.lvl[LT]; v.lt_tune = p.tune[LT]; v.lt_decay = p.decay[LT];
        v.ht_lvl = p.lvl[HT]; v.ht_tune = p.tune[HT]; v.ht_decay = p.decay[HT];
        v.cp_lvl = p.lvl[CP]; v.cp_tune = p.tune[CP]; v.cp_decay = p.decay[CP]; v.cp_noise = p.cpNoise;
        v.ch_lvl = p.lvl[CH]; v.ch_tune = p.tune[CH]; v.ch_decay = p.decay[CH];
        v.oh_lvl = p.lvl[OH]; v.oh_tune = p.tune[OH]; v.oh_decay = p.decay[OH];
        v.cy_lvl = p.lvl[CY]; v.cy_tune = p.tune[CY]; v.cy_decay = p.decay[CY]; v.cy_tone = p.cyTone;
        double* mute[] = { &v.mute_bd, &v.mute_sd, &v.mute_lt, &v.mute_ht, &v.mute_cp, &v.mute_ch, &v.mute_oh, &v.mute_cy };
        double* solo[] = { &v.solo_bd, &v.solo_sd, &v.solo_lt, &v.solo_ht, &v.solo_cp, &v.solo_ch, &v.solo_oh, &v.solo_cy };
        for (int i = 0; i < kNumVoices; ++i) { *mute[i] = p.mute[i] ? 1 : 0; *solo[i] = p.solo[i] ? 1 : 0; }
    }

    // a hit from the keyboard (velocity 1..127; above 100 adds accent, like the device)
    void trigger (int voice, int velocity)
    {
        auto& v = *g;
        double* tr[] = { &v.tr_bd, &v.tr_sd, &v.tr_lt, &v.tr_ht, &v.tr_cp, &v.tr_ch, &v.tr_oh, &v.tr_cy };
        trigCount[voice] = (trigCount[voice] + 1) % 65536;
        *tr[voice] = (velocity & 127) + 128.0 * trigCount[voice];
    }

    // the sequencer reached `step`; rows are this pattern's masks (row 0 = accent)
    void sequencerStep (int step, const uint16_t* rows, bool seqOn)
    {
        auto& v = *g;
        v.m_ac = rows[0]; v.m_bd = rows[1]; v.m_sd = rows[2]; v.m_lt = rows[3]; v.m_ht = rows[4];
        v.m_cp = rows[5]; v.m_ch = rows[6]; v.m_oh = rows[7]; v.m_cy = rows[8];
        v.seqon = seqOn ? 1 : 0;
        v.stp = step;
        v.seqtick = std::fmod (v.seqtick + 1.0, 65536.0);
    }

    inline float tick() { return (float) g->tick(); }

private:
    std::unique_ptr<GenVoices> g;
    int trigCount[kNumVoices] {};
};
} // namespace tross606
