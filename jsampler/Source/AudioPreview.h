/*
  ==============================================================================

    AudioPreview.h
    Created: 4 Nov 2019 12:07:56pm
    Author:  wozas

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class AudioPreview    : public Component,
    public ChangeListener

{
public:
    AudioPreview()
        : thumbnailCache (5),
        thumbnail (512, *formatManager, thumbnailCache)
    {
        addAndMakeVisible (&loadButton);
        loadButton.setButtonText ("Load sample");
        loadButton.onClick = [this] { loadButtonClicked(); };
        loadButton.setColour (TextButton::buttonColourId, Colours::mediumslateblue);
        loadButton.setEnabled (true);


        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play sample");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (TextButton::buttonColourId, Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&recButton);
        recButton.setButtonText ("Record chops");
        recButton.onClick = [this] { recButtonClicked(); };
        recButton.setColour (TextButton::buttonColourId, Colours::red);
        recButton.setEnabled (false);


        thumbnail.addChangeListener (this);
    }

    ~AudioPreview()
    {
    }

    void paint (Graphics& g) override
    {
        auto area = getBounds();
        auto thumbnailBounds = area.removeFromLeft(area.getWidth() / 1.1);
        if (thumbnail.getNumChannels() == 0)
            paintIfNoFileLoaded (g, thumbnailBounds, area);
        else
            paintIfFileLoaded (g, thumbnailBounds, area);
    }

    void paintIfNoFileLoaded (Graphics& g, const Rectangle<int>& thumbnailBounds, Rectangle<int>& buttonArea)
    {
        g.setColour (Colours::darkgrey);
        g.fillRect (thumbnailBounds);
        g.setColour (Colours::white);
        g.drawFittedText ("No File Loaded", thumbnailBounds, Justification::centred, 1.0f);

        auto buttonAreaHeight = buttonArea.getHeight();
        loadButton.setBounds (buttonArea.removeFromTop(buttonAreaHeight / 3));
        playButton.setBounds (buttonArea.removeFromTop(buttonAreaHeight / 3));
        recButton.setBounds (buttonArea.removeFromTop(buttonAreaHeight / 3));
    }

    void paintIfFileLoaded (Graphics& g, const Rectangle<int>& thumbnailBounds, Rectangle<int>& buttonArea)
    {
        g.setColour (Colours::white);                                     // [8]
        auto audioLength (thumbnail.getTotalLength());                                      // [12
        thumbnail.drawChannels (g,                                      // [9]
                                thumbnailBounds,
                                0.0,                                    // start time
                                audioLength,             // end time
                                1.0f);                                  // vertical zoom
        g.setColour (Colours::green);
        auto audioPosition ((*transportSource).getCurrentPosition());
        auto drawPosition ((audioPosition / audioLength) * thumbnailBounds.getWidth()
                           + thumbnailBounds.getX());                                        // [13]
        g.drawLine (drawPosition, thumbnailBounds.getY(), drawPosition,
                    thumbnailBounds.getBottom(), 2.0f);                                      // [14]
    }

    void resized() override
    {
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)        
            thumbnailChanged();
    }

    void thumbnailChanged()
    {
        repaint();
    }

    void loadButtonClicked()
    {
        // load file
        if ((*processor).loadFile(thumbnail))
        {
            playButton.setEnabled (true);
            recButton.setEnabled (true);
        }
    }

    void playButtonClicked()
    {
        // play whole sample
        // get AudioFormatReader from audioFormatManager into AudioFormatReaderSource 
        // play with AudioTransportSource
    }

    void recButtonClicked()
    {
        // enable flag to register transportSource's currentPosition as chop start times
    }

    std::shared_ptr<JsamplerAudioProcessor> processor;

    AudioThumbnailCache thumbnailCache;                  // [1]
    AudioThumbnail thumbnail;                            // [2]
    std::shared_ptr<AudioFormatManager> formatManager;
    std::shared_ptr<AudioTransportSource> transportSource;


    TextButton loadButton;
    TextButton playButton;
    TextButton recButton;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPreview)
};