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

    bool appliesToNote (int	midiNoteNumber) override
    {
        return true; // TODO return true if midiNoteNumber = chop's midi note
    }

    bool appliesToChannel (int	midiNoteNumber) override
    {
        return true;
    }


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerSynthSound)
};
