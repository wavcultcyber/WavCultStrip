#pragma once
#include <JuceHeader.h>

/**
 * Analog Saturation Module
 *
 * Hybrid tape/tube saturation with:
 * - Asymmetric soft clipping (tube character)
 * - Even + odd harmonic generation
 * - Input-dependent color shift
 * - Oversampled processing to avoid aliasing
 * - Subtle high-frequency roll-off (tape head emulation)
 */
class Saturation
{
public:
    Saturation();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void setDrive(float amount);     // 0.0 to 1.0
    void setMix(float pct);          // 0 to 100
    void setEnabled(bool enabled) { isEnabled = enabled; }

private:
    double sr = 44100.0;
    bool isEnabled = true;

    float drive = 0.0f;
    float mix = 100.0f;

    // 2x oversampling
    juce::dsp::Oversampling<float> oversampling { 2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

    // Post-saturation tone filter (tape roll-off)
    juce::dsp::IIR::Filter<float> toneFilterL, toneFilterR;

    static float saturate(float x, float driveAmount);
    void updateToneFilter();
};
