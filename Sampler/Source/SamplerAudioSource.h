/*
  ==============================================================================

    SamplerAudioSource.h
    Created: 27 Mar 2020 9:20:18am
    Author:  azohc

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "Constants.h"


//==============================================================================
/*
*/
class SamplerAudioSource    : public AudioSource, public Value::Listener
{
public:    
    SamplerAudioSource (MidiKeyboardState& keyState) : keyboardState (keyState) {
        synth.addVoice (new SamplerVoice());
    }

    void makeSoundsFromChops(AudioFormatReader* formatReader, ValueTree chopTree)
    {
        synth.clearSounds();
        numChops = chopTree.getNumChildren();
        for (auto i = 0; i < chopTree.getNumChildren(); ++i) {
            Chop chop (chopTree.getChild(i));
            int chopId = chop.getId();
            int64 startSample = chop.getStartSample();
            int64 endSample = chop.getEndSample();
            int rootNote = chop.getTriggerNote();
            auto audioSSReader = new AudioSubsectionReader (formatReader, startSample, endSample - startSample, false);
            double startTime = chop.getStartTime();
            double endTime = chop.getEndTime();

            BigInteger singleNoteRange;
            singleNoteRange.setBit(rootNote);
            auto sound = new SamplerSound (String (chopId), *audioSSReader, singleNoteRange, rootNote, 0.1, 0.1, endTime - startTime);
            auto params = ADSR::Parameters();
            chopSounds.set (chopId, std::make_pair(sound, params));
            synth.addSound (sound);
            delete audioSSReader;
        }
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        midiCollector.reset (sampleRate);
        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // the synth always adds its output to the audio buffer, so we have to clear it
        // first..
        bufferToFill.clearActiveBufferRegion();

        // fill a midi buffer with incoming messages from the midi input.
        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, bufferToFill.numSamples);

        // pass these messages to the keyboard state so that it can update the component
        // to show on-screen which keys are being pressed on the physical midi keyboard.
        // This call will also add midi messages to the buffer which were generated by
        // the mouse-clicking on the on-screen keyboard.
        keyboardState.processNextMidiBuffer (incomingMidi, 0, bufferToFill.numSamples, true);

        // and now get the synth to process the midi events and generate its output.
        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);
    }

    void valueChanged(Value &value) override
    {
        if (value.refersToSameSourceAs (playbackMode))
        {
            synth.clearVoices();
            if (playbackMode == POLY)
            {
                for (int i = 0; i < numChops; ++i)
                {
                    synth.addVoice (new SamplerVoice());
                }
            } else
            {
                synth.addVoice (new SamplerVoice());
            }
        }
    }

    void setPlaybackModeListener(Value &value) 
    {
        playbackMode = Value(value);
        playbackMode.addListener (this);
    }

    HashMap<int, std::pair<SamplerSound*, ADSR::Parameters>> chopSounds;

    MidiMessageCollector midiCollector;

private:
    MidiKeyboardState& keyboardState;
    Synthesiser synth;
    Value playbackMode;

    int numChops = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioSource)
};
