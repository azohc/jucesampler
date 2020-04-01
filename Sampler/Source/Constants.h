/*
  ==============================================================================

    Constants.h
    Created: 9 Dec 2019 7:50:56pm
    Author:  chosa

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

void print(String s) {
  Logger::getCurrentLogger()->writeToLog(s);
}

// Colors
static const Colour COLOR_BG_DARK = Colour::fromString("FF252420");
static const Colour COLOR_BG =  Colour::fromString("FF544F4C");
static const Colour COLOR_BG_LIGHT = Colour::fromString("FFA1988F");
static const Colour COLOR_FG =  Colour::fromString("FFEADED2");
static const Colour COLOR_GRAY = Colour::fromString("FF605B58");
static const Colour COLOR_GRAY_LIGHT = Colour::fromString("FFD3D4D9");
static const Colour COLOR_RED = Colour::fromString("FFD62734");
static const Colour COLOR_BLUE_DARK = Colour::fromString("FF252D39");
static const Colour COLOR_BLUE = Colour::fromString("FF525C65");
static const Colour COLOR_TAN = Colour::fromString("FFEDB183");
static const Colour COLOR_SEA = Colour::fromString("FF1E555C");


// ValueTree Identifiers
static const Identifier ID_CHOPTREE = "ChopData";
static const Identifier ID_CHOP = "Chop";
static const Identifier PROP_ID = "ID";
static const Identifier PROP_START_TIME = "Start";
static const Identifier PROP_END_TIME = "End";
static const Identifier PROP_START_SAMPLE = "StartSample";
static const Identifier PROP_END_SAMPLE = "EndSample";
static const Identifier PROP_TRIGGER = "Trigger";
static const Identifier PROP_HIDDEN = "Hidden";
static const Identifier PROP_SAMPLER_SOUND = "SamplerSound";

static const int NONE = -110;


// ChopList row actions
const String ROW_DELETE = "Delete chop";
const String ROW_DEL_ALL = "Delete all chops";
const String ROW_HIDDEN = "Hide markers";


// ChopList XML tags
const String COLUMNS = "ChopListColumns";
const String COLUMN = "Column";
const String COLUMN_ID = "columnId";
const String COLUMN_NAME = "name";
const String COLATR_WIDTH = "width";


// ChopList XML Columns
const String COLNAME_ID = "ID";
const String COLNAME_START = "Start";
const String COLNAME_END = "End";
const String COL_TRIGG = "Trigger";


// Onset detection methods
const String ONSET_ENERGY = "energy";
const String ONSET_HFC = "hfc";
const String ONSET_COMPLEX = "complex";
const String ONSET_PHASE = "phase";
const String ONSET_SPECDIFF = "specdiff";
const String ONSET_KL = "kl";
const String ONSET_MKL = "mkl";
const String ONSET_SPECFLUX = "specflux";
const String ONSET_METHODS[] = { ONSET_ENERGY, ONSET_HFC, ONSET_COMPLEX, ONSET_PHASE, ONSET_SPECDIFF, ONSET_KL, ONSET_MKL, ONSET_SPECFLUX };

// TODO use for popup dialog on delete all chops
const String MSG_DEL_ALL = "Are you sure you want to delete every chop?";
const String MSG_CONFIRM = "Confirm";
const String MSG_DECLINE = "Decline";

const int INIT_NOTE_AUTO_ASSIGN = 72; // C4

// IDs for Radio buttons
enum RadioButtonIds
{
    LoopMode = 1001
};

// IDs for ChopList right click menu
enum RowMenuIds
{
    ROWMENUID_HIDDEN = 1100,
    ROWMENUID_DELETE = 1101,
    ROWMENUID_DEL_ALL = 1102
};

// XML Column IDs for TableList
enum ColumnIds
{
    COLID_ID = 1010,
    COLID_START = 1011,
    COLID_END = 1012,
    COLID_TRIGG = 1013
};

struct Chop
{
  Chop (const ValueTree &v) : state(v) { jassert (v.hasType (ID_CHOP)); }

  int getId() const { return state[PROP_ID]; }
  void setId (int id) { state.setProperty (PROP_ID, id, nullptr); }

  double getStartTime() const { return state[PROP_START_TIME]; }

  void setStartTime (double time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_START_TIME, time, nullptr);
  }

  double getEndTime() const { return state[PROP_END_TIME]; }

  void setEndTime (double time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_END_TIME, time, nullptr);
  }

  int getStartSample() const { return state[PROP_START_SAMPLE]; }

  void setStartSample (int time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_START_SAMPLE, time, nullptr);
  }

  int getEndSample() const { return state[PROP_END_SAMPLE]; }

  void setEndSample (int time) {
    jassert (time >= 0.0);
    state.setProperty (PROP_END_SAMPLE, time, nullptr);
  }   

  int getTriggerNote() const { return state[PROP_TRIGGER]; }
  void setTriggerNote (int note) { state.setProperty (PROP_TRIGGER, note, nullptr); }
  
  bool getHidden() const { return state[PROP_HIDDEN]; }
  void setHidden (bool hidden) { state.setProperty (PROP_HIDDEN, hidden, nullptr); } 

  ValueTree state;
};