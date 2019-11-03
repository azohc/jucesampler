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
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(false, false);
    setSize (1024, 576);
    

    // these define the parameters of our slider object
    midiVolume.setSliderStyle (Slider::LinearBarVertical);
    midiVolume.setRange(0.0, 127.0, 1.0);
    midiVolume.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled (true, false, this);
    midiVolume.setTextValueSuffix (" Volume");
    midiVolume.setValue(1.0);

    // this function adds the slider to the editor
    addAndMakeVisible (&midiVolume);
    // add the listener to the slider
    midiVolume.addListener (this);

}

JsamplerAudioProcessorEditor::~JsamplerAudioProcessorEditor()
{
}

//==============================================================================
void JsamplerAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::ghostwhite);
    g.fillRect (topArea);

    g.setColour (Colours::navajowhite);
    g.fillRect (topRightArea);
    //g.setColour (Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void JsamplerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getBounds();
    topArea = area.removeFromTop(area.getHeight() - (area.getHeight() / goldenRatio));

    topRightArea = topArea.removeFromRight(topArea.getWidth() - (topArea.getWidth() / goldenRatio));

    midiVolume.setBounds (topRightArea.getX() + (topRightArea.getWidth() * 0.75), \
                          topRightArea.getY() + (topRightArea.getHeight() * 0.05), \
                          (topRightArea.getWidth() * 0.15), (topRightArea.getHeight() * 0.75));
}

void JsamplerAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    processor.volume = midiVolume.getValue();
}