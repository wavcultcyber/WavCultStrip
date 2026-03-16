#pragma once
#include <JuceHeader.h>

/**
 * Reverb Engine with 3 Modes
 *
 * Designed for vocal production:
 * - PLATE: Bright, dense, upfront. Classic vocal plate reverb.
 * - HALL: Lush, wide, cinematic. For spacious vocal arrangements.
 * - ROOM: Tight, natural, intimate. Adds depth without washing out.
 *
 * Uses a Schroeder/Moorer style network with modulated allpass
 * chains for lush, non-metallic character.
 */
class ReverbEngine
{
public:
    enum Mode { PLATE = 0, HALL, ROOM };

    ReverbEngine();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void setMode(Mode mode);
    void setAmount(float amt);       // 0.0 to 1.0 (dry/wet)
    void setDecay(float seconds);    // 0.2 to 8.0
    void setPreDelay(float ms);      // 0 to 100
    void setDamping(float amt);      // 0.0 to 1.0
    void setEnabled(bool enabled) { isEnabled = enabled; }

private:
    double sr = 44100.0;
    bool isEnabled = true;
    Mode currentMode = PLATE;

    float amount = 0.0f;
    float decay = 1.5f;
    float preDelayMs = 20.0f;
    float damping = 0.5f;

    // Pre-delay line
    std::vector<float> preDelayBufferL, preDelayBufferR;
    int preDelayWritePos = 0;
    int preDelaySamples = 0;

    // FDN (Feedback Delay Network) - 8 delay lines
    static constexpr int NUM_DELAYS = 8;
    std::vector<float> delayLines[NUM_DELAYS];
    int delayWritePos[NUM_DELAYS] = {};
    int delayLengths[NUM_DELAYS] = {};
    float feedback[NUM_DELAYS] = {};

    // Damping filters (one per delay line)
    float dampState[NUM_DELAYS] = {};

    // Allpass diffusers
    static constexpr int NUM_ALLPASS = 4;
    std::vector<float> allpassLines[NUM_ALLPASS];
    int allpassWritePos[NUM_ALLPASS] = {};
    int allpassLengths[NUM_ALLPASS] = {};
    static constexpr float allpassCoeff = 0.6f;

    // Modulation
    float modPhase = 0.0f;
    float modRate = 0.5f;
    float modDepth = 0.3f;

    // Pre-EQ to filter input to reverb
    juce::dsp::IIR::Filter<float> inputHPF, inputLPF;

    void configureForMode();
    float processAllpass(int index, float input);
    void updatePreDelay();
};
