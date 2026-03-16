#pragma once
#include <JuceHeader.h>

/**
 * Delay Engine with 3 Modes
 *
 * Vocal-focused delay for depth and width:
 * - STEREO: Clean stereo delay with separate L/R times
 * - PINGPONG: Bouncing stereo delay, great for wide vocal throws
 * - TAPE: Warm, degrading tape echo with wow/flutter and saturation
 *
 * Features tempo-syncable delay times, filtering in feedback path,
 * and ducking to keep delays from cluttering the vocal.
 */
class DelayEngine
{
public:
    enum Mode { STEREO = 0, PINGPONG, TAPE };

    DelayEngine();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void setMode(Mode mode);
    void setAmount(float amt);       // 0.0 to 1.0 (dry/wet)
    void setTime(float ms);          // 1 to 2000
    void setFeedback(float amt);     // 0.0 to 0.95
    void setHighCut(float hz);       // 1000 to 20000
    void setLowCut(float hz);        // 20 to 500
    void setEnabled(bool enabled) { isEnabled = enabled; }

private:
    double sr = 44100.0;
    bool isEnabled = true;
    Mode currentMode = STEREO;

    float amount = 0.0f;
    float delayTimeMs = 250.0f;
    float feedbackAmt = 0.3f;
    float highCutHz = 8000.0f;
    float lowCutHz = 100.0f;

    // Delay buffers (stereo)
    static constexpr int MAX_DELAY_SAMPLES = 192001; // ~4s at 48kHz
    std::vector<float> delayBufferL, delayBufferR;
    int writePos = 0;

    // Feedback filters
    juce::dsp::IIR::Filter<float> fbHPF_L, fbHPF_R;
    juce::dsp::IIR::Filter<float> fbLPF_L, fbLPF_R;

    // Tape mode state
    float tapeModPhase = 0.0f;
    float tapeSatState = 0.0f;

    // Feedback state
    float feedbackL = 0.0f;
    float feedbackR = 0.0f;

    void updateFilters();
    float readDelay(const std::vector<float>& buffer, float delaySamples);
    float tapeSaturate(float x);
};
