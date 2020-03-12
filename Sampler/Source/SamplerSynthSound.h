/*
  ==============================================================================

    SamplerSynthSound.h
    Created: 12 Mar 2020 7:41:28pm
    Author:  chosa

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SamplerSynthSound    : public SynthesiserSound
{
public:
    SamplerSynthSound()
    {
    }

    ~SamplerSynthSound()
    {
    }

    virtual bool appliesToNote (int	midiNoteNumber)
    {
        return true; // TODO return true if midinotenumber = chop's midi note
    }

    virtual bool appliesToChannel (int	midiNoteNumber)
    {
        return true;
    }


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerSynthSound)
};
