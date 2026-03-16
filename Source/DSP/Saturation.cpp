#include "Saturation.h"
#include <cmath>

Saturation::Saturation() {}

void Saturation::prepare(double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;

    oversampling.initProcessing(samplesPerBlock);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 1 };
    toneFilterL.prepare(spec);
    toneFilterR.prepare(spec);
    updateToneFilter();
}

void Saturation::reset()
{
    oversampling.reset();
    toneFilterL.reset();
    toneFilterR.reset();
}

void Saturation::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled || drive < 0.001f) return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Create audio block for oversampling
    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversampling.processSamplesUp(block);

    const int osNumSamples = (int)oversampledBlock.getNumSamples();
    float mixAmt = mix * 0.01f;

    for (int i = 0; i < osNumSamples; ++i)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = oversampledBlock.getChannelPointer(ch);
            float dry = data[i];
            float wet = saturate(dry, drive);
            data[i] = dry * (1.0f - mixAmt) + wet * mixAmt;
        }
    }

    oversampling.processSamplesDown(block);

    // Apply tape-style high frequency roll-off
    auto* left  = buffer.getWritePointer(0);
    auto* right = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        left[i] = toneFilterL.processSample(left[i]);
        if (right != nullptr)
            right[i] = toneFilterR.processSample(right[i]);
    }
}

void Saturation::setDrive(float amount) { drive = amount; updateToneFilter(); }
void Saturation::setMix(float pct)      { mix = pct; }

float Saturation::saturate(float x, float driveAmount)
{
    // Hybrid tube/tape saturation
    // Input gain based on drive
    float gained = x * (1.0f + driveAmount * 8.0f);

    // Asymmetric soft clipping (tube character)
    // Positive half gets slightly more compression (even harmonics)
    float out;
    if (gained >= 0.0f)
    {
        // Tube-style positive clipping - softer, richer harmonics
        out = std::tanh(gained * 1.2f) * 0.9f;
    }
    else
    {
        // Slightly harder negative clipping - adds odd harmonics
        out = std::tanh(gained) * 0.95f;
    }

    // Blend in some tape-style saturation (softer, warmer)
    float tapeChar = gained / (1.0f + std::abs(gained));
    out = out * 0.7f + tapeChar * 0.3f;

    // Normalize output level
    return out * (1.0f / (1.0f + driveAmount * 0.5f));
}

void Saturation::updateToneFilter()
{
    // Tape head high-frequency roll-off
    // More drive = more roll-off (darker tone at higher drive)
    float cutoff = 16000.0f - drive * 6000.0f;
    cutoff = std::max(cutoff, 4000.0f);

    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, cutoff, 0.707f);
    *toneFilterL.coefficients = *coeffs;
    *toneFilterR.coefficients = *coeffs;
}
