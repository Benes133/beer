#pragma once
#include <JuceHeader.h>
#include "BubbleParticle.h"
#include <cmath>
#include <algorithm>
#include <vector>

//==============================================================================
/**
 * Reactor Core / Fermentation Chamber Visualizer
 * Inspired by Output Thermal & Baby Audio Atoms.
 * Renders a high-end circular reactor window filled with turbulent amber liquid,
 * fluid wave contours, rising carbonation bubbles, and a glowing bio-foam head.
 */
class BeerGlassComponent  : public juce::Component,
                             private juce::Timer
{
public:
    BeerGlassComponent();
    ~BeerGlassComponent() override;

    void setFillLevel   (float level);   // 0..1 (Pour / Density)
    void setRmsLevel    (float rms);     // 0..1 (Audio RMS)
    void setClipLevel   (float clip);    // 0..1 (Foam saturation / clipping)
    void setCrushAmount (float crush);   // 0..1 (Particle jitter / crunch)

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    // Drawing stages
    void drawReactorHousing (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCoreLiquid     (juce::Graphics& g, juce::Rectangle<float> coreBounds);
    void drawContourWaves   (juce::Graphics& g, juce::Rectangle<float> coreBounds);
    void drawMicroBubbles   (juce::Graphics& g, juce::Rectangle<float> coreBounds);
    void drawFoamCap        (juce::Graphics& g, juce::Rectangle<float> coreBounds);
    void drawGlassBezel     (juce::Graphics& g, juce::Rectangle<float> coreBounds);
    void drawCenterReticle  (juce::Graphics& g, juce::Rectangle<float> coreBounds);

    void spawnBubble();
    void updatePhysics (float dt);

    double lastTickMs { 0.0 };
    float  animTime   { 0.0f };

    // Parameters
    float fillLevel   { 0.5f };
    float rmsLevel    { 0.0f };
    float clipLevel   { 0.0f };
    float crushAmount { 0.0f };

    // Fluid dynamics & Wave phases
    float wavePhase1  { 0.0f };
    float wavePhase2  { 0.0f };
    float wavePhase3  { 0.0f };
    float fluidTurbulence { 0.0f };
    float prevRms     { 0.0f };

    // Carbonation bubble system
    std::vector<BubbleParticle> bubbles;
    float bubbleSpawnAccum { 0.0f };
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeerGlassComponent)
};
