#include "BusCompressor.h"
#include <cmath>
#include <algorithm>

BusCompressor::BusCompressor() {}

void BusCompressor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;
    envdB = 0.0f;
    currentGR = 0.0f;

    juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
    scFilterL.prepare(spec);
    scFilterR.prepare(spec);

    updateCoefficients();
    updateSCFilter();
}

void BusCompressor::reset()
{
    envdB = 0.0f;
    currentGR = 0.0f;
    scFilterL.reset();
    scFilterR.reset();
}

void BusCompressor::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled) return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        // Sidechain: get peak and apply HPF
        float scL = buffer.getSample(0, i);
        float scR = numChannels > 1 ? buffer.getSample(1, i) : scL;

        scL = scFilterL.processSample(scL);
        scR = scFilterR.processSample(scR);

        float scPeak = std::max(std::abs(scL), std::abs(scR));
        float scdB = (scPeak > 1e-6f) ? 20.0f * std::log10(scPeak) : -120.0f;

        // Compute target gain
        float targetGR = computeGain(scdB) - scdB;

        // Auto-release: faster recovery for short transients, slower for sustained compression
        float currentReleaseCoeff = releaseCoeff;
        if (autoRelease)
        {
            float grDepth = std::abs(targetGR);
            if (grDepth > 6.0f)
                currentReleaseCoeff = std::exp(-1.0f / (float(sr) * 0.6f)); // ~600ms
            else if (grDepth > 2.0f)
                currentReleaseCoeff = std::exp(-1.0f / (float(sr) * 0.2f)); // ~200ms
            else
                currentReleaseCoeff = std::exp(-1.0f / (float(sr) * 0.05f)); // ~50ms fast
        }

        // VCA-style ballistics (punchy, precise)
        if (targetGR < envdB)
            envdB = attackCoeff * envdB + (1.0f - attackCoeff) * targetGR;
        else
            envdB = currentReleaseCoeff * envdB + (1.0f - currentReleaseCoeff) * targetGR;

        currentGR = envdB;

        // Apply gain with makeup
        float gainLin = juce::Decibels::decibelsToGain(envdB + makeupGain);

        float mixAmt = mix * 0.01f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float dry = buffer.getSample(ch, i);
            float wet = dry * gainLin;
            buffer.setSample(ch, i, dry * (1.0f - mixAmt) + wet * mixAmt);
        }
    }
}

void BusCompressor::setThreshold(float dB)    { threshold = dB; }
void BusCompressor::setRatio(float r)         { ratio = r; }
void BusCompressor::setAttack(float ms)       { attackMs = ms; updateCoefficients(); }
void BusCompressor::setRelease(float ms)      { releaseMs = ms; updateCoefficients(); }
void BusCompressor::setMakeupGain(float dB)   { makeupGain = dB; }
void BusCompressor::setMix(float pct)         { mix = pct; }
void BusCompressor::setSidechainHPF(float hz) { scHPFFreq = hz; updateSCFilter(); }

void BusCompressor::updateCoefficients()
{
    attackCoeff = std::exp(-1.0f / (float(sr) * attackMs * 0.001f));
    releaseCoeff = std::exp(-1.0f / (float(sr) * releaseMs * 0.001f));
}

void BusCompressor::updateSCFilter()
{
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, scHPFFreq);
    *scFilterL.coefficients = *coeffs;
    *scFilterR.coefficients = *coeffs;
}

float BusCompressor::computeGain(float inputdB)
{
    // Hard-knee like SSL bus comp
    if (inputdB <= threshold)
        return inputdB;

    return threshold + (inputdB - threshold) / ratio;
}
