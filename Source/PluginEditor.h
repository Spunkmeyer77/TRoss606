#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Vintage drum-machine look: black knurled knobs on brushed silver, rubber buttons, LED pads
class TrossLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TrossLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
    void drawBubble (juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>& tip, const juce::Rectangle<float>& body) override;
    juce::Font getSliderPopupFont (juce::Slider&) override;
};

// Lower section: step numbers, play LEDs and the 9 x 16 pad grid. Click to toggle, drag to paint.
class StepGrid : public juce::Component
{
public:
    explicit StepGrid (Tross606Processor& p) : proc (p) { setOpaque (true); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { dragRow = -1; }

    static juce::Rectangle<float> padRect (int row, int step);

private:
    Tross606Processor& proc;
    int dragRow = -1, lastDragStep = -1;
    bool dragValue = false;
    bool hit (juce::Point<float>, int& row, int& step) const;
};

// 12 pattern buttons, 2 across x 6 down. LED = slot playing (blinks when queued), light cap = selected.
class PatternPad : public juce::Component
{
public:
    explicit PatternPad (Tross606Processor& p) : proc (p) { setOpaque (true); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    static juce::Rectangle<float> buttonRect (int slot);
private:
    Tross606Processor& proc;
};

// Amber read-out in the master section: playing pattern, and the queued one
class StatusLcd : public juce::Component
{
public:
    explicit StatusLcd (Tross606Processor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
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

    static constexpr int kWidth = 1120, kHeight = 455;

private:
    // everything is laid out at 1120 x 455 here; the editor scales it
    struct Content : public juce::Component
    {
        explicit Content (Tross606Editor& e) : ed (e) { setOpaque (true); }
        void paint (juce::Graphics& g) override { ed.paintPanel (g); }
        Tross606Editor& ed;
    };

    Tross606Processor& proc;
    TrossLookAndFeel lnf;
    Content content { *this };
    StepGrid grid { proc };
    PatternPad patternPad { proc };
    StatusLcd lcd { proc };

    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::Button>> buttons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> bAtts;
    juce::TextButton clearBtn { "CLEAR" };
    // standalone app only
    juce::TextButton playBtn { "PLAY" };
    juce::Slider bpmBox;

    int lastVersion = -1, lastStep = -2, lastSlot = -1, lastPending = -2, lastLen = -1, lastSel = -1;

    juce::Slider& addKnob (const juce::String& paramId, juce::Point<int> centre, int diameter);
    juce::Button& addButton (const juce::String& paramId, const juce::String& text, juce::Rectangle<int> r, const char* style);
    void paintPanel (juce::Graphics&);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tross606Editor)
};
