/*
  ==============================================================================

    SamplerThumbnail.h
    Created: 4 Nov 2019 7:03:25pm
    Author:  wozas

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SamplerThumbnail: 
    public Component, 
    public ChangeListener,
    public FileDragAndDropTarget,
    public ChangeBroadcaster,
    private ScrollBar::Listener,
    private Timer
{
public:
    SamplerThumbnail(AudioFormatManager& formatManager,
                     AudioTransportSource& source,
                     Slider& slider,
                     HashMap<String, Colour>& colorMap):
        transportSource (source),
        zoomSlider (slider),
        thumbnail (512, formatManager, thumbnailCache),
        colors (colorMap)
    {
        thumbnail.addChangeListener (this);

        addAndMakeVisible (scrollbar);
        scrollbar.setRangeLimits (visibleRange);
        scrollbar.setAutoHide (false);
        scrollbar.addListener (this);
        scrollbar.setColour (ScrollBar::backgroundColourId, colors["bluedark"]);
        scrollbar.setColour (ScrollBar::thumbColourId, colors["blue"]);


        currentPositionMarker.setFill (colors["graylite"]);
        addAndMakeVisible (currentPositionMarker);

    }

    ~SamplerThumbnail()
    {
        scrollbar.removeListener (this);
        thumbnail.removeChangeListener (this);
    }

    void setFile (const File& file)
    {
        thumbnail.setSource (new FileInputSource (file));

        Range<double> newRange (0.0, thumbnail.getTotalLength());
        scrollbar.setRangeLimits (newRange);
        setRange (newRange);

        startTimerHz (40);
    }

    File getLastDroppedFile() const noexcept { return lastFileDropped; }

    void setZoomFactor (double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            auto newScale = jmax (0.001, thumbnail.getTotalLength() * (1.0 - jlimit (0.0, 0.99, amount)));
            auto timeAtCentre = xToTime (getWidth() / 2.0f);

            setRange ({ timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5 });
        }
    }

    void setRange (Range<double> newRange)
    {
        visibleRange = newRange;
        scrollbar.setCurrentRange (visibleRange);
        updateCursorPosition();
        repaint();
    }

    void setFollowsTransport (bool shouldFollow)
    {
        isFollowingTransport = shouldFollow;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (colors["bluedark"]);
        g.setColour (colors["fg"]);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto thumbArea = getLocalBounds();

            thumbArea.removeFromBottom (scrollbar.getHeight() + 4);
            thumbnail.drawChannels (g, thumbArea.reduced (2),
                                    visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        } else
        {
            g.setFont (14.0f);
            g.drawFittedText ("Drag and drop audio file here to load it", getLocalBounds(), Justification::centred, 2);
        }
    }

    void resized() override
    {
        scrollbar.setBounds (getLocalBounds().removeFromBottom (14).reduced (2));
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    bool isInterestedInFileDrag (const StringArray&) override
    {
        return true;
    }

    void filesDropped (const StringArray& files, int, int) override
    {
        lastFileDropped = File (files[0]);
        newFileDropped = true;
        sendChangeMessage();
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (canMoveTransport())
        {
            auto newPos = jmin (jmax (0.0, xToTime ((float) e.x)), transportSource.getLengthInSeconds());
            transportSource.setPosition (newPos);
        }
    }

    void mouseUp (const MouseEvent&) override
    {
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto newStart = visibleRange.getStart() + wheel.deltaX * (visibleRange.getLength()) / 10.0;
            newStart = jlimit (0.0, jmax (0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

            if (canMoveTransport())
                setRange ({ newStart, newStart + visibleRange.getLength() });

            if (wheel.deltaY != 0.0f)
                zoomSlider.setValue (zoomSlider.getValue() + wheel.deltaY);

            repaint();
        }
    }

    double getCurrentPosition ()
    {
        if (thumbnail.getTotalLength() > 0)
        {
            return transportSource.getCurrentPosition();
        } else
        {
            return 0;
        }
    }

    bool newFileDropped = false;

private:
    AudioTransportSource& transportSource;
    Slider& zoomSlider;
    ScrollBar scrollbar { false };

    AudioThumbnailCache thumbnailCache { 5 };
    AudioThumbnail thumbnail;
    Range<double> visibleRange;
    bool isFollowingTransport = false;

    File lastFileDropped;

    DrawableRectangle currentPositionMarker;
    HashMap<String, Colour>& colors;
   
    float timeToX (const double time) const
    {
        if (visibleRange.getLength() <= 0)
            return 0;

        return getWidth() * (float) ((time - visibleRange.getStart()) / visibleRange.getLength());
    }

    double xToTime (const float x) const
    {
        return (x / getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    bool canMoveTransport() const noexcept
    {
        return !(isFollowingTransport && transportSource.isPlaying());
    }

    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        if (scrollBarThatHasMoved == &scrollbar)
            if (!(isFollowingTransport && transportSource.isPlaying()))
                setRange (visibleRange.movedToStartAt (newRangeStart));
    }

    void timerCallback() override
    {
        if (canMoveTransport())
            updateCursorPosition();
        else
            setRange (visibleRange.movedToStartAt (transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }

    void updateCursorPosition()
    {
        currentPositionMarker.setVisible (thumbnail.getTotalLength() > 0);

        currentPositionMarker.setRectangle (Rectangle<float> (timeToX (transportSource.getCurrentPosition()) - 0.75f, 0,
                                            1.5f, (float) (getHeight() - scrollbar.getHeight())));
        sendChangeMessage();
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerThumbnail)
};
