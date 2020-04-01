/*
  ==============================================================================

    PluginProcessor.h
    Created: 4 Nov 2019
    Author:  Juan Chozas Sumbera

  ==============================================================================
*/


#pragma once

#define JUCE_USE_MP3AUDIOFORMAT 1

#include "../JuceLibraryCode/JuceHeader.h"
#include "Constants.h"
#include "aubio.h"
#include "SamplerAudioSource.h"

//==============================================================================

class SamplerAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    SamplerAudioProcessor();
    ~SamplerAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    HashMap<int, ValueTree>* getChopMap();
    ValueTree getChopTree() const;
    void clearChopTree();
    int addChop(Chop& chop);

private:
    //==============================================================================
    
    SamplerAudioSource samplerSource { keyboardState };
    AudioTransportSource transportSource;
    MixerAudioSource mixerSource;    // combines sampler & transport
    AudioSourcePlayer sourcePlayer;
    AudioDeviceManager deviceManager;

    MidiKeyboardState keyboardState;

    Synthesiser synth;

    ValueTree chopTree;
    HashMap<int, ValueTree> chopMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessor)
};
