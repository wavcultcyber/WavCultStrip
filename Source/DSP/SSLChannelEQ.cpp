#include "SSLChannelEQ.h"
#include <cmath>

SSLChannelEQ::SSLChannelEQ() {}

void SSLChannelEQ::prepare(double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 1 };

    lowShelfL.prepare(spec);  lowShelfR.prepare(spec);
    lowMidL.prepare(spec);    lowMidR.prepare(spec);
    hiMidL.prepare(spec);     hiMidR.prepare(spec);
    highShelfL.prepare(spec); highShelfR.prepare(spec);

    updateLowShelf();
    updateLowMid();
    updateHiMid();
    updateHighShelf();
}

void SSLChannelEQ::reset()
{
    lowShelfL.reset();  lowShelfR.reset();
    lowMidL.reset();    lowMidR.reset();
    hiMidL.reset();     hiMidR.reset();
    highShelfL.reset(); highShelfR.reset();
}

void SSLChannelEQ::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled) return;

    const int numSamples = buffer.getNumSamples();
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        // Process left channel through all bands
        float L = left[i];
        L = lowShelfL.processSample(L);
        L = lowMidL.processSample(L);
        L = hiMidL.processSample(L);
        L = highShelfL.processSample(L);

        // Subtle analog saturation - SSL transformer coloration
        L = analogSaturate(L);
        left[i] = L;

        if (right != nullptr)
        {
            float R = right[i];
            R = lowShelfR.processSample(R);
            R = lowMidR.processSample(R);
            R = hiMidR.processSample(R);
            R = highShelfR.processSample(R);
            R = analogSaturate(R);
            right[i] = R;
        }
    }
}

void SSLChannelEQ::setLowGain(float dB)   { lowGain = dB; updateLowShelf(); }
void SSLChannelEQ::setLowFreq(float hz)   { lowFreq = hz; updateLowShelf(); }
void SSLChannelEQ::setLowMidGain(float dB) { lmGain = dB; updateLowMid(); }
void SSLChannelEQ::setLowMidFreq(float hz) { lmFreq = hz; updateLowMid(); }
void SSLChannelEQ::setLowMidQ(float q)    { lmQ = q; updateLowMid(); }
void SSLChannelEQ::setHiMidGain(float dB)  { hmGain = dB; updateHiMid(); }
void SSLChannelEQ::setHiMidFreq(float hz)  { hmFreq = hz; updateHiMid(); }
void SSLChannelEQ::setHiMidQ(float q)     { hmQ = q; updateHiMid(); }
void SSLChannelEQ::setHighGain(float dB)   { highGain = dB; updateHighShelf(); }
void SSLChannelEQ::setHighFreq(float hz)   { highFreq = hz; updateHighShelf(); }

void SSLChannelEQ::updateLowShelf()
{
    auto coeffs = Coefficients::makeLowShelf(sr, lowFreq, 0.71f,
        juce::Decibels::decibelsToGain(lowGain));
    *lowShelfL.coefficients = *coeffs;
    *lowShelfR.coefficients = *coeffs;
}

void SSLChannelEQ::updateLowMid()
{
    auto coeffs = Coefficients::makePeakFilter(sr, lmFreq, lmQ,
        juce::Decibels::decibelsToGain(lmGain));
    *lowMidL.coefficients = *coeffs;
    *lowMidR.coefficients = *coeffs;
}

void SSLChannelEQ::updateHiMid()
{
    auto coeffs = Coefficients::makePeakFilter(sr, hmFreq, hmQ,
        juce::Decibels::decibelsToGain(hmGain));
    *hiMidL.coefficients = *coeffs;
    *hiMidR.coefficients = *coeffs;
}

void SSLChannelEQ::updateHighShelf()
{
    auto coeffs = Coefficients::makeHighShelf(sr, highFreq, 0.71f,
        juce::Decibels::decibelsToGain(highGain));
    *highShelfL.coefficients = *coeffs;
    *highShelfR.coefficients = *coeffs;
}

float SSLChannelEQ::analogSaturate(float x)
{
    // Very subtle transformer saturation - SSL character
    // Adds slight 2nd and 3rd harmonic content
    const float amount = 0.02f; // Very subtle
    return x - amount * x * x * x;
}
