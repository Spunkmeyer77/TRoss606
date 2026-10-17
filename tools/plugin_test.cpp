// End-to-end check of the plug-in: host-synced sequencer, pattern queueing, keyboard recording,
// state save/restore, and an editor snapshot.
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"
#include "../Source/MidiExport.h"
#include "../Source/PluginEditor.h"

struct FakeHost : juce::AudioPlayHead
{
    double ppq = 0.0, bpm = 120.0;
    bool playing = false;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setBpm (bpm);
        p.setPpqPosition (ppq);
        p.setIsPlaying (playing);
        return p;
    }
};

static int failures = 0;
static void check (bool ok, const char* what)
{
    std::printf ("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if (! ok) ++failures;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File outDir = argc > 1 ? juce::File (argv[1]) : juce::File::getCurrentWorkingDirectory();
    const double sr = 48000.0;
    const int block = 480;   // 10 ms; a 16th at 120 bpm = 125 ms = 6000 samples

    Tross606Processor proc;
    FakeHost host;
    proc.setPlayHead (&host);
    proc.setRateAndBufferSizeDetails (sr, block);
    proc.prepareToPlay (sr, block);
    std::printf ("params: %d\n", (int) proc.getParameters().size());
    auto pv = [&] (const char* id) { return proc.apvts.getRawParameterValue (id)->load(); };
    check (std::abs (pv ("bd_tune") - 1.0f) < 0.01f && std::abs (pv ("bd_decay") - 15.8f) < 0.01f && pv ("bd_attack") < 0.01f
           && std::abs (pv ("sd_tune")) < 0.01f && std::abs (pv ("sd_decay") - 75.5f) < 0.01f && std::abs (pv ("sd_snappy") - 57.2f) < 0.01f
           && std::abs (pv ("cp_tune") - 2.6f) < 0.01f && std::abs (pv ("cp_decay") - 100.0f) < 0.01f && std::abs (pv ("cp_noise") - 37.9f) < 0.01f,
           "defaults: BD Tune 1 st, Decay 15.8 %, Attack 0 %; SD Tune 0 st, Decay 75.5 %, Snappy 57.2 %; CP Tune 2.6 st, Decay 100 %, Noise 37.9 %");
    check (proc.apvts.getParameter ("pattern")->getNumSteps() == 12, "12 pattern slots");
    std::printf ("BD Decay shows as \"%s\"\n", proc.apvts.getParameter ("bd_decay")->getCurrentValueAsText().toRawUTF8());

    auto& pats = proc.patterns;
    auto put = [&] (int slot, int row, const char* s) { for (int i = 0; i < 16; ++i) pats.setCell (slot, row, i, s[i] == 'x'); };
    put (0, 0, "x.......x.......");
    put (0, 1, "x...x...x...x...");
    put (0, 2, "....x.......x...");
    put (0, 6, "x.x.x.x.x.x.x.xx");
    put (0, 7, "..............x.");
    put (1, 1, "x..x..x...x..x..");
    put (1, 5, "....x.......x...");
    put (1, 8, "x...............");
    put (11, 1, "x.x.x.x.x.x.x.x.");

    const int totalOuts = proc.getTotalNumOutputChannels();
    std::printf ("output buses: %d, channels: %d\n", proc.getBusCount (false), totalOuts);
    juce::AudioBuffer<float> buf (totalOuts, block);
    juce::AudioBuffer<float> all (2, (int) sr * 9);
    int written = 0, stepsSeen = 0, lastStep = -1, bad = 0;
    float peak = 0;
    std::vector<int> slotAtStep0;

    auto run = [&] (int numBlocks, std::function<void (int, juce::MidiBuffer&)> perBlock = {})
    {
        for (int b = 0; b < numBlocks; ++b)
        {
            juce::MidiBuffer midi;
            if (perBlock) perBlock (b, midi);
            proc.processBlock (buf, midi);
            if (host.playing) host.ppq += block / sr * host.bpm / 60.0;
            const int st = proc.getPlayStep();
            if (st != lastStep && st >= 0)
            {
                ++stepsSeen;
                if (st == 0) slotAtStep0.push_back (proc.getCurrentSlot());
            }
            lastStep = st;
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < block; ++i)
                {
                    const float v = buf.getSample (c, i);
                    if (! std::isfinite (v)) ++bad;
                    peak = juce::jmax (peak, std::abs (v));
                }
            if (written + block <= all.getNumSamples())
            {
                for (int c = 0; c < 2; ++c) all.copyFrom (c, written, buf, c, 0, block);
                written += block;
            }
        }
    };

    // 1. stopped: nothing from the sequencer, keyboard still plays
    run (20);
    check (peak == 0.0f && proc.getPlayStep() == -1, "silent and no steps while the transport is stopped");
    run (50, [] (int b, juce::MidiBuffer& m) { if (b == 0) m.addEvent (juce::MidiMessage::noteOn (10, 36, (juce::uint8) 110), 0); });
    check (peak > 0.05f, "keyboard note C1 plays the bass drum while stopped");

    // 2. play two bars of pattern 1
    host.playing = true; host.ppq = 0.0;
    stepsSeen = 0; slotAtStep0.clear();
    run (400);   // 4 s = 2 bars
    check (stepsSeen == 32, ("32 steps in two bars (got " + juce::String (stepsSeen) + ")").toRawUTF8());

    // 3. pick pattern 2 in the middle of a bar: it must wait for step 1
    auto* patParam = proc.apvts.getParameter ("pattern");
    host.ppq = 1.5;   // halfway through bar 1
    lastStep = -1; slotAtStep0.clear();
    patParam->setValueNotifyingHost (patParam->convertTo0to1 (1.0f));
    run (10);
    check (proc.getCurrentSlot() == 0 && proc.getPendingSlot() == 1, "pattern 2 queued while pattern 1 finishes");
    run (300);
    check (! slotAtStep0.empty() && slotAtStep0.front() == 1 && proc.getPendingSlot() == -1, "pattern 2 starts on step 1");

    // 4. LEN = 12: steps wrap at 12
    auto* lenParam = proc.apvts.getParameter ("seqlen");
    lenParam->setValueNotifyingHost (lenParam->convertTo0to1 (12.0f));
    int maxStep = 0;
    run (20);   // let the step that was playing finish
    for (int b = 0; b < 300; ++b) { run (1); maxStep = juce::jmax (maxStep, proc.getPlayStep()); }
    check (maxStep == 11, ("LEN 12 plays steps 1..12 only (highest " + juce::String (maxStep + 1) + ")").toRawUTF8());
    lenParam->setValueNotifyingHost (lenParam->convertTo0to1 (16.0f));

    // 5. record: snare (D1) hit just before step 7 at velocity 120 -> SD + accent on step 7 of pattern 2
    host.ppq = 0.0;
    run (1);
    const int samplesTo7 = (int) std::round ((6.0 / 4.0 - 0.06) * 60.0 / host.bpm * sr);   // 6th 16th minus a bit
    const int hitBlock = samplesTo7 / block;
    run (hitBlock + 2, [&] (int b, juce::MidiBuffer& m)
    {
        if (b == hitBlock - 1) m.addEvent (juce::MidiMessage::noteOn (10, 38, (juce::uint8) 120), samplesTo7 % block);
    });
    check (pats.cell (1, 2, 6) && pats.cell (1, 0, 6), "recorded snare + accent into the nearest step (7)");
    check (! pats.cell (0, 2, 6), "recording went into the playing pattern only");

    // 6. stop: a queued slot switches at once
    host.playing = false;
    patParam->setValueNotifyingHost (patParam->convertTo0to1 (0.0f));
    run (2);
    check (proc.getCurrentSlot() == 0, "pattern change while stopped is immediate");

    // 6b. multi-out: BD on its own output, unaffected by Volume; back on MAIN when switched off
    check (proc.getBusCount (false) == 9 && totalOuts == 10, "main stereo out + 8 mono drum outs");
    {
        auto* own = proc.apvts.getParameter ("bd_out");
        auto* vol = proc.apvts.getParameter ("master");
        const int bdCh = proc.getChannelIndexInProcessBlockBuffer (false, 1, 0);
        auto hitBd = [&] (float& mainPeak, float& bdPeak, float& sdPeak)
        {
            mainPeak = bdPeak = sdPeak = 0;
            for (int b = 0; b < 30; ++b)
            {
                juce::MidiBuffer m;
                if (b == 0) m.addEvent (juce::MidiMessage::noteOn (10, 36, (juce::uint8) 100), 0);
                proc.processBlock (buf, m);
                for (int i = 0; i < block; ++i)
                {
                    mainPeak = juce::jmax (mainPeak, std::abs (buf.getSample (0, i)));
                    bdPeak   = juce::jmax (bdPeak, std::abs (buf.getSample (bdCh, i)));
                    sdPeak   = juce::jmax (sdPeak, std::abs (buf.getSample (bdCh + 1, i)));
                }
            }
        };
        float mainPeak, bdPeak, sdPeak;
        run (200);   // let earlier sounds die away
        own->setValueNotifyingHost (1.0f);
        vol->setValueNotifyingHost (0.0f);   // Volume all the way down
        hitBd (mainPeak, bdPeak, sdPeak);
        check (mainPeak < 1e-4f && bdPeak > 0.05f && sdPeak == 0.0f,
               ("BD Own Out: kick only on the BD output, ignores Volume (main " + juce::String (mainPeak, 5)
                + ", BD out " + juce::String (bdPeak, 3) + ")").toRawUTF8());
        own->setValueNotifyingHost (0.0f);
        vol->setValueNotifyingHost (vol->getDefaultValue());
        run (200);
        hitBd (mainPeak, bdPeak, sdPeak);
        check (mainPeak > 0.05f && bdPeak == 0.0f, "BD back on MAIN: kick in the main mix, BD output silent");
    }

    // 6c. name-strip pads: play when stopped (no recording); record (+ accent on Shift) while playing with REC on
    {
        run (200);
        juce::uint16 before = pats.get (proc.getCurrentSlot(), 5);
        proc.panelHit (4, false);   // clap, stopped
        float p = 0;
        for (int b = 0; b < 20; ++b) { juce::MidiBuffer m; proc.processBlock (buf, m); for (int i = 0; i < block; ++i) p = juce::jmax (p, std::abs (buf.getSample (0, i))); }
        check (p > 0.02f && pats.get (proc.getCurrentSlot(), 5) == before, "clap pad plays while stopped and leaves the pattern alone");

        const int slot = proc.getCurrentSlot();
        pats.clear (slot);
        host.playing = true; host.ppq = 0.0;
        run (1);
        host.ppq = 2.0;   // exactly step 9
        proc.panelHit (1, true);    // snare, Shift-click
        proc.panelHit (5, false);   // closed hat, normal click
        run (1);
        check (pats.cell (slot, 2, 8) && pats.cell (slot, 0, 8) && pats.cell (slot, 6, 8),
               "pad clicks while playing: snare + accent and closed hat recorded on step 9");
        auto* recP = proc.apvts.getParameter ("rec");
        recP->setValueNotifyingHost (0.0f);
        proc.panelHit (0, false);   // bass drum
        run (1);
        check (! pats.cell (slot, 1, 8) && ! pats.cell (slot, 1, 9), "with REC off a pad click only plays");
        recP->setValueNotifyingHost (1.0f);
        host.playing = false;
        run (2);
    }

    // 6d. MIDI export: write a known pattern, read the file back, check notes, velocities, timing, length
    {
        tross606::PatternMemory pm;
        auto put2 = [&] (int slot, int row, const char* s) { for (int i = 0; i < 16; ++i) pm.setCell (slot, row, i, s[i] == 'x'); };
        put2 (2, 0, "x...............");   // accent on step 1
        put2 (2, 1, "x.......x.......");   // BD
        put2 (2, 2, "....x.......x...");   // SD
        put2 (2, 6, "x.x.x.x.x.x.x.x.");   // CH
        put2 (7, 8, "x...............");   // CY in slot 8
        auto f = outDir.getChildFile ("export_test.mid");
        check (tross606::MidiExport::write (tross606::MidiExport::make (pm, { 2 }, 16, 128.0, "TRoss 606 Pattern 3"), f), "MIDI file written");
        juce::MidiFile back;
        juce::FileInputStream in (f);
        const bool read = in.openedOk() && back.readFrom (in);
        int ons = 0, accented = 0, bdOn = 0, chOn = 0, ch10 = 0;
        double lastTick = 0, tempo = 0;
        if (read)
            for (auto* e : *back.getTrack (0))
            {
                const auto& m = e->message;
                lastTick = juce::jmax (lastTick, m.getTimeStamp());
                if (m.isTempoMetaEvent()) tempo = 60.0 / m.getTempoSecondsPerQuarterNote();
                if (! m.isNoteOn()) continue;
                ++ons; if (m.getChannel() == 10) ++ch10;
                if (m.getVelocity() == 127) ++accented;
                if (m.getNoteNumber() == 36) ++bdOn;
                if (m.getNoteNumber() == 42) ++chOn;
            }
        std::printf ("MIDI export: %d notes, %d accented, tempo %.1f, last tick %.0f, ticks/quarter %d\n",
                     ons, accented, tempo, lastTick, (int) back.getTimeFormat());
        check (read && ons == 12 && bdOn == 2 && chOn == 8 && accented == 2 && ch10 == 12, "exported notes: BD x2, SD x2, CH x8, accent on step 1 (BD + CH at 127), channel 10");
        check (std::abs (tempo - 128.0) < 0.01 && std::abs (lastTick - 16 * 24) < 0.5, "exported tempo 128 BPM and clip length 16 steps");
        auto row = tross606::MidiExport::make (pm, { 2, 7 }, 12, 120.0, "row");
        double endTick = 0;
        for (auto* e : *row.getTrack (0)) endTick = juce::jmax (endTick, e->message.getTimeStamp());
        check (std::abs (endTick - 2 * 12 * 24) < 0.5, "two patterns in a row at LENGTH 12 = 24 steps");
        check (tross606::MidiExport::isEmpty (pm, 0) && ! tross606::MidiExport::isEmpty (pm, 7), "empty patterns are detected");
    }

    // 6e. copy + chain: copy pattern 1 to 4, chain 4,2 locked to the song position, saved with the state
    {
        const int len = 16;
        pats.copySlot (0, 3);
        bool same = true;
        for (int r = 0; r < tross606::kNumRows; ++r) same &= pats.get (3, r) == pats.get (0, r);
        check (same && ! pats.isEmpty (3), "Copy to: pattern 1 copied into pattern 4");

        proc.chainAppend (3); proc.chainAppend (1);
        check (proc.getChainLength() == 2, "chain built: 4, 2");
        host.playing = true; host.ppq = 0.0;
        std::vector<int> barSlots;
        int prevStep = -1;
        for (int b = 0; b < 1600 && barSlots.size() < 4; ++b)   // 4 bars at 120 BPM = 8 s
        {
            run (1);
            const int st = proc.getPlayStep();
            if (st == 0 && prevStep != 0) barSlots.push_back (proc.getCurrentSlot());
            prevStep = st;
        }
        check (barSlots.size() == 4 && barSlots[0] == 3 && barSlots[1] == 1 && barSlots[2] == 3 && barSlots[3] == 1,
               "chain plays 4, 2, 4, 2 bar by bar");
        host.ppq = 4.0 * 5 + 0.5;   // jump into bar 6 -> chain step 2 -> pattern 2
        run (60);
        check (proc.getCurrentSlot() == 1 && proc.getChainPos() == 1, "after a jump the chain lands on the right pattern");

        juce::MemoryBlock st; proc.getStateInformation (st);
        Tross606Processor other;
        other.setStateInformation (st.getData(), (int) st.getSize());
        check (other.getChainLength() == 2 && other.getChain() == std::vector<int> ({ 3, 1 }), "chain saved and restored with the project");

        auto chainMidi = tross606::MidiExport::make (pats, proc.getChain(), len, 120.0, "chain");
        double endTick = 0;
        for (auto* e : *chainMidi.getTrack (0)) endTick = juce::jmax (endTick, e->message.getTimeStamp());
        check (std::abs (endTick - 2 * 16 * 24) < 0.5, "chain export = 2 patterns in a row");

        proc.chainClear();
        check (proc.getChainLength() == 0, "chain cleared");
        host.playing = false;
        run (2);
        pats.clear (3);
    }

    // 6f. note repeat (Option-click held on a name strip)
    {
        auto* rate = proc.apvts.getParameter ("repeat_rate");
        auto stepsOf = [&] (int slot, int row) { juce::String s; for (int i = 0; i < 16; ++i) s << (pats.cell (slot, row, i) ? "x" : "."); return s; };
        const int slot = proc.getCurrentSlot();
        auto oneBar = [&] (int voice, float rateIndex)
        {
            pats.clear (slot);
            rate->setValueNotifyingHost (rate->convertTo0to1 (rateIndex));
            host.playing = true; host.ppq = 0.0;
            run (1);
            host.ppq = 0.0;
            proc.startRepeat (voice, false);
            run (400);            // one bar at 120 BPM = 2 s = 200 blocks; hold for two
            proc.stopRepeat (voice);
            run (5);
            host.playing = false; run (2);
        };
        oneBar (5, 2.0f);   // closed hat at 1/16
        const auto ch16 = stepsOf (slot, 6);
        check (ch16 == "xxxxxxxxxxxxxxxx", ("repeat 1/16 records every step (" + ch16 + ")").toRawUTF8());
        oneBar (0, 0.0f);   // bass drum at 1/4
        const auto bd4 = stepsOf (slot, 1);
        check (bd4 == "x...x...x...x...", ("repeat 1/4 records steps 1, 5, 9, 13 (" + bd4 + ")").toRawUTF8());
        oneBar (1, 1.0f);   // snare at 1/8
        const auto sd8 = stepsOf (slot, 2);
        check (sd8 == "x.x.x.x.x.x.x.x.", ("repeat 1/8 records every other step (" + sd8 + ")").toRawUTF8());

        // stopped: plays at the rate from the press, records nothing
        pats.clear (slot);
        rate->setValueNotifyingHost (rate->convertTo0to1 (1.0f));   // 1/8 at 120 BPM = 4 per second
        const int before = proc.getRepeatHits (4);
        proc.startRepeat (4, false);
        run (100);   // 1 s
        proc.stopRepeat (4);
        const int hits = proc.getRepeatHits (4) - before;
        run (50);
        const int after = proc.getRepeatHits (4) - before;
        check (hits >= 4 && hits <= 5 && after == hits && pats.isEmpty (slot),
               ("repeat while stopped: " + juce::String (hits) + " hits in 1 s at 1/8, stops on release, pattern untouched").toRawUTF8());
        rate->setValueNotifyingHost (rate->convertTo0to1 (2.0f));
    }

    // 6g. flams: grace notes before the step, kept with the pattern
    {
        const int slot = proc.getCurrentSlot();
        pats.clear (slot);
        pats.setCell (slot, 2, 4, true);   // snare on step 5
        pats.setCell (slot, 1, 0, true);   // plain kick on step 1
        tross606::FlamStyle drag; drag.graces = 2;
        pats.setFlam (slot, 2, 4, true);
        pats.setFlamStyle (slot, 2, 4, drag.encode());
        check (pats.flam (slot, 2, 4) && pats.flamStyle (slot, 2, 4).graces == 2, "drag set on SD step 5");
        pats.setFlam (slot, 0, 3, true);
        check (! pats.flam (slot, 0, 3), "no flams on the accent row");

        const int g0 = proc.graceCount.load();
        host.playing = true; host.ppq = 0.0;
        run (400);   // two bars
        host.playing = false; run (2);
        const int graces = proc.graceCount.load() - g0;
        check (graces == 4, ("drag plays 2 grace notes per bar (" + juce::String (graces) + " in 2 bars)").toRawUTF8());

        // copy keeps flams; switching the step off drops its flam
        pats.copySlot (slot, 9);
        check (pats.flam (9, 2, 4) && pats.flamStyle (9, 2, 4).graces == 2, "Copy to: flams copied with the pattern");
        pats.setCell (9, 2, 4, false);
        check (! pats.flam (9, 2, 4), "step off drops its flam");
        pats.clear (9);

        proc.flamWholeRow (1, drag.encode());
        check (pats.flam (slot, 1, 0), "flam on every hit in the row");
        proc.clearRowFlams (1);
        check (pats.getFlams (slot, 1) == 0, "remove all flams in the row");

        // MIDI export writes the grace notes
        auto midi = tross606::MidiExport::make (pats, { slot }, 16, 120.0, "flam");
        int ons = 0;
        if (auto* tr = midi.getTrack (0))
            for (auto* ev : *tr) if (ev->message.isNoteOn()) ++ons;
        check (ons == 4, ("MIDI export: kick + snare + 2 grace notes (" + juce::String (ons) + " notes)").toRawUTF8());
    }

    // 6h. parameter locks: step 5 of the kick locked to Level -inf, step 9 locked to Decay 100 %
    {
        const int slot = proc.getCurrentSlot();
        pats.clear (slot);
        for (int s : { 0, 4, 8, 12 }) pats.setCell (slot, 1, s, true);
        pats.setLock (slot, 0, 4, tross606::LockLevel, tross606::encodeLock (0.0f));
        pats.setLock (slot, 0, 8, tross606::LockDecay, tross606::encodeLock (1.0f));
        check (pats.hasLocks (slot) && pats.lock (slot, 0, 4, tross606::LockLevel) == 1, "level lock set on BD step 5");

        // one bar, peak per quarter note (at 120 BPM a step is 0.125 s; each hit gets 4 steps)
        host.playing = true; host.ppq = 0.0;
        const int perStep = (int) (sr * 0.125);
        std::vector<float> quarterPeak (4, 0.0f), quarterTail (4, 0.0f);
        int at = 0;
        for (int b = 0; b < (int) (sr * 2.0) / block; ++b)
        {
            juce::MidiBuffer m;
            proc.processBlock (buf, m);
            host.ppq += block / sr * host.bpm / 60.0;
            for (int i = 0; i < block; ++i, ++at)
            {
                const int qn = juce::jlimit (0, 3, at / (perStep * 4));
                const float a = std::abs (buf.getSample (0, i));
                quarterPeak[(size_t) qn] = juce::jmax (quarterPeak[(size_t) qn], a);
                if ((at % (perStep * 4)) > perStep * 3) quarterTail[(size_t) qn] = juce::jmax (quarterTail[(size_t) qn], a);
            }
        }
        host.playing = false; run (2);
        std::printf ("lock peaks %.3f %.3f %.3f %.3f  tails %.4f %.4f %.4f %.4f\n", quarterPeak[0], quarterPeak[1], quarterPeak[2], quarterPeak[3],
                     quarterTail[0], quarterTail[1], quarterTail[2], quarterTail[3]);
        check (quarterPeak[0] > 0.05f && quarterPeak[1] < 0.02f * quarterPeak[0], "Level lock: BD step 5 silent, step 1 plays");
        check (quarterTail[2] > 3.0f * quarterTail[0], "Decay lock: BD step 9 rings much longer");
        check (quarterPeak[3] > 0.7f * quarterPeak[0] && quarterTail[3] < 1.5f * quarterTail[0] + 1e-4f,
               "after a locked hit the next hit is back on the knobs");

        pats.copySlot (slot, 10);
        check (pats.lock (10, 0, 8, tross606::LockDecay) != 0, "Copy to: locks copied");
        pats.setCell (10, 1, 8, false);
        check (pats.lock (10, 0, 8, tross606::LockDecay) != 0, "a lock stays when its note is switched off");
        pats.clear (10);
        proc.lockVoiceSel = 0; proc.lockKnobSel = tross606::LockDecay;
        // put back the drag from 6g for the state test
        tross606::FlamStyle drag; drag.graces = 2;
        pats.setCell (slot, 2, 4, true);
        pats.setFlam (slot, 2, 4, true);
        pats.setFlamStyle (slot, 2, 4, drag.encode());
        pats.setLock (slot, 0, 12, tross606::LockDecay, tross606::encodeLock (0.6f));   // for the snapshot
        pats.setLock (slot, 0, 0, tross606::LockDecay, tross606::encodeLock (0.3f));
    }

    // 7. state round trip
    juce::MemoryBlock state; proc.getStateInformation (state);
    {
        Tross606Processor other;
        other.setStateInformation (state.getData(), (int) state.getSize());
        bool same = true;
        for (int s = 0; s < tross606::kNumSlots; ++s)
            for (int r = 0; r < tross606::kNumRows; ++r)
                same &= other.patterns.get (s, r) == pats.get (s, r);
        check (same, "all 12 patterns restored from the saved state");
        const int sl = proc.getCurrentSlot();
        check (other.patterns.lock (sl, 0, 4, tross606::LockLevel) == 1 && other.patterns.lock (sl, 0, 8, tross606::LockDecay) == 255, "locks restored");
        check (other.patterns.flam (sl, 2, 4) && other.patterns.flamStyle (sl, 2, 4).graces == 2, "flams restored from the saved state");
    }
    proc.apvts.getParameter ("bd_tune")->setValueNotifyingHost (0.25f);
    proc.setStateInformation (state.getData(), (int) state.getSize());
    check (std::abs (proc.apvts.getRawParameterValue ("bd_tune")->load() - 1.0f) < 0.01f, "parameters restored");

    check (bad == 0, "no NaN / inf in the output");
    std::printf ("peak %.3f\n", peak);

    juce::WavAudioFormat wav;
    auto file = outDir.getChildFile ("sequencer.wav");
    file.deleteFile();
    if (auto os = std::unique_ptr<juce::OutputStream> (file.createOutputStream()))
        if (auto w = wav.createWriterFor (os, juce::AudioFormatWriterOptions().withSampleRate (sr).withNumChannels (2).withBitsPerSample (16)))
            w->writeFromAudioSampleBuffer (all, 0, written);

    // editor snapshot at 1x and 2x
    pats.setCell (0, 3, 10, true);
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    std::printf ("editor size %d x %d\n", ed->getWidth(), ed->getHeight());
    juce::PNGImageFormat png;
    const int baseW = ed->getWidth(), baseH = ed->getHeight();
    for (float scale : { 1.0f, 2.0f })
    {
        ed->setSize ((int) (baseW * scale), (int) (baseH * scale));
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        auto pngFile = outDir.getChildFile ("editor_" + juce::String ((int) scale) + "x.png");
        pngFile.deleteFile();
        if (auto os = pngFile.createOutputStream()) png.writeImageToStream (img, *os);
        ed->setSize (baseW, baseH);
    }
    // About window, and the manual inside the plug-in
    if (auto* te = dynamic_cast<Tross606Editor*> (ed.get()))
    {
        te->showAbout();
        check (te->isAboutShowing(), "logo click opens the About window");
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        auto pngFile = outDir.getChildFile ("about.png");
        pngFile.deleteFile();
        if (auto os = pngFile.createOutputStream()) png.writeImageToStream (img, *os);
    }
    {   // the P-LOCK screen
        proc.showLocks = true;
        std::unique_ptr<juce::AudioProcessorEditor> led (proc.createEditor());
        led->setSize (led->getWidth() * 2, led->getHeight() * 2);
        auto img = led->createComponentSnapshot (led->getLocalBounds(), true, 1.0f);
        auto pngFile = outDir.getChildFile ("locks.png");
        pngFile.deleteFile();
        if (auto os = pngFile.createOutputStream()) png.writeImageToStream (img, *os);
        proc.showLocks = false;
    }
    {
        auto man = AboutOverlay::writeManual();
        juce::MemoryBlock mb; man.loadFileAsData (mb);
        check (man.existsAsFile() && mb.getSize() > 10000 && juce::String ((const char*) mb.getData(), 5) == "%PDF-", "manual PDF built in and written out");
    }
    ed.reset();

    std::printf ("%s\n", failures ? "SOME TESTS FAILED" : "all tests passed");
    return failures ? 1 : 0;
}
