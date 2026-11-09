#include "BeerDSP.h"

BeerDSP::BeerDSP() {}

void BeerDSP::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels      = 2;

    oversampling.initProcessing(spec.maximumBlockSize);
    oversampling.reset();

    rmsSmoothed  = 0.0f;
    clipSmoothed = 0.0f;
    sampleHold   = 0.0f;
    sampleCounter = 0;
}

void BeerDSP::reset()
{
    oversampling.reset();
}

void BeerDSP::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    const float drive  = driveAmount.load();
    const float crush  = crushAmount.load();
    const float foam   = foamAmount.load();
    const float wet    = wetDry.load();

    // Keep dry copy for parallel mix
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Process via oversampling for anti-aliasing on bitcrusher
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling.processSamplesUp(block);

    for (int ch = 0; ch < (int)oversampledBlock.getNumChannels(); ++ch)
    {
        float* data = oversampledBlock.getChannelPointer(ch);
        for (int i = 0; i < (int)oversampledBlock.getNumSamples(); ++i)
            data[i] = processSample(data[i], drive, crush, foam);
    }

    oversampling.processSamplesDown(block);

    // Wet/dry mix
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* dry = dryBuffer.getReadPointer(ch);
        float*       wet_ptr = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            wet_ptr[i] = wet_ptr[i] * wet + dry[i] * (1.0f - wet);
    }

    // RMS metering for GUI
    float rms = 0.0f;
    bool  clipping = false;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            rms += data[i] * data[i];
            if (std::abs(data[i]) >= 0.999f) clipping = true;
        }
    }
    rms = std::sqrt(rms / (float)(numSamples * numChannels));

    const float rmsAlpha  = 0.1f;
    const float clipAlpha = 0.05f;
    rmsSmoothed  = rmsSmoothed  * (1.0f - rmsAlpha)  + rms     * rmsAlpha;
    clipSmoothed = clipSmoothed * (1.0f - clipAlpha)  + (clipping ? 1.0f : 0.0f) * clipAlpha;

    outputLevel  .store(juce::jlimit(0.0f, 1.0f, rmsSmoothed * 2.0f));
    clipIndicator.store(juce::jlimit(0.0f, 1.0f, clipSmoothed));
}

float BeerDSP::processSample(float sample, float drive, float crush, float foam)
{
    // 1. Bitcrusher
    if (crush > 0.01f)
    {
        float bits     = juce::jmap(crush, 0.0f, 1.0f, 24.0f, 3.0f);
        float rateRed  = juce::jmap(crush, 0.0f, 1.0f, 1.0f, 16.0f);
        sample = bitCrush(sample, bits, rateRed);
    }

    // 2. Saturation / soft clip
    if (drive > 0.01f)
        sample = softClip(sample, drive);

    // 3. Foam / peak limiter (hard clip with ceiling)
    if (foam > 0.01f)
    {
        float ceiling = juce::jmap(foam, 0.0f, 1.0f, 1.0f, 0.5f);
        sample = hardClip(sample, ceiling);
    }

    return sample;
}

float BeerDSP::softClip(float x, float drive)
{
    // Tube-style tanh saturation with drive boost
    float gain = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 12.0f);
    x *= gain;
    x  = std::tanh(x);
    x /= std::tanh(gain); // normalize output
    return x;
}

float BeerDSP::bitCrush(float x, float bits, float rateReduction)
{
    // Bit depth reduction
    float levels = std::pow(2.0f, bits) - 1.0f;
    x = std::round(x * levels) / levels;

    // Sample rate reduction (sample & hold)
    int holdSamples = (int)rateReduction;
    if (holdSamples > 1)
    {
        if (sampleCounter <= 0)
        {
            sampleHold   = x;
            sampleCounter = holdSamples;
        }
        x = sampleHold;
        --sampleCounter;
    }

    return x;
}

float BeerDSP::hardClip(float x, float ceiling)
{
    return juce::jlimit(-ceiling, ceiling, x) / ceiling;
}
