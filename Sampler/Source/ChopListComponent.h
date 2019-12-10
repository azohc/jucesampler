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
    ChopListComponent(ValueTree chops, Value selected) :
        chopTree (chops), selectedChopId (selected)
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
                                         30, -1, TableHeaderComponent::defaultFlags | TableHeaderComponent::resizable);
        }

        table.getHeader().setStretchToFitActive (true);
        table.getHeader().setColour (TableHeaderComponent::backgroundColourId, COLOR_BG);
        table.getHeader().setColour (TableHeaderComponent::highlightColourId, COLOR_BGLIGHT);
        table.getHeader().setColour (TableHeaderComponent::textColourId, COLOR_FG);
        table.getHeader().setColour (TableHeaderComponent::outlineColourId, COLOR_BGDARK);
        table.setColour(TableListBox::backgroundColourId, COLOR_BG);

        table.setMultipleSelectionEnabled (false);

        rowClickedMenu = new PopupMenu();

        deletedChopId = NONE;
    }

    ~ChopListComponent()
    {
        delete rowClickedMenu;
        delete chopXml;
        delete columnXml;
    }

    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    int getNumRows() override
    {
        return numRows;
    }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto selectedRowColour = COLOR_BGDARK.interpolatedWith (COLOR_BG, 0.16f);
        auto rowColour = COLOR_BGDARK.interpolatedWith (COLOR_BG, 0.56f);
        auto altRowColour = COLOR_BGDARK.interpolatedWith (COLOR_BG, 0.86f);
        if (rowIsSelected)
            g.fillAll (selectedRowColour);
        else if (rowNumber % 2)
            g.fillAll (altRowColour);
        else
            g.fillAll (rowColour);
    }

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (COLOR_FG);
        g.setFont (font);

        if (auto* rowElement = chopXml->getChildElement (rowNumber))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

            g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }

        g.setColour (COLOR_BGDARK.interpolatedWith (COLOR_BG, 0.86f));
        g.fillRect (width - 1, 0, 1, height);
    }

    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            chopXml->sortChildElements (sorter);

            table.updateContent();
        }
    }

    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
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

        //if (columnId == COLID_VISIB)
        //{
        //    auto* hiddenToggle = static_cast<ToggleButtonColumnComponent*> (existingComponentToUpdate);

        //    if (hiddenToggle == nullptr)
        //        hiddenToggle = new ToggleButtonColumnComponent (*this);

        //    hiddenToggle->setRowAndColumn (rowNumber, columnId);
        //    return hiddenToggle;
        //}

        //// The other columns are editable text columns, for which we use the custom Label component
        //auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        //// same as above...
        //if (textLabel == nullptr)
        //    textLabel = new EditableTextCustomComponent (*this);

        //textLabel->setRowAndColumn (rowNumber, columnId);
        //return textLabel;
    }

    // This is overloaded from TableListBoxModel, and should choose the best width for the specified
    // column.
    int getColumnAutoSizeWidth (int columnId) override
    {
        if (columnId == 5)
            return 100; // (this is the ratings column, containing a custom combobox component)

        int widest = 32;

        // find the widest bit of text in this column..
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

    void setChopVisible (const int rowNumber, const bool hidden)
    {
        auto chop = chopTree.getChildWithProperty(PROP_ID, getChopIdAtRow(rowNumber));
        chop.setProperty(PROP_HIDDEN, hidden, nullptr);
    }

    int getTriggerNote (const int rowNumber) const
    {
        return chopXml->getChildElement (rowNumber)->getIntAttribute (COL_TRIGG);
    }

    void setTriggerNote (const int rowNumber, const int newTrigger)
    {
        chopXml->getChildElement (rowNumber)->setAttribute (COL_TRIGG, newTrigger);
    }

    String getText (const int columnNumber, const int rowNumber) const
    {
        return chopXml->getChildElement (rowNumber)->getStringAttribute (getAttributeNameForColumnId(columnNumber));
    }

    void setText (const int columnNumber, const int rowNumber, const String& newText)
    {
        auto columnName = table.getHeader().getColumnName (columnNumber);
        chopXml->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }

    //==============================================================================
    void resized() override
    {
        // position our table with a gap around its edge
        table.setBoundsInset (BorderSize<int> (1));
    }

    int getChopIdAtRow (int rowNumber) {
        return chopXml->getChildElement (rowNumber)->getIntAttribute (COLNAME_ID);
    }

    ValueTree getChopAtRow (int rowNumber) {
        return chopTree.getChildWithProperty(PROP_ID, getChopIdAtRow(rowNumber));
    }

    void cellClicked(int rowNumber, int columnId, const MouseEvent &e)
    {
        // for popup menu’s item selection use result
        int result = 0;
        if (e.mods.isLeftButtonDown())
        {
        } else if (e.mods.isRightButtonDown())
        {
            rowClickedMenu->clear();
            rowClickedMenu->addItem (ROWMENUID_HIDDEN, ROW_HIDDEN, true, getChopAtRow (rowNumber).getProperty(PROP_HIDDEN));
            rowClickedMenu->addItem (ROWMENUID_DELETE, ROW_DELETE, true, false);
            result = rowClickedMenu->show();
        }

        if (result)
        {
            switch (result)
            {
                case ROWMENUID_HIDDEN:
                    setChopVisible (rowNumber, !getChopAtRow (rowNumber).getProperty(PROP_HIDDEN));
                break;

                case ROWMENUID_DELETE:
                    deletedChopId = chopXml->getChildElement (rowNumber)->getIntAttribute (COLNAME_ID);
                    if (deletedChopId == int (selectedChopId.getValue()))
                    {
                        selectedChopId = NONE;
                    }
                    chopTree.removeChild(chopTree.getChildWithProperty(PROP_ID, deletedChopId), nullptr);
                    reloadData();
                break;
            }
        }
    }

    void selectedRowsChanged (int lastRowChanged) override
    {
        if (lastRowChanged != -1)
        {
            selectedChopId = chopXml->getChildElement (lastRowChanged)->getIntAttribute (COLNAME_ID);
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
        return int(selectedChopId.getValue());
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
    XmlElement* chopXml = nullptr;
    XmlElement* columnXml = nullptr;
    
    PopupMenu* rowClickedMenu = nullptr;

    Value selectedChopId;
    int deletedChopId;
    
    // Row popup menu constants
    enum RowMenuIds
    {
        ROWMENUID_HIDDEN = 1100,
        ROWMENUID_DELETE = 1101
    };
    const String ROW_DELETE = "Delete chop";
    const String ROW_HIDDEN = "Hide markers";

    // Chop XML constants
    enum ColumnIds
    {
        COLID_ID = 1010,
        COLID_START = 1011,
        COLID_END = 1012,
        COLID_TRIGG = 1013
    };

    const String COLUMNS = "ChopListColumns";
    const String COLUMN = "Column";
    const String COLUMN_ID = "columnId";
    const String COLUMN_NAME = "name";
    const String COLATR_WIDTH = "width";

    const String COLNAME_ID = "ID";
    const String COLNAME_START = "Start";
    const String COLNAME_END = "End";
    const String COL_TRIGG = "Trigger";
     

    //==============================================================================
    // This is a custom Label component, which we use for the table's editable text columns.
    class EditableTextCustomComponent : public Label
    {
    public:
        EditableTextCustomComponent (ChopListComponent& td) : owner (td)
        {
            // double click to edit the label text; single click handled below
            setEditable (false, true, false);
        }

        void mouseDown (const MouseEvent& event) override
        {
            // single click on the label should simply select the row
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }

        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), dontSendNotification);
        }

        void paint (Graphics& g) override
        {
            auto& lf = getLookAndFeel();
            if (!dynamic_cast<LookAndFeel_V4*> (&lf))
                lf.setColour (textColourId, Colours::black);

            Label::paint (g);
        }

    private:
        ChopListComponent& owner;
        int row, columnId;
        Colour textColour;
    };

    //==============================================================================
    // custom component containing a toggle button
    class ToggleButtonColumnComponent : public Component
    {
    public:
        ToggleButtonColumnComponent (ChopListComponent& td) : owner (td)
        {
            // just put a combo box inside this component
            addAndMakeVisible (button);
            button.setColour (TextButton::buttonColourId, COLOR_BG);
            button.setColour (TextButton::textColourOffId, COLOR_FG);            
            button.setEnabled (true);
            button.onClick = [this] { owner.setChopVisible (row, button.getToggleState()); };
            button.setWantsKeyboardFocus (false);
        }

        void resized() override
        {
            button.setBoundsInset (BorderSize<int> (2));
        }

        // owner will call this when we may need to update our contents
        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            button.setToggleState (owner.getChopAtRow (newRow)[PROP_HIDDEN], NotificationType::dontSendNotification);
        }

    private:
        ChopListComponent& owner;
        ToggleButton button;
        int row, columnId;
    };

    //==============================================================================
    // custom component containing a combo box
    class TriggerNoteColumnComponent : public Component
    {
    public:
        TriggerNoteColumnComponent (ChopListComponent& td) : owner (td)
        {
            comboBox.setColour (ComboBox::outlineColourId, COLOR_BGDARK);
            comboBox.setColour (ComboBox::backgroundColourId, COLOR_BG);
            comboBox.setColour (ComboBox::buttonColourId, COLOR_BGDARK);
            comboBox.setColour (ComboBox::arrowColourId, COLOR_BGDARK);
            comboBox.setColour (ComboBox::textColourId, COLOR_FG);
            addAndMakeVisible (comboBox);
            comboBox.addItem ("C4", 60);
            comboBox.addItem ("C#4", 61);
            comboBox.addItem ("D4", 62);
            comboBox.addItem ("D#4", 63);
            comboBox.addItem ("E4", 64);
            comboBox.addItem ("F4", 65);
            comboBox.addItem ("F#4", 66);
            comboBox.addItem ("G4", 67);
            comboBox.addItem ("G#4", 68);
            comboBox.addItem ("A4", 69);
            comboBox.addItem ("A#4", 70);
            comboBox.addItem ("B4", 71);
                             
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

    //==============================================================================
    // A comparator used to sort our data when the user clicks a column header
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

        //columnXml->writeTo(File::getCurrentWorkingDirectory().getChildFile ("columns.xml"), {});
    }

    void chopsToXml()
    {
        if (chopXml != nullptr)     
            delete chopXml;
        
        chopXml = chopTree.createXml().release();

        //chopTree.createXml()->writeTo(File::getCurrentWorkingDirectory().getChildFile ("chops.xml"), {});
    }

    //==============================================================================

    // (a utility method to search our XML for the attribute that matches a column ID)
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
