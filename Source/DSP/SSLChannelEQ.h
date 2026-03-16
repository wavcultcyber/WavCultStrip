#pragma once
#include <JuceHeader.h>

/**
 * SSL-style 4-band Channel EQ
 *
 * Modeled after the SSL 4000 E/G series channel EQ character:
 * - Low shelf (60-300 Hz) with gentle analog slope
 * - Low-mid parametric bell (200 Hz - 2 kHz)
 * - High-mid parametric bell (600 Hz - 8 kHz)
 * - High shelf (1.5 kHz - 16 kHz) with air
 *
 * Each band uses analog-modeled biquad filters with
 * slight nonlinear saturation to emulate transformer coloration.
 */
class SSLChannelEQ
{
public:
    SSLChannelEQ();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Low shelf
    void setLowGain(float dB);       // -15 to +15
    void setLowFreq(float hz);       // 60 - 300

    // Low-mid bell
    void setLowMidGain(float dB);    // -15 to +15
    void setLowMidFreq(float hz);    // 200 - 2000
    void setLowMidQ(float q);        // 0.5 - 4.0

    // High-mid bell
    void setHiMidGain(float dB);     // -15 to +15
    void setHiMidFreq(float hz);     // 600 - 8000
    void setHiMidQ(float q);         // 0.5 - 4.0

    // High shelf
    void setHighGain(float dB);      // -15 to +15
    void setHighFreq(float hz);      // 1500 - 16000

    void setEnabled(bool enabled) { isEnabled = enabled; }

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    // Stereo filter pairs
    Filter lowShelfL, lowShelfR;
    Filter lowMidL, lowMidR;
    Filter hiMidL, hiMidR;
    Filter highShelfL, highShelfR;

    double sr = 44100.0;
    bool isEnabled = true;

    // Current parameter values
    float lowGain = 0.0f, lowFreq = 100.0f;
    float lmGain = 0.0f, lmFreq = 600.0f, lmQ = 1.0f;
    float hmGain = 0.0f, hmFreq = 3000.0f, hmQ = 1.0f;
    float highGain = 0.0f, highFreq = 8000.0f;

    void updateLowShelf();
    void updateLowMid();
    void updateHiMid();
    void updateHighShelf();

    // Subtle analog warmth
    static float analogSaturate(float x);
};
