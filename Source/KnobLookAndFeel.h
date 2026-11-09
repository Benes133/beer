#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
 * High-End Industrial LookAndFeel inspired by Teenage Engineering, Output Thermal,
 * and Baby Audio Atoms.
 * - Anodized matte dark metal knobs with micro-bevels & amber indicator needle.
 * - Glowing radial neon gauge arc.
 * - Recessed hardware-groove linear sliders with illuminated track.
 */
class KnobLookAndFeel  : public juce::LookAndFeel_V4
{
public:
    KnobLookAndFeel();
    ~KnobLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const juce::Slider::SliderStyle,
                           juce::Slider&) override;

    void drawLabel (juce::Graphics&, juce::Label&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobLookAndFeel)
};
