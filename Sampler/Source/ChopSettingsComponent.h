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
    ChopSettingsComponent(Value selectedChop, HashMap<int, SamplerSound*>* chopSoundsMap) :
        selectedChopId (selectedChop), chopSounds (chopSoundsMap)
    {
        addAndMakeVisible (selectedChopLabel);
        selectedChopLabel.setFont (Font (15.00f, Font::plain));
        selectedChopLabel.setJustificationType (Justification::centredTop);
        selectedChopLabel.setEditable (false, false, false);
        //selectedChopLabel.setColour (Label::backgroundWhenEditingColourId)
        selectedChopLabel.setColour (Label::backgroundColourId, COLOR_BG);
        selectedChopLabel.setColour (Label::textColourId, COLOR_FG);
        selectedChopLabel.setText (SEL_CHOP_NONE, NotificationType::dontSendNotification);

    }

    ~ChopSettingsComponent()
    {

    }


    void displayChop(int selectedChopId)
    {
        if (selectedChopId == NONE)
        {
            selectedChopLabel.setText(SEL_CHOP_NONE, NotificationType::dontSendNotification);
        } else
        {
            selectedChopLabel.setText("Chop " + String(selectedChopId) + " selected", NotificationType::dontSendNotification);
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (COLOR_BG);

        g.setColour (Colours::white);
        g.setFont (14.0f);
        g.drawText ("ChopSettingsComponent", getLocalBounds(),
                    Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (4);

        selectedChopLabel.setBounds(r);
    }

private:
    HashMap<int, SamplerSound*>* chopSounds;
    Value selectedChopId;

    Label selectedChopLabel;

    const String SEL_CHOP_NONE = "No Chop Selected";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopSettingsComponent)
};
