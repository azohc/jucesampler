/*
  ==============================================================================

    PluginEditor.h
    Created: 4 Nov 2019
    Author:  Juan Chozas Sumbera

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
    public ChangeListener,
    public ValueTree::Listener,
    public Value::Listener
{
public:
    SamplerAudioProcessorEditor (SamplerAudioProcessor& p, 
                                 AudioTransportSource& transport, 
                                 AudioSourcePlayer& player, 
                                 AudioDeviceManager& device) 
        : AudioProcessorEditor (&p), 
        processor (p), 
        state (Stopped),
        transportSource (transport), 
        sourcePlayer (player),
        deviceManager (device)
    {
        // THUMBNAIL 
        thumbnail.reset (new SamplerThumbnail (formatManager, transportSource, zoomSlider, processor.getChopTree(), selectedChopId, userSelectionActive));
        addAndMakeVisible (thumbnail.get());

        // THUMBNAIL FUNCTIONS
        addAndMakeVisible (currentPositionLabel);
        currentPositionLabel.setFont (Font (15.00f, Font::plain));
        currentPositionLabel.setJustificationType (Justification::left);
        currentPositionLabel.setEditable (false, false, false); // TODO make editable
        //currentPositionLabel.setColour (Label::backgroundWhenEditingColourId)
        currentPositionLabel.setColour (Label::backgroundColourId, COLOR_BG);
        currentPositionLabel.setColour (Label::textColourId, COLOR_FG);
        
        addAndMakeVisible (loopLabel);
        loopLabel.setFont (Font (15.00f, Font::plain));
        loopLabel.setJustificationType (Justification::left);
        loopLabel.setEditable (false, false, false);
        loopLabel.setColour (Label::backgroundColourId, COLOR_BG);
        loopLabel.setColour (Label::textColourId, COLOR_FG);

        addAndMakeVisible (fullLoopToggle);
        fullLoopToggle.setRadioGroupId(LoopMode);
        fullLoopToggle.setColour (TextButton::buttonColourId, COLOR_BG);
        fullLoopToggle.setColour (TextButton::textColourOffId, COLOR_FG);
        fullLoopToggle.setEnabled (false);
        fullLoopToggle.onClick = [this] () { 
            auto l = !currentAudioFileSource.get()->isLooping();
            currentAudioFileSource.get()->setLooping(l);
            fullLoopToggle.setToggleState (l, dontSendNotification);
        };

        addAndMakeVisible (selectionLoopToggle);
        selectionLoopToggle.setRadioGroupId(LoopMode);
        selectionLoopToggle.setColour (TextButton::buttonColourId, COLOR_BG);
        selectionLoopToggle.setColour (TextButton::textColourOffId, COLOR_FG);
        selectionLoopToggle.setEnabled (false);
        selectionLoopToggle.setClickingTogglesState(true);
        selectionLoopToggle.onClick = [this] () { 
            currentAudioFileSource.get()->setLooping(selectionLoopToggle.getToggleState());
        };

        addAndMakeVisible (startTimeChopButton);
        startTimeChopButton.setColour (TextButton::buttonColourId, COLOR_BG);
        startTimeChopButton.setColour (TextButton::textColourOffId, COLOR_FG);
        startTimeChopButton.onClick = [this] { startTimeChopClicked(); };
        startTimeChopButton.setEnabled (false);

        addAndMakeVisible (selectionChopButton);
        selectionChopButton.setColour (TextButton::buttonColourId, COLOR_BG);
        selectionChopButton.setColour (TextButton::textColourOffId, COLOR_FG);
        selectionChopButton.onClick = [this] { selectionChopClicked(); };
        selectionChopButton.setEnabled (false);


        addAndMakeVisible (followTransportButton);
        followTransportButton.onClick = [this] { updateFollowTransportState(); };
        followTransportButton.setEnabled (false);

        // THUMBNAIL SLIDER
        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.onValueChange = [this] { thumbnail->setZoomFactor (zoomSlider.getValue()); };
        zoomSlider.setEnabled (false);
        zoomSlider.setSkewFactor (2);
        zoomSlider.setColour (Slider::backgroundColourId, COLOR_BLUE.darker(0.6));
        zoomSlider.setColour (Slider::trackColourId, COLOR_BLUE.darker(0.3));
        zoomSlider.setColour (Slider::thumbColourId, COLOR_BLUE);


        // BUTTONS
        addAndMakeVisible (playButton);
        playButton.setColour (TextButton::buttonColourId, COLOR_BG);
        playButton.setColour (TextButton::textColourOffId, COLOR_FG);
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setEnabled (false);

        addAndMakeVisible (loadButton);
        loadButton.setColour (TextButton::buttonColourId, COLOR_BG);
        loadButton.setColour (TextButton::textColourOffId, COLOR_FG);
        loadButton.onClick = [this] { loadFile(); };
        loadButton.setEnabled (true);

        addAndMakeVisible (stopButton);
        stopButton.setColour (TextButton::buttonColourId, COLOR_BG);
        stopButton.setColour (TextButton::textColourOffId, COLOR_FG);
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setEnabled (false);


        // CHOPLIST
        chopList.reset(new ChopListComponent(processor.getChopTree(), selectedChopId));
        addAndMakeVisible (chopList.get());

        // formats
        formatManager.registerBasicFormats();

        // listeners
        thumbnail->addChangeListener (this);
        transportSource.addChangeListener (this);

        selectedChopId = NONE;
        selectedChopId.addListener (this);

        userSelectionActive = false;
        userSelectionActive.addListener (this);

        thread.startThread (3);     

        setOpaque (true);
        setSize (1024, 576);
    }

    ~SamplerAudioProcessorEditor()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (COLOR_BGDARK);

        g.setColour (COLOR_BG);
        g.fillRect (getLocalBounds().reduced(4));
        
        g.setColour (COLOR_BLUEDARK);
        g.fillRect (rectThumbnail);

        auto strokeRect = [&g] (Rectangle<int> r, int s)
        {
            auto path = Path();
            path.addRectangle (r);
            g.strokePath (path, PathStrokeType (s), {});
        };


        g.setColour (COLOR_BGDARK);
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

        startTimeChopButton.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth).reduced(3));  // flex3
        selectionChopButton.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth).reduced(3));  // flex3

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
    TextButton selectionChopButton { "Chop selection", "Creates a new chop out of the current selection" };

    ToggleButton followTransportButton { "Follow Transport" };

    Label loopLabel { "loopLabel", "Loop" };
    ToggleButton fullLoopToggle { "F" };
    ToggleButton selectionLoopToggle { "S" };

    Value selectedChopId;
    Value userSelectionActive;
    
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

    TransportState state;

    //==============================================================================

    void loadFile()
    {
        FileChooser fc ("Choose a Wave or MP3 file...", {}, "*wav;*mp3", true);

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
            if (processor.getChopTree().getNumChildren()) 
            {
                processor.clearChopTree();
                chopList->reloadData();
            }

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
        auto chop = Chop { currentTime, transportSource.getLengthInSeconds(), "" , false };
        int chopKey = processor.addChop (chop);
    }

    void selectionChopClicked()
    {
        auto bounds = thumbnail->getSelectionBounds();
        auto chop = Chop { bounds.first, bounds.second, "", false };
        int chopKey = processor.addChop (chop);
    }

    void valueChanged (Value& value)
    {
        if (value.refersToSameSourceAs(selectedChopId))
        {
            int selectedChopIdValue = int(value.getValue());
            thumbnail->setSelectedChopId(selectedChopIdValue);
            chopList->selectRow(selectedChopIdValue);
        }

        if (value.refersToSameSourceAs(userSelectionActive))
        {
            selectionChopButton.setEnabled (bool (userSelectionActive.getValue()));
            selectionLoopToggle.setEnabled (bool (userSelectionActive.getValue()));
            if (!bool (userSelectionActive.getValue()) && selectionLoopToggle.getToggleState())
            {
                selectionLoopToggle.setToggleState (false, dontSendNotification);
                currentAudioFileSource.get()->setLooping(false);
            }
        }
    }

    void updateFollowTransportState()
    {
        thumbnail->setFollowsTransport (followTransportButton.getToggleState());
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else if (state == Stopping || state == Playing)
                changeState (Stopped);
            else if (state == Pausing)
                changeState (Paused);
        }
        if (source == thumbnail.get())
        {
            if (thumbnail.get()->getNewFileDropped())
            {
                showAudioResource (File (thumbnail->getLastDroppedFile()));
                thumbnail->unsetNewFileDropped();
            }
            else
            {
                if (selectionLoopToggle.getToggleState())
                {
                    auto bounds = thumbnail->getSelectionBounds();
                    if (transportSource.getCurrentPosition() >= bounds.second)
                    {
                        transportSource.setPosition (bounds.first);
                    }
                }
                currentPositionLabel.setText ("Position: " + (String) transportSource.getCurrentPosition(),
                                              NotificationType::dontSendNotification);
            }
        }
    }
    
    void valueTreeChildAdded (ValueTree& parentTree,
                              ValueTree& childWhichHasBeenAdded)
    {
        thumbnail->addChopMarker(childWhichHasBeenAdded[PROP_ID]);
        chopList->reloadData();
    }
   
    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged,
                                   const Identifier& property)
    {

    }

    void valueTreeChildRemoved (ValueTree& parentTree,
                                ValueTree& childWhichHasBeenRemoved,
                                int indexFromWhichChildWasRemoved)
    {
        thumbnail->removeChopMarker(childWhichHasBeenRemoved[PROP_ID]);
        processor.getChopMap()->remove(childWhichHasBeenRemoved[PROP_ID]);

        if (!processor.getChopTree().getNumChildren())
        {
            thumbnail->setSelectedChopId(NONE);
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
