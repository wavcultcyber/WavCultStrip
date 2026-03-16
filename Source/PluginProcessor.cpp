#include "PluginProcessor.h"
#include "PluginEditor.h"

WavCultStripProcessor::WavCultStripProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

WavCultStripProcessor::~WavCultStripProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout WavCultStripProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === NOISE GATE ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("gateOn", 1), "Gate On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateThresh", 1), "Gate Threshold",
        juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f), -40.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateAttack", 1), "Gate Attack",
        juce::NormalisableRange<float>(0.01f, 50.0f, 0.01f, 0.5f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateRelease", 1), "Gate Release",
        juce::NormalisableRange<float>(1.0f, 500.0f, 1.0f, 0.5f), 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateHold", 1), "Gate Hold",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f, 0.5f), 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateRange", 1), "Gate Range",
        juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f), -80.0f));

    // === INPUT / OUTPUT ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("inputGain", 1), "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("outputGain", 1), "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    // === EQ ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("eqOn", 1), "EQ On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowGain", 1), "EQ Low Gain",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowFreq", 1), "EQ Low Freq",
        juce::NormalisableRange<float>(60.0f, 300.0f, 1.0f, 0.5f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLMGain", 1), "EQ Low-Mid Gain",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLMFreq", 1), "EQ Low-Mid Freq",
        juce::NormalisableRange<float>(200.0f, 2000.0f, 1.0f, 0.5f), 600.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLMQ", 1), "EQ Low-Mid Q",
        juce::NormalisableRange<float>(0.5f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHMGain", 1), "EQ Hi-Mid Gain",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHMFreq", 1), "EQ Hi-Mid Freq",
        juce::NormalisableRange<float>(600.0f, 8000.0f, 1.0f, 0.5f), 3000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHMQ", 1), "EQ Hi-Mid Q",
        juce::NormalisableRange<float>(0.5f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighGain", 1), "EQ High Gain",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighFreq", 1), "EQ High Freq",
        juce::NormalisableRange<float>(1500.0f, 16000.0f, 1.0f, 0.5f), 8000.0f));

    // === COMPRESSOR ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compOn", 1), "Comp On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compThresh", 1), "Comp Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), -18.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRatio", 1), "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compAttack", 1), "Comp Attack",
        juce::NormalisableRange<float>(0.1f, 80.0f, 0.1f, 0.5f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRelease", 1), "Comp Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.5f), 80.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compMakeup", 1), "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compMix", 1), "Comp Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));

    // === SATURATION ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("satOn", 1), "Saturation On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("satDrive", 1), "Saturation Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("satMix", 1), "Saturation Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));

    // === REVERB ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("reverbOn", 1), "Reverb On", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("reverbMode", 1), "Reverb Mode",
        juce::StringArray{ "Plate", "Hall", "Room" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbAmt", 1), "Reverb Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbDecay", 1), "Reverb Decay",
        juce::NormalisableRange<float>(0.2f, 8.0f, 0.1f, 0.5f), 1.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbPreDelay", 1), "Reverb Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbDamp", 1), "Reverb Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // === DELAY ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("delayOn", 1), "Delay On", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("delayMode", 1), "Delay Mode",
        juce::StringArray{ "Stereo", "Ping-Pong", "Tape" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayAmt", 1), "Delay Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayTime", 1), "Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f, 0.5f), 250.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayFeedback", 1), "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayHighCut", 1), "Delay High Cut",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.5f), 8000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayLowCut", 1), "Delay Low Cut",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.5f), 100.0f));

    // === BUS COMPRESSOR ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("busCompOn", 1), "Bus Comp On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("busCompThresh", 1), "Bus Comp Threshold",
        juce::NormalisableRange<float>(-30.0f, 0.0f, 0.1f), -10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("busCompRatio", 1), "Bus Comp Ratio",
        juce::NormalisableRange<float>(2.0f, 10.0f, 0.1f), 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("busCompAttack", 1), "Bus Comp Attack",
        juce::NormalisableRange<float>(0.1f, 30.0f, 0.1f, 0.5f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("busCompRelease", 1), "Bus Comp Release",
        juce::NormalisableRange<float>(50.0f, 1200.0f, 1.0f, 0.5f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("busCompMakeup", 1), "Bus Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("busCompMix", 1), "Bus Comp Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("busCompSCHPF", 1), "Bus Comp SC HPF",
        juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.5f), 80.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("busCompAutoRel", 1), "Bus Comp Auto Release", true));

    return { params.begin(), params.end() };
}

void WavCultStripProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    gate.prepare(sampleRate, samplesPerBlock);
    eq.prepare(sampleRate, samplesPerBlock);
    claComp.prepare(sampleRate, samplesPerBlock);
    sat.prepare(sampleRate, samplesPerBlock);
    reverb.prepare(sampleRate, samplesPerBlock);
    delay.prepare(sampleRate, samplesPerBlock);
    busComp.prepare(sampleRate, samplesPerBlock);
}

void WavCultStripProcessor::releaseResources()
{
    gate.reset();
    eq.reset();
    claComp.reset();
    sat.reset();
    reverb.reset();
    delay.reset();
    busComp.reset();
}

bool WavCultStripProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void WavCultStripProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    updateParameters();

    // Input gain
    float inGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGain")->load());
    buffer.applyGain(inGain);

    // Signal chain
    gate.process(buffer);
    eq.process(buffer);
    claComp.process(buffer);
    sat.process(buffer);
    delay.process(buffer);
    reverb.process(buffer);
    busComp.process(buffer);

    // Output gain
    float outGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("outputGain")->load());
    buffer.applyGain(outGain);
}

void WavCultStripProcessor::updateParameters()
{
    // Noise Gate
    gate.setEnabled(apvts.getRawParameterValue("gateOn")->load() > 0.5f);
    gate.setThreshold(apvts.getRawParameterValue("gateThresh")->load());
    gate.setAttack(apvts.getRawParameterValue("gateAttack")->load());
    gate.setRelease(apvts.getRawParameterValue("gateRelease")->load());
    gate.setHold(apvts.getRawParameterValue("gateHold")->load());
    gate.setRange(apvts.getRawParameterValue("gateRange")->load());

    // EQ
    eq.setEnabled(apvts.getRawParameterValue("eqOn")->load() > 0.5f);
    eq.setLowGain(apvts.getRawParameterValue("eqLowGain")->load());
    eq.setLowFreq(apvts.getRawParameterValue("eqLowFreq")->load());
    eq.setLowMidGain(apvts.getRawParameterValue("eqLMGain")->load());
    eq.setLowMidFreq(apvts.getRawParameterValue("eqLMFreq")->load());
    eq.setLowMidQ(apvts.getRawParameterValue("eqLMQ")->load());
    eq.setHiMidGain(apvts.getRawParameterValue("eqHMGain")->load());
    eq.setHiMidFreq(apvts.getRawParameterValue("eqHMFreq")->load());
    eq.setHiMidQ(apvts.getRawParameterValue("eqHMQ")->load());
    eq.setHighGain(apvts.getRawParameterValue("eqHighGain")->load());
    eq.setHighFreq(apvts.getRawParameterValue("eqHighFreq")->load());

    // Compressor
    claComp.setEnabled(apvts.getRawParameterValue("compOn")->load() > 0.5f);
    claComp.setThreshold(apvts.getRawParameterValue("compThresh")->load());
    claComp.setRatio(apvts.getRawParameterValue("compRatio")->load());
    claComp.setAttack(apvts.getRawParameterValue("compAttack")->load());
    claComp.setRelease(apvts.getRawParameterValue("compRelease")->load());
    claComp.setMakeupGain(apvts.getRawParameterValue("compMakeup")->load());
    claComp.setMix(apvts.getRawParameterValue("compMix")->load());

    // Saturation
    sat.setEnabled(apvts.getRawParameterValue("satOn")->load() > 0.5f);
    sat.setDrive(apvts.getRawParameterValue("satDrive")->load());
    sat.setMix(apvts.getRawParameterValue("satMix")->load());

    // Reverb
    reverb.setEnabled(apvts.getRawParameterValue("reverbOn")->load() > 0.5f);
    reverb.setMode(static_cast<ReverbEngine::Mode>(
        (int)apvts.getRawParameterValue("reverbMode")->load()));
    reverb.setAmount(apvts.getRawParameterValue("reverbAmt")->load());
    reverb.setDecay(apvts.getRawParameterValue("reverbDecay")->load());
    reverb.setPreDelay(apvts.getRawParameterValue("reverbPreDelay")->load());
    reverb.setDamping(apvts.getRawParameterValue("reverbDamp")->load());

    // Delay
    delay.setEnabled(apvts.getRawParameterValue("delayOn")->load() > 0.5f);
    delay.setMode(static_cast<DelayEngine::Mode>(
        (int)apvts.getRawParameterValue("delayMode")->load()));
    delay.setAmount(apvts.getRawParameterValue("delayAmt")->load());
    delay.setTime(apvts.getRawParameterValue("delayTime")->load());
    delay.setFeedback(apvts.getRawParameterValue("delayFeedback")->load());
    delay.setHighCut(apvts.getRawParameterValue("delayHighCut")->load());
    delay.setLowCut(apvts.getRawParameterValue("delayLowCut")->load());

    // Bus Compressor
    busComp.setEnabled(apvts.getRawParameterValue("busCompOn")->load() > 0.5f);
    busComp.setThreshold(apvts.getRawParameterValue("busCompThresh")->load());
    busComp.setRatio(apvts.getRawParameterValue("busCompRatio")->load());
    busComp.setAttack(apvts.getRawParameterValue("busCompAttack")->load());
    busComp.setRelease(apvts.getRawParameterValue("busCompRelease")->load());
    busComp.setMakeupGain(apvts.getRawParameterValue("busCompMakeup")->load());
    busComp.setMix(apvts.getRawParameterValue("busCompMix")->load());
    busComp.setSidechainHPF(apvts.getRawParameterValue("busCompSCHPF")->load());
    busComp.setAutoRelease(apvts.getRawParameterValue("busCompAutoRel")->load() > 0.5f);
}

void WavCultStripProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WavCultStripProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavCultStripProcessor();
}
