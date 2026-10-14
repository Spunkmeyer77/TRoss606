#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/TrossEngine.h"

class Tross606Processor : public juce::AudioProcessor
{
public:
    Tross606Processor();
    ~Tross606Processor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

    // ---- pattern memory / sequencer, shared with the editor
    tross606::PatternMemory patterns;
    int  getCurrentSlot() const { return currentSlot.load(); }   // the slot that plays and is edited
    int  getPendingSlot() const { return pendingSlot.load(); }   // queued slot, -1 = none
    int  getPlayStep() const    { return playStep.load(); }      // step being played, -1 = stopped
    void clearCurrentPattern()  { patterns.clear (currentSlot.load()); }

    // Standalone app only: there is no host transport, so it gets its own play button and tempo
    bool isStandalone() const { return wrapperType == wrapperType_Standalone; }
    std::atomic<bool>  internalPlay { false };
    std::atomic<float> internalBpm { 120.0f };

private:
    tross606::Engine engine;
    tross606::SoundParams sound;
    double sampleRate = 44100.0;

    std::atomic<int> currentSlot { 0 }, pendingSlot { -1 }, playStep { -1 };
    int lastPatternParam = 0;

    // transport following
    bool wasPlaying = false;
    juce::int64 lastSixteenth = 0;
    double internalPpq = 0.0;

    struct Raw
    {
        std::atomic<float>* lvl[tross606::kNumVoices];
        std::atomic<float>* tune[tross606::kNumVoices];
        std::atomic<float>* decay[tross606::kNumVoices];
        std::atomic<float>* mute[tross606::kNumVoices];
        std::atomic<float>* solo[tross606::kNumVoices];
        std::atomic<float> *bdAttack, *sdSnappy, *cpNoise, *cyTone, *accent, *master,
                           *seqOn, *seqLen, *pattern, *rec;
    } raw {};

    void pullParams();
    void doStep (int step, bool seqOn);
    void recordHit (int voice, int velocity, double sixteenthPos, int len);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tross606Processor)
};
