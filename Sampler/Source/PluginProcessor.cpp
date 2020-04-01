/*
  ==============================================================================

    PluginProcessor.cpp
    Created: 4 Nov 2019
    Author:  Juan Chozas Sumbera

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

SamplerAudioProcessor::SamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    chopTree = ValueTree { ID_CHOPTREE, {} };
};

SamplerAudioProcessor::~SamplerAudioProcessor()
{
}

//==============================================================================
const String SamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SamplerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SamplerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SamplerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SamplerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SamplerAudioProcessor::setCurrentProgram (int index)
{
}

const String SamplerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SamplerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    samplerSource.prepareToPlay(samplesPerBlock, sampleRate);

    mixerSource.addInputSource(&transportSource, false);
    mixerSource.addInputSource(&samplerSource, false);

    sourcePlayer.prepareToPlay(sampleRate, samplesPerBlock);
    sourcePlayer.setSource (&mixerSource);   
    
    deviceManager.initialiseWithDefaultDevices(getTotalNumInputChannels(), getTotalNumOutputChannels());
    deviceManager.addAudioCallback (&sourcePlayer);
    deviceManager.addMidiInputDeviceCallback ("20-0", &(samplerSource.midiCollector));
}

void SamplerAudioProcessor::releaseResources()
{
    deviceManager.removeAudioCallback (&sourcePlayer);
    transportSource.setSource (nullptr);
    mixerSource.releaseResources();
    sourcePlayer.setSource (nullptr);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SamplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SamplerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool SamplerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SamplerAudioProcessor::createEditor()
{
    auto editor = new SamplerAudioProcessorEditor (*this, transportSource, samplerSource, sourcePlayer, deviceManager, keyboardState);
    chopTree.addListener(editor);
    return editor;
}

//==============================================================================
void SamplerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

HashMap<int, ValueTree>* SamplerAudioProcessor::getChopMap()
{
    return &chopMap;
}

ValueTree SamplerAudioProcessor::getChopTree() const
{
    return chopTree;
}

void SamplerAudioProcessor::clearChopTree()
{
    chopTree.removeAllChildren(nullptr);
    chopMap.clear();
}

int SamplerAudioProcessor::addChop(Chop& chop) 
{
    auto newKey = chopTree.getNumChildren();
    while (chopMap.contains(newKey)) {
        newKey++;
    }
    chop.setId (newKey);
    chopMap.set (newKey, chop.state);
    chopTree.appendChild (chop.state, nullptr);
    
    return newKey;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SamplerAudioProcessor();
}
