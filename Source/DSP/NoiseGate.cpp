#include "NoiseGate.h"

NoiseGate::NoiseGate() {}

void NoiseGate::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;
    updateCoefficients();
    reset();
}

void NoiseGate::reset()
{
    envelope = 0.0f;
    gateGain = 0.0f;
    holdCounter = 0;
}

void NoiseGate::setThreshold(float dB)  { threshold = dB; }
void NoiseGate::setAttack(float ms)     { attackMs = ms; updateCoefficients(); }
void NoiseGate::setRelease(float ms)    { releaseMs = ms; updateCoefficients(); }
void NoiseGate::setHold(float ms)       { holdMs = ms; updateCoefficients(); }
void NoiseGate::setRange(float dB)      { range = dB; }

void NoiseGate::updateCoefficients()
{
    attackCoeff  = std::exp(-1.0f / (float)(sr * attackMs * 0.001));
    releaseCoeff = std::exp(-1.0f / (float)(sr * releaseMs * 0.001));
    holdSamples  = (int)(sr * holdMs * 0.001);
}

void NoiseGate::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    float rangeGain = juce::Decibels::decibelsToGain(range);

    for (int s = 0; s < numSamples; ++s)
    {
        // Peak detection across all channels
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            peak = std::max(peak, std::abs(buffer.getSample(ch, s)));

        float peakdB = juce::Decibels::gainToDecibels(peak, -100.0f);

        // Gate logic
        if (peakdB >= threshold)
        {
            // Signal above threshold — open gate
            holdCounter = holdSamples;
            gateGain = attackCoeff * gateGain + (1.0f - attackCoeff) * 1.0f;
        }
        else if (holdCounter > 0)
        {
            // In hold phase — keep gate open
            --holdCounter;
            gateGain = attackCoeff * gateGain + (1.0f - attackCoeff) * 1.0f;
        }
        else
        {
            // Below threshold, hold expired — close gate
            gateGain = releaseCoeff * gateGain + (1.0f - releaseCoeff) * rangeGain;
        }

        // Apply gain
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.setSample(ch, s, buffer.getSample(ch, s) * gateGain);
    }
}
