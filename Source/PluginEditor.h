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
    void showStepMenu (int row, int step);   // right-click a drum pad: the step and its flam

private:
    Tross606Processor& proc;
    int dragRow = -1, lastDragStep = -1;
    bool dragValue = false;
    bool hit (juce::Point<float>, int& row, int& step) const;

};

// Parameter-lock screen, drawn over the step pads: choose a drum (left) and a knob (keys below), then
// drag across the steps' bars to lock that knob's value per step. Right-drag / double-click removes locks.
class LockGrid : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit LockGrid (Tross606Processor& p) : proc (p) { setOpaque (true); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { dragging = false; lastCol = -1; }
    void mouseDoubleClick (const juce::MouseEvent&) override;
    static juce::Rectangle<float> tabRect (int i);
    static juce::Rectangle<float> voiceRect (int v);
private:
    Tross606Processor& proc;
    bool dragging = false, erasing = false;
    int lastCol = -1;
    int voice() const;
    int knob() const;
    int columnAt (float x) const;
    float normAt (float y) const;
    void apply (int col, float y);
};

// 12 pattern buttons, 2 across x 6 down. LED = slot playing (blinks when queued), light cap = selected.
// A drum's black name strip, played like a pad: click = hit, Shift-click = accented hit
class NameStrip : public juce::Component, public juce::SettableTooltipClient
{
public:
    NameStrip (Tross606Processor& p, int v, const juce::String& t) : proc (p), voice (v), title (t)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void pollRepeat();   // called by the editor's timer: flash on each note-repeat hit
    bool isFlashing() const { return juce::Time::getMillisecondCounter() - lastHit < 180; }
    juce::uint32 lastHit = 0;
    bool lastAccent = false;
private:
    Tross606Processor& proc;
    int voice;
    bool repeating = false;
    int lastRepeatHits = 0;
    juce::String title;
};

// EXPORT MIDI key: click = menu (this pattern / all in a row / all as separate files), drag = drop the
// playing pattern onto a DAW track as a MIDI clip
class ExportKey : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit ExportKey (Tross606Processor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { pressed = true; dragged = false; repaint(); }
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
private:
    Tross606Processor& proc;
    bool pressed = false, dragged = false;
    std::unique_ptr<juce::FileChooser> chooser;
    void showMenu();
    void exportPlaying();
    void exportAllInARow();
    void exportAllSeparate();
    void exportChain (const std::vector<int>& chain);
    std::vector<int> nonEmptySlots() const;
    int length() const;
};

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

// The About window: opens over the panel when you click the TRoss 606 logo.
// Click anywhere (or press Esc) to close it.
class AboutOverlay : public juce::Component
{
public:
    explicit AboutOverlay (Tross606Processor& p);
    void paint (juce::Graphics&) override;
    void resized() override;
    // the manual built into the plug-in, written to a temporary file; returns it (or an empty File)
    static juce::File writeManual();
    juce::TextButton manualBtn { "OPEN THE MANUAL" };
    static constexpr const char* kContact = "ticcoross@gmail.com";
    juce::HyperlinkButton contactLink { kContact, juce::URL (juce::String ("mailto:") + kContact) };
    void mouseDown (const juce::MouseEvent&) override { setVisible (false); }
    bool keyPressed (const juce::KeyPress& k) override { if (k == juce::KeyPress::escapeKey) { setVisible (false); return true; } return false; }
    juce::Rectangle<float> card() const;
    static juce::String formatAndHost (const Tross606Processor&);
private:
    Tross606Processor& proc;
};

// Invisible click area over the logo
class LogoButton : public juce::Component, public juce::SettableTooltipClient
{
public:
    LogoButton() { setMouseCursor (juce::MouseCursor::PointingHandCursor); setTooltip ("About TRoss 606"); }
    void mouseUp (const juce::MouseEvent& e) override { if (onClick && getLocalBounds().contains (e.getPosition())) onClick(); }
    std::function<void()> onClick;
};

class Tross606Editor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit Tross606Editor (Tross606Processor&);
    ~Tross606Editor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int kWidth = 1120, kHeight = 473;

private:
    // everything is laid out at 1120 x 473 here; the editor scales it
    struct Content : public juce::Component
    {
        explicit Content (Tross606Editor& e) : ed (e) { setOpaque (true); }
        void paint (juce::Graphics& g) override { ed.paintPanel (g); }
        // double-click the header strip: bigger window / back to normal size
        void mouseDoubleClick (const juce::MouseEvent& e) override { if (e.y < 52) ed.toggleSize(); }
        Tross606Editor& ed;
    };

    Tross606Processor& proc;
    TrossLookAndFeel lnf;
    Content content { *this };
    StepGrid grid { proc };
    LockGrid lockGrid { proc };
    PatternPad patternPad { proc };
    StatusLcd lcd { proc };
    std::vector<std::unique_ptr<NameStrip>> strips;
    ExportKey exportKey { proc };
    juce::Slider repeatKnob, repeatBox;

    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::Button>> buttons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> bAtts;
    juce::TextButton clearBtn { "CLEAR" };
    juce::TextButton lockBtn { "P-LOCK" };   // note screen <-> parameter-lock screen
    // standalone app only
    juce::TextButton playBtn { "PLAY" };
    juce::Slider bpmBox;
    juce::TextButton helpBtn { "HELP" };   // hover tips on / off (remembered for every TRoss 606)
    std::unique_ptr<juce::TooltipWindow> tooltips;
    void updateHelp();
    LogoButton logoBtn;
    AboutOverlay about { proc };

    int lastVersion = -1, lastStep = -2, lastSlot = -1, lastPending = -2, lastLen = -1, lastSel = -1, lastChain = -1;

    juce::Slider& addKnob (const juce::String& paramId, juce::Point<int> centre, int diameter);
    juce::Button& addButton (const juce::String& paramId, const juce::String& text, juce::Rectangle<int> r, const char* style);
    void paintPanel (juce::Graphics&);
public:
    void showAbout();
    void toggleSize();   // 100 % <-> 150 %
    bool isAboutShowing() const { return about.isVisible(); }
private:
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tross606Editor)
};
