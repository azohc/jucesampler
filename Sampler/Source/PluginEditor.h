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
#include "ChopListComponent.h"
//==============================================================================
/**
*/
class SamplerAudioProcessorEditor :
    public AudioProcessorEditor,
    public ChangeListener
{
public:
    SamplerAudioProcessorEditor (SamplerAudioProcessor& p, 
                                 AudioTransportSource& transport, 
                                 AudioSourcePlayer& player, 
                                 AudioDeviceManager& manager,
                                 Array<Chop>& chops) 
        : AudioProcessorEditor (&p), 
        processor (p), 
        state (Stopped),
        transportSource (transport), 
        sourcePlayer (player),
        deviceManager (manager)
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
        
        addAndMakeVisible (loopLabel);
        loopLabel.setFont (Font (15.00f, Font::plain));
        loopLabel.setJustificationType (Justification::left);
        loopLabel.setEditable (false, false, false); // TODO make editable
        //loopLabel.setColour (Label::backgroundWhenEditingColourId)
        loopLabel.setColour (Label::backgroundColourId, colorBg);
        loopLabel.setColour (Label::textColourId, colorFg);

        addAndMakeVisible (fullLoopToggle);
        fullLoopToggle.setRadioGroupId(LoopMode);
        fullLoopToggle.setColour (TextButton::buttonColourId, colorBg);
        fullLoopToggle.setColour (TextButton::textColourOffId, colorFg);
        fullLoopToggle.setEnabled (false);
        fullLoopToggle.onClick = [this] () { 
            auto l = !currentAudioFileSource.get()->isLooping();
            currentAudioFileSource.get()->setLooping(l);
            fullLoopToggle.setToggleState (l, dontSendNotification);
        };

        addAndMakeVisible (selectionLoopToggle);
        selectionLoopToggle.setRadioGroupId(LoopMode);
        selectionLoopToggle.setColour (TextButton::buttonColourId, colorBg);
        selectionLoopToggle.setColour (TextButton::textColourOffId, colorFg);
        selectionLoopToggle.setEnabled (false); // TODO enable when selection exists
        selectionLoopToggle.setClickingTogglesState (true);

        addAndMakeVisible (startTimeChopButton);
        startTimeChopButton.setColour (TextButton::buttonColourId, colorBg);
        startTimeChopButton.setColour (TextButton::textColourOffId, colorFg);
        startTimeChopButton.onClick = [this] { startTimeChopClicked(); };
        startTimeChopButton.setEnabled (false);

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
        playButton.setEnabled (false);

        addAndMakeVisible (loadButton);
        loadButton.setColour (TextButton::buttonColourId, colorBg);
        loadButton.setColour (TextButton::textColourOffId, colorFg);
        loadButton.onClick = [this] { loadFile(); };
        loadButton.setEnabled (true);

        addAndMakeVisible (stopButton);
        stopButton.setColour (TextButton::buttonColourId, colorBg);
        stopButton.setColour (TextButton::textColourOffId, colorFg);
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setEnabled (false);


        // CHOPLIST
        chopList.reset(new ChopListComponent(chops, colors));
        addAndMakeVisible (chopList.get());

        // audio setup
        formatManager.registerBasicFormats();
        thumbnail->addChangeListener (this);
        transportSource.addChangeListener (this);
        thread.startThread (3);     

        setOpaque (true);
        setSize (1024, 576);
    }

    ~SamplerAudioProcessorEditor()
    {
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
        stopButton.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));


        // Thumbnail functions      TODO use flex to distribute evenly
        auto rectThumbnailFunctsAux = rectThumbnailFuncts.reduced(1);
        auto functionWidth = small (small (rectThumbnailFunctsAux.getWidth()));

        currentPositionLabel.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth)); // flex1

        rectLoopFunctions = rectThumbnailFunctsAux.removeFromLeft (functionWidth);
        auto loopFns = rectLoopFunctions;                                                       // flex2
        loopLabel.setBounds (loopFns.removeFromLeft (small (loopFns.getWidth())));
        fullLoopToggle.setBounds (loopFns.removeFromLeft (loopFns.getWidth() / 2));
        selectionLoopToggle.setBounds (loopFns.removeFromLeft (loopFns.getWidth()));

        startTimeChopButton.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth));  // flex3

        //followTransportButton.setBounds (rectThumbnailFunctsAux.removeFromLeft (small (small (rectThumbnailFunctsAux.getWidth()))));
        

        // Choplist
        chopList->setBounds (rectChopList);
    

        // Thumbnail
        rectThumbnail = r;
        auto rectThumbnailAux = rectThumbnail;
        zoomSlider.setBounds (rectThumbnailAux.removeFromRight (small (small (rectThumbnailAux.getHeight()))));
        thumbnail->setBounds (rectThumbnailAux.reduced(1));
    }


private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping,
        Pausing,
        Paused
    };
    enum RadioButtonIds
    {
        LoopMode = 1001
    };

    SamplerAudioProcessor& processor;

    AudioDeviceManager& deviceManager;
    AudioFormatManager formatManager;
    TimeSliceThread thread { "audio file preview" };

    File currentAudioFile;
    AudioSourcePlayer& sourcePlayer;
    AudioTransportSource& transportSource;
    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;

    std::unique_ptr<SamplerThumbnail> thumbnail;
    Slider zoomSlider { Slider::LinearVertical, Slider::NoTextBox };

    std::unique_ptr<ChopListComponent> chopList;
    //==============================================================================
    // CONTROLS
    TextButton playButton { "Play" };
    TextButton stopButton { "Stop" };   
    TextButton loadButton { "Load" };

    //==============================================================================
    // THUMBNAIL FUNCTIONS
    Label currentPositionLabel { "currentPositionLabel", "Position: " };
    TextButton startTimeChopButton { "Chop from here", "Creates a new chop with a start time equal to the current position" };
    ToggleButton followTransportButton { "Follow Transport" };

    Label loopLabel { "loopLabel", "Loop" };
    ToggleButton fullLoopToggle { "F" };
    ToggleButton selectionLoopToggle { "S" };
    
    // TODO MOVE&ZOOM on chop selection

    //==============================================================================
    // RECTANGLES
    Rectangle<int> rectChopEdit;
    Rectangle<int> rectControls;
    Rectangle<int> rectChopList;
    Rectangle<int> rectThumbnailFuncts;
    Rectangle<int> rectThumbnail;
    Rectangle<int> rectLoopFunctions;

    //==============================================================================
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

    TransportState state;

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
            playButton.setEnabled (true);
            fullLoopToggle.setEnabled (true);
            startTimeChopButton.setEnabled (true);
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
                                       32768,                   // tells it to buffer this many samples ahead TODO necessary? if not 0, nullptr
                                       &thread,                 // this is the background thread to use for reading-ahead TODO necessary? 
                                       reader->sampleRate);     // allows for sample rate correction

            return true;
        }

        return false;
    }

    void playButtonClicked()
    {
        if (!playButton.isEnabled()) return;

        if (state == Paused || state == Stopped)
        {
            changeState (Starting);
        } else if (state == Playing)
        {
            changeState (Pausing);
        }
    }

    void stopButtonClicked()
    {
        if (state == Playing)
            changeState (Stopping);
        else
            changeState (Stopped);
    }

    void startTimeChopClicked()
    {
        auto currentTime = transportSource.getCurrentPosition();
        auto chops = processor.getChopList();
        chops->add (Chop { currentTime, transportSource.getLengthInSeconds(), "" });
        chopList->reloadData();

        //for (auto i = 0; i < chops->size(); i++)
        //{
        //    Logger::getCurrentLogger()->writeToLog((String) chops->operator[](i).start);
        //}
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
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else if (state == Stopping || state == Playing)
                changeState (Stopped);
            else if (state == Pausing)
                changeState (Paused);
        }
    }

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Starting:
                    transportSource.start();
                    break;

                case Pausing:
                    transportSource.stop();
                    break;

                case Stopping:
                    transportSource.stop();
                    break;

                case Playing:
                    playButton.setButtonText ("Pause");
                    stopButton.setEnabled (true);
                    break;

                case Paused:
                    playButton.setButtonText ("Play");
                    break;

                case Stopped:
                    playButton.setButtonText ("Play");
                    stopButton.setEnabled (false);
                    transportSource.setPosition (0.0);
                    break;
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessorEditor)
};
