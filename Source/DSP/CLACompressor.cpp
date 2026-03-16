#include "CLACompressor.h"
#include <cmath>
#include <algorithm>

CLACompressor::CLACompressor() {}

void CLACompressor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;
    envdB = 0.0f;
    releaseAccumulator = 0.0f;
    currentGR = 0.0f;
    updateCoefficients();
}

void CLACompressor::reset()
{
    envdB = 0.0f;
    releaseAccumulator = 0.0f;
    currentGR = 0.0f;
}

void CLACompressor::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled) return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        // Get peak level across channels
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            peak = std::max(peak, std::abs(buffer.getSample(ch, i)));

        // Convert to dB
        float inputdB = (peak > 1e-6f) ? 20.0f * std::log10(peak) : -120.0f;

        // Compute desired gain reduction
        float desiredGR = computeGain(inputdB) - inputdB;

        // Program-dependent release (CLA opto character)
        // Deeper compression = slower release (opto behavior)
        float progRelease = releaseMs;
        if (desiredGR < -10.0f)
            progRelease *= 1.5f;
        if (desiredGR < -20.0f)
            progRelease *= 2.0f;

        float progReleaseCoeff = std::exp(-1.0f / (float(sr) * progRelease * 0.001f));

        // Smooth envelope follower with attack/release
        if (desiredGR < envdB)
            envdB = attackCoeff * envdB + (1.0f - attackCoeff) * desiredGR;
        else
            envdB = progReleaseCoeff * envdB + (1.0f - progReleaseCoeff) * desiredGR;

        currentGR = envdB;

        // Apply gain
        float gainLin = juce::Decibels::decibelsToGain(envdB + makeupGain);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float dry = buffer.getSample(ch, i);
            float wet = dry * gainLin;

            // Add subtle harmonic warmth (CLA analog character)
            // 2nd harmonic generation from the opto cell
            wet += 0.005f * wet * wet * (wet > 0 ? 1.0f : -1.0f);

            // Parallel compression mix
            float mixAmt = mix * 0.01f;
            buffer.setSample(ch, i, dry * (1.0f - mixAmt) + wet * mixAmt);
        }
    }
}

void CLACompressor::setThreshold(float dB)    { threshold = dB; }
void CLACompressor::setRatio(float r)         { ratio = r; }
void CLACompressor::setAttack(float ms)       { attackMs = ms; updateCoefficients(); }
void CLACompressor::setRelease(float ms)      { releaseMs = ms; updateCoefficients(); }
void CLACompressor::setMakeupGain(float dB)   { makeupGain = dB; }
void CLACompressor::setMix(float pct)         { mix = pct; }

void CLACompressor::updateCoefficients()
{
    attackCoeff = std::exp(-1.0f / (float(sr) * attackMs * 0.001f));
    releaseCoeff = std::exp(-1.0f / (float(sr) * releaseMs * 0.001f));
}

float CLACompressor::computeGain(float inputdB)
{
    // Soft-knee compression (CLA smooth character)
    float diff = inputdB - threshold;

    if (diff <= -kneeWidth / 2.0f)
    {
        return inputdB; // Below threshold
    }
    else if (diff >= kneeWidth / 2.0f)
    {
        // Above threshold - full compression
        return threshold + diff / ratio;
    }
    else
    {
        // In the knee region - smooth interpolation
        float kneeRatio = 1.0f + (ratio - 1.0f) *
            ((diff + kneeWidth / 2.0f) / kneeWidth);
        return inputdB + (1.0f / kneeRatio - 1.0f) *
            (diff + kneeWidth / 2.0f) * (diff + kneeWidth / 2.0f) / (2.0f * kneeWidth);
    }
}
