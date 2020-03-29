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

struct Chop
{
  Chop (const ValueTree &v) : state(v) { jassert (v.hasType (ID_CHOP)); }

  int getId() const { return state[PROP_ID]; }
  void setId (int id) { state.setProperty (PROP_ID, id, nullptr); }

  double getStartTime() const { return state[PROP_START_TIME]; }

  void setStartTime (double time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_START_TIME, time, nullptr);
  }

  double getEndTime() const { return state[PROP_END_TIME]; }

  void setEndTime (double time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_END_TIME, time, nullptr);
  }

  int getStartSample() const { return state[PROP_START_TIME]; }

  void setStartSample (int time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_START_TIME, time, nullptr);
  }

  int getEndSample() const { return state[PROP_END_TIME]; }

  void setEndSample (int time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_END_TIME, time, nullptr);
  }   

  int getTriggerNote() const { return state[PROP_TRIGGER]; }
  void getTriggerNote (int note) { state.setProperty (PROP_TRIGGER, note, nullptr); }
  
  bool getHidden() const { return state[PROP_HIDDEN]; }
  void setHidden (bool hidden) { state.setProperty (PROP_HIDDEN, hidden, nullptr); } 

  ValueTree state;
};

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
