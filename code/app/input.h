#ifndef APP_INPUT_H
#define APP_INPUT_H

#include "app/preset.h"

typedef u8 InputEvents;
enum {
  InputEvent_None = 0,

  InputEvent_ToggleFullscreen,
  InputEvent_ToggleAutomation,
  InputEvent_RandomizePreset,
  InputEvent_LoadPreset,
  InputEvent_UpdateBlendValue,
  InputEvent_UpdateBeatTransitionTime,
  InputEvent_StopRunning,
  InputEvent_DumpState,
  InputEvent_ClearTextures,
  InputEvent_ResetPoints,
  InputEvent_TakeScreenshot,

  InputEvent_COUNT,
};

typedef u8 PresetSlot;
enum {
  PresetSlot_Primary = 0,
  PresetSlot_Secondary = 1,
  PresetSlot_Beat = 2,
};

struct PresetData {
  PresetSlot preset_slot;
  PresetNames preset_name;
};

struct InputEventNode {
  InputEventNode *next;
  InputEvents event;
  void *data;
};

struct InputEventList {
  InputEventNode *first;
  InputEventNode *last;
  u64 node_count;
};

function InputEventList app_process_input(M_Arena *arena, MidiDeviceHandle *midi_handle);

#endif // APP_INPUT_H

