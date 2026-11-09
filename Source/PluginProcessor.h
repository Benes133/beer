#pragma once

#include <JuceHeader.h>
#include "BeerDSP.h"

//==============================================================================
class BeerCrusherAudioProcessor  : public juce::AudioProcessor
{
public:
    //==========================================================================
    BeerCrusherAudioProcessor();
    ~BeerCrusherAudioProcessor() override;

    //==========================================================================
    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==========================================================================
    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    BeerDSP dsp;

private:
    // Raw parameter pointers for lock-free access in processBlock
    std::atomic<float>* crushParam  { nullptr };
    std::atomic<float>* driveParam  { nullptr };
    std::atomic<float>* foamParam   { nullptr };
    std::atomic<float>* wetdryParam { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeerCrusherAudioProcessor)
};
