#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BeerGlassComponent.h"
#include "KnobLookAndFeel.h"

//==============================================================================
/**
 * Modern High-End Audio UI for BeerCrusher.
 * Layout inspired by Output Thermal & Teenage Engineering:
 * - Left: Giant Fermentation Reactor Visualizer (Liquid wave contours & bubbles)
 * - Right: Tactile Hardware Control Rack with Big Knob & Groove Sliders
 */
class BeerCrusherAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit BeerCrusherAudioProcessorEditor (BeerCrusherAudioProcessor&);
    ~BeerCrusherAudioProcessorEditor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    void timerCallback() override;

    BeerCrusherAudioProcessor& processorRef;

    // Centerpiece: Reactor Window
    BeerGlassComponent reactorCore;

    // Custom LookAndFeel
    KnobLookAndFeel laf;

    // Hardware Controls
    juce::Slider driveSlider;    // Big Knob (POUR / DENSITY)
    juce::Slider crushSlider;    // Rotary Knob (CRUSH)
    juce::Slider foamSlider;     // Rotary Knob (FOAM)
    juce::Slider wetdrySlider;   // Horizontal Groove Slider (MIX)

    // Control Labels
    juce::Label driveLabel;
    juce::Label crushLabel;
    juce::Label foamLabel;
    juce::Label wetdryLabel;

    // Value readouts
    juce::Label driveVal;
    juce::Label crushVal;
    juce::Label foamVal;
    juce::Label wetdryVal;

    // APVTS Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> driveAttachment;
    std::unique_ptr<Attachment> crushAttachment;
    std::unique_ptr<Attachment> foamAttachment;
    std::unique_ptr<Attachment> wetdryAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeerCrusherAudioProcessorEditor)
};
