#pragma once
#include <JuceHeader.h>
#include "DSP/NoiseGate.h"
#include "DSP/SSLChannelEQ.h"
#include "DSP/CLACompressor.h"
#include "DSP/Saturation.h"
#include "DSP/ReverbEngine.h"
#include "DSP/DelayEngine.h"
#include "DSP/BusCompressor.h"

class WavCultStripProcessor : public juce::AudioProcessor
{
public:
    WavCultStripProcessor();
    ~WavCultStripProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // For metering
    float getCompressorGR() const { return claComp.getGainReduction(); }
    float getBusCompGR()    const { return busComp.getGainReduction(); }

private:
    // Signal chain order (optimized for modern vocal production):
    // 1. Noise Gate (clean up noise first)
    // 2. EQ (shape tone)
    // 3. Compressor (control dynamics on shaped signal)
    // 4. Saturation (add warmth/harmonics)
    // 5. Delay (time-based before reverb)
    // 6. Reverb (spatial last in time-based)
    // 7. Bus Compressor (glue everything together)

    NoiseGate       gate;
    SSLChannelEQ    eq;
    CLACompressor   claComp;
    Saturation      sat;
    ReverbEngine    reverb;
    DelayEngine     delay;
    BusCompressor   busComp;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Parameter listeners
    void updateParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavCultStripProcessor)
};
