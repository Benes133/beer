#include "BeerGlassComponent.h"

BeerGlassComponent::BeerGlassComponent()
{
    startTimerHz (60);
    lastTickMs = juce::Time::getMillisecondCounterHiRes();

    // Pre-populate carbonation bubbles
    for (int i = 0; i < 40; ++i)
        spawnBubble();
}

BeerGlassComponent::~BeerGlassComponent()
{
    stopTimer();
}

void BeerGlassComponent::setFillLevel (float level)
{
    fillLevel = juce::jlimit (0.05f, 1.0f, level);
}

void BeerGlassComponent::setRmsLevel (float rms)
{
    rmsLevel = juce::jlimit (0.0f, 1.0f, rms);
}

void BeerGlassComponent::setClipLevel (float clip)
{
    clipLevel = juce::jlimit (0.0f, 1.0f, clip);
}

void BeerGlassComponent::setCrushAmount (float crush)
{
    crushAmount = juce::jlimit (0.0f, 1.0f, crush);
}

void BeerGlassComponent::resized()
{
}

void BeerGlassComponent::timerCallback()
{
    const double nowMs = juce::Time::getMillisecondCounterHiRes();
    float dt = static_cast<float>((nowMs - lastTickMs) * 0.001);
    lastTickMs = nowMs;

    if (dt <= 0.0f || dt > 0.1f)
        dt = 0.016f;

    animTime += dt;
    updatePhysics (dt);
    repaint();
}

void BeerGlassComponent::updatePhysics (float dt)
{
    // Fluid turbulence increases with audio impact and crush amount
    const float deltaRms = rmsLevel - prevRms;
    if (deltaRms > 0.04f)
        fluidTurbulence += deltaRms * 3.5f;

    prevRms = rmsLevel;

    // Damp turbulence back to idle
    fluidTurbulence = juce::jmax (0.0f, fluidTurbulence - dt * 2.2f);

    const float speed = 1.0f + rmsLevel * 3.0f + fluidTurbulence + crushAmount * 2.0f;
    wavePhase1 += dt * speed * 2.2f;
    wavePhase2 += dt * speed * 1.5f;
    wavePhase3 += dt * speed * 0.8f;

    // Update Bubbles
    const auto b = getLocalBounds().toFloat();
    const float radius = juce::jmin (b.getWidth(), b.getHeight()) * 0.42f;
    const auto center = b.getCentre();
    const float surfaceY = center.y + radius * (1.0f - 2.0f * fillLevel);

    for (auto& bubble : bubbles)
    {
        bubble.wobblePhase += bubble.wobbleFreq * dt;
        bubble.x += std::sin (bubble.wobblePhase) * bubble.wobbleAmp * dt * 8.0f;
        bubble.y -= (bubble.speed + rmsLevel * 40.0f) * dt;

        // Fade out near surface or edge of circular chamber
        const float distFromCenter = std::hypot (bubble.x - center.x, bubble.y - center.y);
        const float distToSurface  = bubble.y - surfaceY;

        if (distFromCenter > radius * 0.92f || distToSurface < 10.0f)
            bubble.alpha -= dt * 3.0f;
    }

    bubbles.erase (
        std::remove_if (bubbles.begin(), bubbles.end(),
            [](const BubbleParticle& p) { return p.y <= 0.0f || p.alpha <= 0.02f; }),
        bubbles.end());

    // Spawn new carbonation micro-bubbles
    const float spawnRate = 20.0f + fillLevel * 30.0f + crushAmount * 60.0f + rmsLevel * 40.0f;
    bubbleSpawnAccum += spawnRate * dt;

    while (bubbleSpawnAccum >= 1.0f && bubbles.size() < 120)
    {
        spawnBubble();
        bubbleSpawnAccum -= 1.0f;
    }
}

void BeerGlassComponent::spawnBubble()
{
    const auto b = getLocalBounds().toFloat();
    const float radius = juce::jmin (b.getWidth(), b.getHeight()) * 0.42f;
    const auto center = b.getCentre();

    BubbleParticle p;
    // Spawn mostly in lower half inside circle
    const float angle = rng.nextFloat() * juce::MathConstants<float>::twoPi;
    const float dist  = rng.nextFloat() * (radius * 0.82f);

    p.x = center.x + std::cos (angle) * dist;
    p.y = center.y + std::abs (std::sin (angle)) * dist * 0.9f + radius * 0.1f;

    p.size = 1.5f + rng.nextFloat() * (3.5f + crushAmount * 4.0f);
    p.speed = 25.0f + rng.nextFloat() * 45.0f;
    p.wobbleAmp = 1.5f + rng.nextFloat() * 3.0f;
    p.wobbleFreq = 2.0f + rng.nextFloat() * 4.0f;
    p.wobblePhase = rng.nextFloat() * juce::MathConstants<float>::twoPi;
    p.alpha = 0.4f + rng.nextFloat() * 0.5f;
    p.lifetime = 1.0f;

    bubbles.push_back (p);
}

// ─── Render Pipeline ──────────────────────────────────────────────────────────

void BeerGlassComponent::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float minDim = juce::jmin (b.getWidth(), b.getHeight());
    const float coreSize = minDim * 0.86f;
    auto coreBounds = b.withSizeKeepingCentre (coreSize, coreSize);

    drawReactorHousing (g, b);

    // Clip rendering strictly to core circular chamber
    juce::Path circleClip;
    circleClip.addEllipse (coreBounds);

    {
        juce::Graphics::ScopedSaveState sss (g);
        g.reduceClipRegion (circleClip);

        drawCoreLiquid   (g, coreBounds);
        drawContourWaves (g, coreBounds);
        drawMicroBubbles (g, coreBounds);
        drawFoamCap      (g, coreBounds);
    }

    // Outer glass & bezel hardware drawn unclipped
    drawGlassBezel    (g, coreBounds);
    drawCenterReticle (g, coreBounds);
}

void BeerGlassComponent::drawReactorHousing (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const auto center = bounds.getCentre();
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.48f;

    // Atmospheric dark backdrop with deep warm amber glow radiating outwards (Thermal style)
    juce::ColourGradient deepGlow (
        juce::Colour (0x33FF6A00), center.x, center.y,
        juce::Colour (0x00000000), center.x, center.y - radius * 1.3f,
        true);
    deepGlow.addColour (0.5, juce::Colour (0x12FF8C00));
    g.setGradientFill (deepGlow);
    g.fillEllipse (bounds.expanded (30.0f));

    // Concentric topographic rings in chassis background
    g.setColour (juce::Colour (0x0cFFFFFF));
    for (int i = 1; i <= 3; ++i)
    {
        const float r = radius + i * 16.0f;
        g.drawEllipse (center.x - r, center.y - r, r * 2.0f, r * 2.0f, 1.0f);
    }
}

void BeerGlassComponent::drawCoreLiquid (juce::Graphics& g, juce::Rectangle<float> coreBounds)
{
    const auto center = coreBounds.getCentre();
    const float radius = coreBounds.getWidth() * 0.5f;

    // Dark void at top, boiling liquid towards bottom
    g.fillAll (juce::Colour (0xff0c0d10));

    // Liquid surface coordinate
    const float surfaceY = center.y + radius * (1.0f - 2.0f * fillLevel);

    juce::Path liquidPath;
    liquidPath.addRectangle (coreBounds.getX() - 10.0f, surfaceY, coreBounds.getWidth() + 20.0f, coreBounds.getHeight());

    // Multi-stop liquid gradient: deep mahogany ale to glowing liquid gold
    juce::ColourGradient beerGrad (
        juce::Colour (0xffFFB800), center.x, surfaceY,
        juce::Colour (0xff7A2000), center.x, coreBounds.getBottom(),
        false);
    beerGrad.addColour (0.25, juce::Colour (0xffFF7700));
    beerGrad.addColour (0.65, juce::Colour (0xffB53200));

    g.setGradientFill (beerGrad);
    g.fillPath (liquidPath);

    // Internal back-light hot spot
    const float hotR = radius * (0.4f + rmsLevel * 0.2f);
    juce::ColourGradient hotSpot (
        juce::Colour (0x55FFAA00), center.x, center.y + radius * 0.3f,
        juce::Colour (0x00FF5500), center.x, center.y + radius * 0.3f + hotR,
        true);
    g.setGradientFill (hotSpot);
    g.fillEllipse (center.x - hotR, center.y + radius * 0.3f - hotR, hotR * 2.0f, hotR * 2.0f);
}

void BeerGlassComponent::drawContourWaves (juce::Graphics& g, juce::Rectangle<float> coreBounds)
{
    const auto center = coreBounds.getCentre();
    const float radius = coreBounds.getWidth() * 0.5f;
    const float baseSurfaceY = center.y + radius * (1.0f - 2.0f * fillLevel);

    // Organic wavy contours layered through the liquid (Thermal topographic style)
    const int numLayers = 4;
    const float amplitudes[4] = { 7.0f, 13.0f, 18.0f, 24.0f };
    const float opacities[4]  = { 0.5f,  0.35f,  0.22f,  0.14f };

    for (int layer = 0; layer < numLayers; ++layer)
    {
        const float amp = (amplitudes[layer] + rmsLevel * 14.0f + fluidTurbulence * 8.0f) * (0.4f + fillLevel * 0.6f);
        const float yOffset = layer * 18.0f;
        const float curY = baseSurfaceY + yOffset;

        juce::Path wave;
        const int steps = 40;
        const float stepWidth = coreBounds.getWidth() / steps;

        for (int i = 0; i <= steps; ++i)
        {
            const float x = coreBounds.getX() + i * stepWidth;
            const float normX = static_cast<float>(i) / steps;

            const float y = curY
                + std::sin (normX * 6.28f * 1.5f + wavePhase1 + layer * 0.9f) * amp
                + std::sin (normX * 6.28f * 3.0f - wavePhase2 + layer * 1.4f) * (amp * 0.45f)
                + std::cos (normX * 6.28f * 0.8f + wavePhase3) * (amp * 0.25f);

            if (i == 0)
                wave.startNewSubPath (x, y);
            else
                wave.lineTo (x, y);
        }

        // Draw flowing wave contour
        g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.78f, 0.2f, opacities[layer]));
        g.strokePath (wave, juce::PathStrokeType (1.8f - layer * 0.3f));
    }
}

void BeerGlassComponent::drawMicroBubbles (juce::Graphics& g, juce::Rectangle<float> /*coreBounds*/)
{
    for (const auto& bubble : bubbles)
    {
        // Glowing bubble dot
        const float a = bubble.alpha;
        g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.88f, 0.4f, a * 0.8f));
        g.fillEllipse (bubble.x - bubble.size * 0.5f,
                       bubble.y - bubble.size * 0.5f,
                       bubble.size, bubble.size);

        // Subtly larger halo for crunchy high-speed bubbles
        if (crushAmount > 0.3f || rmsLevel > 0.4f)
        {
            g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.5f, 0.0f, a * 0.25f));
            g.drawEllipse (bubble.x - bubble.size,
                           bubble.y - bubble.size,
                           bubble.size * 2.0f, bubble.size * 2.0f, 1.0f);
        }
    }
}

void BeerGlassComponent::drawFoamCap (juce::Graphics& g, juce::Rectangle<float> coreBounds)
{
    const auto center = coreBounds.getCentre();
    const float radius = coreBounds.getWidth() * 0.5f;
    const float surfaceY = center.y + radius * (1.0f - 2.0f * fillLevel);

    const float foamHeight = 16.0f + clipLevel * 35.0f + fillLevel * 10.0f;
    const float foamTop = surfaceY - foamHeight;

    // Glowing organic foam body (creamy ivory to warm golden yellow)
    juce::Path foamPath;
    const int steps = 30;
    const float stepWidth = coreBounds.getWidth() / steps;

    // Top edge with turbulent bubbly wobbles
    for (int i = 0; i <= steps; ++i)
    {
        const float x = coreBounds.getX() + i * stepWidth;
        const float normX = static_cast<float>(i) / steps;
        const float wobble = std::sin (normX * 18.0f + wavePhase1 * 1.5f) * (3.0f + clipLevel * 6.0f);
        const float y = foamTop + wobble;

        if (i == 0)
            foamPath.startNewSubPath (x, y);
        else
            foamPath.lineTo (x, y);
    }

    foamPath.lineTo (coreBounds.getRight(), surfaceY + 10.0f);
    foamPath.lineTo (coreBounds.getX(), surfaceY + 10.0f);
    foamPath.closeSubPath();

    juce::ColourGradient foamGrad (
        juce::Colour (0xffFFF8EE), center.x, foamTop,
        juce::Colour (0xddFFD588), center.x, surfaceY + 6.0f,
        false);
    g.setGradientFill (foamGrad);
    g.fillPath (foamPath);

    // If clipping / maximizing heavily, add electric foam corona
    if (clipLevel > 0.15f)
    {
        g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.4f, 0.0f, clipLevel * 0.7f));
        g.strokePath (foamPath, juce::PathStrokeType (2.5f + clipLevel * 2.0f));
    }
}

void BeerGlassComponent::drawGlassBezel (juce::Graphics& g, juce::Rectangle<float> coreBounds)
{
    const auto center = coreBounds.getCentre();
    const float radius = coreBounds.getWidth() * 0.5f;

    // 1. Inner vignette shadow inside glass
    juce::ColourGradient innerShadow (
        juce::Colour (0x00000000), center.x, center.y,
        juce::Colour (0x99000000), center.x, center.y - radius,
        true);
    innerShadow.addColour (0.75, juce::Colour (0x10000000));
    innerShadow.addColour (0.95, juce::Colour (0xbb000000));
    g.setGradientFill (innerShadow);
    g.fillEllipse (coreBounds);

    // 2. High-precision Bezel rings (Baby Audio / Output style)
    // Dark metallic rim
    g.setColour (juce::Colour (0xff1c1e24));
    g.drawEllipse (coreBounds.expanded (1.0f), 3.0f);

    // Bright outer chamfer bevel
    juce::ColourGradient chamfer (
        juce::Colour (0x88FFFFFF), center.x - radius, center.y - radius,
        juce::Colour (0x22111111), center.x + radius, center.y + radius,
        false);
    g.setGradientFill (chamfer);
    g.drawEllipse (coreBounds.expanded (3.0f), 1.5f);

    // Outer dark groove
    g.setColour (juce::Colour (0xff0a0b0d));
    g.drawEllipse (coreBounds.expanded (6.0f), 2.0f);

    // Glowing Neon Accent Ring around Reactor
    juce::Colour neonColour = juce::Colour (0xffff6a00).interpolatedWith (
        juce::Colour (0xffffcc00), rmsLevel);
    g.setColour (neonColour.withAlpha (0.4f + rmsLevel * 0.5f));
    g.drawEllipse (coreBounds.expanded (9.0f), 1.5f);

    // Precision tick marks around circumference
    const int ticks = 48;
    for (int i = 0; i < ticks; ++i)
    {
        const float angle = (i / static_cast<float>(ticks)) * juce::MathConstants<float>::twoPi;
        const bool isMajor = (i % 6 == 0);
        const float len = isMajor ? 6.0f : 3.0f;
        const float r1 = radius + 12.0f;
        const float r2 = r1 + len;

        const float x1 = center.x + std::cos (angle) * r1;
        const float y1 = center.y + std::sin (angle) * r1;
        const float x2 = center.x + std::cos (angle) * r2;
        const float y2 = center.y + std::sin (angle) * r2;

        g.setColour (isMajor ? juce::Colour (0x99FF8800) : juce::Colour (0x28FFFFFF));
        g.drawLine (x1, y1, x2, y2, isMajor ? 1.5f : 1.0f);
    }
}

void BeerGlassComponent::drawCenterReticle (juce::Graphics& g, juce::Rectangle<float> coreBounds)
{
    const auto center = coreBounds.getCentre();

    // Central precision crosshair / HUD ring
    const float reticleR = 24.0f + rmsLevel * 8.0f;
    g.setColour (juce::Colour (0x44FFAA00));
    g.drawEllipse (center.x - reticleR, center.y - reticleR, reticleR * 2.0f, reticleR * 2.0f, 1.0f);

    // Crosshair ticks
    g.setColour (juce::Colour (0x66FF8800));
    g.drawLine (center.x - reticleR - 6.0f, center.y, center.x - reticleR + 4.0f, center.y, 1.0f);
    g.drawLine (center.x + reticleR - 4.0f, center.y, center.x + reticleR + 6.0f, center.y, 1.0f);
    g.drawLine (center.x, center.y - reticleR - 6.0f, center.x, center.y - reticleR + 4.0f, 1.0f);
    g.drawLine (center.x, center.y + reticleR - 4.0f, center.x, center.y + reticleR + 6.0f, 1.0f);

    // Center focal dot
    g.setColour (juce::Colour (0xddFFFFFF));
    g.fillEllipse (center.x - 2.0f, center.y - 2.0f, 4.0f, 4.0f);
}
