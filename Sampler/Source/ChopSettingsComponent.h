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
class ChopSettingsComponent    : public Component
{
public:
    ChopSettingsComponent(Value selected, HashMap<int, SamplerSound*>* chopSoundsMap, ValueTree chops) :
        selectedChop (selected), chopSounds (chopSoundsMap), chopTree(chops)
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
        midiLearnButton.setColour (TextButton::buttonColourId, COLOR_GRAY);
        midiLearnButton.setColour (TextButton::textColourOffId, COLOR_BG);
        midiLearnButton.setEnabled (false);
        midiLearnButton.setVisible (false);
        midiLearnButton.onClick = [this] { };// TODO Note Update with MIDILEARN 

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

        triggerNoteComboBox.onChange = [this] { }; // TODO Note Update
        setWantsKeyboardFocus (true); // TODO prev/next chop buttons & keyboard binds
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
            prevChopArrow.setEnabled (false);
            nextChopArrow.setEnabled (false);
            prevChopArrow.setVisible (false);
            nextChopArrow.setVisible (false);
        } else
        {
            auto chop = Chop(chopTree.getChildWithProperty (PROP_ID, selectedChopId));
            auto numChops = chopTree.getNumChildren();
            triggerNoteLabel.setVisible (true);
            triggerNoteComboBox.setVisible (true);
            midiLearnButton.setVisible (true);
            prevChopArrow.setEnabled (numChops > 1);
            nextChopArrow.setEnabled (numChops > 1);
            prevChopArrow.setVisible (true);
            nextChopArrow.setVisible (true);
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

        rectSettingsAndArrows = r.removeFromTop (r.getHeight() * 0.11);
        auto rectSettingsAndArrowsAux = rectSettingsAndArrows;
        auto arrowButtonWidth = rectSettingsAndArrowsAux.getWidth() * 0.02;
        prevChopArrow.setBounds (rectSettingsAndArrowsAux.removeFromLeft(arrowButtonWidth));
        prevChopArrow.setTopLeftPosition(prevChopArrow.getX(), r.getHeight() / 2);
        nextChopArrow.setBounds (rectSettingsAndArrowsAux.removeFromRight(arrowButtonWidth));
        nextChopArrow.setTopLeftPosition(nextChopArrow.getX(), r.getHeight() / 2);

        //// todo adsr component (slider + label) in r
    }

private:
    HashMap<int, SamplerSound*>* chopSounds;
    ValueTree chopTree;
    Value selectedChop;


    Rectangle<int> rectChopIdAndTrigger;
    Rectangle<int> rectSettingsAndArrows;
    Label selectedChopLabel;
    Label triggerNoteLabel;
    ComboBox triggerNoteComboBox;
    TextButton midiLearnButton;
    ArrowButton prevChopArrow { "PREV", 0.5, COLOR_GRAY_LIGHT };
    ArrowButton nextChopArrow { "NEXT", 0.0, COLOR_GRAY_LIGHT };

    const String SEL_CHOP_NONE = "No Chop Selected";
    const String TRIGGER_NOTE_LABEL = "Trigger Note: ";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopSettingsComponent)
};
