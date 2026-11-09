#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Custom JUCE LookAndFeel_V4 that renders beer-themed copper/brass knobs
 * with an animated amber glow arc.
 */
class KnobLookAndFeel  : public juce::LookAndFeel_V4
{
public:
    KnobLookAndFeel()  = default;
    ~KnobLookAndFeel() = default;

    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobLookAndFeel)
};
