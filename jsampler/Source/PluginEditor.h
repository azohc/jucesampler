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
class JsamplerAudioProcessorEditor : public AudioProcessorEditor,
                                        private Slider::Listener
{
public:
    JsamplerAudioProcessorEditor (JsamplerAudioProcessor&);
    ~JsamplerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JsamplerAudioProcessor& processor;

    void sliderValueChanged (Slider* slider) override; // [3]


    Slider midiVolume; // [1]

    Rectangle<int> topArea;
    Rectangle<int> topRightArea;

    float goldenRatio = 1.618f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JsamplerAudioProcessorEditor)
};
