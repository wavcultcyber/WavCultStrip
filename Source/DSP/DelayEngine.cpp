#include "DelayEngine.h"
#include <cmath>
#include <algorithm>

DelayEngine::DelayEngine() {}

void DelayEngine::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;

    delayBufferL.resize(MAX_DELAY_SAMPLES, 0.0f);
    delayBufferR.resize(MAX_DELAY_SAMPLES, 0.0f);
    writePos = 0;

    feedbackL = 0.0f;
    feedbackR = 0.0f;
    tapeModPhase = 0.0f;

    juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
    fbHPF_L.prepare(spec); fbHPF_R.prepare(spec);
    fbLPF_L.prepare(spec); fbLPF_R.prepare(spec);

    updateFilters();
}

void DelayEngine::reset()
{
    std::fill(delayBufferL.begin(), delayBufferL.end(), 0.0f);
    std::fill(delayBufferR.begin(), delayBufferR.end(), 0.0f);
    writePos = 0;
    feedbackL = 0.0f;
    feedbackR = 0.0f;
    tapeModPhase = 0.0f;

    fbHPF_L.reset(); fbHPF_R.reset();
    fbLPF_L.reset(); fbLPF_R.reset();
}

void DelayEngine::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled || amount < 0.001f) return;

    const int numSamples = buffer.getNumSamples();
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    float delaySamplesBase = delayTimeMs * 0.001f * (float)sr;

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = left[i];
        float inR = right != nullptr ? right[i] : inL;

        float delaySamplesL = delaySamplesBase;
        float delaySamplesR = delaySamplesBase;

        // Mode-specific behavior
        switch (currentMode)
        {
            case STEREO:
                // Slight offset for stereo width
                delaySamplesR = delaySamplesBase * 1.08f;
                break;

            case PINGPONG:
                // Same delay time, cross-feed in feedback
                break;

            case TAPE:
            {
                // Wow and flutter modulation
                tapeModPhase += 2.5f / (float)sr;
                if (tapeModPhase >= 1.0f) tapeModPhase -= 1.0f;

                float wow = std::sin(tapeModPhase * 2.0f * juce::MathConstants<float>::pi);
                float flutter = std::sin(tapeModPhase * 11.3f * 2.0f * juce::MathConstants<float>::pi) * 0.3f;

                float modAmount = (wow + flutter) * delaySamplesBase * 0.002f;
                delaySamplesL += modAmount;
                delaySamplesR += modAmount * 0.8f;
                break;
            }
        }

        // Read from delay lines (linear interpolation)
        float delayedL = readDelay(delayBufferL, delaySamplesL);
        float delayedR = readDelay(delayBufferR, delaySamplesR);

        // Filter feedback
        delayedL = fbLPF_L.processSample(fbHPF_L.processSample(delayedL));
        delayedR = fbLPF_R.processSample(fbHPF_R.processSample(delayedR));

        // Tape saturation in feedback path
        if (currentMode == TAPE)
        {
            delayedL = tapeSaturate(delayedL);
            delayedR = tapeSaturate(delayedR);
        }

        // Write to delay lines with feedback
        float writeL, writeR;
        if (currentMode == PINGPONG)
        {
            // Cross-feed for ping-pong
            writeL = inL + delayedR * feedbackAmt;
            writeR = inR + delayedL * feedbackAmt;
        }
        else
        {
            writeL = inL + delayedL * feedbackAmt;
            writeR = inR + delayedR * feedbackAmt;
        }

        delayBufferL[writePos] = writeL;
        delayBufferR[writePos] = writeR;
        writePos = (writePos + 1) % MAX_DELAY_SAMPLES;

        // Mix
        left[i]  = inL * (1.0f - amount) + delayedL * amount;
        if (right != nullptr)
            right[i] = inR * (1.0f - amount) + delayedR * amount;
    }
}

void DelayEngine::setMode(Mode mode)       { currentMode = mode; }
void DelayEngine::setAmount(float amt)     { amount = amt; }
void DelayEngine::setTime(float ms)        { delayTimeMs = ms; }
void DelayEngine::setFeedback(float amt)   { feedbackAmt = std::min(amt, 0.95f); }
void DelayEngine::setHighCut(float hz)     { highCutHz = hz; updateFilters(); }
void DelayEngine::setLowCut(float hz)      { lowCutHz = hz; updateFilters(); }

void DelayEngine::updateFilters()
{
    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, lowCutHz);
    auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, highCutHz);

    *fbHPF_L.coefficients = *hpCoeffs;
    *fbHPF_R.coefficients = *hpCoeffs;
    *fbLPF_L.coefficients = *lpCoeffs;
    *fbLPF_R.coefficients = *lpCoeffs;
}

float DelayEngine::readDelay(const std::vector<float>& buffer, float delaySamples)
{
    delaySamples = std::max(delaySamples, 1.0f);
    delaySamples = std::min(delaySamples, (float)(MAX_DELAY_SAMPLES - 1));

    int pos0 = writePos - (int)delaySamples;
    if (pos0 < 0) pos0 += MAX_DELAY_SAMPLES;

    int pos1 = pos0 - 1;
    if (pos1 < 0) pos1 += MAX_DELAY_SAMPLES;

    float frac = delaySamples - std::floor(delaySamples);
    return buffer[pos0] * (1.0f - frac) + buffer[pos1] * frac;
}

float DelayEngine::tapeSaturate(float x)
{
    // Gentle tape saturation in feedback path
    return std::tanh(x * 1.5f) * 0.75f;
}
