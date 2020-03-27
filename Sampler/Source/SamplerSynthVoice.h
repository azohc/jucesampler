/*
  ==============================================================================

    SamplerSynthVoice.h
    Created: 17 Mar 2020 11:25:19am
    Author:  azohc

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../Sampler/Source/SamplerSynthSound.h"

//==============================================================================
/*
*/
class SamplerSynthVoice    : public SynthesiserVoice
{
public:
    SamplerSynthVoice() {}

    virtual bool canPlaySound (SynthesiserSound *sound) override
    {
        return dynamic_cast<SamplerSynthSound*> (sound) != nullptr;
    }

    virtual void startNote (int midiNoteNumber, float velocity, 
                            SynthesiserSound * sound, int currentPitchWheelPosition) override
    {
        DBG("STARTNOTE: " + midiNoteNumber + " VELOCITY: " + velocity);
    }   

    virtual void stopNote (float velocity, bool allowTailOff)
    {
        clearCurrentNote();
    }

    void pitchWheelMoved (int /*newValue*/) override                              {}
    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override    {}

    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
    }

    using SynthesiserVoice::renderNextBlock;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerSynthVoice)
};
