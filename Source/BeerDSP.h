#pragma once
#include <JuceHeader.h>

//==============================================================================
// DSP Engine: Bitcrusher + Soft Saturator + Clipper/Maximizer
//==============================================================================
class BeerDSP
{
public:
    BeerDSP();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Parameters (0.0 - 1.0 normalized)
    std::atomic<float> crushAmount  { 0.0f }; // Bitcrusher intensity
    std::atomic<float> driveAmount  { 0.0f }; // Saturation/Drive
    std::atomic<float> foamAmount   { 0.0f }; // Clipper/Maximizer ceiling
    std::atomic<float> wetDry       { 1.0f }; // Wet/Dry mix

    // Output level for GUI meter (RMS, 0.0-1.0)
    std::atomic<float> outputLevel  { 0.0f };
    std::atomic<float> clipIndicator{ 0.0f }; // 1.0 when clipping

private:
    float processSample(float sample, float drive, float crush, float foam);
    float softClip(float x, float drive);
    float bitCrush(float x, float bits, float rateReduction);
    float hardClip(float x, float ceiling);

    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;

    float sampleHold     = 0.0f;
    int   sampleCounter  = 0;

    // RMS smoothing
    float rmsSmoothed    = 0.0f;
    float clipSmoothed   = 0.0f;

    juce::dsp::Oversampling<float> oversampling { 2, 1,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
};
