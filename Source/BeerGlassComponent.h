#pragma once

#include <JuceHeader.h>
#include "BubbleParticle.h"

//==============================================================================
/**
 * Animated beer glass component rendered at 60 fps.
 * Call the setter methods from the editor's timer to drive the animation.
 */
class BeerGlassComponent  : public juce::Component,
                             private juce::Timer
{
public:
    BeerGlassComponent();
    ~BeerGlassComponent() override;

    //==========================================================================
    // Setters (called from PluginEditor timerCallback)
    void setFillLevel   (float level);   // 0..1
    void setRmsLevel    (float rms);     // 0..1
    void setClipLevel   (float clip);    // 0..1
    void setCrushAmount (float crush);   // 0..1

    //==========================================================================
    void paint (juce::Graphics&) override;
    void resized() override {}

private:
    //==========================================================================
    // Timer
    void timerCallback() override;
    double lastTickMs { 0.0 };
    float  animTime   { 0.0f };

    //==========================================================================
    // State
    float fillLevel   { 0.5f };
    float rmsLevel    { 0.0f };
    float clipLevel   { 0.0f };
    float crushAmount { 0.0f };

    //==========================================================================
    // Wave physics
    float waveDisplace   { 0.0f };
    float waveVelocity   { 0.0f };
    float wavePhase      { 0.0f };
    float prevRmsLevel   { 0.0f };
    float bubbleSpawnAccum { 0.0f };

    static constexpr float waveSpring { 18.0f };
    static constexpr float waveDamp   {  4.0f };

    //==========================================================================
    // Bubbles
    std::vector<BubbleParticle> bubbles;

    //==========================================================================
    // Layout
    juce::Rectangle<float> glassRect;

    //==========================================================================
    // Draw helpers
    void drawBackground   (juce::Graphics&);
    void drawGlass        (juce::Graphics&);
    void drawLiquid       (juce::Graphics&);
    void drawWaveSurface  (juce::Graphics&);
    void drawBubbles      (juce::Graphics&);
    void drawFoam         (juce::Graphics&);
    void drawGlassShine   (juce::Graphics&);
    void drawOverflowFoam (juce::Graphics&);

    //==========================================================================
    // Simulation
    void updateWave    (float dt);
    void spawnBubble   ();
    void updateBubbles (float dt);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeerGlassComponent)
};
