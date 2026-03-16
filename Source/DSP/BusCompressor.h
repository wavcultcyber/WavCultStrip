#pragma once
#include <JuceHeader.h>

/**
 * Bus Compressor (SSL-style Glue Compression)
 *
 * Modeled after the SSL G-Bus compressor character:
 * - VCA-style compression with punchy transients
 * - Auto-release with program-dependent behavior
 * - Sidechain high-pass filter to prevent low-end pumping
 * - Stepped ratio (2:1, 4:1, 10:1)
 * - Mix knob for parallel bus compression
 *
 * Placed last in chain as the "glue" stage.
 */
class BusCompressor
{
public:
    BusCompressor();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void setThreshold(float dB);     // -30 to 0
    void setRatio(float ratio);      // 2, 4, or 10
    void setAttack(float ms);        // 0.1 to 30
    void setRelease(float ms);       // 50 to 1200 (auto mode available)
    void setMakeupGain(float dB);    // 0 to 12
    void setMix(float pct);          // 0 to 100
    void setSidechainHPF(float hz);  // 20 to 300
    void setAutoRelease(bool on) { autoRelease = on; }
    void setEnabled(bool enabled) { isEnabled = enabled; }

    float getGainReduction() const { return currentGR; }

private:
    double sr = 44100.0;
    bool isEnabled = true;
    bool autoRelease = true;

    float threshold = -10.0f;
    float ratio = 4.0f;
    float attackMs = 1.0f;
    float releaseMs = 100.0f;
    float makeupGain = 0.0f;
    float mix = 100.0f;
    float scHPFFreq = 80.0f;

    // Envelope state
    float envdB = 0.0f;
    float currentGR = 0.0f;

    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Sidechain HPF
    juce::dsp::IIR::Filter<float> scFilterL, scFilterR;

    void updateCoefficients();
    void updateSCFilter();
    float computeGain(float inputdB);
};
