// TRoss 606 engine: the drum voices (GenVoices.h) plus the pattern memory and sequencer logic.
// No JUCE in here, so it can be tested on its own (tools/render_test.cpp).
#pragma once
#include "GenVoices.h"
#include <algorithm>
#include <cmath>
#include <atomic>
#include <memory>

namespace tross606
{
enum Voice { BD, SD, LT, HT, CP, CH, OH, CY, kNumVoices };
// sequencer rows: 0 = accent, 1..8 = the voices above (row = voice + 1)
constexpr int kNumRows = 9, kNumSteps = 16, kNumSlots = 12;

// keyboard note -> voice (General MIDI drum layout), -1 = not mapped
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

// Sound parameters (units as shown on the panel)
struct SoundParams
{
    float master = -6, accent = 50;
    float lvl[kNumVoices]  { 0, 0, 0, 0, 0, 0, 0, 0 };   // dB, -70..6
    float tune[kNumVoices] { 1.0f, 0, 0, 0, 2.6f, 0, 0, 0 }; // semitones
    float decay[kNumVoices] { 15.8f, 75.5f, 80, 80, 100.0f, 70, 70, 70 }; // %
    float bdAttack = 0.0f, sdSnappy = 57.2f, cpNoise = 37.9f, cyTone = 50;
    bool mute[kNumVoices] {}, solo[kNumVoices] {};
    bool ownOut[kNumVoices] {};   // true = the drum leaves the main mix and plays on its own output
};

// ---- how a flam plays, per step: one byte (same format as TRoss Seq).
//   bits 0-1: grace notes  (0 or 1 = flam: 1, 2 = drag: 2, 3 = ruff: 3)
//   bits 2-4: spacing      (0 = default 25 ms, 1..4 = the kSpacingMs presets)
//   bits 5-7: grace level  (0 = default 60 %, 1..4 = the kLevelPct presets)
struct FlamStyle
{
    static constexpr int kNumSpacings = 5, kNumLevels = 5;
    static constexpr float kSpacingMs[kNumSpacings] = { 25.0f, 12.0f, 25.0f, 40.0f, 60.0f };   // [0] = default
    static constexpr float kLevelPct[kNumLevels]    = { 60.0f, 30.0f, 50.0f, 75.0f, 100.0f };  // [0] = default
    static constexpr float kMaxSpacingMs = 60.0f;   // sets how far ahead grace notes are looked for

    int graces = 1, spacing = 0, level = 0;

    static FlamStyle decode (uint8_t b)
    {
        FlamStyle f;
        f.graces = (b & 3) <= 1 ? 1 : (b & 3);
        f.spacing = std::min ((b >> 2) & 7, kNumSpacings - 1);
        f.level = std::min ((b >> 5) & 7, kNumLevels - 1);
        return f;
    }
    uint8_t encode() const
    {
        const int gr = graces <= 1 ? 0 : std::min (graces, 3);
        return (uint8_t) (gr | (std::clamp (spacing, 0, kNumSpacings - 1) << 2) | (std::clamp (level, 0, kNumLevels - 1) << 5));
    }
    float spacingMs() const { return kSpacingMs[spacing]; }
    float levelPct() const  { return kLevelPct[level]; }
};

// ---- parameter locks: per drum, per step, per lockable knob. One byte each: 0 = no lock,
// 1..255 = the knob's normalised position (0..1) in 254 steps.
enum LockParam { LockTune = 0, LockDecay, LockLevel, LockExtra, kNumLockParams };
inline bool hasExtraKnob (int voice) { return voice == BD || voice == SD || voice == CP || voice == CY; }
inline uint8_t encodeLock (float norm) { return (uint8_t) (1 + std::lround (std::clamp (norm, 0.0f, 1.0f) * 254.0f)); }
inline float decodeLock (uint8_t b)    { return (float) (b - 1) / 254.0f; }

// 12 slots x 9 rows x 16 steps, one 16-bit mask per row. Edited from the UI thread,
// read and recorded into from the audio thread, so every row is atomic.
class PatternMemory
{
public:
    uint16_t get (int slot, int row) const          { return masks[slot][row].load (std::memory_order_relaxed); }
    void set (int slot, int row, uint16_t m)
    {
        masks[slot][row].store (m, std::memory_order_relaxed);
        // writing a row also drops flams on steps that are now off
        const auto before = flams[slot][row].load (std::memory_order_relaxed);
        if ((before & ~m) != 0) setFlams (slot, row, (uint16_t) (before & m));
        ++version;
    }

    // ---- flams: grace notes just before a step (drum rows only; a flam needs its step on)
    uint16_t getFlams (int slot, int row) const     { return flams[slot][row].load (std::memory_order_relaxed); }
    void setFlams (int slot, int row, uint16_t m)
    {
        const auto before = flams[slot][row].load (std::memory_order_relaxed);
        const auto now = (uint16_t) (m & get (slot, row));
        flams[slot][row].store (now, std::memory_order_relaxed);
        for (int s = 0; s < kNumSteps; ++s)   // a flam that's gone forgets its style
            if ((((before & ~now) >> s) & 1) != 0) styles[slot][row][s].store (0, std::memory_order_relaxed);
        ++version;
    }
    uint8_t getFlamStyle (int slot, int row, int step) const  { return styles[slot][row][step].load (std::memory_order_relaxed); }
    void setFlamStyle (int slot, int row, int step, uint8_t b) { styles[slot][row][step].store (b, std::memory_order_relaxed); ++version; }
    FlamStyle flamStyle (int slot, int row, int step) const   { return FlamStyle::decode (getFlamStyle (slot, row, step)); }
    bool flam (int slot, int row, int step) const  { return (getFlams (slot, row) >> step) & 1; }
    void setFlam (int slot, int row, int step, bool on)
    {
        if (row == 0) return;   // no flams on the accent row
        auto m = getFlams (slot, row);
        m = on ? (uint16_t) (m | (1u << step)) : (uint16_t) (m & ~(1u << step));
        setFlams (slot, row, m);
    }
    // ---- parameter locks (voice 0..7). A lock stays when its note is switched off, and plays again when it's back
    uint8_t lock (int slot, int voice, int step, int param) const { return locks[slot][voice][step][param].load (std::memory_order_relaxed); }
    void setLock (int slot, int voice, int step, int param, uint8_t b)
    {
        locks[slot][voice][step][param].store (b, std::memory_order_relaxed);
        ++version;
    }
    bool stepHasLock (int slot, int voice, int step) const
    {
        for (int k = 0; k < kNumLockParams; ++k) if (lock (slot, voice, step, k) != 0) return true;
        return false;
    }
    void clearLocks (int slot, int voice, int param)   // voice / param -1 = all
    {
        for (int v = 0; v < kNumVoices; ++v)
            for (int s = 0; s < kNumSteps; ++s)
                for (int k = 0; k < kNumLockParams; ++k)
                    if ((voice < 0 || v == voice) && (param < 0 || k == param)) locks[slot][v][s][k].store (0, std::memory_order_relaxed);
        ++version;
    }
    bool hasLocks (int slot) const
    {
        for (int v = 0; v < kNumVoices; ++v)
            for (int s = 0; s < kNumSteps; ++s)
                for (int k = 0; k < kNumLockParams; ++k)
                    if (lock (slot, v, s, k) != 0) return true;
        return false;
    }

    bool cell (int slot, int row, int step) const  { return (get (slot, row) >> step) & 1; }
    void setCell (int slot, int row, int step, bool on)
    {
        auto m = get (slot, row);
        m = on ? (uint16_t) (m | (1u << step)) : (uint16_t) (m & ~(1u << step));
        set (slot, row, m);
    }
    void clear (int slot) { for (int r = 0; r < kNumRows; ++r) set (slot, r, 0); clearLocks (slot, -1, -1); }
    void copySlot (int from, int to)
    {
        for (int r = 0; r < kNumRows; ++r)
        {
            set (to, r, get (from, r));
            setFlams (to, r, getFlams (from, r));
            for (int s = 0; s < kNumSteps; ++s) setFlamStyle (to, r, s, getFlamStyle (from, r, s));
        }
        for (int v = 0; v < kNumVoices; ++v)
            for (int s = 0; s < kNumSteps; ++s)
                for (int k = 0; k < kNumLockParams; ++k)
                    locks[to][v][s][k].store (lock (from, v, s, k), std::memory_order_relaxed);
        ++version;
    }
    bool isEmpty (int slot) const
    {
        for (int r = 1; r < kNumRows; ++r)   // the accent row alone plays nothing
            if (get (slot, r) != 0) return false;
        return true;
    }
    int getVersion() const { return version.load(); }

private:
    std::atomic<uint16_t> masks[kNumSlots][kNumRows] {};
    std::atomic<uint16_t> flams[kNumSlots][kNumRows] {};
    std::atomic<uint8_t> styles[kNumSlots][kNumRows][kNumSteps] {};
    std::atomic<uint8_t> locks[kNumSlots][kNumVoices][kNumSteps][kNumLockParams] {};
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

    // a hit with parameter locks: vals[k] = the real value (dB, st, %) or NaN = the knob. The values stay
    // until the drum's next hit (Level keeps acting on the ringing sound).
    void setVoiceLocks (int voice, const float* vals)
    {
        bool any = false;
        for (int k = 0; k < kNumLockParams; ++k) { active[voice][k] = vals != nullptr ? vals[k] : NAN; any |= ! std::isnan (active[voice][k]); }
        if (any || hadLocks[voice]) writeVoice (voice);
        hadLocks[voice] = any;
    }

    void setParams (const SoundParams& p)
    {
        base = p;
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
        for (int i = 0; i < kNumVoices; ++i) { *mute[i] = p.mute[i] ? 1 : 0; *solo[i] = p.solo[i] ? 1 : 0; ownOut[i] = p.ownOut[i]; }
        masterGain = p.master > -69.9f ? std::pow (10.0, p.master / 20.0) : 0.0;
        for (int i = 0; i < kNumVoices; ++i) if (hadLocks[i]) writeVoice (i);
    }

    // a hit from the keyboard (velocity 1..127; above 100 adds accent)
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

    // a drum's own output isn't there (standalone, or switched off by the host): keep it in the main mix
    void setOwnOut (int voice, bool on) { ownOut[voice] = on; }

    // one sample of the full mix (every drum, times Volume)
    inline float tick() { return (float) g->tick(); }

    // one sample with routing: returns the main mix (drums on MAIN, times Volume);
    // outs[i] = drum i when it is on its own output (not affected by Volume), else 0
    inline float tick (float* outs)
    {
        g->tick();
        double mainMix = 0.0;
        for (int i = 0; i < kNumVoices; ++i)
        {
            const double v = g->vout[i];
            if (ownOut[i]) outs[i] = (float) v;
            else         { outs[i] = 0.0f; mainMix += v; }
        }
        return (float) (mainMix * masterGain);
    }

private:
    SoundParams base;
    float active[kNumVoices][kNumLockParams] { { NAN, NAN, NAN, NAN }, { NAN, NAN, NAN, NAN }, { NAN, NAN, NAN, NAN }, { NAN, NAN, NAN, NAN },
                                               { NAN, NAN, NAN, NAN }, { NAN, NAN, NAN, NAN }, { NAN, NAN, NAN, NAN }, { NAN, NAN, NAN, NAN } };
    bool hadLocks[kNumVoices] {};

    // one drum's tune / decay / level / extra: the knob, or its lock
    void writeVoice (int i)
    {
        auto& v = *g;
        double* tune[]  = { &v.bd_tune, &v.sd_tune, &v.lt_tune, &v.ht_tune, &v.cp_tune, &v.ch_tune, &v.oh_tune, &v.cy_tune };
        double* decay[] = { &v.bd_decay, &v.sd_decay, &v.lt_decay, &v.ht_decay, &v.cp_decay, &v.ch_decay, &v.oh_decay, &v.cy_decay };
        double* lvl[]   = { &v.bd_lvl, &v.sd_lvl, &v.lt_lvl, &v.ht_lvl, &v.cp_lvl, &v.ch_lvl, &v.oh_lvl, &v.cy_lvl };
        double* extra[] = { &v.bd_attack, &v.sd_snappy, nullptr, nullptr, &v.cp_noise, nullptr, nullptr, &v.cy_tone };
        const float baseExtra[] = { base.bdAttack, base.sdSnappy, 0, 0, base.cpNoise, 0, 0, base.cyTone };
        auto pick = [&] (int k, float knob) { return std::isnan (active[i][k]) ? knob : active[i][k]; };
        *tune[i]  = pick (LockTune, base.tune[i]);
        *decay[i] = pick (LockDecay, base.decay[i]);
        *lvl[i]   = pick (LockLevel, base.lvl[i]);
        if (extra[i] != nullptr) *extra[i] = pick (LockExtra, baseExtra[i]);
    }

    std::unique_ptr<GenVoices> g;
    int trigCount[kNumVoices] {};
    bool ownOut[kNumVoices] {};
    double masterGain = 0.5;
};
} // namespace tross606
