#include "KnobLookAndFeel.h"

KnobLookAndFeel::KnobLookAndFeel()
{
}

void KnobLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPosProportional,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider& slider)
{
    const float cx = x + width  * 0.5f;
    const float cy = y + height * 0.5f;
    const float outerRadius = juce::jmin (width, height) * 0.5f - 3.0f;
    const float knobRadius  = outerRadius - 9.0f;

    const float currentAngle = rotaryStartAngle
                             + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // 1. Recessed background ring cavity (dark indentation in chassis)
    g.setColour (juce::Colour (0xff090a0d));
    g.fillEllipse (cx - outerRadius, cy - outerRadius, outerRadius * 2.0f, outerRadius * 2.0f);
    g.setColour (juce::Colour (0xff1c1e24));
    g.drawEllipse (cx - outerRadius, cy - outerRadius, outerRadius * 2.0f, outerRadius * 2.0f, 1.0f);

    // 2. Glowing Neon Value Arc (Atoms / Thermal style)
    const float arcRadius = outerRadius - 4.0f;
    const auto arcBounds = juce::Rectangle<float> (cx - arcRadius, cy - arcRadius,
                                                   arcRadius * 2.0f, arcRadius * 2.0f);

    // Background track
    {
        juce::Path bgArc;
        bgArc.addArc (arcBounds.getX(), arcBounds.getY(), arcBounds.getWidth(), arcBounds.getHeight(),
                      rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0x18FFFFFF));
        g.strokePath (bgArc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Active value arc with warm neon amber glow
    if (currentAngle > rotaryStartAngle + 0.005f)
    {
        juce::Path activeArc;
        activeArc.addArc (arcBounds.getX(), arcBounds.getY(), arcBounds.getWidth(), arcBounds.getHeight(),
                          rotaryStartAngle, currentAngle, true);

        // Soft glow halo
        g.setColour (juce::Colour (0x33FF6A00));
        g.strokePath (activeArc, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Bright core line
        juce::ColourGradient arcGrad (
            juce::Colour (0xffFF5500), cx - arcRadius, cy + arcRadius,
            juce::Colour (0xffFFAA00), cx + arcRadius, cy - arcRadius,
            false);
        g.setGradientFill (arcGrad);
        g.strokePath (activeArc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Knob Body (Anodized matte aluminum with dual-tone lighting)
    // Drop shadow under knob
    juce::ColourGradient dropShadow (
        juce::Colour (0x88000000), cx, cy + knobRadius * 0.5f,
        juce::Colour (0x00000000), cx, cy + knobRadius * 1.3f,
        true);
    g.setGradientFill (dropShadow);
    g.fillEllipse (cx - knobRadius - 3.0f, cy - knobRadius + 2.0f,
                   (knobRadius + 3.0f) * 2.0f, (knobRadius + 3.0f) * 2.0f);

    // Knob face
    juce::ColourGradient faceGrad (
        juce::Colour (0xff2d3038), cx, cy - knobRadius,
        juce::Colour (0xff141519), cx, cy + knobRadius,
        false);
    g.setGradientFill (faceGrad);
    g.fillEllipse (cx - knobRadius, cy - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    // Fine concentric micro-texture ring (brushed metal effect)
    g.setColour (juce::Colour (0x0effffff));
    g.drawEllipse (cx - knobRadius * 0.72f, cy - knobRadius * 0.72f,
                   knobRadius * 1.44f, knobRadius * 1.44f, 1.0f);

    // Outer machined chamfer bevel
    juce::ColourGradient bevelGrad (
        juce::Colour (0x44ffffff), cx, cy - knobRadius,
        juce::Colour (0x11000000), cx, cy + knobRadius,
        false);
    g.setGradientFill (bevelGrad);
    g.drawEllipse (cx - knobRadius, cy - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.5f);

    // 4. Indicator Needle (High contrast vibrant orange line with tip dot)
    {
        const float needleInner = knobRadius * 0.35f;
        const float needleOuter = knobRadius * 0.88f;

        const float x1 = cx + std::sin (currentAngle) * needleInner;
        const float y1 = cy - std::cos (currentAngle) * needleInner;
        const float x2 = cx + std::sin (currentAngle) * needleOuter;
        const float y2 = cy - std::cos (currentAngle) * needleOuter;

        // Needle glow
        g.setColour (juce::Colour (0x55FF6A00));
        g.drawLine (x1, y1, x2, y2, 4.0f);

        // Core needle
        g.setColour (juce::Colour (0xffFFAA00));
        g.drawLine (x1, y1, x2, y2, 2.0f);

        // Precision tip dot
        g.setColour (juce::Colour (0xffFFF3DC));
        g.fillEllipse (x2 - 1.5f, y2 - 1.5f, 3.0f, 3.0f);
    }
}

void KnobLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPos,
                                        float minSliderPos,
                                        float maxSliderPos,
                                        juce::Slider::SliderStyle /*style*/,
                                        juce::Slider& slider)
{
    // High-end recessed groove slider (Teenage Engineering style)
    const float trackH = 8.0f;
    const float trackY = y + (height - trackH) * 0.5f;
    const float trackX = static_cast<float>(x);
    const float trackW = static_cast<float>(width);

    // Recessed dark track
    juce::Rectangle<float> trackRect (trackX, trackY, trackW, trackH);
    g.setColour (juce::Colour (0xff090a0d));
    g.fillRoundedRectangle (trackRect, 4.0f);
    g.setColour (juce::Colour (0xff1f2229));
    g.drawRoundedRectangle (trackRect, 4.0f, 1.0f);

    // Active illuminated bar
    if (sliderPos > trackX + 1.0f)
    {
        juce::Rectangle<float> activeRect (trackX + 1.0f, trackY + 1.5f, sliderPos - trackX, trackH - 3.0f);
        juce::ColourGradient fillGrad (
            juce::Colour (0xffFF6A00), trackX, trackY,
            juce::Colour (0xffFFAA00), sliderPos, trackY,
            false);
        g.setGradientFill (fillGrad);
        g.fillRoundedRectangle (activeRect, 2.5f);
    }

    // Machined slider thumb capsule
    const float thumbW = 18.0f;
    const float thumbH = 24.0f;
    const float thumbX = sliderPos - thumbW * 0.5f;
    const float thumbY = y + (height - thumbH) * 0.5f;
    auto thumbRect = juce::Rectangle<float> (thumbX, thumbY, thumbW, thumbH);

    // Thumb shadow
    g.setColour (juce::Colour (0x66000000));
    g.fillRoundedRectangle (thumbRect.translated (0.0f, 2.0f), 4.0f);

    // Thumb body
    juce::ColourGradient thumbGrad (
        juce::Colour (0xff383c47), thumbX, thumbY,
        juce::Colour (0xff181a20), thumbX, thumbY + thumbH,
        false);
    g.setGradientFill (thumbGrad);
    g.fillRoundedRectangle (thumbRect, 4.0f);

    // Thumb bevel
    g.setColour (juce::Colour (0x33FFFFFF));
    g.drawRoundedRectangle (thumbRect, 4.0f, 1.0f);

    // Thumb center vertical orange marker line
    g.setColour (juce::Colour (0xffFFAA00));
    g.drawLine (sliderPos, thumbY + 4.0f, sliderPos, thumbY + thumbH - 4.0f, 1.5f);
}

void KnobLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font (label.getFont());

        g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          juce::jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());
    }
}
