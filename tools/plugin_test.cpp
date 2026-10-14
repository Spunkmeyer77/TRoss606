// End-to-end check of the plug-in: host-synced sequencer, pattern queueing, keyboard recording,
// state save/restore, and an editor snapshot.
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"

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
    check (std::abs (proc.apvts.getRawParameterValue ("bd_decay")->load() - 27.2f) < 0.01f
           && std::abs (proc.apvts.getRawParameterValue ("bd_attack")->load() - 21.3f) < 0.01f,
           "defaults: BD Decay 27.2 %, BD Attack 21.3 %");
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
    }
    proc.apvts.getParameter ("bd_tune")->setValueNotifyingHost (0.25f);
    proc.setStateInformation (state.getData(), (int) state.getSize());
    check (std::abs (proc.apvts.getRawParameterValue ("bd_tune")->load() - 2.2f) < 0.01f, "parameters restored");

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
    ed.reset();

    std::printf ("%s\n", failures ? "SOME TESTS FAILED" : "all tests passed");
    return failures ? 1 : 0;
}
