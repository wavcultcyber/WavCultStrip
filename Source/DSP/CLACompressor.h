#pragma once
#include <JuceHeader.h>

/**
 * CLA-style Vocal Compressor
 *
 * Inspired by the Waves CLA-2A / CLA-76 character:
 * - Opto-style program-dependent release
 * - Smooth, musical gain reduction
 * - Adds harmonic warmth and presence
 * - Feed-forward/feedback hybrid topology
 * - Soft-knee for natural vocal compression
 *
 * Perfect for upfront, polished vocal sound.
 */
class CLACompressor
{
public:
    CLACompressor();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void setThreshold(float dB);     // -40 to 0
    void setRatio(float ratio);      // 1:1 to 20:1
    void setAttack(float ms);        // 0.1 to 80
    void setRelease(float ms);       // 10 to 1000
    void setMakeupGain(float dB);    // 0 to 24
    void setMix(float pct);          // 0 to 100 (parallel compression)
    void setEnabled(bool enabled) { isEnabled = enabled; }

    float getGainReduction() const { return currentGR; }

private:
    double sr = 44100.0;
    bool isEnabled = true;

    float threshold = -18.0f;
    float ratio = 4.0f;
    float attackMs = 5.0f;
    float releaseMs = 80.0f;
    float makeupGain = 0.0f;
    float mix = 100.0f;

    // Envelope follower state
    float envdB = 0.0f;
    float currentGR = 0.0f;

    // Smooth coefficients
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Program-dependent release state
    float releaseAccumulator = 0.0f;

    void updateCoefficients();
    float computeGain(float inputdB);

    // Opto-style soft knee
    static constexpr float kneeWidth = 6.0f;
};
