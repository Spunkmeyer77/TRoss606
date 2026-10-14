#include "PluginEditor.h"

using namespace tross606;

namespace
{
namespace col
{
const juce::Colour silverTop   { 0xffd9d9d4 }, silverMid { 0xffc8c8c2 }, silverBottom { 0xffb9b9b3 };
const juce::Colour ink         { 0xff1d1d1d };
const juce::Colour printText   { 0xff2d2d2d };
const juce::Colour softText    { 0xff555555 };
const juce::Colour orange      { 0xffe0531f };
const juce::Colour stripBg     { 0xff2b2b2b };
const juce::Colour stripText   { 0xfff0efe8 };
const juce::Colour knobRing    { 0xffa9a9a3 };
const juce::Colour darkTop     { 0xff2a2a2a }, darkBottom { 0xff1d1d1d };
const juce::Colour darkLabel   { 0xff9a9a92 };
const juce::Colour numText     { 0xffb9b9b0 };
const juce::Colour rowText     { 0xffe9e8e0 };
const juce::Colour accText     { 0xffff8a55 };
const juce::Colour ledOff      { 0xff4a1a10 };
const juce::Colour ledOn       { 0xffff5a1f };
const juce::Colour cream1      { 0xffe7e5dc }, cream2 { 0xffc9c7bd };
const juce::Colour grey1       { 0xffa9a7a0 }, grey2  { 0xff8f8d86 };
const juce::Colour orange1     { 0xffe0531f }, orange2 { 0xffc2410f };
const juce::Colour yellow1     { 0xfff0b23a }, yellow2 { 0xffd6951a };
const juce::Colour muteOn      { 0xffffcf4a }, soloOn { 0xff4f9ef0 };
const juce::Colour lcdBg       { 0xff1a1712 }, lcdText { 0xffff6a2b };
}

juce::Font font (float size, bool bold = false, float kerning = 0.0f)
{
    return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain)).withExtraKerningFactor (kerning);
}

// a glowing or dark LED
void drawLed (juce::Graphics& g, juce::Point<float> c, float d, bool lit, juce::Colour on = col::ledOn)
{
    if (lit)
    {
        g.setGradientFill (juce::ColourGradient (on.withAlpha (0.45f), c, on.withAlpha (0.0f), c.translated (d * 1.0f, 0), true));
        g.fillEllipse (juce::Rectangle<float> (d * 2.0f, d * 2.0f).withCentre (c));
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffff3c4), c, on.darker (0.2f), c.translated (d * 0.5f, 0), true));
    }
    else
        g.setColour (col::ledOff);
    g.fillEllipse (juce::Rectangle<float> (d, d).withCentre (c));
}

// cap with a vertical gradient and a dark "travel" shadow underneath, like a hardware key
void drawKey (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour top, juce::Colour bottom, float radius, float shadow)
{
    g.setColour (juce::Colour (0xff0b0b0b));
    g.fillRoundedRectangle (r.translated (0, shadow), radius);
    g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, radius);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawHorizontalLine ((int) r.getY() + 1, r.getX() + radius, r.getRight() - radius);
}

const char* kVoiceIds[kNumVoices]    = { "bd", "sd", "lt", "ht", "cp", "ch", "oh", "cy" };
const char* kVoiceTitles[kNumVoices] = { "BASS DRUM", "SNARE", "LOW TOM", "HI TOM", "CLAP", "CLOSED HAT", "OPEN HAT", "CYMBAL" };
const char* kVoiceNotes[kNumVoices]  = { "C1", "D1", "A1", "D2", "D#1", "F#1", "A#1", "C#2" };
const char* kExtraIds[kNumVoices]    = { "bd_attack", "sd_snappy", nullptr, nullptr, "cp_noise", nullptr, nullptr, "cy_tone" };
const char* kExtraNames[kNumVoices]  = { "ATTACK", "SNAPPY", nullptr, nullptr, "NOISE", nullptr, nullptr, "TONE" };

// ---- layout (1120 x 455)
constexpr int kMargin = 18, kChanW = 120, kTopH = 222, kSeqY = 225;
constexpr int kKnobRow1 = 93, kKnobRow2 = 146, kKnobD = 34;
constexpr int kMasterX = 984, kMasterW = 118;
int chanX (int v) { return kMargin + v * kChanW; }

// lower section, in StepGrid coordinates (the grid sits at 0, kSeqY)
constexpr float kGridX = 64.0f, kGridR = 914.0f, kColGap = 5.0f, kPadTop = 41.0f, kRowPitch = 19.0f, kPadH = 15.0f;
constexpr float kColW = (kGridR - kGridX - 15 * kColGap) / 16.0f;
} // namespace

// ================================================================================================
TrossLookAndFeel::TrossLookAndFeel()
{
    setColour (juce::BubbleComponent::backgroundColourId, col::lcdBg);
    setColour (juce::BubbleComponent::outlineColourId, juce::Colours::black);
    setColour (juce::TooltipWindow::textColourId, col::lcdText);
}

void TrossLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end,
                                         juce::Slider& s)
{
    const auto area = juce::Rectangle<int> (x, y, w, h).toFloat();

    if (s.getProperties().contains ("seg"))   // red LED number read-out (LENGTH, BPM)
    {
        g.setColour (juce::Colour (0xff120f0c));
        g.fillRoundedRectangle (area, 3.0f);
        g.setColour (juce::Colours::black);
        g.drawRoundedRectangle (area.reduced (0.5f), 3.0f, 1.0f);
        const auto txt = s.getTextFromValue (s.getValue());
        const float size = (float) s.getProperties().getWithDefault ("segSize", 20.0f);
        g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), size, juce::Font::bold)));
        g.setColour (col::ledOn.withAlpha (0.35f));
        g.drawText (txt, area.translated (0, 0.5f).expanded (0.6f), juce::Justification::centred);
        g.setColour (col::ledOn);
        g.drawText (txt, area, juce::Justification::centred);
        return;
    }

    // black knurled knob with a light ring and a white pointer
    const float d = juce::jmin (area.getWidth(), area.getHeight()) - 6.0f;
    const auto c = area.getCentre();
    auto body = juce::Rectangle<float> (d, d).withCentre (c);

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (body.expanded (3.0f).translated (0, 2.0f));
    g.setColour (col::knobRing);
    g.fillEllipse (body.expanded (3.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4a4a), c.x - d * 0.12f, c.y - d * 0.18f,
                                             juce::Colour (0xff0b0b0b), c.x + d * 0.5f, c.y + d * 0.5f, true));
    g.fillEllipse (body);
    // knurling
    g.setColour (juce::Colour (0xff262626));
    g.drawEllipse (body.reduced (1.0f), 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (int i = 0; i < 24; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / 24.0f;
        g.drawLine (juce::Line<float> (c.getPointOnCircumference (d * 0.5f - 2.5f, a), c.getPointOnCircumference (d * 0.5f - 0.5f, a)), 1.0f);
    }

    const float angle = start + pos * (end - start);
    juce::Path p;
    p.startNewSubPath (c.getPointOnCircumference (d * 0.5f - 2.5f, angle));
    p.lineTo (c.getPointOnCircumference (d * 0.5f - d * 0.36f, angle));
    g.setColour (juce::Colour (0xfff2f2ee));
    g.strokePath (p, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void TrossLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    const auto style = b.getProperties()["style"].toString();
    const bool on = b.getToggleState();
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);

    if (style == "rubber")   // small M / S buttons on the silver panel
    {
        r = r.withTrimmedBottom (1.0f);
        const auto onColour = juce::Colour ((juce::uint32) (juce::int64) b.getProperties()["onColour"]);
        const auto top = on ? onColour.brighter (0.15f) : juce::Colour (0xffefefea);
        const auto bot = on ? onColour.darker (0.1f) : juce::Colour (0xffcfcfc9);
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillRoundedRectangle (r.translated (0, 1.0f), 3.0f);
        g.setGradientFill (juce::ColourGradient (top.brighter (highlighted ? 0.05f : 0.0f), 0, r.getY(), bot, 0, r.getBottom(), false));
        g.fillRoundedRectangle (r.translated (0, down ? 1.0f : 0.0f), 3.0f);
        return;
    }

    // hardware keys on the dark section (RUN, CLEAR, REC, PLAY)
    r = r.withTrimmedBottom (2.0f).translated (0, down ? 1.5f : 0.0f);
    const bool rec = style == "rec";
    if (rec && on) drawKey (g, r, col::orange1, col::orange2, 4.0f, down ? 0.5f : 2.0f);
    else           drawKey (g, r, highlighted ? col::cream1.brighter (0.05f) : col::cream1, col::cream2, 4.0f, down ? 0.5f : 2.0f);

    if (b.getProperties().contains ("led"))
        drawLed (g, { r.getX() + 12.0f, r.getCentreY() }, 6.0f, on, rec ? juce::Colours::white : col::ledOn);
}

void TrossLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down)
{
    const auto style = b.getProperties()["style"].toString();
    const bool on = b.getToggleState();
    auto r = b.getLocalBounds();
    if (style == "rubber")
    {
        g.setColour (on && b.getButtonText() == "S" ? juce::Colours::white : juce::Colour (0xff333333));
        g.setFont (font (8.5f, true));
        g.drawText (b.getButtonText(), r.withTrimmedBottom (1).translated (0, down ? 1 : 0), juce::Justification::centred);
        return;
    }
    r = r.withTrimmedBottom (2).translated (0, down ? 1 : 0);
    if (b.getProperties().contains ("led")) r = r.withTrimmedLeft (14);
    g.setColour (style == "rec" && on ? juce::Colours::white : juce::Colour (0xff222222));
    g.setFont (font (9.0f, true, 0.08f));
    g.drawText (b.getButtonText(), r, juce::Justification::centred);
}

void TrossLookAndFeel::drawBubble (juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>& body)
{
    g.setColour (col::lcdBg);
    g.fillRoundedRectangle (body, 3.0f);
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (body.reduced (0.5f), 3.0f, 1.0f);
}

juce::Font TrossLookAndFeel::getSliderPopupFont (juce::Slider&)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::bold));
}

// ================================================================================================
juce::Rectangle<float> StepGrid::padRect (int row, int step)
{
    return { kGridX + (float) step * (kColW + kColGap), kPadTop + (float) row * kRowPitch, kColW, kPadH };
}

void StepGrid::paint (juce::Graphics& g)
{
    static const char* rowNames[kNumRows] = { "ACC", "BD", "SD", "LT", "HT", "CP", "CH", "OH", "CY" };
    static const juce::Colour groupTop[4] = { col::cream1, col::grey1, col::orange1, col::yellow1 };
    static const juce::Colour groupBot[4] = { col::cream2, col::grey2, col::orange2, col::yellow2 };

    const auto h = (float) getHeight();
    g.setGradientFill (juce::ColourGradient (col::darkTop, 0, 0, col::darkBottom, 0, h, false));
    g.fillAll();

    const int slot = proc.getCurrentSlot();
    const int playing = proc.getPlayStep();
    const int len = (int) proc.apvts.getRawParameterValue ("seqlen")->load();

    g.setColour (col::darkLabel);
    g.setFont (font (9.0f, false, 0.2f));
    g.drawText ("STEP", kMargin, 12, 46, 12, juce::Justification::centredLeft);

    for (int s = 0; s < kNumSteps; ++s)
    {
        const auto x = kGridX + (float) s * (kColW + kColGap);
        g.setColour (s % 4 == 0 ? col::orange : col::numText);
        g.setFont (font (9.5f, s % 4 == 0));
        g.drawText (juce::String (s + 1), juce::Rectangle<float> (x, 12.0f, kColW, 12.0f), juce::Justification::centred);
        drawLed (g, { x + kColW * 0.5f, 33.0f }, 7.0f, s == playing);
    }

    for (int row = 0; row < kNumRows; ++row)
    {
        g.setColour (row == 0 ? col::accText : col::rowText);
        g.setFont (font (10.0f, true, 0.06f));
        g.drawText (rowNames[row], juce::Rectangle<float> ((float) kMargin, kPadTop + (float) row * kRowPitch, 44.0f, kPadH),
                    juce::Justification::centredLeft);

        for (int s = 0; s < kNumSteps; ++s)
        {
            const auto r = padRect (row, s);
            const bool on = proc.patterns.cell (slot, row, s);
            const int grp = s / 4;

            g.setColour (juce::Colour (0xff111111));
            g.fillRoundedRectangle (r.translated (0, 1.0f), 3.0f);
            g.setGradientFill (juce::ColourGradient (groupTop[grp], 0, r.getY(), groupBot[grp], 0, r.getBottom(), false));
            g.fillRoundedRectangle (r, 3.0f);

            const auto c = r.getCentre();
            if (on)
            {
                g.setColour (juce::Colour (0xff1a1a1a));
                g.fillEllipse (juce::Rectangle<float> (11.0f, 11.0f).withCentre (c));
                drawLed (g, c, 8.0f, true);
            }
            else
            {
                g.setColour (juce::Colours::black.withAlpha (0.2f));
                g.fillEllipse (juce::Rectangle<float> (5.5f, 5.5f).withCentre (c));
            }

            if (s == playing)
            {
                g.setColour (juce::Colours::white.withAlpha (0.75f));
                g.drawRoundedRectangle (r.expanded (1.5f), 4.0f, 1.0f);
            }
            if (s >= len)   // past LENGTH: dimmed
            {
                g.setColour (col::darkBottom.withAlpha (0.65f));
                g.fillRoundedRectangle (r.expanded (0.5f, 1.5f), 3.0f);
            }
        }
    }
}

bool StepGrid::hit (juce::Point<float> p, int& row, int& step) const
{
    for (row = 0; row < kNumRows; ++row)
        for (step = 0; step < kNumSteps; ++step)
            if (padRect (row, step).expanded (kColGap * 0.5f, 2.0f).contains (p)) return true;
    return false;
}

void StepGrid::mouseDown (const juce::MouseEvent& e)
{
    int row, step;
    if (! hit (e.position, row, step)) return;
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
    if (! hit (e.position, row, step) || row != dragRow || step == lastDragStep) return;
    lastDragStep = step;
    proc.patterns.setCell (proc.getCurrentSlot(), row, step, dragValue);
    repaint();
}

// ================================================================================================
// PatternPad sits at (918, kSeqY), 100 x 230
juce::Rectangle<float> PatternPad::buttonRect (int slot)
{
    return { 12.0f + (float) (slot % 2) * 44.0f, 31.0f + (float) (slot / 2) * 27.0f, 38.0f, 20.0f };
}

void PatternPad::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (col::darkTop, 0, 0, col::darkBottom, 0, (float) getHeight(), false));
    g.fillAll();

    g.setColour (col::darkLabel);
    g.setFont (font (8.5f, false, 0.15f));
    g.drawText ("PATTERN", 0, 12, 94, 12, juce::Justification::centred);

    const int sel = (int) proc.apvts.getRawParameterValue ("pattern")->load();
    const int cur = proc.getCurrentSlot(), pending = proc.getPendingSlot();
    const bool blink = (juce::Time::getMillisecondCounter() / 200) % 2 == 0;

    for (int i = 0; i < kNumSlots; ++i)
    {
        const auto r = buttonRect (i);
        if (i == sel) drawKey (g, r, col::cream1, col::cream2, 4.0f, 2.0f);
        else          drawKey (g, r, col::grey1, col::grey2, 4.0f, 2.0f);
        drawLed (g, { r.getX() + 6.5f, r.getY() + 6.5f }, 5.0f, i == cur || (i == pending && blink));
        g.setColour (juce::Colour (0xff222222));
        g.setFont (font (10.0f, true));
        g.drawText (juce::String (i + 1), r.withTrimmedLeft (6.0f), juce::Justification::centred);
    }
}

void PatternPad::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < kNumSlots; ++i)
        if (buttonRect (i).expanded (2.0f).contains (e.position))
        {
            if (auto* p = proc.apvts.getParameter ("pattern"))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost (p->convertTo0to1 ((float) i));
                p->endChangeGesture();
            }
            repaint();
            return;
        }
}

// ================================================================================================
void StatusLcd::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (col::lcdBg);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    const int cur = proc.getCurrentSlot(), pending = proc.getPendingSlot();
    const juce::String txt = pending >= 0 ? "PAT " + juce::String (cur + 1) + " > " + juce::String (pending + 1)
                                          : "PATTERN " + juce::String (cur + 1);
    g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 11.5f, juce::Font::bold)));
    g.setColour (col::lcdText.withAlpha (0.3f));
    g.drawText (txt, r.expanded (0.5f), juce::Justification::centred);
    g.setColour (col::lcdText);
    g.drawText (txt, r, juce::Justification::centred);
}

// ================================================================================================
Tross606Editor::Tross606Editor (Tross606Processor& p)
    : AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);
    content.setBounds (0, 0, kWidth, kHeight);

    for (int v = 0; v < kNumVoices; ++v)
    {
        const int x = chanX (v);
        const juce::String id = kVoiceIds[v];
        addKnob (id + "_tune",  { x + 30, kKnobRow1 }, kKnobD);
        addKnob (id + "_decay", { x + 82, kKnobRow1 }, kKnobD);
        addKnob (id + "_lvl",   { x + 30, kKnobRow2 }, kKnobD);
        if (kExtraIds[v] != nullptr) addKnob (kExtraIds[v], { x + 82, kKnobRow2 }, kKnobD);
        addButton (id + "_mute", "M", { x + 8,  196, 24, 15 }, "rubber").getProperties().set ("onColour", (juce::int64) col::muteOn.getARGB());
        addButton (id + "_solo", "S", { x + 88, 196, 24, 15 }, "rubber").getProperties().set ("onColour", (juce::int64) col::soloOn.getARGB());
    }
    addKnob ("accent", { kMasterX + 30, 96 }, 40);
    addKnob ("master", { kMasterX + 88, 96 }, 40);
    content.addAndMakeVisible (lcd);
    lcd.setBounds (kMasterX + 7, 128, kMasterW - 14, 20);

    // sequencer
    content.addAndMakeVisible (grid);
    grid.setBounds (0, kSeqY, 918, kHeight - kSeqY);
    content.addAndMakeVisible (patternPad);
    patternPad.setBounds (918, kSeqY, 100, kHeight - kSeqY);

    auto len = std::make_unique<juce::Slider> (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    len->getProperties().set ("seg", true);
    len->setMouseDragSensitivity (120);
    len->setBounds (1034, 256, 48, 30);
    content.addAndMakeVisible (*len);
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "seqlen", *len));
    len->setDoubleClickReturnValue (true, 16.0);
    len->setTooltip ("Drag up/down to set the pattern length");
    sliders.push_back (std::move (len));

    addButton ("seqon", "RUN", { 1027, 296, 62, 24 }, "key").getProperties().set ("led", true);
    clearBtn.getProperties().set ("style", "key");
    clearBtn.setBounds (1027, 326, 62, 24);
    clearBtn.onClick = [this] { proc.clearCurrentPattern(); grid.repaint(); };
    content.addAndMakeVisible (clearBtn);
    addButton ("rec", "REC", { 1027, 356, 62, 24 }, "rec").getProperties().set ("led", true);

    if (proc.isStandalone())   // no host transport: own play button + tempo
    {
        playBtn.getProperties().set ("style", "key");
        playBtn.getProperties().set ("led", true);
        playBtn.setClickingTogglesState (true);
        playBtn.setToggleState (proc.internalPlay.load(), juce::dontSendNotification);
        playBtn.onClick = [this] { proc.internalPlay = playBtn.getToggleState(); };
        playBtn.setBounds (1027, 390, 62, 24);
        content.addAndMakeVisible (playBtn);

        bpmBox.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        bpmBox.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        bpmBox.getProperties().set ("seg", true);
        bpmBox.getProperties().set ("segSize", 13.0f);
        bpmBox.setRange (40.0, 240.0, 1.0);
        bpmBox.setValue (proc.internalBpm.load(), juce::dontSendNotification);
        bpmBox.setMouseDragSensitivity (300);
        bpmBox.textFromValueFunction = [] (double v) { return juce::String ((int) v); };
        bpmBox.onValueChange = [this] { proc.internalBpm = (float) bpmBox.getValue(); };
        bpmBox.setBounds (1034, 431, 48, 18);
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

juce::Slider& Tross606Editor::addKnob (const juce::String& paramId, juce::Point<int> centre, int diameter)
{
    auto s = std::make_unique<juce::Slider> (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    s->setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    const int box = diameter + 6;
    s->setBounds (juce::Rectangle<int> (box, box).withCentre (centre));
    s->setPopupDisplayEnabled (true, true, this, 1200);
    content.addAndMakeVisible (*s);
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, paramId, *s));
    if (auto* p = proc.apvts.getParameter (paramId))
        s->setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    sliders.push_back (std::move (s));
    return *sliders.back();
}

juce::Button& Tross606Editor::addButton (const juce::String& paramId, const juce::String& text, juce::Rectangle<int> r, const char* style)
{
    auto b = std::make_unique<juce::TextButton> (text);
    b->setClickingTogglesState (true);
    b->getProperties().set ("style", style);
    b->setBounds (r);
    content.addAndMakeVisible (*b);
    bAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, paramId, *b));
    buttons.push_back (std::move (b));
    return *buttons.back();
}

void Tross606Editor::paint (juce::Graphics& g) { g.fillAll (col::darkBottom); }

void Tross606Editor::resized()
{
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / kWidth));
}

void Tross606Editor::paintPanel (juce::Graphics& g)
{
    // ---- brushed silver top
    g.setGradientFill (juce::ColourGradient::vertical (col::silverTop, 0.0f, col::silverBottom, (float) kTopH));
    g.fillRect (0, 0, kWidth, kTopH);
    for (int x = 0; x < kWidth; x += 3)
    {
        g.setColour (juce::Colours::white.withAlpha (0.07f));
        g.fillRect (x, 0, 1, kTopH);
        g.setColour (juce::Colours::black.withAlpha (0.025f));
        g.fillRect (x + 1, 0, 2, kTopH);
    }

    // header: name, subtitle, build version
    g.setColour (col::ink);
    auto logo = font (30.0f, true, -0.03f);
    g.setFont (logo);
    const float troW = juce::GlyphArrangement::getStringWidth (logo, "TRoss ");
    g.drawText ("TRoss ", juce::Rectangle<float> ((float) kMargin, 12.0f, troW + 4.0f, 34.0f), juce::Justification::bottomLeft);
    const float numW = juce::GlyphArrangement::getStringWidth (logo, "606");
    g.setColour (col::orange);
    g.drawText ("606", juce::Rectangle<float> ((float) kMargin + troW, 12.0f, numW + 4.0f, 34.0f), juce::Justification::bottomLeft);
    g.setColour (juce::Colour (0xff3a3a3a));
    g.setFont (font (10.5f, false, 0.3f));
    g.drawText ("ANALOG-STYLE DRUM SYNTHESIZER", juce::Rectangle<float> ((float) kMargin + troW + numW + 14.0f, 26.0f, 400.0f, 16.0f),
                juce::Justification::bottomLeft);
    g.setColour (col::softText);
    g.setFont (font (10.0f, false, 0.2f));
    g.drawText ("v" JucePlugin_VersionString, kWidth - kMargin - 200, 26, 200, 16, juce::Justification::bottomRight);
    g.setColour (juce::Colour (0xff2b2b2b));
    g.fillRect (kMargin, 51, kWidth - 2 * kMargin, 2);

    // channel strips
    auto knobLabel = [&] (const juce::String& text, int cx, int y)
    {
        g.setColour (col::printText);
        g.setFont (font (8.5f, false, 0.1f));
        g.drawText (text, cx - 30, y, 60, 11, juce::Justification::centred);
    };
    auto nameStrip = [&] (const juce::String& text, juce::Rectangle<int> r)
    {
        g.setColour (col::stripBg);
        g.fillRoundedRectangle (r.toFloat(), 2.0f);
        g.setColour (col::stripText);
        g.setFont (font (10.0f, true, 0.15f));
        g.drawText (text, r, juce::Justification::centred);
    };

    for (int v = 0; v < kNumVoices; ++v)
    {
        const int x = chanX (v);
        knobLabel ("TUNE", x + 30, 63);
        knobLabel ("DECAY", x + 82, 63);
        knobLabel ("LEVEL", x + 30, 116);
        if (kExtraNames[v] != nullptr) knobLabel (kExtraNames[v], x + 82, 116);
        nameStrip (kVoiceTitles[v], { x + 8, 172, kChanW - 16, 17 });
        g.setColour (col::softText);
        g.setFont (font (9.0f, false, 0.1f));
        g.drawText (kVoiceNotes[v], x + 32, 196, 56, 15, juce::Justification::centred);

        if (v < kNumVoices - 1)   // engraved divider
        {
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.fillRect (x + kChanW - 1, 60, 1, 154);
            g.setColour (juce::Colours::white.withAlpha (0.45f));
            g.fillRect (x + kChanW, 60, 1, 154);
        }
    }

    // master
    g.setColour (juce::Colours::black.withAlpha (0.05f));
    g.fillRoundedRectangle ((float) kMasterX, 58.0f, (float) kMasterW, 158.0f, 6.0f);
    knobLabel ("ACCENT", kMasterX + 30, 63);
    knobLabel ("VOLUME", kMasterX + 88, 63);
    nameStrip ("MASTER", { kMasterX + 7, 172, kMasterW - 14, 17 });

    // ---- dark sequencer section
    g.setColour (col::orange);
    g.fillRect (0, kTopH, kWidth, kSeqY - kTopH);
    g.setGradientFill (juce::ColourGradient::vertical (col::darkTop, (float) kSeqY, col::darkBottom, (float) kHeight));
    g.fillRect (0, kSeqY, kWidth, kHeight - kSeqY);

    g.setColour (col::darkLabel);
    g.setFont (font (8.5f, false, 0.15f));
    g.drawText ("LENGTH", 1018, kSeqY + 12, 80, 12, juce::Justification::centred);
    if (proc.isStandalone())
        g.drawText ("BPM", 1018, 419, 80, 10, juce::Justification::centred);
}

void Tross606Editor::timerCallback()
{
    const int v = proc.patterns.getVersion(), st = proc.getPlayStep();
    const int slot = proc.getCurrentSlot(), pend = proc.getPendingSlot();
    const int len = (int) proc.apvts.getRawParameterValue ("seqlen")->load();
    const int sel = (int) proc.apvts.getRawParameterValue ("pattern")->load();

    if (v != lastVersion || st != lastStep || slot != lastSlot || len != lastLen)
        grid.repaint();
    if (slot != lastSlot || pend != lastPending || pend >= 0 || sel != lastSel)
    {
        patternPad.repaint();
        lcd.repaint();
    }
    lastVersion = v; lastStep = st; lastSlot = slot; lastPending = pend; lastLen = len; lastSel = sel;
}
