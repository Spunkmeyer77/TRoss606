#include "PluginEditor.h"

using namespace tross606;

namespace colours
{
const juce::Colour bg       { 0xff262626 };
const juce::Colour orange   = juce::Colour::fromFloatRGBA (0.937f, 0.451f, 0.106f, 1.0f);
const juce::Colour border   = juce::Colour::fromFloatRGBA (0.5f, 0.5f, 0.5f, 0.45f);
const juce::Colour title    = juce::Colour::fromFloatRGBA (0.08f, 0.08f, 0.08f, 1.0f);
const juce::Colour dimText  = juce::Colour::fromFloatRGBA (0.6f, 0.6f, 0.6f, 1.0f);
const juce::Colour label    = juce::Colour::fromFloatRGBA (0.75f, 0.75f, 0.75f, 1.0f);
const juce::Colour cellOff  = juce::Colour::fromFloatRGBA (0.16f, 0.16f, 0.16f, 1.0f);
const juce::Colour cellLine = juce::Colour::fromFloatRGBA (0.55f, 0.55f, 0.55f, 1.0f);
const juce::Colour muteOn   = juce::Colour::fromFloatRGBA (1.0f, 0.72f, 0.1f, 1.0f);
const juce::Colour soloOn   = juce::Colour::fromFloatRGBA (0.25f, 0.62f, 1.0f, 1.0f);
const juce::Colour ctrlBg   { 0xff3c3c3c };
const juce::Colour track    { 0xff545454 };
const juce::Colour ledOff   { 0xff3a3a3a };
}

static juce::Font bold (float size)  { return juce::Font (juce::FontOptions (size, juce::Font::bold)); }
static juce::Font plain (float size) { return juce::Font (juce::FontOptions (size)); }

// ================================================================================================
TrossLookAndFeel::TrossLookAndFeel()
{
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff2e2e2e));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, colours::orange);
    setColour (juce::TextEditor::backgroundColourId, colours::ctrlBg);
}

void TrossLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end,
                                         juce::Slider& s)
{
    const auto value = s.getTextFromValue (s.getValue());

    if (s.getProperties().contains ("numbox"))   // live.numbox: a box with the number in it
    {
        auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (0.5f);
        g.setColour (colours::ctrlBg);
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (s.isMouseOverOrDragging() ? colours::orange : colours::track);
        g.drawRoundedRectangle (r, 2.0f, 1.0f);
        g.setColour (juce::Colours::white);
        g.setFont (bold (10.0f));
        g.drawText (value, r, juce::Justification::centred);
        return;
    }

    // live.dial: name on top, arc, value below
    auto area = juce::Rectangle<int> (x, y, w, h);
    g.setColour (colours::label);
    g.setFont (plain (9.5f));
    g.drawText (s.getName(), area.removeFromTop (12), juce::Justification::centred);
    auto valueArea = area.removeFromBottom (12);
    g.setColour (s.isMouseOverOrDragging() ? juce::Colours::white : colours::label);
    g.drawText (value, valueArea, juce::Justification::centred);

    const float d = (float) juce::jmin (area.getWidth(), area.getHeight()) - 2.0f;
    const auto c = area.toFloat().getCentre();
    const float rad = d * 0.5f;
    const float angle = start + pos * (end - start);

    juce::Path bgArc, valArc;
    bgArc.addCentredArc (c.x, c.y, rad, rad, 0.0f, start, end, true);
    g.setColour (colours::track);
    g.strokePath (bgArc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

    // bipolar knobs (Tune) fill from the centre
    const bool bipolar = s.getProperties().contains ("bipolar");
    const float from = bipolar ? start + (float) ((0.0 - s.getMinimum()) / (s.getMaximum() - s.getMinimum())) * (end - start) : start;
    if (std::abs (angle - from) > 0.001f)
    {
        valArc.addCentredArc (c.x, c.y, rad, rad, 0.0f, juce::jmin (from, angle), juce::jmax (from, angle), true);
        g.setColour (colours::orange);
        g.strokePath (valArc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }
    juce::Path needle;
    needle.startNewSubPath (c.getPointOnCircumference (rad * 0.25f, angle));
    needle.lineTo (c.getPointOnCircumference (rad + 1.0f, angle));
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.strokePath (needle, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void TrossLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState() || down;
    const auto onColour = b.getProperties().contains ("onColour")
                            ? juce::Colour ((juce::uint32) (juce::int64) b.getProperties()["onColour"]) : colours::orange;
    g.setColour (on ? onColour : colours::ctrlBg.brighter (highlighted ? 0.12f : 0.0f));
    g.fillRoundedRectangle (r, 2.0f);
    g.setColour (on ? onColour.darker (0.3f) : colours::track);
    g.drawRoundedRectangle (r, 2.0f, 1.0f);
}

void TrossLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down)
{
    const bool on = b.getToggleState() || down;
    g.setColour (on ? colours::title : colours::label);
    g.setFont (bold (9.0f));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

void TrossLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    // live.toggle
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (colours::ctrlBg.brighter (highlighted ? 0.12f : 0.0f));
    g.fillRoundedRectangle (r, 2.0f);
    g.setColour (colours::track);
    g.drawRoundedRectangle (r, 2.0f, 1.0f);
    if (b.getToggleState())
    {
        g.setColour (colours::orange);
        g.fillRoundedRectangle (r.reduced (3.0f), 1.5f);
    }
}

// ================================================================================================
// StepGrid sits at (868, 19) in device coordinates
juce::Rectangle<int> StepGrid::cellRect (int row, int step)
{
    return { 899 - 868 + step * 18 + (step / 4) * 4, 33 - 19 + row * 14, 12, 12 };
}
juce::Rectangle<int> StepGrid::ledRect (int step)
{
    return { 900 - 868 + step * 18 + (step / 4) * 4, 22 - 19, 10, 10 };
}

void StepGrid::paint (juce::Graphics& g)
{
    static const char* rowNames[kNumRows] = { "ACC", "BD", "SD", "LT", "HT", "CP", "CH", "OH", "CY" };
    const int slot = proc.getCurrentSlot();
    const int playing = proc.getPlayStep();
    const int len = (int) proc.apvts.getRawParameterValue ("seqlen")->load();

    for (int s = 0; s < kNumSteps; ++s)
    {
        auto r = ledRect (s).toFloat();
        g.setColour (s == playing ? colours::orange : colours::ledOff);
        g.fillEllipse (r);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawEllipse (r, 0.8f);
    }

    g.setFont (bold (9.0f));
    for (int row = 0; row < kNumRows; ++row)
    {
        g.setColour (juce::Colours::white);
        g.drawText (rowNames[row], 2, 12 + row * 14, 28, 15, juce::Justification::centredLeft);
        for (int s = 0; s < kNumSteps; ++s)
        {
            auto r = cellRect (row, s).toFloat();
            const bool on = proc.patterns.cell (slot, row, s);
            g.setColour (on ? colours::orange : colours::cellOff);
            g.fillRect (r);
            if (s == playing && on) { g.setColour (juce::Colours::white.withAlpha (0.45f)); g.fillRect (r); }
            g.setColour (colours::cellLine);
            g.drawRect (r, 1.0f);
            if (s >= len) { g.setColour (colours::bg.withAlpha (0.6f)); g.fillRect (r); }   // beyond LEN: dimmed
        }
    }
}

bool StepGrid::hit (juce::Point<int> p, int& row, int& step) const
{
    for (row = 0; row < kNumRows; ++row)
        for (step = 0; step < kNumSteps; ++step)
            if (cellRect (row, step).expanded (2, 1).contains (p)) return true;
    return false;
}

void StepGrid::mouseDown (const juce::MouseEvent& e)
{
    int row, step;
    if (! hit (e.getPosition(), row, step)) return;
    const int slot = proc.getCurrentSlot();
    dragValue = ! proc.patterns.cell (slot, row, step);
    dragRow = row; lastDragStep = step;
    proc.patterns.setCell (slot, row, step, dragValue);
    repaint();
}

void StepGrid::mouseDrag (const juce::MouseEvent& e)
{
    if (dragRow < 0) return;
    int row, step;
    if (! hit (e.getPosition(), row, step) || row != dragRow || step == lastDragStep) return;
    lastDragStep = step;
    proc.patterns.setCell (proc.getCurrentSlot(), row, step, dragValue);
    repaint();
}

// ================================================================================================
// PatternTab sits at (1240, 34): LEDs at x 4, tab cells at x 16..40
void PatternTab::paint (juce::Graphics& g)
{
    const int sel = (int) proc.apvts.getRawParameterValue ("pattern")->load();
    const int cur = proc.getCurrentSlot(), pending = proc.getPendingSlot();
    const bool blink = (juce::Time::getMillisecondCounter() / 200) % 2 == 0;
    const float h = 126.0f / kNumSlots;
    for (int i = 0; i < kNumSlots; ++i)
    {
        auto cell = juce::Rectangle<float> (16.0f, i * h, 24.0f, h).reduced (0.0f, 0.5f);
        g.setColour (i == sel ? colours::orange : colours::ctrlBg);
        g.fillRect (cell);
        g.setColour (colours::track);
        g.drawRect (cell, 0.8f);
        g.setColour (i == sel ? colours::title : colours::label);
        g.setFont (bold (9.0f));
        g.drawText (juce::String (i + 1), cell, juce::Justification::centred);

        auto led = juce::Rectangle<float> (4.0f, 4.0f + i * 15.75f, 8.0f, 8.0f);
        const bool lit = (i == cur) || (i == pending && blink);
        g.setColour (lit ? colours::orange : colours::ledOff);
        g.fillEllipse (led);
    }
}

void PatternTab::mouseDown (const juce::MouseEvent& e)
{
    const int i = juce::jlimit (0, kNumSlots - 1, (int) (e.position.y / (126.0f / kNumSlots)));
    if (auto* p = proc.apvts.getParameter ("pattern"))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 ((float) i));
        p->endChangeGesture();
    }
    repaint();
}

// ================================================================================================
static const char* kVoiceIds[kNumVoices]    = { "bd", "sd", "lt", "ht", "cp", "ch", "oh", "cy" };
static const char* kVoiceTitles[kNumVoices] = { "BASS DRUM", "SNARE", "LOW TOM", "HI TOM", "CLAP", "CLOSED HAT", "OPEN HAT", "CYMBAL" };
static const char* kVoiceNotes[kNumVoices]  = { "C1", "D1", "A1", "D2", "D#1", "F#1", "A#1", "C#2" };

Tross606Editor::Tross606Editor (Tross606Processor& p)
    : AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);
    content.setBounds (0, 0, kWidth, kHeight);

    for (int v = 0; v < kNumVoices; ++v)
    {
        const int x = 4 + v * 96;
        const juce::String id = kVoiceIds[v];
        addDial (id + "_tune", "Tune", x + 1, 24).getProperties().set ("bipolar", true);
        addDial (id + "_decay", "Decay", x + 47, 24);
        addDial (id + "_lvl", "Level", x + 1, 84);
        if (v == BD) addDial ("bd_attack", "Attack", x + 47, 84);
        if (v == SD) addDial ("sd_snappy", "Snappy", x + 47, 84);
        if (v == CP) addDial ("cp_noise", "Noise", x + 47, 84);
        if (v == CY) addDial ("cy_tone", "Tone", x + 47, 84);
        addTextToggle (id + "_mute", "M", colours::muteOn, { x + 4, 144, 22, 16 });
        addTextToggle (id + "_solo", "S", colours::soloOn, { x + 66, 144, 22, 16 });
    }
    addDial ("accent", "Accent", 773, 24);
    addDial ("master", "Volume", 819, 24);

    // sequencer
    content.addAndMakeVisible (grid);
    grid.setBounds (868, 19, 332, 146);
    content.addAndMakeVisible (tab);
    tab.setBounds (1240, 34, 44, 128);

    auto seqOn = std::make_unique<juce::ToggleButton>();
    seqOn->setBounds (1206, 46, 16, 16);
    content.addAndMakeVisible (*seqOn);
    bAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "seqon", *seqOn));
    buttons.push_back (std::move (seqOn));

    auto len = std::make_unique<juce::Slider> (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    len->getProperties().set ("numbox", true);
    len->setMouseDragSensitivity (120);
    len->setBounds (1204, 86, 38, 15);
    content.addAndMakeVisible (*len);
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "seqlen", *len));
    len->setDoubleClickReturnValue (true, 16.0);
    sliders.push_back (std::move (len));

    clearBtn.setBounds (1204, 118, 38, 16);
    clearBtn.onClick = [this] { proc.clearCurrentPattern(); grid.repaint(); };
    content.addAndMakeVisible (clearBtn);
    addTextToggle ("rec", "REC", juce::Colour (0xffe8463a), { 1204, 140, 38, 16 });

    if (proc.isStandalone())
    {
        playBtn.setClickingTogglesState (true);
        playBtn.setToggleState (proc.internalPlay.load(), juce::dontSendNotification);
        playBtn.onClick = [this] { proc.internalPlay = playBtn.getToggleState(); };
        playBtn.setBounds (778, 100, 38, 16);
        content.addAndMakeVisible (playBtn);

        bpmBox.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        bpmBox.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        bpmBox.getProperties().set ("numbox", true);
        bpmBox.setRange (40.0, 240.0, 1.0);
        bpmBox.setValue (proc.internalBpm.load(), juce::dontSendNotification);
        bpmBox.setMouseDragSensitivity (300);
        bpmBox.textFromValueFunction = [] (double v) { return juce::String ((int) v) + " bpm"; };
        bpmBox.onValueChange = [this] { proc.internalBpm = (float) bpmBox.getValue(); };
        bpmBox.setBounds (820, 100, 40, 16);
        content.addAndMakeVisible (bpmBox);
    }

    setResizable (true, true);
    setResizeLimits (kWidth * 3 / 4, kHeight * 3 / 4, kWidth * 2, kHeight * 2);
    getConstrainer()->setFixedAspectRatio ((double) kWidth / kHeight);
    setSize (kWidth, kHeight);
    startTimerHz (30);
}

Tross606Editor::~Tross606Editor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

juce::Slider& Tross606Editor::addDial (const juce::String& paramId, const juce::String& label, int x, int y)
{
    auto s = std::make_unique<juce::Slider> (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    s->setName (label);
    s->setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    s->setBounds (x, y, 44, 48);
    s->setPopupDisplayEnabled (false, false, nullptr);
    content.addAndMakeVisible (*s);
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, paramId, *s));
    if (auto* p = proc.apvts.getParameter (paramId))
        s->setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    sliders.push_back (std::move (s));
    return *sliders.back();
}

juce::Button& Tross606Editor::addTextToggle (const juce::String& paramId, const juce::String& text, juce::Colour on,
                                             juce::Rectangle<int> r)
{
    auto b = std::make_unique<juce::TextButton> (text);
    b->setClickingTogglesState (true);
    b->getProperties().set ("onColour", (juce::int64) on.getARGB());
    b->setBounds (r);
    content.addAndMakeVisible (*b);
    bAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, paramId, *b));
    buttons.push_back (std::move (b));
    return *buttons.back();
}

void Tross606Editor::paint (juce::Graphics& g) { g.fillAll (colours::bg); }

void Tross606Editor::resized()
{
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / kWidth));
}

void Tross606Editor::paintPanel (juce::Graphics& g)
{
    g.fillAll (colours::bg);

    auto panel = [&] (int x, int w, const juce::String& title)
    {
        g.setColour (colours::border);
        g.drawRoundedRectangle (juce::Rectangle<float> ((float) x, 2.0f, (float) w, 163.0f).reduced (0.5f), 6.0f, 1.0f);
        g.setColour (colours::orange);
        juce::Path head;
        head.addRoundedRectangle ((float) x, 2.0f, (float) w, 17.0f, 6.0f, 6.0f, true, true, false, false);
        g.fillPath (head);
        g.setColour (colours::title);
        g.setFont (bold (9.5f));
        g.drawText (title, x, 3, w, 16, juce::Justification::centred);
    };

    for (int v = 0; v < kNumVoices; ++v)
    {
        const int x = 4 + v * 96;
        panel (x, 92, kVoiceTitles[v]);
        g.setColour (colours::dimText);
        g.setFont (plain (9.0f));
        g.drawText (kVoiceNotes[v], x + 28, 146, 36, 15, juce::Justification::centred);
    }
    panel (772, 92, "TROSS 606");
    panel (868, 428, "SEQUENCER");

    g.setColour (colours::dimText);
    g.setFont (plain (8.5f));
    g.drawText ("Ticco Ross 2026", 772, 140, 92, 14, juce::Justification::centred);

    g.setColour (colours::label);
    g.setFont (bold (9.0f));
    g.drawText ("ON",  1204, 30, 40, 15, juce::Justification::centredLeft);
    g.drawText ("LEN", 1204, 70, 40, 15, juce::Justification::centredLeft);
    g.drawText ("PAT", 1242, 21, 40, 12, juce::Justification::centredLeft);
    if (proc.isStandalone())
        g.drawText ("TRANSPORT", 778, 86, 80, 12, juce::Justification::centredLeft);
}

void Tross606Editor::timerCallback()
{
    const int v = proc.patterns.getVersion(), st = proc.getPlayStep();
    const int slot = proc.getCurrentSlot(), pend = proc.getPendingSlot();
    if (v != lastVersion || st != lastStep || slot != lastSlot)
    {
        lastVersion = v; lastStep = st;
        grid.repaint();
    }
    if (slot != lastSlot || pend != lastPending || pend >= 0)
    {
        lastSlot = slot; lastPending = pend;
        tab.repaint();
    }
    // LEN changes dim the grid; the selected tab follows host automation
    const int len = (int) proc.apvts.getRawParameterValue ("seqlen")->load();
    const int sel = (int) proc.apvts.getRawParameterValue ("pattern")->load();
    if (len != lastLen) { lastLen = len; grid.repaint(); }
    if (sel != lastSel) { lastSel = sel; tab.repaint(); }
}
