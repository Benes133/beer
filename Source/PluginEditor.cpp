#include "PluginEditor.h"

static void configureKnob (juce::Slider& slider, KnobLookAndFeel& laf)
{
    slider.setLookAndFeel (&laf);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
}

static void configureSlider (juce::Slider& slider, KnobLookAndFeel& laf)
{
    slider.setLookAndFeel (&laf);
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
}

static void configureHeaderLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::Font (11.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colour (0xffFFAA00));
    label.setJustificationType (juce::Justification::centred);
}

static void configureValueLabel (juce::Label& label)
{
    label.setFont (juce::Font (10.5f, juce::Font::plain));
    label.setColour (juce::Label::textColourId, juce::Colour (0x99FFFFFF));
    label.setJustificationType (juce::Justification::centred);
}

//==============================================================================
BeerCrusherAudioProcessorEditor::BeerCrusherAudioProcessorEditor (BeerCrusherAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Modern widescreen plugin canvas (Thermal / Atoms aspect ratio)
    setSize (740, 480);

    // 1. Reactor Core
    addAndMakeVisible (reactorCore);

    // 2. Hardware Controls
    configureKnob (driveSlider, laf);
    configureKnob (crushSlider, laf);
    configureKnob (foamSlider,  laf);
    configureSlider (wetdrySlider, laf);

    addAndMakeVisible (driveSlider);
    addAndMakeVisible (crushSlider);
    addAndMakeVisible (foamSlider);
    addAndMakeVisible (wetdrySlider);

    // 3. Labels
    configureHeaderLabel (driveLabel,  "POUR / DENSITY");
    configureHeaderLabel (crushLabel,  "CRUSH");
    configureHeaderLabel (foamLabel,   "FOAM CAP");
    configureHeaderLabel (wetdryLabel, "DRY / WET MIX");

    addAndMakeVisible (driveLabel);
    addAndMakeVisible (crushLabel);
    addAndMakeVisible (foamLabel);
    addAndMakeVisible (wetdryLabel);

    configureValueLabel (driveVal);
    configureValueLabel (crushVal);
    configureValueLabel (foamVal);
    configureValueLabel (wetdryVal);

    addAndMakeVisible (driveVal);
    addAndMakeVisible (crushVal);
    addAndMakeVisible (foamVal);
    addAndMakeVisible (wetdryVal);

    // 4. APVTS Bindings
    driveAttachment  = std::make_unique<Attachment> (p.apvts, "drive",  driveSlider);
    crushAttachment  = std::make_unique<Attachment> (p.apvts, "crush",  crushSlider);
    foamAttachment   = std::make_unique<Attachment> (p.apvts, "foam",   foamSlider);
    wetdryAttachment = std::make_unique<Attachment> (p.apvts, "wetdry", wetdrySlider);

    startTimerHz (60);
}

BeerCrusherAudioProcessorEditor::~BeerCrusherAudioProcessorEditor()
{
    stopTimer();

    driveSlider .setLookAndFeel (nullptr);
    crushSlider .setLookAndFeel (nullptr);
    foamSlider  .setLookAndFeel (nullptr);
    wetdrySlider.setLookAndFeel (nullptr);
}

//==============================================================================
void BeerCrusherAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Solid industrial chassis base (#121316 dark obsidian)
    g.fillAll (juce::Colour (0xff121418));

    // Subtle brushed metal sheen gradient
    juce::ColourGradient chassisSheen (
        juce::Colour (0x10FFFFFF), 0.0f, 0.0f,
        juce::Colour (0x00000000), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill (chassisSheen);
    g.fillAll();

    // 2. Right Control Panel division (#16181f with inset bevel)
    const float splitX = 440.0f;
    const auto rightPanel = bounds.withLeft (splitX);

    g.setColour (juce::Colour (0xff15171d));
    g.fillRect (rightPanel);

    // Vertical seam divider
    g.setColour (juce::Colour (0xff0a0b0e));
    g.drawVerticalLine (static_cast<int>(splitX), 0.0f, bounds.getHeight());
    g.setColour (juce::Colour (0x22FFFFFF));
    g.drawVerticalLine (static_cast<int>(splitX + 1.0f), 0.0f, bounds.getHeight());

    // 3. Top Header Bar
    const float headerH = 54.0f;
    const auto headerRect = bounds.removeFromTop (headerH);

    // Header divider line
    g.setColour (juce::Colour (0xff090a0d));
    g.drawHorizontalLine (static_cast<int>(headerH), 0.0f, bounds.getWidth());
    g.setColour (juce::Colour (0x18FFFFFF));
    g.drawHorizontalLine (static_cast<int>(headerH + 1.0f), 0.0f, bounds.getWidth());

    // Title: BEERCRUSHER (Atoms / Teenage Engineering style)
    g.setColour (juce::Colour (0xffF0F2F5));
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawText ("BEERCRUSHER", 24, 14, 180, 24, juce::Justification::left, false);

    // Subtitle / Module tag
    g.setColour (juce::Colour (0xffFF6A00));
    g.setFont (juce::Font (10.0f, juce::Font::bold));
    g.drawText ("// ATOMIC SATURATOR & BREW DSP", 170, 20, 220, 16, juce::Justification::left, false);

    // Status Chip (Right side of header)
    const auto chipRect = juce::Rectangle<float> (bounds.getWidth() - 170.0f, 15.0f, 146.0f, 24.0f);
    g.setColour (juce::Colour (0xff0a0b0e));
    g.fillRoundedRectangle (chipRect, 4.0f);
    g.setColour (juce::Colour (0x22FFFFFF));
    g.drawRoundedRectangle (chipRect, 4.0f, 1.0f);

    // Active status light
    const float clipInd = processorRef.dsp.getClipIndicator();
    const juce::Colour ledCol = (clipInd > 0.1f)
        ? juce::Colour (0xffff2200)
        : juce::Colour (0xff00ff66);
    g.setColour (ledCol);
    g.fillEllipse (chipRect.getX() + 10.0f, chipRect.getY() + 8.0f, 8.0f, 8.0f);

    g.setColour (juce::Colour (0xccFFFFFF));
    g.setFont (juce::Font (10.0f, juce::Font::plain));
    g.drawText ("DSP 64-BIT // OK", static_cast<int>(chipRect.getX() + 24.0f), static_cast<int>(chipRect.getY()), 114, 24,
                juce::Justification::centredLeft, false);

    // 4. Outer Chassis Bevel Border
    g.setColour (juce::Colour (0x33FFFFFF));
    g.drawRect (getLocalBounds().toFloat(), 1.0f);
}

//==============================================================================
void BeerCrusherAudioProcessorEditor::resized()
{
    const int topMargin = 56;
    const int splitX    = 440;

    // Left Area: Giant Reactor Window
    reactorCore.setBounds (20, topMargin + 10, splitX - 40, getHeight() - topMargin - 30);

    // Right Area: Control Strip (Width: 300)
    const int rightX = splitX + 25;
    const int rightW = getWidth() - splitX - 50;

    // Big Knob: POUR / DENSITY (Center right)
    const int bigKnobSize = 110;
    const int bigKnobX = rightX + (rightW - bigKnobSize) / 2;
    const int bigKnobY = topMargin + 30;

    driveLabel.setBounds (rightX, bigKnobY - 24, rightW, 18);
    driveSlider.setBounds (bigKnobX, bigKnobY, bigKnobSize, bigKnobSize);
    driveVal.setBounds (rightX, bigKnobY + bigKnobSize + 4, rightW, 16);

    // Secondary Knobs Row: CRUSH and FOAM
    const int rowY = bigKnobY + bigKnobSize + 30;
    const int smallKnobSize = 78;
    const int spacing = (rightW - 2 * smallKnobSize) / 3;

    const int crushX = rightX + spacing;
    const int foamX  = crushX + smallKnobSize + spacing;

    crushLabel.setBounds (crushX - 10, rowY - 20, smallKnobSize + 20, 16);
    crushSlider.setBounds (crushX, rowY, smallKnobSize, smallKnobSize);
    crushVal.setBounds (crushX - 10, rowY + smallKnobSize + 3, smallKnobSize + 20, 14);

    foamLabel.setBounds (foamX - 10, rowY - 20, smallKnobSize + 20, 16);
    foamSlider.setBounds (foamX, rowY, smallKnobSize, smallKnobSize);
    foamVal.setBounds (foamX - 10, rowY + smallKnobSize + 3, smallKnobSize + 20, 14);

    // Bottom Slider: DRY / WET MIX
    const int sliderY = rowY + smallKnobSize + 36;
    wetdryLabel.setBounds (rightX, sliderY - 18, rightW, 16);
    wetdrySlider.setBounds (rightX, sliderY, rightW, 26);
    wetdryVal.setBounds (rightX, sliderY + 28, rightW, 14);
}

//==============================================================================
void BeerCrusherAudioProcessorEditor::timerCallback()
{
    auto& dsp = processorRef.dsp;

    const float driveValRaw  = processorRef.apvts.getRawParameterValue ("drive")->load();
    const float crushValRaw  = processorRef.apvts.getRawParameterValue ("crush")->load();
    const float foamValRaw   = processorRef.apvts.getRawParameterValue ("foam")->load();
    const float wetdryValRaw = processorRef.apvts.getRawParameterValue ("wetdry")->load();

    // Update Reactor Core simulation
    reactorCore.setRmsLevel    (dsp.getOutputLevel());
    reactorCore.setFillLevel   (0.2f + driveValRaw * 0.75f);
    reactorCore.setClipLevel   (dsp.getClipIndicator() + foamValRaw * 0.4f);
    reactorCore.setCrushAmount (crushValRaw);

    // Update numeric readout strings
    driveVal.setText  (juce::String (static_cast<int>(driveValRaw  * 100)) + " %", juce::dontSendNotification);
    crushVal.setText  (juce::String (static_cast<int>(crushValRaw  * 100)) + " %", juce::dontSendNotification);
    foamVal.setText   (juce::String (static_cast<int>(foamValRaw   * 100)) + " %", juce::dontSendNotification);
    wetdryVal.setText (juce::String (static_cast<int>(wetdryValRaw * 100)) + " %", juce::dontSendNotification);
}
