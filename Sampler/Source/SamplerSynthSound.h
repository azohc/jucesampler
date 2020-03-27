/*
  ==============================================================================

    SamplerSynthSound.h
    Created: 17 Mar 2020 11:25:01am
    Author:  azohc

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
    SamplerSynthSound() {}
    // TODO return true if midiNoteNumber = chop's midi note 
    bool appliesToNote (int midiNoteNumber) override    { return true; }
    bool appliesToChannel (int midiNoteNumber) override { return true; }


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerSynthSound)
};
