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
class SamplerAudioProcessorEditor :
    public AudioProcessorEditor,
    public ChangeListener
{
public:
    SamplerAudioProcessorEditor (SamplerAudioProcessor& p) : AudioProcessorEditor (&p), processor (p)
    {
        colors.set("bgdark", colorBgDark);
        colors.set("bglite", colorBgLight);
        colors.set("bg", colorBg);
        colors.set("fg", colorFg);
        colors.set("red", colorRed);
        colors.set("bluedark", colorBlueDark);
        colors.set("blue", colorBlue);
        colors.set("tan", colorTan);
        colors.set("sea", colorSea);
        colors.set("gray", colorGray);
        colors.set("graylite", colorGrayLight);


        // THUMBNAIL 
        thumbnail.reset (new SamplerThumbnail (formatManager, transportSource, zoomSlider, colors));
        addAndMakeVisible (thumbnail.get());

        // THUMBNAIL FUNCTIONS
        addAndMakeVisible (currentPositionLabel);
        currentPositionLabel.setFont (Font (15.00f, Font::plain));
        currentPositionLabel.setJustificationType (Justification::left);
        currentPositionLabel.setEditable (false, false, false); // TODO make editable
        //currentPositionLabel.setColour (Label::backgroundWhenEditingColourId)
        currentPositionLabel.setColour (Label::backgroundColourId, colorBg);
        currentPositionLabel.setColour (Label::textColourId, colorFg);
        

        addAndMakeVisible (followTransportButton);
        followTransportButton.onClick = [this] { updateFollowTransportState(); };
        followTransportButton.setEnabled (false);

        // THUMBNAIL SLIDER
        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.onValueChange = [this] { thumbnail->setZoomFactor (zoomSlider.getValue()); };
        zoomSlider.setEnabled (false);
        zoomSlider.setSkewFactor (2);
        zoomSlider.setColour (Slider::backgroundColourId, colorBlue.darker(0.6));
        zoomSlider.setColour (Slider::trackColourId, colorBlue.darker(0.3));
        zoomSlider.setColour (Slider::thumbColourId, colorBlue);



        // BUTTONS
        addAndMakeVisible (playButton);
        playButton.setColour (TextButton::buttonColourId, colorBg);
        playButton.setColour (TextButton::textColourOffId, colorFg);
        playButton.onClick = [this] { playButtonClicked(); };

        addAndMakeVisible (loadButton);
        loadButton.setColour (TextButton::buttonColourId, colorBg);
        loadButton.setColour (TextButton::textColourOffId, colorFg);
        loadButton.onClick = [this] { loadFile(); };


        // audio setup
        formatManager.registerBasicFormats();
        thumbnail->addChangeListener (this);
        //thread.startThread (3);
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
    }

    void paint (Graphics& g) override
    {
        g.fillAll (colorBgDark);

        g.setColour (colorBg);
        g.fillRect (getLocalBounds().reduced(4));
        
        g.setColour (colorBlueDark);
        g.fillRect (rectThumbnail);

        auto strokeRect = [&g] (Rectangle<int> r, int s)
        {
            auto path = Path();
            path.addRectangle (r);
            g.strokePath (path, PathStrokeType (s), {});
        };

        g.setColour (colorBgDark);
        strokeRect (rectControls, 2);
        strokeRect (rectThumbnail, 2);
        strokeRect (rectThumbnailFuncts, 2);
        strokeRect (rectChopEdit, 2);
        strokeRect (rectChopList, 2);
    }

    void resized() override
    {
        auto goldenr = 1.618;
        auto small = [goldenr] (int d) { return d - (d / goldenr); };
        auto large = [goldenr] (int d) { return d / goldenr; };

        auto r = getLocalBounds().reduced (4);

        rectChopEdit = r.removeFromBottom (small (r.getHeight()));
        rectControls = r.removeFromLeft (small (small (r.getWidth())));
        rectChopList = r.removeFromBottom (small (r.getHeight()));
        rectThumbnailFuncts = r.removeFromTop (small (small (r.getHeight())));
        
        // Controls
        auto rectControlsAux = rectControls;
        auto buttonHeight = small (small (rectControlsAux.getHeight()));
        loadButton.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));
        playButton.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));

        // Thumbnail functions
        auto rectThumbnailFunctsAux = rectThumbnailFuncts.reduced(1);
        currentPositionLabel.setBounds (rectThumbnailFunctsAux.removeFromLeft (small (rectThumbnailFunctsAux.getWidth())));
        //followTransportButton.setBounds (rectThumbnailFunctsAux.removeFromLeft (small (small (rectThumbnailFunctsAux.getWidth()))));
        
        rectThumbnail = r;
        auto rectThumbnailAux = rectThumbnail;
        zoomSlider.setBounds (rectThumbnailAux.removeFromRight (small (small (rectThumbnailAux.getHeight()))));
        thumbnail->setBounds (rectThumbnailAux.reduced(1));
    }


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SamplerAudioProcessor& processor;

    AudioDeviceManager audioDeviceManager;
    AudioFormatManager formatManager;
    //TimeSliceThread thread { "audio file preview" };

    File currentAudioFile;
    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;

    std::unique_ptr<SamplerThumbnail> thumbnail;
    Slider zoomSlider { Slider::LinearVertical, Slider::NoTextBox };
    TextButton playButton { "Play" };
    TextButton stopButton { "Stop" };   // TODO add buttons to controls
    TextButton loopButton { "Loop" };
    TextButton loadButton { "Load sample" };

    //==============================================================================
    // THUMBNAIL FUNCTIONS
    Label currentPositionLabel { "currentPositionLabel", "Position: " };
    TextButton startTimeChop { "Chop from here", "Creates a new chop with a start time equal to the current position" };
    ToggleButton followTransportButton { "Follow Transport" };

    // MOVE&ZOOM on chop selection

    //==============================================================================
    // RECTANGLES
    Rectangle<int> rectChopEdit;
    Rectangle<int> rectControls;
    Rectangle<int> rectChopList;
    Rectangle<int> rectThumbnailFuncts;
    Rectangle<int> rectThumbnail;

    // COLORS
    Colour colorBgDark = Colour::fromString("FF252420");
    Colour colorBg =  Colour::fromString("FF544F4C");
    Colour colorBgLight = Colour::fromString("FFA1988F");
    Colour colorFg =  Colour::fromString("FFEADED2");
    Colour colorGray = Colour::fromString("FF605B58");
    Colour colorGrayLight = Colour::fromString("FFD3D4D9");
    Colour colorRed = Colour::fromString("FFD62734");
    Colour colorBlueDark = Colour::fromString("FF252D39");
    Colour colorBlue = Colour::fromString("FF525C65");
    Colour colorTan = Colour::fromString("FFEDB183");
    Colour colorSea = Colour::fromString("FF1E555C");

    HashMap<String, Colour> colors;

    //==============================================================================
    void loadFile()
    {
        FileChooser fc ("Choose a Wave file...", {}, "*wav", true);

        if (fc.browseForFileToOpen())
        {
            auto file = fc.getResult();
            showAudioResource(file);
        }
    }

    void showAudioResource (const File& file)
    {
        if (loadFileIntoTransport (file))
        {
            zoomSlider.setEnabled (true);
            followTransportButton.setEnabled (true);
            zoomSlider.setValue (0, dontSendNotification);
            thumbnail->setFile (file);
        }
    }

    bool loadFileIntoTransport (const File& file)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource.reset();

        AudioFormatReader* reader = nullptr;

        if (reader == nullptr)
            reader = formatManager.createReaderFor (file);

        if (reader != nullptr)
        {
            currentAudioFileSource.reset (new AudioFormatReaderSource (reader, true));

            // ..and plug it into our transport source
            transportSource.setSource (currentAudioFileSource.get(),
                                       0,                   // tells it to buffer this many samples ahead TODO necessary? if not 0, nullptr
                                       nullptr,                 // this is the background thread to use for reading-ahead TODO necessary? 
                                       reader->sampleRate);     // allows for sample rate correction

            return true;
        }

        return false;
    }

    void playButtonClicked()
    {
        if (transportSource.isPlaying())
        {
            transportSource.setPosition(0);
        } else
        {
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
        {
            if (thumbnail.get()->newFileDropped)
                showAudioResource (File (thumbnail->getLastDroppedFile()));
            else 
                currentPositionLabel.setText (String::formatted("Position: " + (String) thumbnail->getCurrentPosition()), NotificationType::dontSendNotification);
        }

    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessorEditor)
};
