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
#include "SamplerThumbnail.h"

//==============================================================================

class SamplerAudioProcessor  : public AudioProcessor, Value::Listener
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
    ValueTree getChopTree() const;
    void clearChopTree();
    int addChop(Chop& chop);
    void removeChop(int id);
    HashMap<int, std::pair<DrawableRectangle*, DrawableRectangle*>>* getChopBounds();
    void setListenerForMidiLearn (Value &value);
    Value getLastRecordedMidiNote() const;
    void valueChanged (Value &v);
    SamplerThumbnail* getThumbnail();
    void resetThumbnailTo (SamplerThumbnail *tn);
    AudioFormatReaderSource* getFileReaderSource();
    void resetFileReaderSource();
    void resetFileReaderSourceTo (AudioFormatReaderSource *source);
    AudioFormatManager* getFormatManager();
    File getFile() const;
    void setCurrentFile (File &file);
    Value playbackMode;

private:
    //==============================================================================
    
    AudioDeviceManager deviceManager;
    
    MixerAudioSource mixerSource;    // combines sampler & transport
    AudioTransportSource transportSource;
    AudioSourcePlayer sourcePlayer;

    SamplerAudioSource samplerSource { &synth, keyboardState, &midiCollector, playbackMode };
    Synthesiser synth;
    MidiKeyboardState keyboardState;
    MidiMessageCollector midiCollector;

    ValueTree chopTree;
    std::unique_ptr<SamplerThumbnail> thumbnail;
    HashMap<int, std::pair<DrawableRectangle*, DrawableRectangle*>> chopBounds;
    AudioFormatManager formatManager;
    File currentAudioFile;
    std::unique_ptr<AudioFormatReaderSource> fileReaderSource;

    Value selectedChopId;
    Value userSelectionActive;
    Value listenForMidiLearn;
    Value lastRecordedMidiNote;
    bool updateLastRecordedMidiNote = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessor)
};
