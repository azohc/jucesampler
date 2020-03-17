/*
  ==============================================================================

    SamplerSynthVoice.h
    Created: 17 Mar 2020 11:25:19am
    Author:  azohc

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SamplerSynthVoice    : public SynthesiserVoice
{
public:
    SamplerSynthVoice()
    {
    }

    ~SamplerSynthVoice()
    {
    }

    int getCurrentlyPlayingNote () const
    {
        return 0;
    }

    SynthesiserSound::Ptr getCurrentlyPlayingSound() const
    {
        return SynthesiserSound::Ptr();
    }

    virtual bool canPlaySound (SynthesiserSound *)
    {
        return false;
    }


    virtual void startNote (int midiNoteNumber, float velocity, 
        SynthesiserSound * sound, int currentPitchWheelPosition)
    {

    }   

    virtual void stopNote (float velocity, bool allowTailOff)
    {

    }

    virtual bool isVoiceActive() const
    {
        return false;
    }

    virtual void pitchWheelMoved (int newPitchWheelValue)
    {

    }

    virtual void controllerMoved (int controllerNumber, int newControllerValue)
    {
        
    }

    virtual void aftertouchChanged (int newAftertouchValue)
    {

    }   

    virtual void channelPressureChanged (int newChannelPressureValue)
    {

    }


    virtual void renderNextBlock (AudioBuffer<float> & outputBuffer, int startSample, int numSamples)
    {

    }

    virtual void renderNextBlock (AudioBuffer<double> & outputBuffer, int startSample, int numSamples)      
    {

    }

    virtual void setCurrentPlaybackSampleRate (double newRate)
    {

    }

    virtual bool isPlayingChannel (int midiChannel) const
    {
        return false;
    }

    double getSampleRate() const
    {
        double sr = 0;
        
        return sr;
    }

    bool isKeyDown() const
    {
        return false;
    }
    
    void setKeyDown (bool isNowDown)
    {

    }
    
    bool isSustainPedalDown () const
    {
        return false;
    }
    
    void setSustainPedalDown (bool isNowDown)
    {

    }
    
    bool isSostenutoPedalDown () const
    {
        return false;
    }
    
    void setSostenutoPedalDown (bool isNowDown)
    {

    }
    
    bool isPlayingButReleased () const
    {
        return false;
    }
    
    bool wasStartedBefore (const SynthesiserVoice & other) const
    {
        return false;
    }
    
    void clearCurrentNote ()
    {

    }

    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerSynthVoice)
};
