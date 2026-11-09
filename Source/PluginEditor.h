#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BeerGlassComponent.h"
#include "KnobLookAndFeel.h"

//==============================================================================
class BeerCrusherAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit BeerCrusherAudioProcessorEditor (BeerCrusherAudioProcessor&);
    ~BeerCrusherAudioProcessorEditor() override;

    //==========================================================================
    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    //==========================================================================
    void timerCallback() override;

    //==========================================================================
    BeerCrusherAudioProcessor& processorRef;

    // Animated beer glass
    BeerGlassComponent beerGlass;

    // Custom look and feel
    KnobLookAndFeel laf;

    // Knobs
    juce::Slider crushSlider;
    juce::Slider driveSlider;
    juce::Slider foamSlider;
    juce::Slider wetdrySlider;

    // Labels
    juce::Label crushLabel;
    juce::Label driveLabel;
    juce::Label foamLabel;
    juce::Label wetdryLabel;

    // APVTS attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> crushAttachment;
    std::unique_ptr<Attachment> driveAttachment;
    std::unique_ptr<Attachment> foamAttachment;
    std::unique_ptr<Attachment> wetdryAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeerCrusherAudioProcessorEditor)
};
