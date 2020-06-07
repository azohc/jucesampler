/*
  ==============================================================================

    ChopListComponent.h
    Created: 10 Nov 2019
    Author:  Juan Chozas Sumbera

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Constants.h"

//==============================================================================
/*
*/
class ChopListComponent: 
    public Component, 
    public TableListBoxModel
{
public:
    ChopListComponent(ValueTree chops, 
                      HashMap<int, ValueTree> &cm, 
                      HashMap<int, std::pair<SamplerSound*, ADSR::Parameters>> &cs,
                      HashMap<int, std::pair<DrawableRectangle*, DrawableRectangle*>> &cb,
                      Value selected) :
        chopTree (chops), chopMap (cm), chopSounds(cs), chopBounds(cb), selectedChop (selected)
    {
        initChopListColumns();
        numRows = 0;

        // Create our table component and add it to this component..
        addAndMakeVisible (table);
        table.setModel (this);

        // Add some columns to the table header, based on the column list
        forEachXmlChildElement (*columnXml, column)
        {
            table.getHeader().addColumn (column->getStringAttribute (COLUMN_NAME),
                                         column->getIntAttribute (COLUMN_ID),
                                         column->getIntAttribute (COLATR_WIDTH),
                                         30, -1, TableHeaderComponent::visible | TableHeaderComponent::appearsOnColumnMenu);
        }

        table.getHeader().setStretchToFitActive (true);
        table.getHeader().setColour (TableHeaderComponent::backgroundColourId, COLOR_BG);
        table.getHeader().setColour (TableHeaderComponent::highlightColourId, COLOR_BG_LIGHT);
        table.getHeader().setColour (TableHeaderComponent::textColourId, COLOR_FG);
        table.getHeader().setColour (TableHeaderComponent::outlineColourId, COLOR_BG_DARK);
        table.setColour(TableListBox::backgroundColourId, COLOR_BG);
        table.setMultipleSelectionEnabled (false);
        rowClickedMenu = new PopupMenu();

        deletedChopId = NONE;

        int midiNoteNr = 24;     // start on C0
        for (auto octave: { 0, 1, 2, 3, 4, 5, 6, 7, 8 }) {
            for (String note: { "C", "D", "E", "F", "G", "A", "B" }) {
                midiNoteMap.set(midiNoteNr++, note + String(octave));
                if (note != "E" && note != "B") {
                    midiNoteMap.set(midiNoteNr++, note + String(octave) + "#");
                }
            }
        }
    }

    ~ChopListComponent()
    {
        delete rowClickedMenu;
        delete chopXml;
        delete columnXml;
    }

    int getNumRows() override
    {
        return numRows;
    }

    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto selectedRowColour = COLOR_BG_DARK.interpolatedWith (COLOR_BG, 0.16f);
        auto rowColour = COLOR_BG_DARK.interpolatedWith (COLOR_BG, 0.56f);
        auto altRowColour = COLOR_BG_DARK.interpolatedWith (COLOR_BG, 0.86f);
        if (rowIsSelected)
            g.fillAll (selectedRowColour);
        else if (rowNumber % 2)
            g.fillAll (altRowColour);
        else
            g.fillAll (rowColour);
    }

    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override
    {
        g.setColour (COLOR_FG);
        g.setFont (font);

        if (auto* rowElement = chopXml->getChildElement (rowNumber))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

            g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }

        g.setColour (COLOR_BG_DARK.interpolatedWith (COLOR_BG, 0.86f));
        g.fillRect (width - 1, 0, 1, height);
    }

    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            if (numRows) chopXml->sortChildElements (sorter);

            table.updateContent();
        }
    }

    Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
                                        Component* existingComponentToUpdate) override
    {
        if (columnId != COLID_TRIGG)    // only assert for custom columns
        {
            jassert (existingComponentToUpdate == nullptr);
            return nullptr;
        }

        if (columnId == COLID_TRIGG)
        {
            auto* triggerComboBox = static_cast<TriggerNoteColumnComponent*> (existingComponentToUpdate);

            if (triggerComboBox == nullptr)
                triggerComboBox = new TriggerNoteColumnComponent (*this);

            triggerComboBox->   setRowAndColumn (rowNumber, columnId);
            return triggerComboBox;
        }
    }

    int getColumnAutoSizeWidth (int columnId) override
    {
        if (columnId == 5)
            return 100;

        int widest = 32;

        for (int i = getNumRows(); --i >= 0;)
        {
            if (auto* rowElement = chopXml->getChildElement (i))
            {
                auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

                widest = jmax (widest, font.getStringWidth (text));
            }
        }

        return widest + 8;
    }

    int getTriggerNote (const int rowNumber) const
    {
        return chopXml->getChildElement (rowNumber)->getIntAttribute (COL_TRIGG);
    }

    void setTriggerNote (const int rowNumber, const int newTrigger)
    {
        auto chop = Chop (chopTree, getChopIdAtRow(rowNumber));
        chop.setTriggerNote (newTrigger);
        chopXml->getChildElement (rowNumber)->setAttribute (COL_TRIGG, newTrigger);
    }

    //==============================================================================
    void resized() override
    {
        table.setBoundsInset (BorderSize<int> (1));
    }

    int getChopIdAtRow (int rowNumber) {
        return chopXml->getChildElement (rowNumber)->getIntAttribute (COLNAME_ID);
    }

    Chop getChopAtRow (int rowNumber) {
        return Chop (chopTree, getChopIdAtRow(rowNumber));
    }

    void cellClicked(int rowNumber, int columnId, const MouseEvent &e)
    {
        int result = 0;
        if (e.mods.isLeftButtonDown())
        {
        } else if (e.mods.isRightButtonDown())
        {
            rowClickedMenu->clear();
            rowClickedMenu->addItem (ROWMENUID_HIDDEN, ROW_HIDDEN, true, getChopAtRow (rowNumber).getHidden());
            rowClickedMenu->addItem (ROWMENUID_DELETE, ROW_DELETE, true, false);
            rowClickedMenu->addItem (ROWMENUID_DEL_ALL, ROW_DEL_ALL, true, false);
            result = rowClickedMenu->show();
        }

        if (result)
        {
            switch (result)
            {
                case ROWMENUID_HIDDEN:
                    Chop(chopTree, rowNumber).setHidden (!getChopAtRow (rowNumber).getHidden());
                break;

                case ROWMENUID_DELETE:
                    deletedChopId = getChopIdAtRow(rowNumber);
                    if (deletedChopId == int (selectedChop.getValue()))
                    {
                        selectedChop = NONE;
                    }
                    chopMap.remove(deletedChopId);
                    chopSounds.remove(deletedChopId);
                    for (int i = 0; i < chopTree.getNumChildren(); i++)
                    {
                        Chop chop (chopTree.getChild(i));
                        if (chop.getId() > deletedChopId)
                        {
                            auto id = chop.getId();
                            auto v = chopMap[id];
                            chopMap.remove(id);
                            chopMap.set(id - 1, v);

                            auto p = chopSounds[id];
                            chopSounds.remove(id);
                            chopSounds.set(id - 1, p);

                            chop.setId(id - 1);
                        }
                    }
                    chopTree.removeChild (deletedChopId, nullptr);
                    reloadData();
                break;

                case ROWMENUID_DEL_ALL:
                    chopTree.removeAllChildren(nullptr);
                    reloadData();
            }
        }
    }

    void selectedRowsChanged (int lastRowChanged) override
    {
        if (lastRowChanged != -1)
        {
            selectedChop = chopXml->getChildElement (lastRowChanged)->getIntAttribute (COLNAME_ID);
        }
    }

    void reloadData()
    {
        chopsToXml();
        numRows = chopXml->getNumChildElements();
        table.updateContent();
    }

    int getDeletedChopId()
    {
        return deletedChopId;
    }

    void unsetDeletedChopId()
    {
        deletedChopId = NONE;
    }

    int getSelectedChopId()
    {
        return int(selectedChop.getValue());
    }

    void selectRow(int row)
    {
        if (row == NONE)
        {
            table.deselectAllRows();
        } else
        {
            table.selectRow(row);
        }
    }

private:
    TableListBox table;     
    int numRows;
    Font font { 14.0f };
    
    ValueTree chopTree;
    HashMap<int, ValueTree> &chopMap;
    HashMap<int, std::pair<SamplerSound*, ADSR::Parameters>> &chopSounds;
    HashMap<int, std::pair<DrawableRectangle*, DrawableRectangle*>> &chopBounds;
    XmlElement* chopXml = nullptr;
    XmlElement* columnXml = nullptr;
    
    PopupMenu* rowClickedMenu = nullptr;

    Value selectedChop;
    int deletedChopId;

    HashMap<int, String> midiNoteMap;

    //==============================================================================
    // custom component containing a combo box
    class TriggerNoteColumnComponent : public Component
    {
    public:
        TriggerNoteColumnComponent (ChopListComponent& chopListComp) : owner (chopListComp)
        {
            comboBox.setColour (ComboBox::outlineColourId, COLOR_BG_DARK);
            comboBox.setColour (ComboBox::backgroundColourId, COLOR_BG);
            comboBox.setColour (ComboBox::buttonColourId, COLOR_BG_DARK);
            comboBox.setColour (ComboBox::arrowColourId, COLOR_BG_DARK);
            comboBox.setColour (ComboBox::textColourId, COLOR_FG);
            addAndMakeVisible (comboBox);
            int midiNoteNr = FIRST_MIDI_NOTE;

            for (auto octave: { 0, 1, 2, 3, 4, 5 }) {
                for (String note: { "C", "D", "E", "F", "G", "A", "B" }) {
                    comboBox.addItem (note + String(octave), midiNoteNr++);
                    if (note != "E" && note != "B") {
                        comboBox.addItem(note + String(octave) + "#" , midiNoteNr++);
                    }
                }
            }
                            
            comboBox.onChange = [this] { owner.setTriggerNote (row, comboBox.getSelectedId()); };
            comboBox.setWantsKeyboardFocus (false);
        }

        void resized() override
        {
            comboBox.setBoundsInset (BorderSize<int> (2));
        }

        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            comboBox.setSelectedId (owner.getTriggerNote (row), dontSendNotification);
        }

    private:
        ChopListComponent& owner;
        ComboBox comboBox;
        int row, columnId;
    };

    class DemoDataSorter
    {
    public:
        DemoDataSorter (const String& attributeToSortBy, bool forwards)
            : attributeToSort (attributeToSortBy),
            direction (forwards ? 1 : -1)
        {
        }

        int compareElements (XmlElement* first, XmlElement* second) const
        {
            auto result = first->getStringAttribute (attributeToSort)
                .compareNatural (second->getStringAttribute (attributeToSort));

            if (result == 0)
                result = first->getStringAttribute ("ID")
                .compareNatural (second->getStringAttribute ("ID"));

            return direction * result;
        }

    private:
        String attributeToSort;
        int direction;
    };

    void initChopListColumns()
    {
        columnXml = new XmlElement(COLUMNS);

        auto columnWidth = getBounds().getWidth() || 25;

        XmlElement* columnId = columnXml->createNewChildElement(COLUMN);
        columnId->setAttribute (COLUMN_ID, COLID_ID);
        columnId->setAttribute (COLUMN_NAME, COLNAME_ID);
        columnId->setAttribute (COLATR_WIDTH, 10);

        XmlElement* columnStart = columnXml->createNewChildElement(COLUMN);
        columnStart->setAttribute (COLUMN_ID, COLID_START);
        columnStart->setAttribute (COLUMN_NAME, COLNAME_START);
        columnStart->setAttribute (COLATR_WIDTH, columnWidth);

        XmlElement* columnEnd = columnXml->createNewChildElement(COLUMN);
        columnEnd->setAttribute (COLUMN_ID, COLID_END);
        columnEnd->setAttribute (COLUMN_NAME, COLNAME_END);
        columnEnd->setAttribute (COLATR_WIDTH, columnWidth);

        XmlElement* columnMappedTo = columnXml->createNewChildElement(COLUMN);
        columnMappedTo->setAttribute (COLUMN_ID, COLID_TRIGG);
        columnMappedTo->setAttribute (COLUMN_NAME, COL_TRIGG);
        columnMappedTo->setAttribute (COLATR_WIDTH, columnWidth);
    }

    void chopsToXml()
    {
        if (chopXml != nullptr)     
            delete chopXml;
        
        chopXml = chopTree.createXml().release();        
    }

    String getAttributeNameForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*columnXml, column)
        {
            if (column->getIntAttribute (COLUMN_ID) == columnId)
                return column->getStringAttribute (COLUMN_NAME);
        }

        return {};
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopListComponent)
};
