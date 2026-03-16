#include "ReverbEngine.h"
#include <cmath>
#include <algorithm>

// Prime-number delay lengths for metallic-free reverb
static const int kDelayLengthsPlate[] = { 1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116 };
static const int kDelayLengthsHall[]  = { 2999, 3169, 2791, 2593, 2417, 2237, 2111, 1973 };
static const int kDelayLengthsRoom[]  = { 887,  929,  811,  733,  661,  601,  557,  491  };

static const int kAllpassLengths[] = { 225, 556, 441, 341 };

ReverbEngine::ReverbEngine() {}

void ReverbEngine::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;

    // Scale delay lengths for sample rate
    double srRatio = sr / 44100.0;

    for (int i = 0; i < NUM_ALLPASS; ++i)
    {
        allpassLengths[i] = (int)(kAllpassLengths[i] * srRatio);
        allpassLines[i].resize(allpassLengths[i] + 1, 0.0f);
        allpassWritePos[i] = 0;
    }

    // Pre-delay (max 100ms)
    int maxPreDelay = (int)(sr * 0.1) + 1;
    preDelayBufferL.resize(maxPreDelay, 0.0f);
    preDelayBufferR.resize(maxPreDelay, 0.0f);
    preDelayWritePos = 0;

    // Input filters
    juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
    inputHPF.prepare(spec);
    inputLPF.prepare(spec);

    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, 120.0f);
    auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, 10000.0f);
    *inputHPF.coefficients = *hpCoeffs;
    *inputLPF.coefficients = *lpCoeffs;

    configureForMode();
    updatePreDelay();
}

void ReverbEngine::reset()
{
    for (int i = 0; i < NUM_DELAYS; ++i)
    {
        std::fill(delayLines[i].begin(), delayLines[i].end(), 0.0f);
        dampState[i] = 0.0f;
    }
    for (int i = 0; i < NUM_ALLPASS; ++i)
        std::fill(allpassLines[i].begin(), allpassLines[i].end(), 0.0f);

    std::fill(preDelayBufferL.begin(), preDelayBufferL.end(), 0.0f);
    std::fill(preDelayBufferR.begin(), preDelayBufferR.end(), 0.0f);

    inputHPF.reset();
    inputLPF.reset();
    modPhase = 0.0f;
}

void ReverbEngine::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled || amount < 0.001f) return;

    const int numSamples = buffer.getNumSamples();
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = left[i];
        float inR = right != nullptr ? right[i] : inL;

        // Mix to mono for reverb input and filter
        float monoIn = (inL + inR) * 0.5f;
        monoIn = inputHPF.processSample(monoIn);
        monoIn = inputLPF.processSample(monoIn);

        // Pre-delay
        preDelayBufferL[preDelayWritePos] = monoIn;
        preDelayBufferR[preDelayWritePos] = monoIn;
        int preDelayReadPos = preDelayWritePos - preDelaySamples;
        if (preDelayReadPos < 0) preDelayReadPos += (int)preDelayBufferL.size();
        float pdOut = preDelayBufferL[preDelayReadPos];
        preDelayWritePos = (preDelayWritePos + 1) % (int)preDelayBufferL.size();

        // Diffusion through allpass chain
        float diffused = pdOut;
        for (int ap = 0; ap < NUM_ALLPASS; ++ap)
            diffused = processAllpass(ap, diffused);

        // Modulation LFO
        modPhase += modRate / (float)sr;
        if (modPhase >= 1.0f) modPhase -= 1.0f;
        float lfo = std::sin(modPhase * 2.0f * juce::MathConstants<float>::pi);

        // FDN processing
        float outputs[NUM_DELAYS];
        for (int d = 0; d < NUM_DELAYS; ++d)
        {
            // Modulated read position
            float modOffset = (d < 4) ? lfo * modDepth : -lfo * modDepth;
            int readPos = delayWritePos[d] - delayLengths[d] + (int)(modOffset);
            if (readPos < 0) readPos += (int)delayLines[d].size();
            readPos = readPos % (int)delayLines[d].size();

            outputs[d] = delayLines[d][readPos];

            // Damping (one-pole lowpass)
            dampState[d] = outputs[d] + damping * (dampState[d] - outputs[d]);
            outputs[d] = dampState[d];
        }

        // Householder feedback matrix (energy-preserving)
        float sum = 0.0f;
        for (int d = 0; d < NUM_DELAYS; ++d)
            sum += outputs[d];
        sum *= 2.0f / NUM_DELAYS;

        for (int d = 0; d < NUM_DELAYS; ++d)
        {
            float fb = (sum - outputs[d]) * feedback[d];
            delayLines[d][delayWritePos[d]] = diffused + fb;
            delayWritePos[d] = (delayWritePos[d] + 1) % (int)delayLines[d].size();
        }

        // Tap outputs for stereo (alternating L/R)
        float wetL = 0.0f, wetR = 0.0f;
        for (int d = 0; d < NUM_DELAYS; ++d)
        {
            if (d % 2 == 0) wetL += outputs[d];
            else             wetR += outputs[d];
        }
        wetL *= 0.25f;
        wetR *= 0.25f;

        // Mix
        left[i]  = inL * (1.0f - amount) + wetL * amount;
        if (right != nullptr)
            right[i] = inR * (1.0f - amount) + wetR * amount;
    }
}

void ReverbEngine::setMode(Mode mode)     { currentMode = mode; configureForMode(); }
void ReverbEngine::setAmount(float amt)    { amount = amt; }
void ReverbEngine::setDecay(float s)       { decay = s; configureForMode(); }
void ReverbEngine::setPreDelay(float ms)   { preDelayMs = ms; updatePreDelay(); }
void ReverbEngine::setDamping(float amt)   { damping = amt; }

void ReverbEngine::configureForMode()
{
    double srRatio = sr / 44100.0;

    const int* lengths;
    switch (currentMode)
    {
        case PLATE: lengths = kDelayLengthsPlate; modRate = 1.0f; modDepth = 0.5f; break;
        case HALL:  lengths = kDelayLengthsHall;  modRate = 0.3f; modDepth = 1.0f; break;
        case ROOM:  lengths = kDelayLengthsRoom;  modRate = 0.8f; modDepth = 0.2f; break;
        default:    lengths = kDelayLengthsPlate; break;
    }

    // Calculate feedback from decay time
    for (int d = 0; d < NUM_DELAYS; ++d)
    {
        delayLengths[d] = (int)(lengths[d] * srRatio);
        delayLines[d].resize(delayLengths[d] + 64, 0.0f); // Extra for modulation
        delayWritePos[d] = 0;

        // RT60 feedback calculation
        float delayTimeSec = delayLengths[d] / (float)sr;
        feedback[d] = std::pow(10.0f, -3.0f * delayTimeSec / decay);
        feedback[d] = std::min(feedback[d], 0.98f);
    }
}

float ReverbEngine::processAllpass(int index, float input)
{
    int readPos = allpassWritePos[index] - allpassLengths[index];
    if (readPos < 0) readPos += (int)allpassLines[index].size();

    float delayed = allpassLines[index][readPos];
    float output = -input + delayed;
    allpassLines[index][allpassWritePos[index]] = input + delayed * allpassCoeff;
    allpassWritePos[index] = (allpassWritePos[index] + 1) % (int)allpassLines[index].size();

    return output;
}

void ReverbEngine::updatePreDelay()
{
    preDelaySamples = (int)(preDelayMs * 0.001f * sr);
    preDelaySamples = std::min(preDelaySamples, (int)preDelayBufferL.size() - 1);
}
