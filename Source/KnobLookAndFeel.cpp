#include "KnobLookAndFeel.h"

void KnobLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPosProportional,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider& /*slider*/)
{
    const float cx = x + width  * 0.5f;
    const float cy = y + height * 0.5f;
    const float radius = juce::jmin(width, height) * 0.5f - 4.0f;
    const float currentAngle = rotaryStartAngle
                               + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // ── 1. Outer glow (soft shadow) ──────────────────────────────────────────
    for (int i = 4; i >= 1; --i)
    {
        float expand = i * 2.5f;
        float alpha  = 0.04f * (5 - i);
        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.718f, 0.012f, alpha));
        g.fillEllipse(cx - radius - expand,
                      cy - radius - expand,
                      (radius + expand) * 2.0f,
                      (radius + expand) * 2.0f);
    }

    // ── 2. Main knob body — radial gradient (dark center → copper edge) ──────
    juce::ColourGradient bodyGrad(
        juce::Colour(0xff2A2A40), cx, cy,
        juce::Colour(0xffB87333), cx, cy - radius,
        true /* radial */);
    bodyGrad.addColour(0.75, juce::Colour(0xff8B5A2B));
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    // Copper/brass outer ring
    juce::ColourGradient ringGrad(
        juce::Colour(0xffB87333), cx - radius, cy,
        juce::Colour(0xffFFD700), cx + radius, cy,
        false);
    g.setGradientFill(ringGrad);
    g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 2.5f);

    // ── 3. Amber arc — start angle to current angle ───────────────────────────
    {
        juce::Path arcPath;
        arcPath.addArc(cx - radius + 5.0f, cy - radius + 5.0f,
                       (radius - 5.0f) * 2.0f, (radius - 5.0f) * 2.0f,
                       rotaryStartAngle, currentAngle, true);

        juce::PathStrokeType stroke(3.0f, juce::PathStrokeType::curved,
                                    juce::PathStrokeType::rounded);
        g.setColour(juce::Colour(0xffFFB703));
        g.strokePath(arcPath, stroke);
    }

    // ── 4. Remaining (dark) arc — current angle to end angle ─────────────────
    if (currentAngle < rotaryEndAngle)
    {
        juce::Path remainPath;
        remainPath.addArc(cx - radius + 5.0f, cy - radius + 5.0f,
                          (radius - 5.0f) * 2.0f, (radius - 5.0f) * 2.0f,
                          currentAngle, rotaryEndAngle, true);

        juce::PathStrokeType stroke(1.0f, juce::PathStrokeType::curved,
                                    juce::PathStrokeType::rounded);
        g.setColour(juce::Colour(0xff333333));
        g.strokePath(remainPath, stroke);
    }

    // ── 5. Center indicator line (white, rotated to current angle) ────────────
    {
        const float lineLen = radius * 0.55f;
        const float lineX = cx + lineLen * std::sin(currentAngle);
        const float lineY = cy - lineLen * std::cos(currentAngle);

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.drawLine(cx, cy, lineX, lineY, 2.0f);

        // Small center dot
        g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
    }
}
