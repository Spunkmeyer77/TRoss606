// Turns TRoss 606 patterns into standard MIDI files.
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "dsp/TrossEngine.h"

namespace tross606
{
struct MidiExport
{
    static constexpr int kTicksPerQuarter = 96, kTicksPerStep = 24, kNoteTicks = 23, kChannel = 10;
    static constexpr int kVelocity = 100, kAccentVelocity = 127;

    // the note each drum is written on: TRoss 606's own map, which is also the General MIDI drum map
    static int noteFor (int voice)
    {
        static const int notes[kNumVoices] = { 36, 38, 45, 50, 39, 42, 46, 49 };   // BD SD LT HT CP CH OH CY
        return notes[voice];
    }

    static bool isEmpty (const PatternMemory& p, int slot)
    {
        for (int r = 1; r < kNumRows; ++r)   // the accent row alone plays nothing
            if (p.get (slot, r) != 0) return false;
        return true;
    }

    // One track: the given slots one after another, each `length` steps long.
    static juce::MidiFile make (const PatternMemory& p, const std::vector<int>& slots, int length, double bpm,
                                const juce::String& name)
    {
        juce::MidiMessageSequence seq;
        auto meta = [&] (juce::MidiMessage m) { m.setTimeStamp (0); seq.addEvent (m); };
        meta (juce::MidiMessage::textMetaEvent (3, name));
        meta (juce::MidiMessage::tempoMetaEvent (juce::roundToInt (60000000.0 / juce::jlimit (20.0, 999.0, bpm))));
        meta (juce::MidiMessage::timeSignatureMetaEvent (4, 4));

        int start = 0;
        for (int slot : slots)
        {
            for (int s = 0; s < length; ++s)
            {
                const bool accent = p.cell (slot, 0, s);
                for (int v = 0; v < kNumVoices; ++v)
                    if (p.cell (slot, v + 1, s))
                    {
                        const int t = start + s * kTicksPerStep;
                        const auto vel = (juce::uint8) (accent ? kAccentVelocity : kVelocity);
                        if (p.flam (slot, v + 1, s))   // grace notes just before, softer (flam 1, drag 2, ruff 3)
                        {
                            const auto fs = p.flamStyle (slot, v + 1, s);
                            const int span = juce::jmax (1, juce::roundToInt (fs.spacingMs() / 1000.0 * bpm / 60.0 * kTicksPerQuarter));
                            const int gap = juce::jmax (1, span / fs.graces);
                            const auto gv = (juce::uint8) juce::jlimit (1, 100, juce::roundToInt (vel * fs.levelPct() / 100.0f));
                            for (int k = 0; k < fs.graces; ++k)
                            {
                                const int gt = juce::jmax (0, t - span + k * gap);
                                seq.addEvent (juce::MidiMessage::noteOn (kChannel, noteFor (v), gv), gt);
                                seq.addEvent (juce::MidiMessage::noteOff (kChannel, noteFor (v)), juce::jmin (t, gt + juce::jmax (1, gap * 3 / 4)));
                            }
                        }
                        seq.addEvent (juce::MidiMessage::noteOn (kChannel, noteFor (v), vel), t);
                        seq.addEvent (juce::MidiMessage::noteOff (kChannel, noteFor (v)), t + kNoteTicks);
                    }
            }
            start += length * kTicksPerStep;
        }
        seq.updateMatchedPairs();
        seq.addEvent (juce::MidiMessage::endOfTrack(), start);

        juce::MidiFile file;
        file.setTicksPerQuarterNote (kTicksPerQuarter);
        file.addTrack (seq);
        return file;
    }

    static bool write (const juce::MidiFile& f, const juce::File& dest)
    {
        dest.deleteFile();
        juce::FileOutputStream out (dest);
        return out.openedOk() && f.writeTo (out, 0);
    }

    static juce::String fileName (int slot) { return "TRoss606 Pattern " + juce::String (slot + 1) + ".mid"; }
};
} // namespace tross606
