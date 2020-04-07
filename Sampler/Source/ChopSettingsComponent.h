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
                          HashMap<int, SamplerSound*>* chopSoundsMap, 
                          ValueTree chops,
                          Value lastRecordedMidiNoteValue) :
        selectedChop (selected), 
        chopSounds (chopSoundsMap), 
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

        addAndMakeVisible (midiLearnButton);
        midiLearnButton.setButtonText (MIDI_LEARN);
        midiLearnButton.setColour (ToggleButton::textColourId, COLOR_FG);
        midiLearnButton.setColour (ToggleButton::tickColourId, COLOR_RED);
        midiLearnButton.setEnabled (true);
        midiLearnButton.setVisible (false);
        midiLearnButton.onClick = [this] { 
            auto toggledOn = midiLearnButton.getToggleState();
            listenForMidiLearn = toggledOn;
            midiLearningLabel.setVisible (toggledOn);
            if (!toggledOn && lastRecordedMidiNoteDirty)
            { // button toggled off
                chopTree.getChildWithProperty (ID_CHOPID, selectedChop.getValue()).setProperty (
                    ID_TRIGGER, lastRecordedMidiNote.getValue(), nullptr
                );
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

        addAndMakeVisible (prevChopArrow);
        prevChopArrow.setEnabled (false); 
        prevChopArrow.setVisible (false);
        prevChopArrow.onClick = [this] { selectedChop = getPrevChopId(); };

        addAndMakeVisible (nextChopArrow);
        nextChopArrow.setEnabled (false);
        nextChopArrow.setVisible (false);
        nextChopArrow.onClick = [this] { selectedChop = getNextChopId(); };

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
            chopTree.getChildWithProperty (ID_CHOPID, selectedChop.getValue()).setProperty (ID_TRIGGER, noteId, nullptr);
        };

        lastRecordedMidiNote.addListener (this);
    }

    ~ChopSettingsComponent()
    {

    }


    void displayChop(int selectedChopId)
    {
        if (selectedChopId == NONE)
        {
            selectedChopLabel.setText(SEL_CHOP_NONE, dontSendNotification);
            triggerNoteLabel.setVisible (false);
            triggerNoteComboBox.setVisible (false);
            midiLearnButton.setVisible (false);
            prevChopArrow.setVisible (false);
            nextChopArrow.setVisible (false);
            midiLearnButton.setVisible (false);
        } else
        {
            auto chop = Chop(chopTree.getChildWithProperty (ID_CHOPID, selectedChopId));
            auto numChops = chopTree.getNumChildren();
            triggerNoteLabel.setVisible (true);
            triggerNoteComboBox.setVisible (true);
            midiLearnButton.setVisible (true);
            prevChopArrow.setEnabled (numChops > 1);
            nextChopArrow.setEnabled (numChops > 1);
            prevChopArrow.setVisible (true);
            nextChopArrow.setVisible (true);
            midiLearnButton.setVisible (true);
            selectedChopLabel.setText ("Chop " + String(selectedChopId) + " selected", dontSendNotification);
            triggerNoteLabel.setText ("Mapped To", dontSendNotification);
            triggerNoteComboBox.setSelectedId (chop.getTriggerNote(), dontSendNotification);
        }
    }

    Array<int> getChopIds()
    {
        Array<int> chopIds;
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
        return i + 1 == chopIds.size() ? 0 : i + 1;
    }

    int getPrevChopId()
    {
        auto chopIds = getChopIds();
        auto i = chopIds.indexOf(selectedChop.getValue());
        jassert(i != -1);
        return i - 1 == -1 ? chopIds.size() - 1 : i - 1;
    }

    void valueChanged(Value &value) override
    {
        if (bool(listenForMidiLearn.getValue()) && value.refersToSameSourceAs(lastRecordedMidiNote))
        {
            lastRecordedMidiNoteDirty = true;
            triggerNoteComboBox.setSelectedId (lastRecordedMidiNote.getValue(), false);
            midiLearnButton.triggerClick();
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
    
        auto chopIdLabelWidth = rectChopIdAndTriggerAux.getWidth() * 0.15;
        auto triggerLabelWidth = rectChopIdAndTriggerAux.getWidth() * 0.09;
        auto triggerComboBoxWidth = rectChopIdAndTriggerAux.getWidth() * 0.066;
        selectedChopLabel.setBounds (rectChopIdAndTriggerAux.removeFromLeft (chopIdLabelWidth));
        triggerNoteLabel.setBounds (rectChopIdAndTriggerAux.removeFromLeft (triggerLabelWidth));
        triggerNoteComboBox.setBounds (rectChopIdAndTriggerAux.removeFromLeft (triggerComboBoxWidth));
        midiLearnButton.setBounds (rectChopIdAndTriggerAux.removeFromLeft (triggerLabelWidth));
        midiLearningLabel.setBounds (rectChopIdAndTriggerAux);

        rectSettingsAndArrows = r.removeFromTop (r.getHeight() * 0.11);
        auto rectSettingsAndArrowsAux = rectSettingsAndArrows;
        auto arrowButtonWidth = rectSettingsAndArrowsAux.getWidth() * 0.02;
        prevChopArrow.setBounds (rectSettingsAndArrowsAux.removeFromLeft(arrowButtonWidth));
        prevChopArrow.setTopLeftPosition(prevChopArrow.getX(), r.getHeight() / 2);
        nextChopArrow.setBounds (rectSettingsAndArrowsAux.removeFromRight(arrowButtonWidth));
        nextChopArrow.setTopLeftPosition(nextChopArrow.getX(), r.getHeight() / 2);

        //// todo adsr component (slider + label) in r
    }
    Value listenForMidiLearn;

private:
    HashMap<int, SamplerSound*>* chopSounds;
    ValueTree chopTree;
    Value selectedChop;
    Value lastRecordedMidiNote;
    bool lastRecordedMidiNoteDirty = false;

    Rectangle<int> rectChopIdAndTrigger;
    Rectangle<int> rectSettingsAndArrows;
    Label selectedChopLabel;
    Label triggerNoteLabel;
    Label midiLearningLabel;
    ComboBox triggerNoteComboBox;
    ToggleButton midiLearnButton;
    ArrowButton prevChopArrow { "PREV", 0.5, COLOR_GRAY_LIGHT };
    ArrowButton nextChopArrow { "NEXT", 0.0, COLOR_GRAY_LIGHT };

    const String SEL_CHOP_NONE = "No Chop Selected";
    const String TRIGGER_NOTE_LABEL = "Trigger Note: ";
    const String MIDI_LEARN = "MIDI Learn";
    const String LISTENING = "Press New Trigger Note";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopSettingsComponent)
};
