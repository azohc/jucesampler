/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "AudioPreview.h"
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

    Colour bgColor = Colour::fromString("FF292C36");
    float goldenRatio = 1.618f;


private:
    std::shared_ptr<JsamplerAudioProcessor> processor;

    void sliderValueChanged (Slider* slider) override; // [3]
    Slider midiVolume; // [1]

    AudioPreview audioPreview;

    Rectangle<int> topArea;
    Rectangle<int> topAreaRightSideBar;
    Rectangle<int> topAreaLeftSideBar;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JsamplerAudioProcessorEditor)
};
