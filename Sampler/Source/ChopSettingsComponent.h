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
                          HashMap<int, std::pair<SamplerSound*, ADSR::Parameters>> &chopSoundsMap,
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

        addAndMakeVisible (attackLS);
        attackLS.setVisible (false);
        //attackLS.addListener (this);
        addAndMakeVisible (decayLS);
        decayLS.setVisible (false);
        //decayLS.addListener (this);
        addAndMakeVisible (sustainLS);
        sustainLS.setVisible (false);
        //sustainLS.addListener (this);
        addAndMakeVisible (releaseLS);
        releaseLS.setVisible (false);
        //releaseLS.addListener (this);

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
        triggerNoteLabel.setVisible (selectedChopId != NONE);
        triggerNoteComboBox.setVisible (selectedChopId != NONE);
        midiLearnButton.setVisible (selectedChopId != NONE);
        prevChopArrow.setVisible (selectedChopId != NONE);
        nextChopArrow.setVisible (selectedChopId != NONE);
        attackLS.setVisible (selectedChopId != NONE);
        decayLS.setVisible (selectedChopId != NONE);
        sustainLS.setVisible (selectedChopId != NONE);
        releaseLS.setVisible (selectedChopId != NONE);
        if (selectedChopId == NONE)
        {
            selectedChopLabel.setText(SEL_CHOP_NONE, dontSendNotification);
        } else
        {
            auto chop = Chop(chopTree.getChildWithProperty (ID_CHOPID, selectedChopId));
            auto numChops = chopTree.getNumChildren();
            prevChopArrow.setEnabled (numChops > 1);
            nextChopArrow.setEnabled (numChops > 1);
            selectedChopLabel.setText ("Chop " + String(selectedChopId) + " selected", dontSendNotification);
            triggerNoteLabel.setText ("Mapped To", dontSendNotification);
            triggerNoteComboBox.setSelectedId (chop.getTriggerNote(), dontSendNotification);
            
            auto params = chopSounds[selectedChopId].second;
            attackLS.setSliderValue (params.attack);
            decayLS.setSliderValue (params.decay);
            sustainLS.setSliderValue (params.sustain);
            releaseLS.setSliderValue (params.release);
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

    void sliderDragEnded (Slider * slider)
    {
        auto chopPair = chopSounds[selectedChop.getValue()];
        auto params = chopPair.second;
        if (slider == &attackLS.slider)
        {
            params.attack = slider->getValue();
        } else if (slider == &decayLS.slider)
        {
            params.decay = slider->getValue();
        } else if (slider == &sustainLS.slider)
        {
            params.sustain = slider->getValue();
        } else
        {
            params.release = slider->getValue();
        }
        chopPair.first->setEnvelopeParameters(params);
        chopSounds.set(selectedChop.getValue(), std::make_pair(chopPair.first, params));
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

        rectSettingsAndArrows = r;
        auto rectSettingsAndArrowsAux = rectSettingsAndArrows;
        auto arrowRectWidth = rectSettingsAndArrowsAux.getWidth() * 0.03;
        auto arrowSize = rectSettingsAndArrows.getHeight() * 0.1;
        prevChopArrow.setBounds (rectSettingsAndArrowsAux.removeFromLeft(arrowRectWidth));
        prevChopArrow.setTopLeftPosition (prevChopArrow.getX() + (arrowRectWidth * 0.2), 
                                         rectSettingsAndArrows.getCentreY());
        prevChopArrow.setSize (arrowSize, arrowSize);
        nextChopArrow.setBounds (rectSettingsAndArrowsAux.removeFromRight(arrowRectWidth));
        nextChopArrow.setSize (arrowSize, arrowSize);
        nextChopArrow.setTopLeftPosition (nextChopArrow.getX() + (arrowRectWidth * 0.36),
                                          rectSettingsAndArrows.getCentreY());

        auto adsrLSWidth = rectSettingsAndArrowsAux.getWidth() / 4;
        attackLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
        decayLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
        sustainLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
        releaseLS.setBounds (rectSettingsAndArrowsAux.removeFromLeft (adsrLSWidth));
    }
    Value listenForMidiLearn;

private:
    class LabelledSlider : public Component
    {
    public:
        LabelledSlider (const String& labelText, ChopSettingsComponent& chopSettings) : owner(chopSettings)
        {
            addAndMakeVisible (slider);
            slider.setEnabled (true);
            slider.setSliderStyle (Slider::LinearVertical);
            slider.setColour (Slider::trackColourId, COLOR_BG);
            slider.setColour (Slider::backgroundColourId, COLOR_BG_DARK);
            slider.setColour (Slider::thumbColourId, COLOR_FG);
            slider.setRange (0.0, 1.0, 0.01);
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
            jassert(d >= 0.0 && d <= 1.0);
            slider.setValue(d);
        }

 /*       void addListener(juce::Slider::Listener *listener)
        {
            slider.addListener(listener);
        }*/

        Slider slider { Slider::LinearVertical, Slider::TextBoxBelow };
    private:
        ChopSettingsComponent &owner;
        Label label;
    };

    HashMap<int, std::pair<SamplerSound*, ADSR::Parameters>> &chopSounds;
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
    LabelledSlider attackLS { "Attack", *this }, decayLS { "Decay", *this }, sustainLS { "Sustain", *this }, releaseLS { "Release", *this };


    const String SEL_CHOP_NONE = "No Chop Selected";
    const String TRIGGER_NOTE_LABEL = "Trigger Note: ";
    const String MIDI_LEARN = "MIDI Learn";
    const String LISTENING = "Press New Trigger Note";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopSettingsComponent)
};
