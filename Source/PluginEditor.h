#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Look of the Max for Live device: live.dial knobs, live.text buttons, live.numbox
class TrossLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TrossLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

// The 9 x 16 step grid with the step LEDs above it. Click to toggle, drag to paint.
class StepGrid : public juce::Component
{
public:
    explicit StepGrid (Tross606Processor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { dragRow = -1; }

    // geometry in this component's coordinates (it sits at x = 868, y = 19 of the device)
    static juce::Rectangle<int> cellRect (int row, int step);
    static juce::Rectangle<int> ledRect (int step);

private:
    Tross606Processor& proc;
    int dragRow = -1, lastDragStep = -1;
    bool dragValue = false;
    bool hit (juce::Point<int>, int& row, int& step) const;
};

// Vertical 1..8 pattern selector with the "playing" LEDs next to it
class PatternTab : public juce::Component
{
public:
    explicit PatternTab (Tross606Processor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    Tross606Processor& proc;
};

class Tross606Editor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit Tross606Editor (Tross606Processor&);
    ~Tross606Editor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int kWidth = 1300, kHeight = 167;

private:
    // all controls live here at the device's own pixel positions; the editor scales it
    struct Content : public juce::Component
    {
        explicit Content (Tross606Editor& e) : ed (e) {}
        void paint (juce::Graphics& g) override { ed.paintPanel (g); }
        Tross606Editor& ed;
    };

    Tross606Processor& proc;
    TrossLookAndFeel lnf;
    Content content { *this };
    StepGrid grid { proc };
    PatternTab tab { proc };

    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::Button>> buttons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> bAtts;
    juce::TextButton clearBtn { "CLEAR" };
    // standalone app only
    juce::TextButton playBtn { "PLAY" };
    juce::Slider bpmBox;

    int lastVersion = -1, lastStep = -2, lastSlot = -1, lastPending = -2;
    int lastLen = -1, lastSel = -1;

    juce::Slider& addDial (const juce::String& paramId, const juce::String& label, int x, int y);
    juce::Button& addTextToggle (const juce::String& paramId, const juce::String& text, juce::Colour on, juce::Rectangle<int> r);
    void paintPanel (juce::Graphics&);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tross606Editor)
};
