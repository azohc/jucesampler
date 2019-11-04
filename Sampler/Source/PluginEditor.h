/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SamplerAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    SamplerAudioProcessorEditor (SamplerAudioProcessor&) : AudioProcessorEditor (&p), processor (p)
    {
        // Make sure that before the constructor has finished, you've set the
        // editor's size to whatever you need it to be.
        setSize (400, 300);
    }

    ~SamplerAudioProcessorEditor()
    {
    }

    //==============================================================================
    void paint (Graphics&) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        g.setColour (Colours::white);
        g.setFont (15.0f);
        g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
    }

    void resized() override
    {
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
    }


    //==============================================================================


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SamplerAudioProcessor& processor;


    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessorEditor)
};
