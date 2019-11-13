/*
  ==============================================================================

    ChopListComponent.h
    Created: 10 Nov 2019 7:57:01pm
    Author:  wozas

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class ChopListComponent    : public Component, public TableListBoxModel
{
public:
    ChopListComponent(HashMap<int, Chop>& chopMap,
                      HashMap<String, Colour>& colorMap): 
        chops (chopMap), 
        colors (colorMap)
    {
        // Load some data from an embedded XML file..
        initTableData();

        // Create our table component and add it to this component..
        addAndMakeVisible (table);
        table.setModel (this);

        // Add some columns to the table header, based on the column list
        forEachXmlChildElement (*chopXml->getChildByName (COLUMNS), columnXml)
        {
            table.getHeader().addColumn (columnXml->getStringAttribute (COLATR_NAME),
                                         columnXml->getIntAttribute (COLATR_ID),
                                         columnXml->getIntAttribute (COLATR_WIDTH),
                                         30, -1, TableHeaderComponent::notResizable);
        }

        table.getHeader().setSortColumnId (COLID_START, true); // sort forwards by the ID column
        table.getHeader().setStretchToFitActive (true);
        table.getHeader().setColour (TableHeaderComponent::backgroundColourId, colors["bg"]);
        table.getHeader().setColour (TableHeaderComponent::highlightColourId, colors["bglite"]);
        table.getHeader().setColour (TableHeaderComponent::textColourId, colors["fg"]);
        table.getHeader().setColour (TableHeaderComponent::outlineColourId, colors["bgdark"]);
        table.setColour(TableListBox::backgroundColourId, colors["bg"]);

        table.setMultipleSelectionEnabled (false);
    }

    ~ChopListComponent()
    {
        delete chopXml;
        chopXml = nullptr;
    }

    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    int getNumRows() override
    {
        return numRows;
    }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto selectedRowColour = colors["bgdark"].interpolatedWith (colors["bg"], 0.26f);
        auto altRowColour = colors["bgdark"].interpolatedWith (colors["bg"], 0.66f);
        if (rowIsSelected)
            g.fillAll (selectedRowColour);
        else if (rowNumber % 2)
            g.fillAll (altRowColour);
    }

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (colors["fg"]);
        g.setFont (font);

        if (auto* rowElement = chopXml->getChildByName (DATA)->getChildElement (rowNumber))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

            g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }

        g.setColour (colors["bgdark"].interpolatedWith (colors["bg"], 0.86f));
        g.fillRect (width - 1, 0, 1, height);
    }

    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            chopXml->getChildByName (DATA)->sortChildElements (sorter);

            table.updateContent();
        }
    }

    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
        //if (columnId == 1 || columnId == 7) // The ID and Length columns do not have a custom component
        //{
        jassert (existingComponentToUpdate == nullptr);
        return nullptr;
        //}

        //if (columnId == 5) // For the ratings column, we return the custom combobox component
        //{
        //    auto* ratingsBox = static_cast<RatingColumnCustomComponent*> (existingComponentToUpdate);

        //    // If an existing component is being passed-in for updating, we'll re-use it, but
        //    // if not, we'll have to create one.
        //    if (ratingsBox == nullptr)
        //        ratingsBox = new RatingColumnCustomComponent (*this);

        //    ratingsBox->setRowAndColumn (rowNumber, columnId);
        //    return ratingsBox;
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
            if (auto* rowElement = chopXml->getChildByName (DATA)->getChildElement (i))
            {
                auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

                widest = jmax (widest, font.getStringWidth (text));
            }
        }

        return widest + 8;
    }

    // A couple of quick methods to set and get cell values when the user changes them
    int getRating (const int rowNumber) const
    {
        return chopXml->getChildByName (DATA)->getChildElement (rowNumber)->getIntAttribute ("Rating");
    }

    void setRating (const int rowNumber, const int newRating)
    {
        chopXml->getChildByName (DATA)->getChildElement (rowNumber)->setAttribute ("Rating", newRating);
    }

    String getText (const int columnNumber, const int rowNumber) const
    {
        return chopXml->getChildByName (DATA)->getChildElement (rowNumber)->getStringAttribute (getAttributeNameForColumnId(columnNumber));
    }

    void setText (const int columnNumber, const int rowNumber, const String& newText)
    {
        auto columnName = table.getHeader().getColumnName (columnNumber);
        chopXml->getChildByName (DATA)->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }

    //==============================================================================
    void resized() override
    {
        // position our table with a gap around its edge
        table.setBoundsInset (BorderSize<int> (1));
    }

    void reloadData()
    {
        chopsToXml();
        numRows = chopXml->getChildByName (DATA)->getNumChildElements();
        table.updateContent();
        if (numRows == 1)
        {
            table.selectRow (0);
        }
    }

    void clearChopXml()
    {
        chopXml->removeChildElement (chopXml->getChildByName (DATA), true);
        numRows = 0;
        table.updateContent();
    }

private:
    TableListBox table;     
    int numRows;
    Font font { 14.0f };
    HashMap<int, Chop>& chops;
    
    XmlElement* chopXml = nullptr;

    HashMap<String, Colour>& colors;

    enum ColumnIds
    {
        COLID_START = 1011,
        COLID_END = 1012,
        COLID_MAPTO = 1013
    };

    const String CHOPS = "CHOPS";
    const String COLUMNS = "COLUMNS";
    const String COLUMN = "COLUMN";
    const String COLATR_ID = "columnId";
    const String COLATR_NAME = "name";
    const String COLATR_WIDTH = "width";

    const String DATA = "DATA";
    const String ITEM = "ITEM";
    const String COL_START = "Start";
    const String COL_END = "End";
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
    // This is a custom component containing a combo box, which we're going to put inside
    // our table's "rating" column.
    class RatingColumnCustomComponent : public Component
    {
    public:
        RatingColumnCustomComponent (ChopListComponent& td) : owner (td)
        {
            // just put a combo box inside this component
            addAndMakeVisible (comboBox);
            comboBox.addItem ("fab", 1);
            comboBox.addItem ("groovy", 2);
            comboBox.addItem ("hep", 3);
            comboBox.addItem ("mad for it", 4);
            comboBox.addItem ("neat", 5);
            comboBox.addItem ("swingin", 6);
            comboBox.addItem ("wild", 7);

            comboBox.onChange = [this] { owner.setRating (row, comboBox.getSelectedId()); };
            comboBox.setWantsKeyboardFocus (false);
        }

        void resized() override
        {
            comboBox.setBoundsInset (BorderSize<int> (2));
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            comboBox.setSelectedId (owner.getRating (row), dontSendNotification);
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

    XmlElement* chopsToXml()
    {
        if (chopXml != nullptr)
            delete chopXml;

        chopXml = new XmlElement(CHOPS);
        XmlElement* columns = chopXml->createNewChildElement(COLUMNS);

        auto columnWidth = getBounds().getWidth() || 40;

        XmlElement* columnStart = columns->createNewChildElement(COLUMN);
        columnStart->setAttribute (COLATR_ID, COLID_START);
        columnStart->setAttribute (COLATR_NAME, COL_START);
        columnStart->setAttribute (COLATR_WIDTH, columnWidth);

        XmlElement* columnEnd = columns->createNewChildElement(COLUMN);
        columnEnd->setAttribute (COLATR_ID, COLID_END);
        columnEnd->setAttribute (COLATR_NAME, COL_END);
        columnEnd->setAttribute (COLATR_WIDTH, columnWidth);

        XmlElement* columnMappedTo = columns->createNewChildElement(COLUMN);
        columnMappedTo->setAttribute (COLATR_ID, COLID_MAPTO);
        columnMappedTo->setAttribute (COLATR_NAME, COL_TRIGG);
        columnMappedTo->setAttribute (COLATR_WIDTH, columnWidth);

        XmlElement* data = chopXml->createNewChildElement(DATA);
        for (auto i = chops.begin(); i != chops.end(); i.next())
        {
            XmlElement* chop = data->createNewChildElement(ITEM);
            chop->setAttribute(COL_START, i.getValue().start);
            chop->setAttribute(COL_END, i.getValue().end);
            chop->setAttribute(COL_TRIGG, i.getValue().mappedTo);
        }
        
        //chopXml->writeTo(File::getCurrentWorkingDirectory().getChildFile ("chops.xml"), {});

        return chopXml;
    }

    //==============================================================================
    // load chops into memory
    void initTableData()
    {
        chopsToXml();        
        numRows = 0;
    }

    // (a utility method to search our XML for the attribute that matches a column ID)
    String getAttributeNameForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*chopXml->getChildByName (COLUMNS), columnXml)
        {
            if (columnXml->getIntAttribute (COLATR_ID) == columnId)
                return columnXml->getStringAttribute (COLATR_NAME);
        }

        return {};
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopListComponent)
};
