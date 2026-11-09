#include "BeerGlassComponent.h"

BeerGlassComponent::BeerGlassComponent()
{
    startTimerHz(60);
    lastTickMs = juce::Time::getMillisecondCounterHiRes();

    // Seed initial bubbles
    for (int i = 0; i < 8; ++i)
        spawnBubble();
}

BeerGlassComponent::~BeerGlassComponent()
{
    stopTimer();
}

void BeerGlassComponent::setFillLevel(float level)
{
    fillLevel = juce::jlimit(0.0f, 1.0f, level);
}

void BeerGlassComponent::setRmsLevel(float rms)
{
    rmsLevel = juce::jlimit(0.0f, 1.0f, rms);
}

void BeerGlassComponent::setClipLevel(float clip)
{
    clipLevel = juce::jlimit(0.0f, 1.0f, clip);
}

void BeerGlassComponent::setCrushAmount(float crush)
{
    crushAmount = juce::jlimit(0.0f, 1.0f, crush);
}

void BeerGlassComponent::timerCallback()
{
    double nowMs = juce::Time::getMillisecondCounterHiRes();
    float dt = static_cast<float>((nowMs - lastTickMs) * 0.001);
    lastTickMs = nowMs;

    animTime += dt;

    updateWave(dt);
    updateBubbles(dt);

    repaint();
}

// ─── Paint ────────────────────────────────────────────────────────────────────

void BeerGlassComponent::paint(juce::Graphics& g)
{
    glassRect = getLocalBounds().toFloat().withSizeKeepingCentre(200.0f, 300.0f);

    // Drunk mode: gentle sway after 10 min
    if (animTime > 600.0f)
    {
        float sway = 4.0f * std::sin(animTime * 0.4f);
        glassRect.translate(sway, 0.0f);
    }

    drawBackground(g);
    drawLiquid(g);
    drawWaveSurface(g);
    drawBubbles(g);
    drawFoam(g);
    drawGlass(g);
    drawGlassShine(g);
    drawOverflowFoam(g);
}

// ─── Draw helpers ─────────────────────────────────────────────────────────────

void BeerGlassComponent::drawBackground(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121316));

    // Subtle radial glow behind glass
    juce::ColourGradient glow(
        juce::Colour(0x22FFB703),
        glassRect.getCentreX(), glassRect.getCentreY(),
        juce::Colour(0x00000000),
        glassRect.getCentreX(), glassRect.getBottom() + 60.0f,
        true);
    g.setGradientFill(glow);
    g.fillEllipse(glassRect.expanded(60.0f, 40.0f));
}

void BeerGlassComponent::drawGlass(juce::Graphics& g)
{
    // Mug shape: slightly narrower at bottom, wider at rim
    float left   = glassRect.getX();
    float right  = glassRect.getRight();
    float top    = glassRect.getY();
    float bottom = glassRect.getBottom();
    float taper  = 12.0f; // px inset at bottom

    juce::Path mug;
    mug.startNewSubPath(left + taper, bottom);
    mug.quadraticTo(left,  bottom, left,  bottom - 10.0f);        // bottom-left round
    mug.lineTo(left,       top + 10.0f);
    mug.quadraticTo(left,  top, left + 8.0f, top);                // top-left round
    mug.lineTo(right - 8.0f, top);
    mug.quadraticTo(right, top, right, top + 10.0f);              // top-right round
    mug.lineTo(right,      bottom - 10.0f);
    mug.quadraticTo(right, bottom, right - taper, bottom);        // bottom-right round
    mug.closeSubPath();

    // Glass rim highlight at top
    juce::Path rim;
    rim.addRoundedRectangle(left - 4.0f, top - 6.0f, glassRect.getWidth() + 8.0f, 12.0f, 4.0f);

    g.setColour(juce::Colour(0x44FFFFFF));
    g.strokePath(mug, juce::PathStrokeType(2.0f));
    g.setColour(juce::Colour(0x66FFFFFF));
    g.strokePath(rim, juce::PathStrokeType(2.5f));
}

void BeerGlassComponent::drawLiquid(juce::Graphics& g)
{
    if (fillLevel <= 0.0f) return;

    float left   = glassRect.getX();
    float right  = glassRect.getRight();
    float bottom = glassRect.getBottom();
    float glassH = glassRect.getHeight();
    float taper  = 12.0f;

    float liquidTop = bottom - glassH * fillLevel + waveDisplace * 8.0f;
    liquidTop = juce::jlimit(glassRect.getY(), bottom, liquidTop);

    juce::Path liquid;
    liquid.startNewSubPath(left + taper, bottom);
    liquid.lineTo(left,       liquidTop);
    liquid.lineTo(right,      liquidTop);
    liquid.lineTo(right - taper, bottom);
    liquid.closeSubPath();

    juce::ColourGradient grad(
        juce::Colour(0xff9B3800), glassRect.getCentreX(), bottom,
        juce::Colour(0xffFFB703), glassRect.getCentreX(), glassRect.getY(),
        false);
    g.setGradientFill(grad);
    g.fillPath(liquid);
}

void BeerGlassComponent::drawWaveSurface(juce::Graphics& g)
{
    if (fillLevel <= 0.0f) return;

    float left   = glassRect.getX();
    float right  = glassRect.getRight();
    float bottom = glassRect.getBottom();
    float glassH = glassRect.getHeight();

    float surfaceY = bottom - glassH * fillLevel + waveDisplace * 8.0f;
    surfaceY = juce::jlimit(glassRect.getY(), bottom, surfaceY);

    float amplitude = 3.0f + rmsLevel * 8.0f;
    float width = glassRect.getWidth();

    juce::Path wave;
    wave.startNewSubPath(left, surfaceY);

    int steps = 60;
    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / steps;
        float x = left + t * width;
        float y = surfaceY
                + amplitude * std::sin(t * juce::MathConstants<float>::twoPi * 2.0f + wavePhase)
                + amplitude * 0.5f * std::sin(t * juce::MathConstants<float>::twoPi * 3.5f + wavePhase * 1.3f)
                + amplitude * 0.3f * std::sin(t * juce::MathConstants<float>::twoPi * 5.0f + wavePhase * 0.7f);

        if (i == 0) wave.startNewSubPath(x, y);
        else        wave.lineTo(x, y);
    }

    g.setColour(juce::Colour(0x88FFD000));
    g.strokePath(wave, juce::PathStrokeType(1.5f));
}

void BeerGlassComponent::drawBubbles(juce::Graphics& g)
{
    for (auto& b : bubbles)
    {
        float alpha = b.alpha * 0.7f;
        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.78f, 0.0f, alpha));
        g.drawEllipse(b.x - b.size * 0.5f, b.y - b.size * 0.5f, b.size, b.size, 1.0f);
    }
}

void BeerGlassComponent::drawFoam(juce::Graphics& g)
{
    if (fillLevel <= 0.02f) return;

    float left   = glassRect.getX();
    float right  = glassRect.getRight();
    float bottom = glassRect.getBottom();
    float glassH = glassRect.getHeight();

    float surfaceY = bottom - glassH * fillLevel + waveDisplace * 8.0f;
    surfaceY = juce::jlimit(glassRect.getY(), bottom, surfaceY);

    float foamThickness = 10.0f + fillLevel * 20.0f + clipLevel * 15.0f;
    float foamTop = surfaceY - foamThickness;

    // Draw several overlapping foam blobs
    juce::Random rng(42);
    float blobW = 22.0f;
    float step  = 14.0f;
    float width = right - left;
    int numBlobs = static_cast<int>(width / step) + 2;

    g.setColour(juce::Colour(0xeeFFF8EB));
    for (int i = 0; i < numBlobs; ++i)
    {
        float bx = left + i * step - 4.0f;
        float by = foamTop + rng.nextFloat() * 4.0f;
        float bh = foamThickness * (0.6f + rng.nextFloat() * 0.4f);
        g.fillEllipse(bx, by, blobW, bh * 0.9f);
    }
}

void BeerGlassComponent::drawGlassShine(juce::Graphics& g)
{
    float left = glassRect.getX();
    float top  = glassRect.getY();
    float h    = glassRect.getHeight();

    // First highlight strip
    juce::Path strip1;
    strip1.addRoundedRectangle(left + 8.0f, top + 10.0f, 8.0f, h * 0.55f, 4.0f);
    juce::ColourGradient sh1(
        juce::Colour(0x55FFFFFF), left + 8.0f, top + 10.0f,
        juce::Colour(0x00FFFFFF), left + 8.0f, top + h * 0.65f,
        false);
    g.setGradientFill(sh1);
    g.fillPath(strip1);

    // Second narrower strip
    juce::Path strip2;
    strip2.addRoundedRectangle(left + 20.0f, top + 20.0f, 4.0f, h * 0.3f, 2.0f);
    juce::ColourGradient sh2(
        juce::Colour(0x33FFFFFF), left + 20.0f, top + 20.0f,
        juce::Colour(0x00FFFFFF), left + 20.0f, top + h * 0.5f,
        false);
    g.setGradientFill(sh2);
    g.fillPath(strip2);
}

void BeerGlassComponent::drawOverflowFoam(juce::Graphics& g)
{
    if (clipLevel <= 0.5f) return;

    float overflowFactor = (clipLevel - 0.5f) * 2.0f; // 0..1
    float left  = glassRect.getX();
    float right = glassRect.getRight();
    float top   = glassRect.getY();

    // Drips down the outside of the glass
    juce::Random rng(99);
    int numDrips = static_cast<int>(overflowFactor * 5) + 1;
    for (int i = 0; i < numDrips; ++i)
    {
        float dripX = (rng.nextFloat() < 0.5f)
            ? left  - 4.0f + rng.nextFloat() * 8.0f
            : right - 4.0f + rng.nextFloat() * 8.0f;
        float dripLen = 20.0f + rng.nextFloat() * 60.0f * overflowFactor;
        float dripW   = 6.0f  + rng.nextFloat() * 8.0f;

        juce::ColourGradient drip(
            juce::Colour(0xccFFF8EB), dripX, top,
            juce::Colour(0x00FFF8EB), dripX, top + dripLen,
            false);
        g.setGradientFill(drip);
        g.fillRoundedRectangle(dripX - dripW * 0.5f, top, dripW, dripLen, dripW * 0.4f);
    }
}

// ─── Physics / simulation ─────────────────────────────────────────────────────

void BeerGlassComponent::updateWave(float dt)
{
    // Beat detection: significant RMS spike
    float rmsDelta = rmsLevel - prevRmsLevel;
    if (rmsDelta > 0.08f)
        waveVelocity += rmsDelta * 2.0f;

    prevRmsLevel = rmsLevel;

    // Spring-damper integration
    float springForce = -waveSpring * waveDisplace;
    float dampForce   = -waveDamp   * waveVelocity;
    waveVelocity += (springForce + dampForce) * dt;
    waveDisplace += waveVelocity * dt;

    // Advance wave phase
    wavePhase += dt * (2.0f + rmsLevel * 4.0f);
    if (wavePhase > juce::MathConstants<float>::twoPi * 100.0f)
        wavePhase -= juce::MathConstants<float>::twoPi * 100.0f;
}

void BeerGlassComponent::spawnBubble()
{
    float left   = glassRect.getX()   + 14.0f;
    float width  = glassRect.getWidth() - 28.0f;
    float bottom = glassRect.getBottom() - 4.0f;

    if (width <= 0.0f) return;

    juce::Random rng;
    BubbleParticle b;
    b.x           = left + rng.nextFloat() * width;
    b.y           = bottom;
    b.size        = 2.0f + rng.nextFloat() * 4.0f;
    b.speed       = 20.0f + rng.nextFloat() * 40.0f;
    b.wobbleAmp   = 2.0f  + rng.nextFloat() * 3.0f;
    b.wobbleFreq  = 2.0f  + rng.nextFloat() * 3.0f;
    b.wobblePhase = rng.nextFloat() * juce::MathConstants<float>::twoPi;
    b.alpha       = 1.0f;
    bubbles.push_back(b);
}

void BeerGlassComponent::updateBubbles(float dt)
{
    float bottom = glassRect.getBottom() - 4.0f;
    float glassH = glassRect.getHeight();
    float surfaceY = bottom - glassH * fillLevel;

    for (auto& b : bubbles)
    {
        b.wobblePhase += b.wobbleFreq * dt;
        b.x += std::sin(b.wobblePhase) * b.wobbleAmp * dt * 10.0f;
        b.y -= b.speed * dt;
        // Fade as they approach surface
        float distToSurface = b.y - surfaceY;
        b.alpha = juce::jlimit(0.0f, 1.0f, distToSurface / 20.0f);
    }

    // Remove popped bubbles
    bubbles.erase(
        std::remove_if(bubbles.begin(), bubbles.end(),
            [&](const BubbleParticle& b) { return b.y <= surfaceY || b.alpha <= 0.0f; }),
        bubbles.end());

    // Spawn rate scales with crush and fill
    float spawnRate = fillLevel * (0.5f + crushAmount * 3.0f);
    bubbleSpawnAccum += spawnRate * dt;

    while (bubbleSpawnAccum >= 1.0f && bubbles.size() < 80)
    {
        spawnBubble();
        bubbleSpawnAccum -= 1.0f;
    }
    if (bubbles.size() >= 80)
        bubbleSpawnAccum = 0.0f;
}
