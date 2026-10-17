#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <array>

using namespace tross606;

namespace
{
using APF = juce::AudioParameterFloat;
using APB = juce::AudioParameterBool;

const char* kIds[kNumVoices]   = { "bd", "sd", "lt", "ht", "cp", "ch", "oh", "cy" };
const char* kNames[kNumVoices] = { "BD", "SD", "LT", "HT", "CP", "CH", "OH", "CY" };

std::unique_ptr<APF> param (juce::String id, juce::String name, float lo, float hi, float def,
                            std::function<juce::String (float, int)> fmt)
{
    return std::make_unique<APF> (juce::ParameterID { id, 1 }, name, juce::NormalisableRange<float> (lo, hi), def,
                                  juce::AudioParameterFloatAttributes().withStringFromValueFunction (std::move (fmt)));
}
auto semis   = [] (float v, int) { return juce::String (v, 1) + " st"; };
auto percent = [] (float v, int) { return juce::String (v, std::abs (v - std::round (v)) < 0.05f ? 0 : 1) + " %"; };
auto decibel = [] (float v, int) { return v <= -69.9f ? juce::String ("-inf dB") : juce::String (v, 1) + " dB"; };

std::unique_ptr<APF> level (juce::String id, juce::String name, float def) { return param (id, name, -70.0f, 6.0f, def, decibel); }
std::unique_ptr<APF> pct (juce::String id, juce::String name, float def)   { return param (id, name, 0.0f, 100.0f, def, percent); }
std::unique_ptr<APB> onOff (juce::String id, juce::String name, bool def)  { return std::make_unique<APB> (juce::ParameterID { id, 1 }, name, def); }

int wrapStep (juce::int64 x, int len) { return (int) (((x % len) + len) % len); }
} // namespace

// main stereo out + one mono out per drum (the standalone app keeps only the main out)
juce::AudioProcessor::BusesProperties Tross606Processor::makeBuses()
{
    auto buses = BusesProperties().withOutput ("Main", juce::AudioChannelSet::stereo(), true);
    if (juce::PluginHostType::getPluginLoadedAs() != juce::AudioProcessor::wrapperType_Standalone)
        for (auto* name : kNames)
            buses = buses.withOutput (name, juce::AudioChannelSet::mono(), true);
    return buses;
}

juce::AudioProcessorValueTreeState::ParameterLayout Tross606Processor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    const SoundParams d;
    for (int v = 0; v < kNumVoices; ++v)
    {
        const juce::String id = kIds[v], nm = kNames[v];
        l.add (param (id + "_tune", nm + " Tune", -12.0f, 12.0f, d.tune[v], semis));
        l.add (pct (id + "_decay", nm + " Decay", d.decay[v]));
        l.add (level (id + "_lvl", nm + " Level", d.lvl[v]));
        if (v == BD) l.add (pct ("bd_attack", "BD Attack", d.bdAttack));
        if (v == SD) l.add (pct ("sd_snappy", "SD Snappy", d.sdSnappy));
        if (v == CP) l.add (pct ("cp_noise", "CP Noise", d.cpNoise));
        if (v == CY) l.add (pct ("cy_tone", "CY Tone", d.cyTone));
        l.add (onOff (id + "_mute", nm + " Mute", false));
        l.add (onOff (id + "_solo", nm + " Solo", false));
        l.add (onOff (id + "_out", nm + " Own Out", false));
    }
    l.add (pct ("accent", "Accent", d.accent));
    l.add (level ("master", "Volume", d.master));
    l.add (onOff ("seqon", "Seq On", true));
    l.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "seqlen", 1 }, "Seq Length", 1, 16, 16));
    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "pattern", 1 }, "Pattern",
                                                         juce::StringArray { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" }, 0));
    l.add (onOff ("rec", "Record", true));
    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "repeat_rate", 1 }, "Repeat Rate",
                                                         juce::StringArray { "1/4", "1/8", "1/16" }, 2));
    return l;
}

Tross606Processor::Tross606Processor()
    : AudioProcessor (makeBuses()),
      apvts (*this, nullptr, "TROSS606", createLayout())
{
    auto p = [this] (const juce::String& id) { return apvts.getRawParameterValue (id); };
    for (int v = 0; v < kNumVoices; ++v)
    {
        const juce::String id = kIds[v];
        raw.lvl[v] = p (id + "_lvl");   raw.tune[v] = p (id + "_tune"); raw.decay[v] = p (id + "_decay");
        raw.mute[v] = p (id + "_mute"); raw.solo[v] = p (id + "_solo"); raw.ownOut[v] = p (id + "_out");
    }
    raw.bdAttack = p ("bd_attack"); raw.sdSnappy = p ("sd_snappy"); raw.cpNoise = p ("cp_noise"); raw.cyTone = p ("cy_tone");
    raw.accent = p ("accent"); raw.master = p ("master");
    raw.repeatRate = p ("repeat_rate");
    raw.seqOn = p ("seqon"); raw.seqLen = p ("seqlen"); raw.pattern = p ("pattern"); raw.rec = p ("rec");
}

bool Tross606Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono()) return false;
    for (int b = 1; b < layouts.outputBuses.size(); ++b)   // each drum output: mono, or switched off by the host
    {
        const auto& set = layouts.outputBuses.getReference (b);
        if (! set.isDisabled() && set != juce::AudioChannelSet::mono()) return false;
    }
    return layouts.inputBuses.isEmpty();
}

void Tross606Processor::prepareToPlay (double sr, int)
{
    sampleRate = sr;
    engine.prepare (sr);
    wasPlaying = false;
    playStep = -1;
    lastPatternParam = (int) raw.pattern->load();
}

void Tross606Processor::pullParams()
{
    for (int v = 0; v < kNumVoices; ++v)
    {
        sound.lvl[v] = raw.lvl[v]->load();   sound.tune[v] = raw.tune[v]->load(); sound.decay[v] = raw.decay[v]->load();
        sound.mute[v] = raw.mute[v]->load() > 0.5f; sound.solo[v] = raw.solo[v]->load() > 0.5f;
        sound.ownOut[v] = raw.ownOut[v]->load() > 0.5f;
    }
    sound.bdAttack = raw.bdAttack->load(); sound.sdSnappy = raw.sdSnappy->load();
    sound.cpNoise = raw.cpNoise->load();   sound.cyTone = raw.cyTone->load();
    sound.accent = raw.accent->load();     sound.master = raw.master->load();
    engine.setParams (sound);
}

// the sequencer reached `step`: a queued pattern takes over on step 1, then the step plays
// `step` starts lookSamples from now: schedule the grace notes of its flams (1 = flam, 2 = drag, 3 = ruff),
// spread evenly before the step, all of them before the main hit
void Tross606Processor::doGrace (int step, long long absStep, int len, juce::int64 lookSamples)
{
    int slot = currentSlot.load();
    if (const int chained = chainSlotFor (absStep, len); chained >= 0) slot = chained;
    else if (step == 0 && pendingSlot.load() >= 0) slot = pendingSlot.load();
    const bool accent = patterns.cell (slot, 0, step);
    const int vel = accent ? 127 : 100;
    for (int v = 0; v < kNumVoices; ++v)
    {
        if (! patterns.cell (slot, v + 1, step) || ! patterns.flam (slot, v + 1, step)) continue;
        const auto fs = patterns.flamStyle (slot, v + 1, step);
        const auto spacing = juce::jlimit ((juce::int64) 2, juce::jmax ((juce::int64) 2, lookSamples),
                                           (juce::int64) std::round (fs.spacingMs() / 1000.0 * sampleRate));
        const int graceVel = juce::jlimit (1, 100, juce::roundToInt (vel * fs.levelPct() / 100.0f));
        const auto gap = juce::jmax ((juce::int64) 1, spacing / fs.graces);
        for (int k = 0; k < fs.graces && numSched < kMaxSched; ++k)
        {
            const auto before = spacing - k * gap;   // first grace furthest from the step
            sched[numSched++] = { v, graceVel, sampleClock + (lookSamples - before) };
        }
    }
}

// ---- pattern chain
int Tross606Processor::chainSlotFor (long long absStep, int len) const
{
    const int n = chainLen.load();
    if (n <= 0) return -1;
    const long long bar = absStep >= 0 ? absStep / len : -((-absStep + len - 1) / len);
    return chain[(int) (((bar % n) + n) % n)].load();
}

void Tross606Processor::setChain (const std::vector<int>& c)
{
    const int n = juce::jmin ((int) c.size(), kMaxChain);
    chainLen = 0;                                   // the audio thread sees no chain while it's rewritten
    for (int i = 0; i < n; ++i) chain[i] = juce::jlimit (0, kNumSlots - 1, c[(size_t) i]);
    chainLen = n;
    if (n == 0) chainPos = -1;
    else if (playStep.load() < 0) { currentSlot = chain[0].load(); chainPos = 0; }   // stopped: show the first one
    ++chainVersion;
}

void Tross606Processor::chainAppend (int slot)
{
    if (chainLen.load() >= kMaxChain) return;
    auto c = getChain(); c.push_back (slot);
    setChain (c);
}

void Tross606Processor::chainClear()      { if (chainLen.load() > 0) setChain ({}); }
void Tross606Processor::chainRemoveLast() { if (chainLen.load() > 0) { auto c = getChain(); c.pop_back(); setChain (c); } }

void Tross606Processor::selectSlot (int slot)
{
    slot = juce::jlimit (0, kNumSlots - 1, slot);
    if (playStep.load() >= 0) pendingSlot = (slot == currentSlot.load()) ? -1 : slot;   // playing: wait for step 1
    else                      { pendingSlot = -1; currentSlot = slot; }
}

void Tross606Processor::doStep (int step, long long absStep, int len, bool seqOn)
{
    const int chained = chainSlotFor (absStep, len);
    if (chained >= 0)   // a chain decides which pattern plays, from the song position
    {
        currentSlot = chained;
        pendingSlot = -1;
        const int n = chainLen.load();
        const long long bar = absStep >= 0 ? absStep / len : -((-absStep + len - 1) / len);
        chainPos = (int) (((bar % n) + n) % n);
    }
    else if (step == 0 && pendingSlot.load() >= 0)
    {
        currentSlot = pendingSlot.load();
        pendingSlot = -1;
    }
    const int slot = currentSlot.load();
    uint16_t rows[kNumRows];
    for (int r = 0; r < kNumRows; ++r) rows[r] = patterns.get (slot, r);
    engine.sequencerStep (step, rows, seqOn);
    playStep = step;
}

// REC while playing: the hit goes into the nearest step; velocity above 100 also sets the accent
void Tross606Processor::recordHit (int voice, int velocity, double sixteenthPos, int len)
{
    const int step = wrapStep ((juce::int64) std::floor (sixteenthPos + 0.5), len);
    const int slot = currentSlot.load();
    patterns.setCell (slot, voice + 1, step, true);
    if (velocity > 100) patterns.setCell (slot, 0, step, true);
}

void Tross606Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    pullParams();

    const bool seqOn = raw.seqOn->load() > 0.5f;
    const bool recOn = raw.rec->load() > 0.5f;
    const int  len   = juce::jlimit (1, kNumSteps, (int) raw.seqLen->load());

    // ---- transport
    bool playing = false;
    double ppq = 0.0, bpm = 120.0;
    bool hostTransport = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            if (auto q = pos->getPpqPosition())
            {
                hostTransport = true;
                playing = pos->getIsPlaying();
                ppq = *q;
            }
        }
    if (! hostTransport && isStandalone())
    {
        bpm = internalBpm.load();
        playing = internalPlay.load();
        if (! playing) internalPpq = 0.0;
        ppq = internalPpq;
    }
    bpm = juce::jlimit (20.0, 999.0, bpm);
    currentBpm.store (bpm);
    const double sixteenthsPerSample = bpm / 60.0 * 4.0 / sampleRate;
    if (isStandalone() && ! hostTransport && playing) internalPpq += numSamples * sixteenthsPerSample / 4.0;

    // ---- pattern selector: while playing, a new slot waits until the pattern is back at step 1
    const int sel = (int) raw.pattern->load();
    if (sel != lastPatternParam)
    {
        lastPatternParam = sel;
        if (chainLen.load() > 0) {}   // a chain is running: it picks the patterns (a plain click ends it)
        else if (playing) pendingSlot = (sel == currentSlot.load()) ? -1 : sel;
        else         { pendingSlot = -1; currentSlot = sel; }
    }
    if (! playing)
    {
        if (pendingSlot.load() >= 0) { currentSlot = pendingSlot.load(); pendingSlot = -1; }
        if (const int chained = chainSlotFor ((long long) std::floor (ppq * 4.0 + 1e-9), len); chained >= 0)
        {
            currentSlot = chained;   // stopped: show the chain step at the song position
            const int n = chainLen.load();
            const long long b = (long long) std::floor (ppq * 4.0 / len + 1e-9);
            chainPos = (int) (((b % n) + n) % n);
        }
        wasPlaying = false;
        playStep = -1;
        numSched = 0;   // grace notes waiting for a step that won't come
    }

    // flams: grace notes are looked for this far ahead (the longest spacing, never more than half a step)
    const double graceLook16 = juce::jmin (0.45, FlamStyle::kMaxSpacingMs / 1000.0 * sampleRate * sixteenthsPerSample);
    const auto graceLookSamples = (juce::int64) std::floor (graceLook16 / sixteenthsPerSample);

    buffer.clear();
    const int numCh = buffer.getNumChannels();
    auto* left  = buffer.getWritePointer (0);
    auto* right = getBus (false, 0) != nullptr && getBus (false, 0)->getNumberOfChannels() > 1 && numCh > 1
                    ? buffer.getWritePointer (1) : nullptr;
    // the drum outputs that the host has switched on (nullptr = not there)
    float* drumOut[kNumVoices] {};
    for (int v = 0; v < kNumVoices; ++v)
        if (auto* bus = getBus (false, v + 1))
            if (bus->isEnabled() && bus->getNumberOfChannels() > 0)
            {
                const int ch = getChannelIndexInProcessBlockBuffer (false, v + 1, 0);
                if (ch >= 0 && ch < numCh) drumOut[v] = buffer.getWritePointer (ch);
            }
    for (int v = 0; v < kNumVoices; ++v) engine.setOwnOut (v, sound.ownOut[v] && drumOut[v] != nullptr);
    float drumSig[kNumVoices];
    auto midiIt = midi.cbegin();
    const double q0 = ppq * 4.0;   // position in 16ths at the block start

    // clicks on the drum name strips since the last block: play at the block start, record if REC is on
    for (int r = hitRead.load (std::memory_order_relaxed); r != hitWrite.load (std::memory_order_acquire); r = (r + 1) % kHitQueue)
    {
        const auto hit = hitQueue[r];
        engine.trigger (hit.voice, hit.velocity);
        if (playing && recOn) recordHit (hit.voice, hit.velocity, q0, len);
        hitRead.store ((r + 1) % kHitQueue, std::memory_order_release);
    }

    // ---- note repeat (Option/Alt held on a name strip): a first hit now, then one on every
    // 1/4, 1/8 or 1/16 of the song grid while playing, or at that rate from the press while stopped
    const int rv = repeatVoice.load();
    const double rate16 = std::array<double, 3> { 4.0, 2.0, 1.0 } [(size_t) juce::jlimit (0, 2, (int) raw.repeatRate->load())];
    const int repVel = repeatAccent.load() ? 127 : 100;
    const double samplesPerRate = rate16 / sixteenthsPerSample;
    auto repeatFire = [&] (double q)
    {
        engine.trigger (rv, repVel);
        if (playing && recOn) recordHit (rv, repVel, q, len);
        repeatHits[rv].fetch_add (1);
    };
    if (rv >= 0 && repeatPress.load() != lastRepeatPress)
    {
        lastRepeatPress = repeatPress.load();
        repeatFire (q0);
        repeatLastTick = (juce::int64) std::floor (q0 / rate16 + 1e-9);
        repeatCountdown = samplesPerRate;
    }

    for (int i = 0; i < numSamples; ++i)
    {
        const double q = q0 + i * sixteenthsPerSample;
        if (playing)
        {
            const auto six = (juce::int64) std::floor (q + 1e-9);
            if (! wasPlaying)
            {
                // starting exactly on a 16th plays it; starting in between waits for the next one
                const double frac = q - std::floor (q);
                lastSixteenth = (frac < sixteenthsPerSample) ? six - 1 : six;
                lastGraceSixteenth = (juce::int64) std::floor (q + graceLook16 + 1e-9);   // no graces for steps already under way
                wasPlaying = true;
            }
            // a flammed step coming up within graceLook16: schedule its grace notes before it
            const auto graceSix = (juce::int64) std::floor (q + graceLook16 + 1e-9);
            if (graceSix != lastGraceSixteenth)
            {
                lastGraceSixteenth = graceSix;
                if (seqOn) doGrace (wrapStep (graceSix, len), graceSix, len, graceLookSamples);
            }
            if (six != lastSixteenth)
            {
                lastSixteenth = six;
                doStep (wrapStep (six, len), six, len, seqOn);
            }
        }

        // grace notes that are due now
        for (int k = 0; k < numSched; )
        {
            if (sched[k].at <= sampleClock)
            {
                engine.trigger (sched[k].voice, sched[k].velocity);
                graceCount.fetch_add (1);
                sched[k] = sched[--numSched];
            }
            else ++k;
        }
        ++sampleClock;

        for (; midiIt != midi.cend() && (*midiIt).samplePosition <= i; ++midiIt)
        {
            const auto m = (*midiIt).getMessage();
            if (! m.isNoteOn()) continue;
            const int voice = noteToVoice (m.getNoteNumber());
            if (voice < 0) continue;
            engine.trigger (voice, m.getVelocity());
            if (playing && recOn) recordHit (voice, m.getVelocity(), q, len);
        }

        if (rv >= 0)
        {
            if (playing)
            {
                const double qi = q0 + i * sixteenthsPerSample;
                const auto tick = (juce::int64) std::floor (qi / rate16 + 1e-9);
                if (tick != repeatLastTick) { repeatLastTick = tick; repeatFire (qi); }
            }
            else if ((repeatCountdown -= 1.0) <= 0.0)
            {
                repeatCountdown += samplesPerRate;
                repeatFire (q0);
            }
        }

        const float s = engine.tick (drumSig);
        left[i] = s;
        if (right) right[i] = s;
        for (int v = 0; v < kNumVoices; ++v)
            if (drumOut[v] != nullptr) drumOut[v][i] = drumSig[v];
    }
}

// ---- state: parameters + the 12 patterns + which slot is playing + the chain
void Tross606Processor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto pats = state.getOrCreateChildWithName ("PATTERNS", nullptr);
    pats.setProperty ("current", currentSlot.load(), nullptr);
    {
        juce::StringArray ch;
        for (int s : getChain()) ch.add (juce::String (s));
        pats.setProperty ("chain", ch.joinIntoString (" "), nullptr);
    }
    for (int s = 0; s < kNumSlots; ++s)
    {
        juce::StringArray rows;
        for (int r = 0; r < kNumRows; ++r) rows.add (juce::String::toHexString ((int) patterns.get (s, r)));
        pats.setProperty ("slot" + juce::String (s), rows.joinIntoString (" "), nullptr);
        juce::StringArray fl;
        juce::String st;
        bool any = false;
        for (int r = 0; r < kNumRows; ++r)
        {
            fl.add (juce::String::toHexString ((int) patterns.getFlams (s, r)));
            any |= patterns.getFlams (s, r) != 0;
            for (int k = 0; k < kNumSteps; ++k) st << juce::String::toHexString ((int) patterns.getFlamStyle (s, r, k)).paddedLeft ('0', 2);
        }
        if (any)
        {
            pats.setProperty ("flams" + juce::String (s), fl.joinIntoString (" "), nullptr);
            pats.setProperty ("styles" + juce::String (s), st, nullptr);
        }
    }
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void Tross606Processor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return;
    auto state = juce::ValueTree::fromXml (*xml);
    auto pats = state.getChildWithName ("PATTERNS");
    if (pats.isValid())
    {
        for (int s = 0; s < kNumSlots; ++s)
        {
            juce::StringArray rows;
            rows.addTokens (pats.getProperty ("slot" + juce::String (s)).toString(), " ", "");
            for (int r = 0; r < kNumRows; ++r)
                patterns.set (s, r, (uint16_t) (r < rows.size() ? rows[r].getHexValue32() & 0xffff : 0));
            juce::StringArray fl;
            fl.addTokens (pats.getProperty ("flams" + juce::String (s)).toString(), " ", "");
            const auto st = pats.getProperty ("styles" + juce::String (s)).toString();
            for (int r = 0; r < kNumRows; ++r)
            {
                patterns.setFlams (s, r, (uint16_t) (r < fl.size() ? fl[r].getHexValue32() & 0xffff : 0));
                for (int k = 0; k < kNumSteps; ++k)
                {
                    const int at = (r * kNumSteps + k) * 2;
                    patterns.setFlamStyle (s, r, k, (uint8_t) (at + 2 <= st.length() ? st.substring (at, at + 2).getHexValue32() & 0xff : 0));
                }
            }
        }
        state.removeChild (pats, nullptr);
    }
    apvts.replaceState (state);
    const int sel = (int) raw.pattern->load();
    currentSlot = pats.isValid() ? juce::jlimit (0, kNumSlots - 1, (int) pats.getProperty ("current", sel)) : sel;
    pendingSlot = -1;
    lastPatternParam = sel;
    {
        juce::StringArray ch;
        ch.addTokens (pats.isValid() ? pats.getProperty ("chain").toString() : juce::String(), " ", "");
        std::vector<int> c;
        for (auto& t : ch) if (t.isNotEmpty()) c.push_back (t.getIntValue());
        setChain (c);
    }
}

juce::AudioProcessorEditor* Tross606Processor::createEditor() { return new Tross606Editor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Tross606Processor(); }

juce::PropertiesFile::Options Tross606Processor::settingsOptions()
{
    juce::PropertiesFile::Options o;
    o.applicationName = "TRoss 606";
    o.filenameSuffix = ".settings";
    o.folderName = "Ticco Ross";
    o.osxLibrarySubFolder = "Application Support";
    o.commonToAllUsers = false;
    return o;
}
