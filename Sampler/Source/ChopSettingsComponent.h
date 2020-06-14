/*
  ==============================================================================

    ChopSettingsComponent.h
    Created: 5 Apr 2020 9:28:32am
    Author:  wozas

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Constants.h"

//==============================================================================
/*
*/
class ChopSettingsComponent    : public Component, public Value::Listener
{
public:
    ChopSettingsComponent(Value selected, 
                          ValueTree chops,
                          Value lastRecordedMidiNoteValue) :
        selectedChop (selected), 
        chopTree(chops),
        lastRecordedMidiNote (lastRecordedMidiNoteValue)
    {
        addAndMakeVisible (selectedChopLabel);
        selectedChopLabel.setFont (Font (15.00f, Font::plain));
        selectedChopLabel.setJustificationType (Justification::centred);
        selectedChopLabel.setEditable (false, false, false);
        //selectedChopLabel.setColour (Label::backgroundWhenEditingColourId)
        selectedChopLabel.setColour (Label::backgroundColourId, COLOR_BG);
        selectedChopLabel.setColour (Label::textColourId, COLOR_FG);
        selectedChopLabel.setColour (Label::outlineColourId, COLOR_BG);
        selectedChopLabel.setText (SEL_CHOP_NONE, dontSendNotification);

        addAndMakeVisible (triggerNoteLabel);
        triggerNoteLabel.setFont (Font (15.00f, Font::plain));
        triggerNoteLabel.setJustificationType (Justification::centredRight);
        triggerNoteLabel.setEditable (false, false, false);
        //triggerNoteLabel.setColour (Label::backgroundWhenEditingColourId)
        triggerNoteLabel.setColour (Label::backgroundColourId, COLOR_BG);
        triggerNoteLabel.setColour (Label::textColourId, COLOR_FG);
        triggerNoteLabel.setColour (Label::outlineColourId, COLOR_BG);
        triggerNoteLabel.setVisible (false);

        addAndMakeVisible (triggerNoteComboBox);
        triggerNoteComboBox.setColour (ComboBox::outlineColourId, COLOR_BG_DARK);
        triggerNoteComboBox.setColour (ComboBox::backgroundColourId, COLOR_BG);
        triggerNoteComboBox.setColour (ComboBox::buttonColourId, COLOR_BG_DARK);
        triggerNoteComboBox.setColour (ComboBox::arrowColourId, COLOR_BG_DARK);
        triggerNoteComboBox.setColour (ComboBox::textColourId, COLOR_FG);
        triggerNoteComboBox.setVisible (false);

        addAndMakeVisible (midiLearnToggleButton);
        midiLearnToggleButton.setButtonText (MIDI_LEARN);
        midiLearnToggleButton.setColour (ToggleButton::textColourId, COLOR_FG);
        midiLearnToggleButton.setColour (ToggleButton::tickColourId, COLOR_RED);
        midiLearnToggleButton.setEnabled (true);
        midiLearnToggleButton.setVisible (false);
        midiLearnToggleButton.onClick = [this] { 
            auto toggledOn = midiLearnToggleButton.getToggleState();
            listenForMidiLearn = toggledOn;
            midiLearningLabel.setVisible (toggledOn);
            if (!toggledOn && lastRecordedMidiNoteDirty)
            { // button toggled off
                auto chop = Chop(chopTree, selectedChop.getValue());
                chop.setTriggerNote(lastRecordedMidiNote.getValue());
            } else { lastRecordedMidiNoteDirty = false; }
        };

        addAndMakeVisible (midiLearningLabel);
        midiLearningLabel.setFont (Font (15.00f, Font::plain));
        midiLearningLabel.setJustificationType (Justification::centredLeft);
        midiLearningLabel.setEditable (false, false, false);
        midiLearningLabel.setColour (Label::backgroundColourId, COLOR_BG);
        midiLearningLabel.setColour (Label::textColourId, COLOR_FG);
        midiLearningLabel.setColour (Label::outlineColourId, COLOR_BG);
        midiLearningLabel.setText (LISTENING, dontSendNotification);
        midiLearningLabel.setVisible (false);

        playbackMode = MONO;
        addAndMakeVisible (playbackButton);
        playbackButton.setButtonText (PLAYBACK_MONO);
        playbackButton.setColour (TextButton::buttonColourId, COLOR_BG_DARK);
        playbackButton.setColour (TextButton::buttonOnColourId, COLOR_BG);
        playbackButton.setEnabled (true);
        playbackButton.setVisible (false);
        playbackButton.onClick = [this]
        {
            if (playbackMode == MONO)
            {
                playbackMode = POLY;
                playbackButton.setButtonText (PLAYBACK_POLY);

            } else
            {
                playbackMode = MONO;
                playbackButton.setButtonText (PLAYBACK_MONO);

            }
        };

        addAndMakeVisible (playbackModeLabel);
        playbackModeLabel.setFont (Font (15.00f, Font::plain));
        playbackModeLabel.setJustificationType (Justification::centredRight);
        playbackModeLabel.setEditable (false, false, false);
        playbackModeLabel.setColour (Label::backgroundColourId, COLOR_BG);
        playbackModeLabel.setColour (Label::textColourId, COLOR_FG);
        playbackModeLabel.setColour (Label::outlineColourId, COLOR_BG);
        playbackModeLabel.setText (PLAYBACK_MODE, dontSendNotification);
        playbackModeLabel.setVisible (false);

        addAndMakeVisible (prevChopArrow);
        prevChopArrow.setEnabled (false); 
        prevChopArrow.setVisible (false);
        prevChopArrow.onClick = [this] { selectedChop = getPrevChopId(); };

        addAndMakeVisible (nextChopArrow);
        nextChopArrow.setEnabled (false);
        nextChopArrow.setVisible (false);
        nextChopArrow.onClick = [this] { selectedChop = getNextChopId(); };

        addAndMakeVisible (attackLS);
        attackLS.setVisible (false);
        addAndMakeVisible (decayLS);
        decayLS.setVisible (false);
        addAndMakeVisible (sustainLS);
        sustainLS.setVisible (false);
        addAndMakeVisible (releaseLS);
        releaseLS.setVisible (false);

        int midiNoteNr = FIRST_MIDI_NOTE;

        for (auto octave : { 0, 1, 2, 3, 4, 5 })
        {
            for (String note : { "C", "D", "E", "F", "G", "A", "B" })
            {
                triggerNoteComboBox.addItem (note + String(octave), midiNoteNr++);
                if (note != "E" && note != "B")
                {
                    triggerNoteComboBox.addItem(note + String(octave) + "#", midiNoteNr++);
                }
            }
        }

        triggerNoteComboBox.onChange = [this] {
            auto noteId = triggerNoteComboBox.getSelectedIdAsValue();
            Chop (chopTree, selectedChop.getValue()).setTriggerNote(noteId.getValue());
        };

        lastRecordedMidiNote.addListener (this);
    }

    ~ChopSettingsComponent()
    {
    }


    void displayChop(int selectedChopId)
    {
        triggerNoteLabel.setVisible (selectedChopId != NONE);
        triggerNoteComboBox.setVisible (selectedChopId != NONE);
        midiLearnToggleButton.setVisible (selectedChopId != NONE);
        prevChopArrow.setVisible (selectedChopId != NONE);
        nextChopArrow.setVisible (selectedChopId != NONE);
        playbackButton.setVisible (selectedChopId != NONE);
        playbackModeLabel.setVisible (selectedChopId != NONE);
        attackLS.setVisible (selectedChopId != NONE);
        decayLS.setVisible (selectedChopId != NONE);
        sustainLS.setVisible (selectedChopId != NONE);
        releaseLS.setVisible (selectedChopId != NONE);
        if (selectedChopId == NONE)
        {
            selectedChopLabel.setText(SEL_CHOP_NONE, dontSendNotification);
        } else
        {
            auto chop = Chop(chopTree, selectedChopId);
            auto numChops = chopTree.getNumChildren();
            prevChopArrow.setEnabled (numChops > 1);
            nextChopArrow.setEnabled (numChops > 1);
            selectedChopLabel.setText ("Chop " + String(selectedChopId) + " selected", dontSendNotification);
            triggerNoteLabel.setText ("Mapped To", dontSendNotification);
            triggerNoteComboBox.setSelectedId (chop.getTriggerNote(), dontSendNotification);
            
            attackLS.setSliderValue (chop.getAttack());
            decayLS.setSliderValue (chop.getDecay());
            sustainLS.setSliderValue (chop.getSustain());
            releaseLS.setSliderValue (chop.getRelease());
        }
    }

    Array<int> getChopIds()
    {
        Array<int> chopIds;
        print("ids:");
        for (auto it = chopTree.begin(); it != chopTree.end(); ++it)
        {
            auto chop = Chop(*it);
            chopIds.add (chop.getId());
            print(String(chop.getId()));
        }
        return chopIds;
    }

    int getNextChopId()
    {
        auto chopIds = getChopIds();
        auto i = chopIds.indexOf(selectedChop.getValue());
        jassert(i != -1);
        while(true)
        {
            if (i == chopIds.size() - 1)
                i = 0;
            else
                i++;
            if (chopIds.contains(i))
                return i;
        }
    }

    int getPrevChopId()
    {
        auto chopIds = getChopIds();
        auto i = chopIds.indexOf(selectedChop.getValue());
        jassert(i != -1);
        while (true)
        {
            if (i == 0)
                i = chopIds.size() - 1;
            else
                i--;
            if (chopIds.contains(i))
                return i;
        }
    }

    void valueChanged(Value &value) override
    {
        if (bool(listenForMidiLearn.getValue()) && value.refersToSameSourceAs(lastRecordedMidiNote))
        {
            lastRecordedMidiNoteDirty = true;
            triggerNoteComboBox.setSelectedId (lastRecordedMidiNote.getValue(), false);
            midiLearnToggleButton.triggerClick();
        }
    }

    void sliderDragEnded (Slider * slider)
    {
        auto chop = Chop(chopTree, selectedChop.getValue());
        if (slider == &attackLS.slider)
        {
            chop.setAttack (slider->getValue());
        } else if (slider == &decayLS.slider)
        {
            chop.setDecay (slider->getValue());
        } else if (slider == &sustainLS.slider)
        {
            chop.setSustain (slider->getValue());
        } else
        {
            chop.setRelease (slider->getValue());
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (COLOR_BG);
        g.setColour (COLOR_BG_DARK);
        strokeRect (g, getLocalBounds(), 2);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (4);
        rectChopIdAndTrigger = r.removeFromTop (r.getHeight() * 0.11);
        auto rectChopIdAndTriggerAux = rectChopIdAndTrigger;
    
        auto chopIdLabelWidth = rectChopIdAndTriggerAux.getWidth() * 0.111;
        auto triggerLabelWidth = rectChopIdAndTriggerAux.getWidth() * 0.09;
        auto triggerComboBoxWidth = rectChopIdAndTriggerAux.getWidth() * 0.066;
        auto arrowRectWidth = rectChopIdAndTriggerAux.getWidth() * 0.01;
        auto arrowSize = rectChopIdAndTriggerAux.getHeight() * 0.6;
        auto midiLearnButtonWidth = rectChopIdAndTriggerAux.getWidth() * 0.09;
        auto midiLearningLabelWidth = rectChopIdAndTriggerAux.getWidth() * 0.15;
        auto playbackButtonWidth = rectChopIdAndTriggerAux.getWidth() * 0.05;

        prevChopArrow.setBounds (rectChopIdAndTriggerAux.removeFromLeft (arrowRectWidth));
        prevChopArrow.setTopLeftPosition (prevChopArrow.getX() + (arrowRectWidth * 0.18),
                                          rectChopIdAndTriggerAux.getCentreY() - rectChopIdAndTriggerAux.getHeight() * 0.2);
        prevChopArrow.setSize (arrowSize, arrowSize);

        selectedChopLabel.setBounds (rectChopIdAndTriggerAux.removeFromLeft (chopIdLabelWidth));

        nextChopArrow.setBounds (rectChopIdAndTriggerAux.removeFromLeft (arrowRectWidth));
        nextChopArrow.setTopLeftPosition (nextChopArrow.getX(),
                                          rectChopIdAndTriggerAux.getCentreY() - rectChopIdAndTriggerAux.getHeight() * 0.2);
        nextChopArrow.setSize (arrowSize, arrowSize);

        triggerNoteLabel.setBounds (rectChopIdAndTriggerAux.removeFromLeft (triggerLabelWidth));
        triggerNoteComboBox.setBounds (rectChopIdAndTriggerAux.removeFromLeft (triggerComboBoxWidth));
        midiLearnToggleButton.setBounds (rectChopIdAndTriggerAux.removeFromLeft (midiLearnButtonWidth));
        midiLearningLabel.setBounds (rectChopIdAndTriggerAux.removeFromLeft (midiLearningLabelWidth));
        playbackButton.setBounds (rectChopIdAndTriggerAux.removeFromRight (playbackButtonWidth));
        playbackModeLabel.setBounds (rectChopIdAndTriggerAux.removeFromRight (midiLearningLabelWidth));

        rectSettingsAndArrows = r;
        auto rectSettingsAndArrowsAux = rectSettingsAndArrows;
        auto adsrLSWidth = rectSettingsAndArrowsAux.getWidth() / 4;
        attackLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
        decayLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
        sustainLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
        releaseLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
    }

    Value listenForMidiLearn;
    Value playbackMode;


private:
    class LabelledSlider : public Component
    {
    public:
        LabelledSlider (const String& labelText, ChopSettingsComponent& chopSettings) : owner(chopSettings)
        {
            addAndMakeVisible (slider);
            slider.setEnabled (true);
            slider.setSliderStyle (Slider::LinearVertical);
            slider.setColour (Slider::trackColourId, COLOR_BG_LIGHT);
            slider.setColour (Slider::backgroundColourId, COLOR_BG_DARK);
            slider.setColour (Slider::thumbColourId, COLOR_FG);
            slider.setRange (0.0, 10.0, 0.01);
            slider.onDragEnd = [this]
            {
                owner.sliderDragEnded (&slider);
            };

            addAndMakeVisible (label);
            label.setColour (Label::textColourId, COLOR_FG);
            label.setJustificationType (Justification::horizontallyCentred);
            label.setText (labelText, dontSendNotification);
        }

        void paint (Graphics& g) override
        {
            g.fillAll (COLOR_BG);
            g.setColour (COLOR_BG_DARK);
        }

        void resized() override
        {
            auto r = getLocalBounds().reduced (4);
            auto sliderHeight = r.getHeight() * 0.75;
            auto labelHeight = r.getHeight() * 0.1;
            auto width = r.getWidth();
            slider.setTextBoxStyle (Slider::TextBoxBelow, false, width*0.4, labelHeight);

            label.setBounds (r.removeFromBottom (labelHeight));
            slider.setBounds (r);
            slider.setCentrePosition (r.getWidth() / 2, r.getHeight() / 2);
        }
        void setSliderValue(double d)
        {
            slider.setValue(d);
        }

        Slider slider { Slider::LinearVertical, Slider::TextBoxBelow };
    private:
        ChopSettingsComponent &owner;
        Label label;
    };

    ValueTree chopTree;
    Value selectedChop;
    Value lastRecordedMidiNote;
    bool lastRecordedMidiNoteDirty = false;

    Rectangle<int> rectChopIdAndTrigger;
    Rectangle<int> rectSettingsAndArrows;
    Label selectedChopLabel;
    Label triggerNoteLabel;
    Label midiLearningLabel;
    Label playbackModeLabel;
    ComboBox triggerNoteComboBox;
    ToggleButton midiLearnToggleButton;
    TextButton playbackButton;
    ArrowButton prevChopArrow { "PREV", 0.5, COLOR_GRAY_LIGHT };
    ArrowButton nextChopArrow { "NEXT", 0.0, COLOR_GRAY_LIGHT };
    LabelledSlider attackLS { "Attack", *this }, decayLS { "Decay", *this }, sustainLS { "Sustain", *this }, releaseLS { "Release", *this };


    const String SEL_CHOP_NONE = "No Chop Selected";
    const String TRIGGER_NOTE_LABEL = "Trigger Note: ";
    const String MIDI_LEARN = "MIDI Learn";
    const String LISTENING = "Press New Trigger Note";
    const String PLAYBACK_MODE = "Playback Mode";
    const String PLAYBACK_MONO = "MONO";
    const String PLAYBACK_POLY = "POLY";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopSettingsComponent)
};
