#include "PluginEditor.h"
#include "MidiExport.h"
#include "BinaryData.h"

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
constexpr int kMargin = 18, kChanW = 120, kTopH = 240, kSeqY = 243;
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
        if (b.getProperties().contains ("led"))
            drawLed (g, { r.getX() + 8.0f, r.getCentreY() }, 5.0f, on, juce::Colour (0xff49d04a));
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
        if (b.getProperties().contains ("led")) r = r.withTrimmedLeft (12);
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
            if (on && proc.patterns.flam (slot, row, s))   // flam: 1-3 tiny LEDs on the pad's left edge, main LED moves right
            {
                const auto fs = proc.patterns.flamStyle (slot, row, s);
                const auto cm = juce::Point<float> (r.getX() + r.getWidth() * 0.6f, c.y);
                g.setColour (juce::Colour (0xff1a1a1a));
                g.fillEllipse (juce::Rectangle<float> (11.0f, 11.0f).withCentre (cm));
                drawLed (g, cm, 8.0f, true);
                static const float ys[4][3] = { {}, { 0.5f }, { 0.28f, 0.72f }, { 0.2f, 0.5f, 0.8f } };
                const float gx = r.getX() + 5.5f;
                for (int k = 0; k < fs.graces; ++k)
                {
                    const juce::Point<float> gc { gx, r.getY() + r.getHeight() * ys[fs.graces][k] };
                    g.setColour (juce::Colour (0xff1a1a1a));   // dark socket, so it shows on every pad colour
                    const float sd = fs.graces == 3 ? 4.4f : 5.2f;
                    g.fillEllipse (juce::Rectangle<float> (sd, sd).withCentre (gc));
                    drawLed (g, gc, fs.graces == 3 ? 2.9f : 3.4f, true);
                }
                if (fs.spacing != 0 || fs.level != 0)   // its own spacing / level: a small mark under the main LED
                {
                    g.setColour (juce::Colour (0xff1a1a1a).withAlpha (0.55f));
                    g.fillRect (cm.x - 5.0f, r.getBottom() - 2.5f, 10.0f, 1.2f);
                }
            }
            else if (on)
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

            if (row > 0 && proc.patterns.stepHasLock (slot, row - 1, s))   // corner notch: this step has locks
            {
                juce::Path notch;
                notch.addTriangle (r.getRight() - 7.0f, r.getY(), r.getRight(), r.getY(), r.getRight(), r.getY() + 7.0f);
                g.setColour (juce::Colour (0xff1a1a1a));
                g.fillPath (notch);
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
    if (e.mods.isPopupMenu())   // right-click: step menu with the flam settings (flams live only here)
    {
        dragRow = -1;
        if (row > 0) showStepMenu (row, step);
        return;
    }
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
// P-LOCK screen
namespace
{
const char* kLockVoiceNames[kNumVoices] = { "BD", "SD", "LT", "HT", "CP", "CH", "OH", "CY" };
const char* kLockKnobNames[kNumVoices][kNumLockParams] = {
    { "TUNE", "DECAY", "LEVEL", "ATTACK" }, { "TUNE", "DECAY", "LEVEL", "SNAPPY" }, { "TUNE", "DECAY", "LEVEL", "" }, { "TUNE", "DECAY", "LEVEL", "" },
    { "TUNE", "DECAY", "LEVEL", "NOISE" },  { "TUNE", "DECAY", "LEVEL", "" },       { "TUNE", "DECAY", "LEVEL", "" }, { "TUNE", "DECAY", "LEVEL", "TONE" } };
constexpr float kBarTop = 64.0f, kBarBottom = 192.0f;
juce::String lockValueText (int param, float v)
{
    if (param == LockTune)  return (v > 0.05f ? "+" : "") + juce::String (v, 1);
    if (param == LockLevel) return v <= -69.9f ? juce::String ("-inf") : juce::String (v, 1);
    return juce::String (juce::roundToInt (v));
}
}

juce::Rectangle<float> LockGrid::tabRect (int i)  { return { kGridX + (float) i * 76.0f, 203.0f, 70.0f, 18.0f }; }
juce::Rectangle<float> LockGrid::voiceRect (int v) { return { (float) kMargin - 4.0f, kBarTop - 18.0f + (float) v * 18.5f, 44.0f, 16.0f }; }
int LockGrid::voice() const { return juce::jlimit (0, kNumVoices - 1, proc.lockVoiceSel.load()); }
int LockGrid::knob() const
{
    const int k = juce::jlimit (0, kNumLockParams - 1, proc.lockKnobSel.load());
    return k == LockExtra && ! hasExtraKnob (voice()) ? (int) LockTune : k;
}

void LockGrid::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (col::darkTop, 0, 0, col::darkBottom, 0, (float) getHeight(), false));
    g.fillAll();

    const int v = voice(), prm = knob();
    const int slot = proc.getCurrentSlot();
    const int playing = proc.getPlayStep();
    const int len = (int) proc.apvts.getRawParameterValue ("seqlen")->load();
    auto* param = proc.lockParameter (v, prm);
    const float knobNorm = param != nullptr ? param->getValue() : 0.5f;
    auto yFor = [&] (float n) { return kBarBottom - n * (kBarBottom - kBarTop); };
    const float zero = param != nullptr ? param->convertTo0to1 (0.0f) : 0.5f;   // Tune: bars grow from 0 st

    // step numbers and play LEDs, as on the note screen
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

    // the bar field
    const juce::Rectangle<float> field (kGridX - 4.0f, kBarTop - 20.0f, kGridR - kGridX + 8.0f, kBarBottom - kBarTop + 32.0f);
    g.setColour (juce::Colour (0xff141414));
    g.fillRoundedRectangle (field, 4.0f);
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (field.reduced (0.5f), 4.0f, 1.0f);

    // drum selector
    g.setColour (col::accText);
    g.setFont (font (9.0f, true, 0.2f));
    g.drawText ("LOCK", juce::Rectangle<float> ((float) kMargin, kBarTop - 36.0f, 44.0f, 12.0f), juce::Justification::centredLeft);
    for (int d = 0; d < kNumVoices; ++d)
    {
        const auto r = voiceRect (d);
        if (d == v) drawKey (g, r, col::orange1, col::orange2, 3.0f, 1.5f);
        g.setColour (d == v ? juce::Colours::white : col::rowText);
        g.setFont (font (10.0f, true, 0.08f));
        g.drawText (kLockVoiceNames[d], r.withTrimmedLeft (5.0f), juce::Justification::centredLeft);
        bool any = false;
        for (int s = 0; s < kNumSteps; ++s) any |= proc.patterns.stepHasLock (slot, d, s);
        if (any) drawLed (g, { r.getRight() - 7.0f, r.getCentreY() }, 4.5f, true, d == v ? juce::Colours::white : col::ledOn);
    }

    // knob value as a dotted guide, centre line for Tune
    if (prm == LockTune)
    {
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.fillRect (field.getX() + 4.0f, yFor (zero), field.getWidth() - 8.0f, 1.0f);
    }
    g.setColour (juce::Colours::white.withAlpha (0.28f));
    for (float x = field.getX() + 6.0f; x < field.getRight() - 6.0f; x += 6.0f)
        g.fillRect (x, yFor (knobNorm), 3.0f, 1.0f);

    for (int s = 0; s < kNumSteps; ++s)
    {
        const auto x = kGridX + (float) s * (kColW + kColGap);
        if (s > 0 && s % 4 == 0)   // beat lines
        {
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.fillRect (x - kColGap * 0.5f - 0.5f, field.getY() + 4.0f, 1.0f, field.getHeight() - 8.0f);
        }
        const bool note = proc.patterns.cell (slot, v + 1, s);
        const float a = (note ? 1.0f : 0.4f) * (s < len ? 1.0f : 0.5f);
        // a small mark under each step where the drum plays
        g.setColour ((note ? col::ledOn : juce::Colours::white).withAlpha (note ? 0.9f * a : 0.12f));
        g.fillRect (x + kColW * 0.5f - 4.0f, kBarBottom + 4.0f, 8.0f, 3.0f);
        if (s == playing)
        {
            g.setColour (juce::Colours::white.withAlpha (0.06f));
            g.fillRect (x - 1.0f, field.getY() + 2.0f, kColW + 2.0f, field.getHeight() - 4.0f);
        }

        const auto b = proc.patterns.lock (slot, v, s, prm);
        if (b != 0)
        {
            const float n = decodeLock (b);
            const float y = yFor (n), base = prm == LockTune ? yFor (zero) : kBarBottom;
            auto bar = juce::Rectangle<float> (x + 4.0f, juce::jmin (y, base), kColW - 8.0f, juce::jmax (2.0f, std::abs (base - y)));
            g.setColour (col::ledOn.withAlpha (0.85f * a));
            g.fillRect (bar);
            g.setColour (juce::Colour (0xfffff3c4).withAlpha (a));
            g.fillRect (bar.getX(), y - 0.75f, bar.getWidth(), 1.5f);
            g.setColour (col::lcdText.withAlpha (a));
            g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold)));
            const float ty = juce::jlimit (kBarTop - 15.0f, kBarBottom - 12.0f,
                                           prm == LockTune && n < zero ? bar.getBottom() + 1.0f : bar.getY() - 12.0f);
            if (param != nullptr)
                g.drawText (lockValueText (prm, param->convertFrom0to1 (n)), juce::Rectangle<float> (x - 4.0f, ty, kColW + 8.0f, 11.0f),
                            juce::Justification::centred);
        }
    }

    // knob keys
    for (int i = 0; i < kNumLockParams; ++i)
    {
        if (i == LockExtra && ! hasExtraKnob (v)) continue;
        const auto r = tabRect (i);
        if (i == prm) drawKey (g, r, col::orange1, col::orange2, 3.0f, 2.0f);
        else          drawKey (g, r, col::cream1, col::cream2, 3.0f, 2.0f);
        g.setColour (i == prm ? juce::Colours::white : juce::Colour (0xff222222));
        g.setFont (font (8.5f, true, 0.06f));
        g.drawText (kLockKnobNames[v][i], r, juce::Justification::centred);
    }
    g.setColour (col::darkLabel);
    g.setFont (font (8.5f, false, 0.05f));
    g.drawText ("drag across the bars to lock  -  right-drag or double-click removes  -  Shift-right-click: clear menu",
                juce::Rectangle<float> (kGridR - 520.0f, 203.0f, 520.0f, 18.0f), juce::Justification::centredRight);
}

int LockGrid::columnAt (float x) const
{
    const int c = (int) std::floor ((x - kGridX + kColGap * 0.5f) / (kColW + kColGap));
    return juce::jlimit (0, kNumSteps - 1, c);
}

float LockGrid::normAt (float y) const
{
    return juce::jlimit (0.0f, 1.0f, (kBarBottom - y) / (kBarBottom - kBarTop));
}

void LockGrid::apply (int col, float y)
{
    const int slot = proc.getCurrentSlot(), v = voice(), prm = knob();
    const int from = lastCol < 0 ? col : lastCol;
    for (int c = juce::jmin (from, col); c <= juce::jmax (from, col); ++c)
        proc.patterns.setLock (slot, v, c, prm, erasing ? (uint8_t) 0 : encodeLock (normAt (y)));
    lastCol = col;
    repaint();
}

void LockGrid::mouseDown (const juce::MouseEvent& e)
{
    for (int d = 0; d < kNumVoices; ++d)
        if (voiceRect (d).contains (e.position)) { proc.lockVoiceSel = d; repaint(); return; }
    for (int i = 0; i < kNumLockParams; ++i)
        if (tabRect (i).contains (e.position) && (i != LockExtra || hasExtraKnob (voice()))) { proc.lockKnobSel = i; repaint(); return; }
    if (e.position.y < kBarTop - 8.0f || e.position.y > kBarBottom + 10.0f || e.position.x < kGridX - 6.0f || e.position.x > kGridR + 6.0f) return;

    if (e.mods.isPopupMenu() && e.mods.isShiftDown())   // Shift-right-click: clearing menu
    {
        const int v = voice(), prm = knob();
        juce::PopupMenu m;
        m.addItem (1, juce::String ("Clear the ") + kLockVoiceNames[v] + " " + juce::String (kLockKnobNames[v][prm]).toLowerCase() + " locks");
        m.addItem (2, juce::String ("Clear all ") + kLockVoiceNames[v] + " locks");
        m.addItem (3, "Clear every lock in this pattern");
        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                         [safe = juce::Component::SafePointer<LockGrid> (this), v, prm] (int id)
                         {
                             if (safe == nullptr || id == 0) return;
                             safe->proc.patterns.clearLocks (safe->proc.getCurrentSlot(), id == 3 ? -1 : v, id == 1 ? prm : -1);
                             safe->repaint();
                         });
        return;
    }
    dragging = true;
    erasing = e.mods.isPopupMenu();
    lastCol = -1;
    apply (columnAt (e.position.x), e.position.y);
}

void LockGrid::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging) apply (columnAt (e.position.x), e.position.y);
}

void LockGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.position.y < kBarTop - 8.0f || e.position.y > kBarBottom + 10.0f || e.position.x < kGridX - 6.0f) return;
    proc.patterns.setLock (proc.getCurrentSlot(), voice(), columnAt (e.position.x), knob(), 0);
    repaint();
}

// right-click on a drum pad: the step and its flam / drag / ruff
void StepGrid::showStepMenu (int row, int step)
{
    auto& pm = proc.patterns;
    const int slot = proc.getCurrentSlot();
    const bool on = pm.cell (slot, row, step), fl = on && pm.flam (slot, row, step);
    const auto fs = pm.flamStyle (slot, row, step);

    juce::PopupMenu m;
    m.addSectionHeader (juce::String (kVoiceTitles[row - 1]) + "  -  step " + juce::String (step + 1));
    m.addItem (1, on ? "Step off" : "Step on");
    m.addSeparator();
    m.addSectionHeader ("Flam");
    m.addItem (10, "No flam", true, ! fl);
    m.addItem (11, "Flam  (1 grace note)", true, fl && fs.graces == 1);
    m.addItem (12, "Drag  (2 grace notes)", true, fl && fs.graces == 2);
    m.addItem (13, "Ruff  (3 grace notes)", true, fl && fs.graces == 3);
    juce::PopupMenu sp, lv;
    // spacing 25 ms appears once, as the default
    sp.addItem (20, "25 ms  (default)", true, fl && (fs.spacing == 0 || fs.spacing == 2));
    for (int i : { 1, 3, 4 })
        sp.addItem (20 + i, juce::String (FlamStyle::kSpacingMs[i], 0) + " ms", true, fl && fs.spacing == i);
    lv.addItem (30, juce::String (FlamStyle::kLevelPct[0], 0) + " %  (default)", true, fl && fs.level == 0);
    for (int i = 1; i < FlamStyle::kNumLevels; ++i)
        lv.addItem (30 + i, juce::String (FlamStyle::kLevelPct[i], 0) + " %", true, fl && fs.level == i);
    m.addSubMenu ("Spacing  (before the step)", sp);
    m.addSubMenu ("Grace note level", lv);
    m.addSeparator();
    m.addItem (40, "Use this flam on every hit in this row", fl);
    m.addItem (41, "Remove all flams in this row", pm.getFlams (slot, row) != 0);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withTargetScreenArea (
                         localAreaToGlobal (padRect (row, step).toNearestInt())),
                     [safe = juce::Component::SafePointer<StepGrid> (this), row, step, slot] (int id)
                     {
                         if (safe == nullptr || id <= 0) return;
                         auto& p = safe->proc;
                         auto& mem = p.patterns;
                         auto style = mem.flamStyle (slot, row, step);
                         auto ensureFlam = [&]
                         {
                             if (! mem.cell (slot, row, step)) mem.setCell (slot, row, step, true);
                             if (! mem.flam (slot, row, step)) { mem.setFlam (slot, row, step, true); style = FlamStyle(); }
                         };
                         if (id == 1)  mem.setCell (slot, row, step, ! mem.cell (slot, row, step));
                         if (id == 10) mem.setFlam (slot, row, step, false);
                         if (id >= 11 && id <= 13) { ensureFlam(); style.graces = id - 10; mem.setFlamStyle (slot, row, step, style.encode()); }
                         if (id >= 20 && id < 20 + FlamStyle::kNumSpacings) { ensureFlam(); style.spacing = id - 20; mem.setFlamStyle (slot, row, step, style.encode()); }
                         if (id >= 30 && id < 30 + FlamStyle::kNumLevels) { ensureFlam(); style.level = id - 30; mem.setFlamStyle (slot, row, step, style.encode()); }
                         if (id == 40) p.flamWholeRow (row, mem.getFlamStyle (slot, row, step));
                         if (id == 41) p.clearRowFlams (row);
                         safe->repaint();
                     });
}

// ================================================================================================
void NameStrip::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    const float t = juce::jlimit (0.0f, 1.0f, (float) (juce::Time::getMillisecondCounter() - lastHit) / 180.0f);
    const auto flash = lastAccent ? juce::Colour (0xffffffff) : col::orange;
    const auto bg = lastHit != 0 && t < 1.0f ? flash.interpolatedWith (col::stripBg, t) : col::stripBg;
    g.setColour (bg);
    g.fillRoundedRectangle (r, 2.0f);
    g.setColour (bg.getPerceivedBrightness() > 0.5f ? col::stripBg : col::stripText);
    g.setFont (font (10.0f, true, 0.15f));
    g.drawText (title, getLocalBounds(), juce::Justification::centred);
}

void NameStrip::mouseDown (const juce::MouseEvent& e)
{
    lastAccent = e.mods.isShiftDown();
    if (e.mods.isAltDown())   // Option/Alt held: note repeat at the REPEAT rate until the mouse is released
    {
        repeating = true;
        lastRepeatHits = proc.getRepeatHits (voice);
        proc.startRepeat (voice, lastAccent);
    }
    else
        proc.panelHit (voice, lastAccent);
    lastHit = juce::Time::getMillisecondCounter();
    repaint();
}

void NameStrip::mouseUp (const juce::MouseEvent&)
{
    if (repeating) { repeating = false; proc.stopRepeat (voice); }
}

void NameStrip::pollRepeat()
{
    const int h = proc.getRepeatHits (voice);
    if (h != lastRepeatHits) { lastRepeatHits = h; lastHit = juce::Time::getMillisecondCounter(); repaint(); }
}

// ================================================================================================
void ExportKey::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f).withTrimmedBottom (2.0f).translated (0, pressed ? 1.5f : 0.0f);
    drawKey (g, r, isMouseOver() ? col::cream1.brighter (0.05f) : col::cream1, col::cream2, 4.0f, pressed ? 0.5f : 2.0f);
    g.setColour (juce::Colour (0xff222222));
    g.setFont (font (8.5f, true, 0.04f));
    g.drawText ("EXPORT MIDI", r, juce::Justification::centred);
}

int ExportKey::length() const { return juce::jlimit (1, kNumSteps, (int) proc.apvts.getRawParameterValue ("seqlen")->load()); }

std::vector<int> ExportKey::nonEmptySlots() const
{
    std::vector<int> slots;
    for (int s = 0; s < kNumSlots; ++s)
        if (! MidiExport::isEmpty (proc.patterns, s)) slots.push_back (s);
    return slots;
}

void ExportKey::mouseDrag (const juce::MouseEvent& e)
{
    if (dragged || e.getDistanceFromDragStart() < 6) return;
    dragged = true; pressed = false; repaint();
    // write the playing pattern to a temporary file and hand it to the DAW as a file drag
    const int slot = proc.getCurrentSlot();
    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("TRoss606");
    dir.createDirectory();
    const auto file = dir.getChildFile (MidiExport::fileName (slot));
    if (MidiExport::write (MidiExport::make (proc.patterns, { slot }, length(), proc.currentBpm.load(),
                                             "TRoss 606 Pattern " + juce::String (slot + 1)), file))
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this);
}

void ExportKey::mouseUp (const juce::MouseEvent& e)
{
    const bool wasClick = pressed && ! dragged && getLocalBounds().contains (e.getPosition());
    pressed = false; repaint();
    if (wasClick) showMenu();
}

void ExportKey::showMenu()
{
    const int slot = proc.getCurrentSlot();
    const bool playingEmpty = MidiExport::isEmpty (proc.patterns, slot);
    const auto filled = nonEmptySlots();
    juce::PopupMenu m;
    m.addSectionHeader ("Export as MIDI (" + juce::String (length()) + " steps, "
                        + juce::String (juce::roundToInt (proc.currentBpm.load())) + " BPM)");
    m.addItem (1, "Pattern " + juce::String (slot + 1) + " (playing)" + (playingEmpty ? " - empty" : ""), ! playingEmpty);
    m.addItem (2, "All patterns, in a row (" + juce::String ((int) filled.size()) + " with notes)", ! filled.empty());
    m.addItem (3, "All patterns as separate files", ! filled.empty());
    const auto chain = proc.getChain();
    m.addItem (5, "The chain, in a row (" + juce::String ((int) chain.size()) + " steps)", ! chain.empty());
    m.addSeparator();
    m.addItem (4, "Tip: drag this key onto a MIDI track to drop the playing pattern as a clip", false);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                     [safe = juce::Component::SafePointer<ExportKey> (this), chain] (int id)
                     {
                         if (safe == nullptr) return;
                         if (id == 5) safe->exportChain (chain);
                         if (id == 1) safe->exportPlaying();
                         if (id == 2) safe->exportAllInARow();
                         if (id == 3) safe->exportAllSeparate();
                     });
}

static juce::File exportStartFolder()
{
    auto docs = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
    return docs.isDirectory() ? docs : juce::File::getSpecialLocation (juce::File::userHomeDirectory);
}

void ExportKey::exportPlaying()
{
    const int slot = proc.getCurrentSlot();
    auto midi = MidiExport::make (proc.patterns, { slot }, length(), proc.currentBpm.load(),
                                  "TRoss 606 Pattern " + juce::String (slot + 1));
    chooser = std::make_unique<juce::FileChooser> ("Save pattern as MIDI", exportStartFolder().getChildFile (MidiExport::fileName (slot)), "*.mid");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                          | juce::FileBrowserComponent::warnAboutOverwriting,
                          [midi] (const juce::FileChooser& fc)
                          {
                              const auto f = fc.getResult();
                              if (f != juce::File()) MidiExport::write (midi, f.withFileExtension ("mid"));
                          });
}

void ExportKey::exportAllInARow()
{
    auto midi = MidiExport::make (proc.patterns, nonEmptySlots(), length(), proc.currentBpm.load(), "TRoss 606 Patterns");
    chooser = std::make_unique<juce::FileChooser> ("Save all patterns as one MIDI file", exportStartFolder().getChildFile ("TRoss606 Patterns.mid"), "*.mid");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                          | juce::FileBrowserComponent::warnAboutOverwriting,
                          [midi] (const juce::FileChooser& fc)
                          {
                              const auto f = fc.getResult();
                              if (f != juce::File()) MidiExport::write (midi, f.withFileExtension ("mid"));
                          });
}

void ExportKey::exportChain (const std::vector<int>& chain)
{
    auto midi = MidiExport::make (proc.patterns, chain, length(), proc.currentBpm.load(), "TRoss 606 Chain");
    chooser = std::make_unique<juce::FileChooser> ("Save the chain as one MIDI file", exportStartFolder().getChildFile ("TRoss606 Chain.mid"), "*.mid");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                          | juce::FileBrowserComponent::warnAboutOverwriting,
                          [midi] (const juce::FileChooser& fc)
                          {
                              const auto f = fc.getResult();
                              if (f != juce::File()) MidiExport::write (midi, f.withFileExtension ("mid"));
                          });
}

void ExportKey::exportAllSeparate()
{
    std::vector<std::pair<juce::String, juce::MidiFile>> files;
    for (int s : nonEmptySlots())
        files.push_back ({ MidiExport::fileName (s),
                           MidiExport::make (proc.patterns, { s }, length(), proc.currentBpm.load(), "TRoss 606 Pattern " + juce::String (s + 1)) });
    chooser = std::make_unique<juce::FileChooser> ("Choose a folder for the MIDI files", exportStartFolder());
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [files] (const juce::FileChooser& fc)
                          {
                              const auto dir = fc.getResult();
                              if (dir.isDirectory())
                                  for (auto& [name, midi] : files) MidiExport::write (midi, dir.getChildFile (name));
                          });
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
        const bool chained = proc.getChainLength() > 0 && proc.isInChain (i);
        if (chained && i != cur)   // in the chain: a dim LED; the one playing is bright
        {
            g.setColour (col::ledOn.withAlpha (0.45f));
            g.fillEllipse (juce::Rectangle<float> (5.0f, 5.0f).withCentre ({ r.getX() + 6.5f, r.getY() + 6.5f }));
        }
        else
            drawLed (g, { r.getX() + 6.5f, r.getY() + 6.5f }, 5.0f, i == cur || (i == pending && blink));
        g.setColour (juce::Colour (0xff222222));
        g.setFont (font (10.0f, true));
        g.drawText (juce::String (i + 1), r.withTrimmedLeft (6.0f), juce::Justification::centred);
        if (proc.patterns.isEmpty (i))   // small dash for empty slots
        {
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.fillRect (r.getRight() - 9.0f, r.getBottom() - 5.0f, 5.0f, 1.5f);
        }
    }
}

void PatternPad::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < kNumSlots; ++i)
        if (buttonRect (i).expanded (2.0f).contains (e.position))
        {
            if (e.mods.isPopupMenu())   // right-click: copy, clear, chain
            {
                juce::PopupMenu copyTo;
                for (int s = 0; s < kNumSlots; ++s) copyTo.addItem (100 + s, "Pattern " + juce::String (s + 1), s != i);
                juce::PopupMenu m;
                m.addSectionHeader ("Pattern " + juce::String (i + 1));
                m.addSubMenu ("Copy to", copyTo);
                m.addItem (1, "Clear", ! proc.patterns.isEmpty (i));
                m.addSeparator();
                m.addSectionHeader ("Chain  (Shift-click pattern buttons to build it)");
                m.addItem (2, "Add pattern " + juce::String (i + 1) + " to the chain", proc.getChainLength() < Tross606Processor::kMaxChain);
                m.addItem (3, "Remove the last chain step", proc.getChainLength() > 0);
                m.addItem (4, "Clear the chain", proc.getChainLength() > 0);
                m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                                 [safe = juce::Component::SafePointer<PatternPad> (this), i] (int id)
                                 {
                                     if (safe == nullptr) return;
                                     if (id == 1) safe->proc.patterns.clear (i);
                                     if (id == 2) safe->proc.chainAppend (i);
                                     if (id == 3) safe->proc.chainRemoveLast();
                                     if (id == 4) safe->proc.chainClear();
                                     if (id >= 100) safe->proc.patterns.copySlot (i, id - 100);
                                     safe->repaint();
                                 });
                return;
            }
            if (e.mods.isShiftDown()) { proc.chainAppend (i); repaint(); return; }   // build a chain
            proc.chainClear();   // a plain click goes back to a single pattern
            proc.selectSlot (i);
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
    const int chainN = proc.getChainLength();
    const juce::String txt = chainN > 0 ? "CHN " + juce::String (juce::jmax (0, proc.getChainPos()) + 1) + "/" + juce::String (chainN) + " P" + juce::String (cur + 1)
                           : pending >= 0 ? "PAT " + juce::String (cur + 1) + " > " + juce::String (pending + 1)
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
        auto strip = std::make_unique<NameStrip> (proc, v, kVoiceTitles[v]);
        strip->setBounds (x + 8, 172, kChanW - 16, 17);
        strip->setTooltip ("Click to play. Shift-click for an accented hit. Hold Option (Alt) and press to repeat at the REPEAT rate. Records into the pattern while REC is on and the song plays.");
        content.addAndMakeVisible (*strip);
        strips.push_back (std::move (strip));
        addKnob (id + "_tune",  { x + 30, kKnobRow1 }, kKnobD);
        addKnob (id + "_decay", { x + 82, kKnobRow1 }, kKnobD);
        addKnob (id + "_lvl",   { x + 30, kKnobRow2 }, kKnobD);
        if (kExtraIds[v] != nullptr) addKnob (kExtraIds[v], { x + 82, kKnobRow2 }, kKnobD);
        addButton (id + "_mute", "M", { x + 8,  196, 24, 15 }, "rubber").getProperties().set ("onColour", (juce::int64) col::muteOn.getARGB());
        addButton (id + "_solo", "S", { x + 88, 196, 24, 15 }, "rubber").getProperties().set ("onColour", (juce::int64) col::soloOn.getARGB());
        if (! proc.isStandalone())   // the standalone app has only the main output
        {
            auto& own = addButton (id + "_out", "OWN OUT", { x + 30, 217, 60, 15 }, "rubber");
            own.getProperties().set ("onColour", (juce::int64) col::orange.getARGB());
            own.setTooltip ("On: this drum leaves the main mix and plays on its own mono output");
        }
    }
    addKnob ("accent", { kMasterX + 30, 96 }, 40);
    addKnob ("master", { kMasterX + 88, 96 }, 40);
    content.addAndMakeVisible (lcd);
    lcd.setBounds (kMasterX + 7, 128, kMasterW - 14, 20);

    // sequencer
    content.addAndMakeVisible (grid);
    grid.setBounds (0, kSeqY, 918, kHeight - kSeqY);
    content.addChildComponent (lockGrid);
    lockGrid.setBounds (0, kSeqY, 918, kHeight - kSeqY);
    lockGrid.setVisible (proc.showLocks.load());
    grid.setVisible (! proc.showLocks.load());
    lockGrid.setTooltip ("Parameter locks. Pick a drum on the left and a knob below, then drag across the steps' bars to lock that value "
                         "for each step. Right-drag or double-click removes locks; Shift-right-click for more.");
    content.addAndMakeVisible (patternPad);
    patternPad.setBounds (918, kSeqY, 100, kHeight - kSeqY);

    auto len = std::make_unique<juce::Slider> (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    len->getProperties().set ("seg", true);
    len->setMouseDragSensitivity (120);
    len->setBounds (1034, 274, 48, 30);
    content.addAndMakeVisible (*len);
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "seqlen", *len));
    len->setDoubleClickReturnValue (true, 16.0);
    len->setTooltip ("Drag up/down to set the pattern length");
    sliders.push_back (std::move (len));

    addButton ("seqon", "RUN", { 1027, 314, 62, 24 }, "key").getProperties().set ("led", true);
    clearBtn.getProperties().set ("style", "key");
    clearBtn.setBounds (1027, 344, 62, 24);
    clearBtn.onClick = [this] { proc.clearCurrentPattern(); grid.repaint(); };
    content.addAndMakeVisible (clearBtn);
    addButton ("rec", "REC", { 1027, 374, 62, 24 }, "rec").getProperties().set ("led", true);
    // P-LOCK: switches the pad area between the notes and the parameter locks (under pattern keys 11 and 12)
    lockBtn.getProperties().set ("style", "key");
    lockBtn.getProperties().set ("led", true);
    lockBtn.setClickingTogglesState (true);
    lockBtn.setToggleState (proc.showLocks.load(), juce::dontSendNotification);
    lockBtn.setBounds (930, 438, 82, 24);
    lockBtn.setTooltip ("Switches the pads between notes and parameter locks: per-step Tune, Decay, Level or the drum's extra knob");
    lockBtn.onClick = [this]
    {
        proc.showLocks = lockBtn.getToggleState();
        lockGrid.setVisible (proc.showLocks.load());
        grid.setVisible (! proc.showLocks.load());
    };
    content.addAndMakeVisible (lockBtn);

    exportKey.setBounds (1025, 406, 66, 26);
    exportKey.setTooltip ("Click: export patterns as MIDI files. Drag onto a MIDI track: drop the playing pattern as a clip.");
    content.addAndMakeVisible (exportKey);

    // NOTE REPEAT rate (1/4, 1/8, 1/16): a knob and a read-out on the same parameter
    repeatKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    repeatKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    repeatKnob.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    repeatKnob.setMouseDragSensitivity (90);
    repeatKnob.setBounds (990, 198, 34, 34);
    repeatKnob.setTooltip ("Note repeat rate: hold Option (Alt) and press a drum name to repeat it");
    content.addAndMakeVisible (repeatKnob);
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "repeat_rate", repeatKnob));
    repeatBox.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    repeatBox.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    repeatBox.getProperties().set ("seg", true);
    repeatBox.getProperties().set ("segSize", 12.0f);
    repeatBox.setMouseDragSensitivity (90);
    repeatBox.setBounds (1034, 211, 58, 20);
    content.addAndMakeVisible (repeatBox);
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "repeat_rate", repeatBox));

    if (proc.isStandalone())   // no host transport: own play button + tempo
    {
        playBtn.getProperties().set ("style", "key");
        playBtn.setClickingTogglesState (true);
        playBtn.setToggleState (proc.internalPlay.load(), juce::dontSendNotification);
        playBtn.onClick = [this] { proc.internalPlay = playBtn.getToggleState(); };
        playBtn.setBounds (1025, 440, 34, 22);
        content.addAndMakeVisible (playBtn);

        bpmBox.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        bpmBox.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        bpmBox.getProperties().set ("seg", true);
        bpmBox.getProperties().set ("segSize", 10.5f);
        bpmBox.setRange (40.0, 240.0, 1.0);
        bpmBox.setValue (proc.internalBpm.load(), juce::dontSendNotification);
        bpmBox.setMouseDragSensitivity (300);
        bpmBox.textFromValueFunction = [] (double v) { return juce::String ((int) v); };
        bpmBox.onValueChange = [this] { proc.internalBpm = (float) bpmBox.getValue(); };
        bpmBox.setBounds (1062, 441, 30, 20);
        content.addAndMakeVisible (bpmBox);
    }

    // HELP: the hover tips on/off (remembered for every TRoss 606)
    helpBtn.getProperties().set ("style", "rubber");
    helpBtn.getProperties().set ("led", true);
    helpBtn.getProperties().set ("onColour", (juce::int64) juce::Colour (0xffdfe8d8).getARGB());
    helpBtn.setClickingTogglesState (true);
    helpBtn.setBounds (kWidth - kMargin - 112, 29, 58, 15);
    {
        juce::PropertiesFile settings (Tross606Processor::settingsOptions());
        helpBtn.setToggleState (settings.getBoolValue ("showHelp", true), juce::dontSendNotification);
    }
    helpBtn.onClick = [this]
    {
        juce::PropertiesFile settings (Tross606Processor::settingsOptions());
        settings.setValue ("showHelp", helpBtn.getToggleState());
        settings.saveIfNeeded();
        updateHelp();
    };
    content.addAndMakeVisible (helpBtn);
    updateHelp();

    logoBtn.setBounds (kMargin, 12, 300, 38);
    logoBtn.onClick = [this] { showAbout(); };
    content.addAndMakeVisible (logoBtn);
    content.addChildComponent (about);   // on top of everything, hidden until the logo is clicked
    about.setBounds (0, 0, kWidth, kHeight);

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
        g.setColour (col::softText);
        g.setFont (font (9.0f, false, 0.1f));
        g.drawText (kVoiceNotes[v], x + 32, 196, 56, 15, juce::Justification::centred);

        if (v < kNumVoices - 1)   // engraved divider
        {
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.fillRect (x + kChanW - 1, 60, 1, 172);
            g.setColour (juce::Colours::white.withAlpha (0.45f));
            g.fillRect (x + kChanW, 60, 1, 172);
        }
    }

    // master
    g.setColour (juce::Colours::black.withAlpha (0.05f));
    g.fillRoundedRectangle ((float) kMasterX, 58.0f, (float) kMasterW, 176.0f, 6.0f);
    knobLabel ("ACCENT", kMasterX + 30, 63);
    knobLabel ("VOLUME", kMasterX + 88, 63);
    nameStrip ("MASTER", { kMasterX + 7, 172, kMasterW - 14, 17 });
    g.setColour (col::printText);
    g.setFont (font (8.5f, false, 0.1f));
    g.drawText ("REPEAT", 1034, 199, 58, 11, juce::Justification::centred);

    // ---- dark sequencer section
    g.setColour (col::orange);
    g.fillRect (0, kTopH, kWidth, kSeqY - kTopH);
    g.setGradientFill (juce::ColourGradient::vertical (col::darkTop, (float) kSeqY, col::darkBottom, (float) kHeight));
    g.fillRect (0, kSeqY, kWidth, kHeight - kSeqY);

    g.setColour (col::darkLabel);
    g.setFont (font (8.5f, false, 0.15f));
    g.drawText ("LENGTH", 1018, kSeqY + 12, 80, 12, juce::Justification::centred);
}

void Tross606Editor::timerCallback()
{
    const int v = proc.patterns.getVersion(), st = proc.getPlayStep();
    const int slot = proc.getCurrentSlot(), pend = proc.getPendingSlot();
    const int len = (int) proc.apvts.getRawParameterValue ("seqlen")->load();
    const int sel = (int) proc.apvts.getRawParameterValue ("pattern")->load();

    if (v != lastVersion || st != lastStep || slot != lastSlot || len != lastLen)
    {
        grid.repaint();
        if (lockGrid.isVisible()) lockGrid.repaint();
    }
    if (v != lastVersion) patternPad.repaint();   // empty-slot dashes follow the pads
    if (slot != lastSlot || pend != lastPending || pend >= 0 || sel != lastSel)
    {
        patternPad.repaint();
        lcd.repaint();
    }
    for (auto& s : strips)
    {
        s->pollRepeat();
        if (s->lastHit != 0 && juce::Time::getMillisecondCounter() - s->lastHit < 260) s->repaint();
    }
    if (const int cv = proc.getChainVersion() * 64 + proc.getChainPos(); cv != lastChain) { patternPad.repaint(); lcd.repaint(); lastChain = cv; }
    lastVersion = v; lastStep = st; lastSlot = slot; lastPending = pend; lastLen = len; lastSel = sel;
}

void Tross606Editor::showAbout()
{
    about.toFront (false);
    about.setVisible (true);
    if (about.isShowing()) about.grabKeyboardFocus();
}

// ================================================================================================
AboutOverlay::AboutOverlay (Tross606Processor& p) : proc (p)
{
    setWantsKeyboardFocus (true);
    setInterceptsMouseClicks (true, true);
    manualBtn.getProperties().set ("style", "key");
    manualBtn.setTooltip ("Opens the TRoss 606 manual (PDF) that comes inside the plug-in");
    manualBtn.onClick = [] { if (auto f = writeManual(); f.existsAsFile()) f.startAsProcess(); };
    addAndMakeVisible (manualBtn);
    contactLink.setFont (font (11.0f, true), false, juce::Justification::centredLeft);
    contactLink.setColour (juce::HyperlinkButton::textColourId, col::ink);
    contactLink.setTooltip (juce::String ("Write to ") + kContact);
    addAndMakeVisible (contactLink);
}

juce::File AboutOverlay::writeManual()
{
    // one file per version, in the temporary folder; written again only when it's missing or different
    auto f = juce::File::getSpecialLocation (juce::File::tempDirectory)
                 .getChildFile ("TRoss 606 Manual " JucePlugin_VersionString ".pdf");
    const auto size = (juce::int64) BinaryData::TRoss_606_Manual_pdfSize;
    if (! f.existsAsFile() || f.getSize() != size)
        if (! f.replaceWithData (BinaryData::TRoss_606_Manual_pdf, (size_t) size))
            return {};
    return f;
}

void AboutOverlay::resized()
{
    const auto c = card();
    manualBtn.setBounds (juce::Rectangle<float> (c.getRight() - 26.0f - 190.0f, c.getY() + 300.0f, 190.0f, 28.0f).toNearestInt());
    contactLink.setBounds (juce::Rectangle<float> (c.getRight() - 26.0f - 190.0f, c.getY() + 84.0f, 190.0f, 18.0f).toNearestInt());
}

juce::Rectangle<float> AboutOverlay::card() const
{
    return juce::Rectangle<float> (760.0f, 400.0f).withCentre (getLocalBounds().toFloat().getCentre());
}

juce::String AboutOverlay::formatAndHost (const Tross606Processor& p)
{
    juce::String format = juce::AudioProcessor::getWrapperTypeDescription (p.wrapperType);
    if (p.wrapperType == juce::AudioProcessor::wrapperType_Undefined) format = "Test build";
    juce::String host = juce::PluginHostType().getHostDescription();
    if (host.isEmpty() || host == "Unknown") return format;
    return format + " in " + host;
}

void AboutOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.55f));
    const auto c = card();

    // card: silver header, dark body (like the panel)
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (c.translated (0, 4.0f), 8.0f);
    const float headerH = 108.0f;
    {
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path clip; clip.addRoundedRectangle (c, 8.0f);
        g.reduceClipRegion (clip);
        g.setGradientFill (juce::ColourGradient::vertical (col::darkTop, c.getY(), col::darkBottom, c.getBottom()));
        g.fillRect (c);
        auto head = c.withHeight (headerH);
        g.setGradientFill (juce::ColourGradient::vertical (col::silverTop, head.getY(), col::silverBottom, head.getBottom()));
        g.fillRect (head);
        for (float x = head.getX(); x < head.getRight(); x += 3.0f)
        {
            g.setColour (juce::Colours::white.withAlpha (0.07f)); g.fillRect (x, head.getY(), 1.0f, headerH);
        }
        g.setColour (col::orange);
        g.fillRect (head.getX(), head.getBottom(), head.getWidth(), 3.0f);
    }
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (c, 8.0f, 1.0f);

    // ---- About
    const float x0 = c.getX() + 26.0f;
    auto logo = font (34.0f, true, -0.03f);
    g.setFont (logo);
    const float troW = juce::GlyphArrangement::getStringWidth (logo, "TRoss ");
    g.setColour (col::ink);
    g.drawText ("TRoss ", juce::Rectangle<float> (x0, c.getY() + 14.0f, troW + 4.0f, 40.0f), juce::Justification::bottomLeft);
    g.setColour (col::orange);
    g.drawText ("606", juce::Rectangle<float> (x0 + troW, c.getY() + 14.0f, 120.0f, 40.0f), juce::Justification::bottomLeft);
    g.setColour (juce::Colour (0xff3a3a3a));
    g.setFont (font (10.5f, false, 0.3f));
    g.drawText ("ANALOG-STYLE DRUM SYNTHESIZER", juce::Rectangle<float> (x0, c.getY() + 58.0f, 400.0f, 14.0f), juce::Justification::centredLeft);
    g.setFont (font (11.0f, true, 0.04f));
    g.setColour (col::printText);
    g.drawText (juce::String (juce::CharPointer_UTF8 ("Made by Ticco Ross  \xc2\xb7  \xc2\xa9 2026")), juce::Rectangle<float> (x0, c.getY() + 78.0f, 400.0f, 16.0f), juce::Justification::centredLeft);

    const float rx = c.getRight() - 26.0f - 280.0f;
    auto infoLine = [&] (const juce::String& label, const juce::String& value, float y)
    {
        g.setFont (font (8.5f, false, 0.15f));
        g.setColour (col::softText);
        g.drawText (label, juce::Rectangle<float> (rx, y, 90.0f, 14.0f), juce::Justification::centredLeft);
        g.setFont (font (11.0f, true));
        g.setColour (col::ink);
        g.drawText (value, juce::Rectangle<float> (rx + 90.0f, y, 190.0f, 14.0f), juce::Justification::centredLeft, true);
    };
    infoLine ("VERSION", "v" JucePlugin_VersionString, c.getY() + 26.0f);
    infoLine ("BUILT", T606_BUILD_DATE, c.getY() + 46.0f);
    infoLine ("RUNNING AS", formatAndHost (proc), c.getY() + 66.0f);
    infoLine ("CONTACT", {}, c.getY() + 86.0f);   // the address is the link on top

    // ---- Quick reference
    const float by = c.getY() + headerH + 18.0f;
    auto heading = [&] (const juce::String& t, float x, float y)
    {
        g.setFont (font (9.0f, true, 0.25f));
        g.setColour (col::accText);
        g.drawText (t, juce::Rectangle<float> (x, y, 300.0f, 14.0f), juce::Justification::centredLeft);
    };
    heading ("QUICK REFERENCE", x0, by);
    struct Row { const char* where; const char* what; };   // UTF-8 text
    static const Row left[] = {
        { "Click pad", "step on / off  \xc2\xb7  drag to paint" },
        { "Right-click pad", "flam / drag / ruff, spacing, level" },
        { "Click name", "play the drum (records with REC)" },
        { "Shift-click name", "play it accented" },
        { "Option / Alt + name", "note repeat at the REPEAT rate" },
        { "P-LOCK", "per-step Tune / Decay / Level / extra" },
    };
    static const Row right[] = {
        { "Click pattern", "play / edit it (queued while playing)" },
        { "Shift-click pattern", "add it to the chain" },
        { "Right-click pattern", "copy, paste, clear, chain options" },
        { "EXPORT MIDI", "click: menu  \xc2\xb7  drag: drop on a track" },
        { "LENGTH", "drag up / down: 1 to 16 steps" },
        { "HELP", "hover tips on / off" },
    };
    auto rows = [&] (const Row* r, int n, float x)
    {
        for (int i = 0; i < n; ++i)
        {
            const float y = by + 22.0f + (float) i * 20.0f;
            g.setFont (font (10.0f, true, 0.02f));
            g.setColour (col::rowText);
            g.drawText (r[i].where, juce::Rectangle<float> (x, y, 136.0f, 16.0f), juce::Justification::centredLeft);
            g.setFont (font (10.0f));
            g.setColour (col::darkLabel.brighter (0.25f));
            g.drawText (juce::String (juce::CharPointer_UTF8 (r[i].what)), juce::Rectangle<float> (x + 138.0f, y, 210.0f, 16.0f), juce::Justification::centredLeft, true);
        }
    };
    rows (left, 6, x0);
    rows (right, 6, c.getCentreX() + 10.0f);

    // ---- Credits
    const float cy = c.getBottom() - 58.0f;
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.fillRect (x0, cy - 8.0f, c.getWidth() - 52.0f, 1.0f);
    heading ("CREDITS", x0, cy);
    g.setFont (font (9.5f));
    g.setColour (col::darkLabel);
    g.drawText ("Made with JUCE.  VST is a registered trademark of Steinberg Media Technologies GmbH.  "
                "Audio Units is a trademark of Apple Inc.",
                juce::Rectangle<float> (x0, cy + 16.0f, c.getWidth() - 52.0f, 14.0f), juce::Justification::centredLeft, true);
    g.setFont (font (8.5f, false, 0.15f));
    g.setColour (col::darkLabel.withAlpha (0.7f));
    g.drawText ("CLICK ANYWHERE TO CLOSE", juce::Rectangle<float> (x0, cy + 32.0f, c.getWidth() - 52.0f, 12.0f), juce::Justification::centredRight);
}

void Tross606Editor::updateHelp()
{
    if (helpBtn.getToggleState())
    {
        if (tooltips == nullptr) tooltips = std::make_unique<juce::TooltipWindow> (this, 600);
        helpBtn.setTooltip ("Hover tips on. Click to switch them off");
    }
    else
    {
        tooltips.reset();   // no tooltip window, no tips (knob value read-outs still show)
        helpBtn.setTooltip ({});
    }
}

void Tross606Editor::toggleSize()
{
    const bool big = getWidth() > kWidth * 9 / 8;
    const double scale = big ? 1.0 : 1.25;
    setSize (juce::roundToInt (kWidth * scale), juce::roundToInt (kHeight * scale));
}
