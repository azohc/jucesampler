/*
  ==============================================================================

    SamplerThumbnail.h
    Created: 4 Nov 2019
    Author:  Juan Chozas Sumbera

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Constants.h"

//==============================================================================
/*
*/
class SamplerThumbnail: 
    public Component, 
    public ChangeListener,
    public ChangeBroadcaster,
    private ScrollBar::Listener,
    private Timer
{
public:
    SamplerThumbnail(AudioFormatManager* formatManager,
                     AudioTransportSource& source,
                     Slider& slider,
                     ValueTree chops,
                     HashMap<int, std::pair<DrawableRectangle*, DrawableRectangle*>>* bounds,
                     Value& selectedChop,
                     Value& selectionActive) :
        transportSource (source),
        zoomSlider (slider),
        thumbnail (512, *formatManager, thumbnailCache),
        chopTree (chops),
        chopBounds (bounds),
        selectedChopId (selectedChop),
        userSelectionActive (selectionActive)
    {
        thumbnail.addChangeListener (this);

        addAndMakeVisible (scrollbar);
        scrollbar.setRangeLimits (visibleRange);
        scrollbar.setAutoHide (false);
        scrollbar.addListener (this);
        scrollbar.setColour (ScrollBar::backgroundColourId, COLOR_BLUE_DARK);
        scrollbar.setColour (ScrollBar::thumbColourId, COLOR_BLUE);

        currentPositionMarker.setFill (COLOR_SEA);
        addAndMakeVisible (currentPositionMarker);

        addAndMakeVisible(selectionRect);
    }

    ~SamplerThumbnail()
    {
        scrollbar.removeListener (this);
        thumbnail.removeChangeListener (this);
        deleteAllMarkers();
    }

    void setFile (const File& file)
    {
        thumbnail.setSource (new FileInputSource (file));

        Range<double> newRange (0.0, thumbnail.getTotalLength());
        scrollbar.setRangeLimits (newRange);
        setRange (newRange);

        startTimerHz (40);
    }

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
        g.fillAll (COLOR_BLUE_DARK);
        g.setColour (COLOR_FG);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto thumbArea = getLocalBounds();

            thumbArea.removeFromBottom (scrollbar.getHeight() + 4);
            thumbnail.drawChannels (g, thumbArea.reduced (2),
                                    visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        } else
        {
            g.setFont (14.0f);
            g.drawFittedText ("Load an audio file to begin", getLocalBounds(), Justification::centred, 2);
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


    void mouseDown (const MouseEvent& e) override
    {
        userSelectionActive = false;
        selectedChopId = NONE;
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown())
        {
            selectionStartTime = jmin (jmax (0.0, xToTime ((float) e.getMouseDownX())), transportSource.getLengthInSeconds());
            selectionEndTime = jmin (jmax (0.0, xToTime ((float) e.getMouseDownX() + e.getOffsetFromDragStart().x)), transportSource.getLengthInSeconds());
            userSelectionActive = true;
        }
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

    void addChopMarker (int key)
    {
        DrawableRectangle* st = new DrawableRectangle();
        DrawableRectangle* en = new DrawableRectangle();
        addAndMakeVisible (st); addAndMakeVisible(en);
        chopBounds->set (key, std::make_pair(st,en));
    }

    void deleteChopMarkers (int key)
    {
        delete chopBounds->operator[](key).first;
        delete chopBounds->operator[](key).second;
        int n = chopBounds->size();
        chopBounds->remove(key);
        for (auto id = 0; id < n; id++)
        {
            if (id > key)
            {
                auto b = chopBounds->operator[](id);
                chopBounds->remove(id);
                chopBounds->set(id - 1, b);
            }
        }
    }

    void deleteAllMarkers()
    {
        for (auto i = chopBounds->begin(); i != chopBounds->end(); i.next())
        {
            delete i.getValue().first;
            delete i.getValue().second;
        }
        chopBounds->clear();
    }

    void setSelectedChopId(int id)
    {
        userSelectionActive = true;
        selectedChopId = id;
        selectionStartTime = Chop(chopTree, id).getStartTime();
        selectionEndTime = Chop(chopTree, id).getEndTime();
    }

    std::pair<double, double> getSelectionBounds()
    {
        return std::make_pair(selectionStartTime, selectionEndTime);
    }

private:
    AudioTransportSource& transportSource;
    Slider& zoomSlider;
    ScrollBar scrollbar { false };

    AudioThumbnailCache thumbnailCache { 5 };
    AudioThumbnail thumbnail;
    Range<double> visibleRange;
    bool isFollowingTransport = false;

    HashMap<int, std::pair<DrawableRectangle*, DrawableRectangle*>>* chopBounds;
    ValueTree chopTree;
     
    double selectionStartTime;
    double selectionEndTime;

    Value selectedChopId;
    Value userSelectionActive;

    DrawableRectangle currentPositionMarker;

    DrawableRectangle selectionRect;
   
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
        auto selectionActive = bool (userSelectionActive.getValue());
        auto selectedChop = int (selectedChopId.getValue());
        currentPositionMarker.setVisible (thumbnail.getTotalLength() > 0);

        currentPositionMarker.setRectangle (Rectangle<float> (timeToX (transportSource.getCurrentPosition()) - 0.75f, 0,
                                            1.5f, (float) (getHeight() - scrollbar.getHeight())));

        auto marker = [this] (double start) {
            return Rectangle<float> (timeToX (start) - 0.75f, 0,
                                1.5f, (float) (getHeight() - scrollbar.getHeight()));
        };

        auto selection = [this] (double start, double end)
        {
            return Rectangle<float> (timeToX (start), 0,
                                     timeToX (end) - timeToX (start),
                                     (float) (getHeight() - scrollbar.getHeight()));
        };

        for (auto it = chopBounds->begin(); it != chopBounds->end(); it.next())
        {
            Chop chop (chopTree, it.getKey());
            double start = chop.getStartTime();
            double end = chop.getEndTime();
            bool hidden = chop.getHidden();

            it.getValue().first->setFill (COLOR_RED);
            it.getValue().first->setRectangle (marker(start));
            it.getValue().first->setVisible (!hidden);

            it.getValue().second->setFill (COLOR_RED.darker(0.8f));
            it.getValue().second->setRectangle (marker(end));
            it.getValue().second->setVisible (!hidden);
            if (it.getKey() == selectedChop)
            {
                selectionRect.setRectangle (selection (start, end));
                selectionRect.setFill (COLOR_BLUE_DARK.brighter(0.11f));
                selectionRect.setAlpha (0.5f);
            }
        }
        if (selectionActive && selectedChop == NONE)
        {
            selectionRect.setRectangle (selection (selectionStartTime, selectionEndTime));
            selectionRect.setFill (COLOR_BLUE_DARK.brighter(0.11f));
            selectionRect.setAlpha (0.5f);
        }
        selectionRect.setVisible (selectionActive);

        sendChangeMessage();
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerThumbnail)
};
