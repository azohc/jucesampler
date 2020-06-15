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
#include "ChopSettingsComponent.h"
#include "SamplerAudioSource.h"

//==============================================================================

class SamplerAudioProcessorEditor :
    public AudioProcessorEditor,
    public ChangeListener,
    public ValueTree::Listener,
    public Value::Listener
{
public:
    SamplerAudioProcessorEditor (SamplerAudioProcessor& p,
                                 AudioTransportSource& transport,
                                 SamplerAudioSource& sampler,
                                 AudioDeviceManager& device,
                                 Value selectedChop,
                                 Value selectionActive)
        : AudioProcessorEditor (&p),
        processor (p),
        state (Stopped),
        transportSource (transport),
        samplerSource (sampler),
        deviceManager (device),
        selectedChopId (selectedChop),
        userSelectionActive (selectionActive)
    {

        // THUMBNAIL FUNCTIONS
        addAndMakeVisible (currentPositionLabel);
        currentPositionLabel.setFont (Font (15.00f, Font::plain));
        currentPositionLabel.setJustificationType (Justification::left);
        currentPositionLabel.setEditable (false, false, false);
        //currentPositionLabel.setColour (Label::backgroundWhenEditingColourId)
        currentPositionLabel.setColour (Label::backgroundColourId, COLOR_BG);
        currentPositionLabel.setColour (Label::textColourId, COLOR_FG);
      
        addAndMakeVisible (fullLoopToggle);
        fullLoopToggle.setColour (TextButton::buttonColourId, COLOR_BG);
        fullLoopToggle.setColour (TextButton::textColourOffId, COLOR_FG);
        fullLoopToggle.setEnabled (false);
        fullLoopToggle.onClick = [this] { 
            auto l = !processor.getFileReaderSource()->isLooping();
            processor.getFileReaderSource()->setLooping(l);
            fullLoopToggle.setToggleState (l, dontSendNotification);
            if (selectionLoopToggle.getToggleState()) {
                selectionLoopToggle.triggerClick();
            }
        };

        addAndMakeVisible (selectionLoopToggle);
        selectionLoopToggle.setColour (TextButton::buttonColourId, COLOR_BG);
        selectionLoopToggle.setColour (TextButton::textColourOffId, COLOR_FG);
        selectionLoopToggle.setEnabled (false);
        selectionLoopToggle.setClickingTogglesState(true);
        selectionLoopToggle.onClick = [this] { 
            processor.getFileReaderSource()->setLooping(selectionLoopToggle.getToggleState());
            if (fullLoopToggle.getToggleState()) {
                fullLoopToggle.triggerClick();
            }
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
        zoomSlider.onValueChange = [this] { processor.getThumbnail()->setZoomFactor (zoomSlider.getValue()); };
        zoomSlider.setEnabled (false);
        // zoomSlider.setSkewFactor (2);
        zoomSlider.setColour (Slider::backgroundColourId, COLOR_BLUE.darker(0.6));
        zoomSlider.setColour (Slider::trackColourId, COLOR_BLUE.darker(0.3));
        zoomSlider.setColour (Slider::thumbColourId, COLOR_BLUE);

        // THUMBNAIL 
        processor.resetThumbnailTo (new SamplerThumbnail (processor.getFormatManager(), transportSource, zoomSlider, processor.getChopTree(), processor.getChopBounds() ,selectedChopId, userSelectionActive));
        addAndMakeVisible (processor.getThumbnail());


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

        addAndMakeVisible (chopButton);
        chopButton.setColour (TextButton::buttonColourId, COLOR_BG);
        chopButton.setColour (TextButton::textColourOffId, COLOR_FG);
        chopButton.onClick = [this] { chopButtonClicked(); };
        chopButton.setEnabled (false);

        addAndMakeVisible (chopThresholdSlider);
        chopThresholdSlider.setSliderStyle(Slider::LinearHorizontal);
        chopThresholdSlider.setColour (Slider::trackColourId, COLOR_BG);
        chopThresholdSlider.setColour (Slider::backgroundColourId, COLOR_BG_DARK);
        chopThresholdSlider.setColour (Slider::thumbColourId, COLOR_FG);
        chopThresholdSlider.setRange (0, 3.6, 0.01);
        chopThresholdSlider.setEnabled (false);
        chopThresholdSlider.onDragEnd = [this] { updateDetectedChopsLabel(); };

        addAndMakeVisible (onsetMethodButton);
        onsetMethodButton.setEnabled (false);
        onsetMethodButton.setColour (TextButton::buttonColourId, COLOR_BG);
        onsetMethodButton.setColour (TextButton::textColourOffId, COLOR_FG);
        onsetMethodButton.setButtonText (ONSET_ENERGY);
        onsetMethodButton.onClick = [this] {
            onsetMethodNumber++;
            if (onsetMethodNumber == sizeof (ONSET_METHODS) / sizeof (ONSET_METHODS[0])) {
                onsetMethodNumber = 0;
            }
            onsetMethodButton.setButtonText (ONSET_METHODS[onsetMethodNumber]);
            updateDetectedChopsLabel();
        };

        addAndMakeVisible (detectedChopNumberLabel);
        detectedChopNumberLabel.setFont (Font (15.00f, Font::plain));
        detectedChopNumberLabel.setJustificationType (Justification::centredTop);
        detectedChopNumberLabel.setEditable (false, false, false);
        //currentPositionLabel.setColour (Label::backgroundWhenEditingColourId)
        detectedChopNumberLabel.setColour (Label::backgroundColourId, COLOR_BG);
        detectedChopNumberLabel.setColour (Label::textColourId, COLOR_FG);

        // CHOPLIST
        chopList.reset(new ChopListComponent(processor.getChopTree(), selectedChopId));
        addAndMakeVisible (chopList.get());

        // CHOPSETTINGS
        chopSettings.reset(new ChopSettingsComponent(selectedChopId, processor.playbackMode, processor.getChopTree(), processor.getLastRecordedMidiNote()));
        processor.setListenerForMidiLearn (chopSettings.get()->listenForMidiLearn);
        addAndMakeVisible (chopSettings.get());

        // listeners
        processor.getThumbnail()->addChangeListener (this);
        transportSource.addChangeListener (this);

        selectedChopId = NONE;
        selectedChopId.addListener (this);

        userSelectionActive = false;
        userSelectionActive.addListener (this);

        onsetMethodNumber = 0; // ONSET_ENERGY as default

        thread.startThread (3);     

        setOpaque (true);
        setSize (1024, 576);
    }

    ~SamplerAudioProcessorEditor()
    {
        transportSource.removeAllChangeListeners();
        processor.getThumbnail()->removeAllChangeListeners();
        processor.getThumbnail()->deleteAllMarkers();
        chopList.reset();
        chopSettings.reset();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (COLOR_BG_DARK);

        g.setColour (COLOR_BG);
        g.fillRect (getLocalBounds().reduced(4));
        
        g.setColour (COLOR_BLUE_DARK);
        g.fillRect (rectThumbnail);

        g.setColour (COLOR_BG_DARK);
        strokeRect (g, rectControls, 2);
        strokeRect (g, rectThumbnail, 2);
        strokeRect (g, rectThumbnailFuncts, 2);
        strokeRect (g, rectChopSettings, 2);
        strokeRect (g, rectChopList, 2);
    }

    void resized() override
    {
        auto goldenr = 1.618;
        auto small = [goldenr] (int d) { return d - (d / goldenr); };
        auto large = [goldenr] (int d) { return d / goldenr; };

        auto r = getLocalBounds().reduced (4);

        rectChopSettings = r.removeFromBottom (small (r.getHeight()));
        rectControls = r.removeFromLeft (small (small (r.getWidth())));
        rectChopList = r.removeFromBottom (small (r.getHeight()));
        rectThumbnailFuncts = r.removeFromTop (small (small (r.getHeight())));
        
        // Controls
        auto rectControlsAux = rectControls;
        auto buttonHeight = small (small (rectControlsAux.getHeight()));

        loadButton.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));
        playButton.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));
        stopButton.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));
        chopButton.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));
        onsetMethodButton.setBounds (rectControlsAux.removeFromTop (buttonHeight * 0.7).reduced (4));
        chopThresholdSlider.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));
        detectedChopNumberLabel.setBounds (rectControlsAux.removeFromTop (buttonHeight).reduced (4));

        auto rectThumbnailFunctsAux = rectThumbnailFuncts.reduced(1);
        auto functionWidth = small (small (rectThumbnailFunctsAux.getWidth()));
        currentPositionLabel.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth));
        fullLoopToggle.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth).reduced(3));
        selectionLoopToggle.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth*2).reduced(3));
        startTimeChopButton.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth).reduced(3));  
        selectionChopButton.setBounds (rectThumbnailFunctsAux.removeFromLeft (functionWidth).reduced(3));

        // Choplist
        chopList->setBounds (rectChopList);

        // Thumbnail
        rectThumbnail = r;
        auto rectThumbnailAux = rectThumbnail;
        zoomSlider.setBounds (rectThumbnailAux.removeFromRight (small (small (rectThumbnailAux.getHeight()))));
        processor.getThumbnail()->setBounds (rectThumbnailAux.reduced(1));

        // Chop settings
        chopSettings.get()->setBounds (rectChopSettings);
    }

    void showAudioResource (const File& file)
    {
        if (loadFileIntoTransport (file))
        {
            zoomSlider.setEnabled (true);
            zoomSlider.setValue (0, dontSendNotification);
            processor.getThumbnail()->setFile (file);
            playButton.setEnabled (true);
            fullLoopToggle.setEnabled (true);
            startTimeChopButton.setEnabled (true);
            chopButton.setEnabled (true);
            chopThresholdSlider.setEnabled (true);
            chopThresholdSlider.setValue (1);
            updateDetectedChopsLabel();
            onsetMethodButton.setEnabled (true);
            chopList->reloadData();
            chopSettings.get()->setMaxSliderValue(transportSource.getLengthInSeconds());
        }
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

    SamplerAudioProcessor& processor;

    AudioDeviceManager& deviceManager;
    TimeSliceThread thread { "audio file preview" };


    AudioTransportSource& transportSource;
    SamplerAudioSource& samplerSource;

    Slider zoomSlider { Slider::LinearVertical, Slider::NoTextBox };

    std::unique_ptr<ChopListComponent> chopList;

    std::unique_ptr<ChopSettingsComponent> chopSettings;


    int onsetMethodNumber;
    int lastMidiNoteAssigned = INIT_NOTE_AUTO_ASSIGN;

    //==============================================================================
    // CONTROLS
    TextButton playButton { "Play" };
    TextButton stopButton { "Stop" };   
    TextButton loadButton { "Load" };
    TextButton chopButton { "Chop" };
    TextButton onsetMethodButton;
    Slider chopThresholdSlider { "Chop Threshold" };
    Label detectedChopNumberLabel { "detectedChopNumberLabel", "0\nDetected Chops" };


    //==============================================================================
    // THUMBNAIL FUNCTIONS
    Label currentPositionLabel { "currentPositionLabel", "Position: " };
    TextButton startTimeChopButton { "Chop From Here", "Creates a new chop with a start time equal to the current position" };
    TextButton selectionChopButton { "Chop Selection", "Creates a new chop out of the current selection" };

    ToggleButton followTransportButton { "Follow Transport" };
    ToggleButton fullLoopToggle { "Toggle Full Loop" };
    ToggleButton selectionLoopToggle { "Toggle Selection Loop" };

    Value selectedChopId;
    Value userSelectionActive;

    //==============================================================================
    // RECTANGLES
    Rectangle<int> rectChopSettings;
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
        FileChooser fc ("Choose a WAVE file...", {}, "*wav", true);

        if (fc.browseForFileToOpen())
        {
            processor.setCurrentFile(fc.getResult());
            processor.getChopTree().removeAllChildren(nullptr);
            showAudioResource(processor.getFile());
        }
    }

    bool loadFileIntoTransport (const File& file)
    {
        transportSource.stop();
        transportSource.setSource (nullptr);

        AudioFormatReader* reader = processor.getFormatManager()->createReaderFor (file);

        if (reader != nullptr)
        {
            processor.resetFileReaderSourceTo (new AudioFormatReaderSource (reader, true));
            transportSource.setSource (processor.getFileReaderSource(), 32768, &thread, reader->sampleRate);
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
        auto sr = processor.getFileReaderSource()->getAudioFormatReader()->sampleRate;
        auto currentTime = transportSource.getCurrentPosition();
        auto lengthTime = transportSource.getLengthInSeconds();

        ValueTree chopState (ID_CHOP);
        Chop c (chopState);
        c.setStartTime (currentTime);
        c.setStartSample (int64(currentTime * sr));
        c.setEndTime (lengthTime);
        c.setEndSample (int64(lengthTime * sr));
        c.setHidden (false);
        c.setTriggerNote (lastMidiNoteAssigned++);
        c.setDefaultADSR();
        processor.addChop (c);
    }

    void selectionChopClicked()
    {
        auto bounds = processor.getThumbnail()->getSelectionBounds();
        auto sr = processor.getFileReaderSource()->getAudioFormatReader()->sampleRate;
        ValueTree chopState (ID_CHOP);
        Chop c (chopState);
        if (bounds.first < bounds.second) {
            c.setStartTime (bounds.first);
            c.setStartSample (bounds.first * sr);
            c.setEndTime (bounds.second);    
            c.setEndSample (bounds.second * sr);
        } else {
            c.setStartTime (bounds.second);
            c.setStartSample (bounds.second * sr);
            c.setEndTime (bounds.first); 
            c.setEndSample (bounds.first * sr);
        }
        c.setTriggerNote (lastMidiNoteAssigned++);
        c.setHidden (false);
        c.setDefaultADSR();
        processor.addChop (c);
    }

    void chopButtonClicked() 
    {
        detectChopsOnset (true);
    }

    int detectChopsOnset (bool createChops) {
        auto file = processor.getFile().getFullPathName().getCharPointer();
        uint_t samplerate = processor.getFileReaderSource()->getAudioFormatReader()->sampleRate;
        uint_t buf_size = 1024;
        uint_t hop_size = 256;
        uint_t n_frames = 0, read = 0;
        auto a_source = new_aubio_source(file, samplerate, hop_size);
        auto a_onset = new_aubio_onset(ONSET_METHODS[onsetMethodNumber].getCharPointer(), buf_size, hop_size, aubio_source_get_samplerate(a_source));
        auto threshset = aubio_onset_set_threshold(a_onset, chopThresholdSlider.getValue());
        fvec_t * in = new_fvec (hop_size);
        fvec_t * out = new_fvec (2);

        auto detections = Array<smpl_t>();
        do {
            aubio_source_do(a_source, in, &read);
            aubio_onset_do(a_onset, in, out);
            if (out->data[0] != 0) {
                detections.addIfNotAlreadyThere(aubio_onset_get_last_s(a_onset));
            }
            n_frames += read;
        } while ( read == hop_size );
        del_aubio_source(a_source);
        del_aubio_onset(a_onset);

        auto numDetectedChops = detections.size();
        if (numDetectedChops == 0)              return 0;
        else if (!createChops)    return numDetectedChops;
        
        for (auto i = 0; i < detections.size(); i++) 
        {
            ValueTree chopState (ID_CHOP);
            Chop c (chopState);
            c.setStartTime (detections[i]);
            c.setStartSample (detections[i] * samplerate);
            c.setEndTime (i == detections.size() - 1 ? transportSource.getLengthInSeconds() : detections[i + 1]);
            c.setEndSample (c.getEndTime() * samplerate);
            c.setHidden (false);
            c.setTriggerNote (lastMidiNoteAssigned++);
            c.setDefaultADSR();
            processor.addChop (c);
        }
        return numDetectedChops;
    }

    void updateDetectedChopsLabel () {
        auto v = chopThresholdSlider.getValue();
        auto n = detectChopsOnset(false);
        detectedChopNumberLabel.setText (String(n) + "\nDetected Chops", dontSendNotification);
    }

    void valueChanged (Value& value) override
    {
        if (value.refersToSameSourceAs(selectedChopId))
        {
            int selectedChopIdValue = int(value.getValue());
            processor.getThumbnail()->setSelectedChopId(selectedChopIdValue);
            chopList->selectRow(selectedChopIdValue);
            chopSettings->displayChop(selectedChopIdValue);
        }

        if (value.refersToSameSourceAs(userSelectionActive))
        {
            selectionChopButton.setEnabled (bool (userSelectionActive.getValue()));
            selectionLoopToggle.setEnabled (bool (userSelectionActive.getValue()));
            if (!bool (userSelectionActive.getValue()) && selectionLoopToggle.getToggleState())
            {
                selectionLoopToggle.setToggleState (false, dontSendNotification);
                processor.getFileReaderSource()->setLooping(false);
            }
        }
    }

    void updateFollowTransportState()
    {
        processor.getThumbnail()->setFollowsTransport (followTransportButton.getToggleState());
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
        if (source == processor.getThumbnail())
        {
            if (selectionLoopToggle.getToggleState())
            {
                auto bounds = processor.getThumbnail()->getSelectionBounds();
                bounds = bounds.first < bounds.second ? std::make_pair(bounds.first, bounds.second) : std::make_pair(bounds.second, bounds.first);
                if (transportSource.getCurrentPosition() >= bounds.second || transportSource.getCurrentPosition() < bounds.first)
                {
                    transportSource.setPosition (bounds.first);
                }
            }
            auto getLabel = [this] (double d) {
                auto str = (String) d;
                return (str.length() > 5) ? str.substring(0, 5) : str;
            };
            currentPositionLabel.setText ("Position: " + getLabel(transportSource.getCurrentPosition()),
                                            dontSendNotification);
        }
    }
    
    void valueTreeChildAdded (ValueTree& parentTree,
                              ValueTree& childWhichHasBeenAdded) override
    {
        processor.getThumbnail()->addChopMarker(childWhichHasBeenAdded[ID_CHOPID]);
        chopList->reloadData();
        samplerSource.makeSoundsFromChops(processor.getFileReaderSource()->getAudioFormatReader(), processor.getChopTree());
    }
   
    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged,
                                   const Identifier& property) override
    {
        chopList->reloadData();
        chopSettings->displayChop(selectedChopId.getValue());
        samplerSource.makeSoundsFromChops(processor.getFileReaderSource()->getAudioFormatReader(), processor.getChopTree());
    }

    void valueTreeChildRemoved (ValueTree& parentTree,
                                ValueTree& childWhichHasBeenRemoved,
                                int indexFromWhichChildWasRemoved) override
    {  
        if (parentTree.getNumChildren() == 0)
        {
            processor.getThumbnail()->setSelectedChopId(NONE);
            lastMidiNoteAssigned = INIT_NOTE_AUTO_ASSIGN;
        }
        processor.removeChop(childWhichHasBeenRemoved[ID_CHOPID]);
        samplerSource.makeSoundsFromChops(processor.getFileReaderSource()->getAudioFormatReader(), processor.getChopTree());
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
