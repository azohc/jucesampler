/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JsamplerAudioProcessorEditor::JsamplerAudioProcessorEditor (JsamplerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (std::shared_ptr<JsamplerAudioProcessor> (&p))

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(false, false);
    setSize (1024, 576);

    addAndMakeVisible (&audioPreview);
    audioPreview.formatManager = (*processor).formatManager;
    audioPreview.transportSource = (*processor).transportSource;
    audioPreview.processor = processor;

    //// these define the parameters of our slider object
    //midiVolume.setSliderStyle (Slider::LinearBarVertical);
    //midiVolume.setRange(0.0, 127.0, 1.0);
    //midiVolume.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    //midiVolume.setPopupDisplayEnabled (true, false, this);
    //midiVolume.setTextValueSuffix (" Volume");
    //midiVolume.setValue(1.0);

    //// this function adds the slider to the editor
    //addAndMakeVisible (&midiVolume);
    //// add the listener to the slider
    //midiVolume.addListener (this);
}

JsamplerAudioProcessorEditor::~JsamplerAudioProcessorEditor()
{
}

//==============================================================================
void JsamplerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (bgColor);
    
    g.setColour (bgColor.darker(1.0f));
    g.fillRect (topAreaLeftSideBar);
    g.fillRect (topAreaRightSideBar);

    g.setColour (bgColor.darker(0.3f));
    g.fillRect (topArea);
}

void JsamplerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getBounds();
    topArea = area.removeFromTop (area.getHeight() - (area.getHeight() / goldenRatio));

    audioPreview.setBounds (topArea);
}

void JsamplerAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    (*processor).volume = midiVolume.getValue();
}

