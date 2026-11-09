#include "PluginEditor.h"

//==============================================================================
static void setupKnob (juce::Slider& slider, KnobLookAndFeel& laf)
{
    slider.setLookAndFeel (&laf);
    slider.setSliderStyle (juce::Slider::Rotary);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
}

static void setupLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::Font (12.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colour (0xffFFB703));
    label.setJustificationType (juce::Justification::centred);
}

//==============================================================================
BeerCrusherAudioProcessorEditor::BeerCrusherAudioProcessorEditor (BeerCrusherAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (500, 600);

    // Beer glass
    addAndMakeVisible (beerGlass);

    // Knobs
    setupKnob (crushSlider,  laf);
    setupKnob (driveSlider,  laf);
    setupKnob (foamSlider,   laf);
    setupKnob (wetdrySlider, laf);

    addAndMakeVisible (crushSlider);
    addAndMakeVisible (driveSlider);
    addAndMakeVisible (foamSlider);
    addAndMakeVisible (wetdrySlider);

    // Labels
    setupLabel (crushLabel,  "CRUSH");
    setupLabel (driveLabel,  "POUR");
    setupLabel (foamLabel,   "FOAM");
    setupLabel (wetdryLabel, "MIX");

    addAndMakeVisible (crushLabel);
    addAndMakeVisible (driveLabel);
    addAndMakeVisible (foamLabel);
    addAndMakeVisible (wetdryLabel);

    // APVTS attachments
    crushAttachment  = std::make_unique<Attachment> (p.apvts, "crush",  crushSlider);
    driveAttachment  = std::make_unique<Attachment> (p.apvts, "drive",  driveSlider);
    foamAttachment   = std::make_unique<Attachment> (p.apvts, "foam",   foamSlider);
    wetdryAttachment = std::make_unique<Attachment> (p.apvts, "wetdry", wetdrySlider);

    startTimerHz (60);
}

BeerCrusherAudioProcessorEditor::~BeerCrusherAudioProcessorEditor()
{
    stopTimer();

    // Clear look and feel before destruction
    crushSlider .setLookAndFeel (nullptr);
    driveSlider .setLookAndFeel (nullptr);
    foamSlider  .setLookAndFeel (nullptr);
    wetdrySlider.setLookAndFeel (nullptr);
}

//==============================================================================
void BeerCrusherAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark background
    g.fillAll (juce::Colour (0xff121316));

    // Title text
    g.setFont (juce::Font ("Arial", 28.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xffFFB703));
    g.drawText ("BEERCRUSHER", getLocalBounds().removeFromTop (50),
                juce::Justification::centred, false);

    // Subtle separator line
    g.setColour (juce::Colour (0x33FFB703));
    g.drawHorizontalLine (52, 20.0f, getWidth() - 20.0f);
}

//==============================================================================
void BeerCrusherAudioProcessorEditor::resized()
{
    const int w = getWidth();

    // Beer glass — centered in top area
    const int glassAreaH = 350;
    const int glassW     = 220;
    const int glassH     = 300;
    beerGlass.setBounds ((w - glassW) / 2, (glassAreaH - glassH) / 2 + 50,
                         glassW, glassH);

    // Knob row at bottom
    const int knobAreaY  = glassAreaH + 10;
    const int knobAreaH  = getHeight() - knobAreaY;
    const int knobSize   = 80;
    const int labelH     = 20;
    const int numKnobs   = 4;
    const int spacing    = (w - numKnobs * knobSize) / (numKnobs + 1);

    juce::Slider* sliders[] = { &crushSlider, &driveSlider, &foamSlider, &wetdrySlider };
    juce::Label*  labels[]  = { &crushLabel,  &driveLabel,  &foamLabel,  &wetdryLabel  };

    for (int i = 0; i < numKnobs; ++i)
    {
        int knobX = spacing + i * (knobSize + spacing);
        int knobY = knobAreaY + (knobAreaH - knobSize - labelH - 4) / 2;

        sliders[i]->setBounds (knobX, knobY, knobSize, knobSize);
        labels [i]->setBounds (knobX, knobY + knobSize + 2, knobSize, labelH);
    }
}

//==============================================================================
void BeerCrusherAudioProcessorEditor::timerCallback()
{
    auto& dsp = processorRef.dsp;

    beerGlass.setRmsLevel    (dsp.getOutputLevel());
    beerGlass.setFillLevel   (0.4f + processorRef.apvts.getRawParameterValue ("drive")->load() * 0.5f);
    beerGlass.setClipLevel   (dsp.getClipIndicator());
    beerGlass.setCrushAmount (processorRef.apvts.getRawParameterValue ("crush")->load());

    beerGlass.repaint();
}
