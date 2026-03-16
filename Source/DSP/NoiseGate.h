#pragma once
#include <JuceHeader.h>

/**
 * Noise Gate
 *
 * Simple, effective noise gate placed at the beginning of the signal chain
 * to clean up unwanted noise before processing.
 *
 * Features:
 * - Adjustable threshold, attack, release, and hold time
 * - Smooth gain transitions to avoid clicks
 * - Range control (how much the gate attenuates)
 */
class NoiseGate
{
public:
    NoiseGate();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void setThreshold(float dB);     // -80 to 0
    void setAttack(float ms);        // 0.01 to 50
    void setRelease(float ms);       // 1 to 500
    void setHold(float ms);          // 0 to 500
    void setRange(float dB);         // -80 to 0 (how much to attenuate)
    void setEnabled(bool enabled) { isEnabled = enabled; }

private:
    double sr = 44100.0;
    bool isEnabled = true;

    float threshold = -40.0f;
    float attackMs = 0.5f;
    float releaseMs = 50.0f;
    float holdMs = 50.0f;
    float range = -80.0f;

    // Envelope state
    float envelope = 0.0f;
    float gateGain = 0.0f;
    int holdCounter = 0;

    // Smooth coefficients
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    int holdSamples = 0;

    void updateCoefficients();
};
