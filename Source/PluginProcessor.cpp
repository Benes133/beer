#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
BeerCrusherAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "crush", "Crush",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "drive", "Drive",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "foam", "Foam",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "wetdry", "Wet/Dry",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    return layout;
}

//==============================================================================
BeerCrusherAudioProcessor::BeerCrusherAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "BeerCrusherParams", createParameterLayout())
{
    crushParam  = apvts.getRawParameterValue ("crush");
    driveParam  = apvts.getRawParameterValue ("drive");
    foamParam   = apvts.getRawParameterValue ("foam");
    wetdryParam = apvts.getRawParameterValue ("wetdry");
}

BeerCrusherAudioProcessor::~BeerCrusherAudioProcessor() {}

//==============================================================================
bool BeerCrusherAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

//==============================================================================
void BeerCrusherAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp.prepare (sampleRate, samplesPerBlock);
}

//==============================================================================
void BeerCrusherAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    // Read parameters (lock-free)
    dsp.setCrush  (crushParam ->load());
    dsp.setDrive  (driveParam ->load());
    dsp.setFoam   (foamParam  ->load());
    dsp.setWetDry (wetdryParam->load());

    dsp.process (buffer);
}

//==============================================================================
juce::AudioProcessorEditor* BeerCrusherAudioProcessor::createEditor()
{
    return new BeerCrusherAudioProcessorEditor (*this);
}

//==============================================================================
void BeerCrusherAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BeerCrusherAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// Plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BeerCrusherAudioProcessor();
}
