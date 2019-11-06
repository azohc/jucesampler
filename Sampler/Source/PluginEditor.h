/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "SamplerThumbnail.h"

//==============================================================================
/**
*/
class SamplerAudioProcessorEditor:
    public AudioProcessorEditor,
    private ChangeListener
{
public:
    SamplerAudioProcessorEditor (SamplerAudioProcessor& p) : AudioProcessorEditor (&p), processor (p)
    {
        addAndMakeVisible (zoomLabel);
        zoomLabel.setFont (Font (15.00f, Font::plain));
        zoomLabel.setJustificationType (Justification::centredRight);
        zoomLabel.setEditable (false, false, false);
        zoomLabel.setColour (TextEditor::textColourId, Colours::black);
        zoomLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (followTransportButton);
        followTransportButton.onClick = [this] { updateFollowTransportState(); };
        followTransportButton.setEnabled (false);

        // TODO ADD FILE LOADING COMPONENT

        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.onValueChange = [this] { thumbnail->setZoomFactor (zoomSlider.getValue()); };
        zoomSlider.setEnabled (false);
        zoomSlider.setSkewFactor (2);

        thumbnail.reset (new SamplerThumbnail (formatManager, transportSource, zoomSlider));
        addAndMakeVisible (thumbnail.get());
        thumbnail->addChangeListener (this);

        addAndMakeVisible (startStopButton);
        startStopButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
        startStopButton.setColour (TextButton::textColourOffId, Colours::black);
        startStopButton.onClick = [this] { startOrStop(); };

        // audio setup
        formatManager.registerBasicFormats();

        thread.startThread (3);
        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);

        setOpaque (true);
        setSize (1024, 576);
    }

    ~SamplerAudioProcessorEditor()
    {
        transportSource.setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);

        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);

        thumbnail->removeChangeListener (this);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colour::fromString("FF292C36"));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (4);
        auto controls = r.removeFromBottom (90);
        auto controlRightBounds = controls.removeFromRight (controls.getWidth() / 3);
        auto zoom = controls.removeFromTop (25);

        zoomLabel.setBounds (zoom.removeFromLeft (50));
        zoomSlider.setBounds (zoom);

        followTransportButton.setBounds (controls.removeFromTop (25));
        startStopButton.setBounds (controls);

        r.removeFromBottom (6);
        thumbnail->setBounds (r.removeFromBottom (140));
        r.removeFromBottom (6);
    }



    //==============================================================================


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SamplerAudioProcessor& processor;

    AudioDeviceManager audioDeviceManager;
    AudioFormatManager formatManager;
    TimeSliceThread thread { "audio file preview" };

    URL currentAudioFile;
    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;

    std::unique_ptr<SamplerThumbnail> thumbnail;
    Label zoomLabel { {}, "Zoom" };
    Slider zoomSlider { Slider::LinearHorizontal, Slider::NoTextBox };
    ToggleButton followTransportButton { "Follow Transport" };
    TextButton startStopButton { "Play/Stop" };

    //==============================================================================

    void showAudioResource (URL resource)
    {
        if (loadURLIntoTransport (resource))
        {
            currentAudioFile = std::move (resource);
            zoomSlider.setEnabled (true);
            followTransportButton.setEnabled (true);
        }

        zoomSlider.setValue (0, dontSendNotification);
        thumbnail->setURL (currentAudioFile); // TODO method that prompts the user for a wav file and loads it
    }

    bool loadURLIntoTransport (const URL& audioURL)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource.reset();

        AudioFormatReader* reader = nullptr;

        if (reader == nullptr)
            reader = formatManager.createReaderFor (audioURL.createInputStream (false));

        if (reader != nullptr)
        {
            currentAudioFileSource.reset (new AudioFormatReaderSource (reader, true));

            // ..and plug it into our transport source
            transportSource.setSource (currentAudioFileSource.get(),
                                       32768,                   // tells it to buffer this many samples ahead
                                       &thread,                 // this is the background thread to use for reading-ahead TODO necessary?
                                       reader->sampleRate);     // allows for sample rate correction

            return true;
        }

        return false;
    }

    void startOrStop()
    {
        if (transportSource.isPlaying())
        {
            transportSource.stop();
        } else
        {
            transportSource.setPosition (0);
            transportSource.start();
        }
    }

    void updateFollowTransportState()
    {
        thumbnail->setFollowsTransport (followTransportButton.getToggleState());
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == thumbnail.get())
            showAudioResource (URL (thumbnail->getLastDroppedFile()));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessorEditor)
};
