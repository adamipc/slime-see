#include "app/input.h"

function void
inputevent_list_push(M_Arena *arena, InputEventList *list, InputEvents event, void* data) {
  InputEventNode *node = push_array(arena, InputEventNode, 1);
  node->event = event;
  node->data = data;

  SLLQueuePush(list->first, list->last, node);
  list->node_count += 1;
}


function InputEventList
app_process_input(M_Arena *arena, MidiDeviceHandle *midi_handle) {
  Assert(sizeof(((InputEventNode*)0)->data) >= sizeof(LoadPresetData));

  InputEventList result = {};
  result.node_count = 0;

  // Read midi
  if (midi_handle != 0) {
    MidiMessage *message = 0;
    while ((message = os_media_midi_read(arena, midi_handle)) != 0) {
      /*
  MidiStatus_NoteOff = 0x80,
  MidiStatus_NoteOn = 0x90,
  MidiStatus_PolyphonicKeyPressure = 0xA0,
  MidiStatus_ControlChange = 0xB0,
  MidiStatus_ProgramChange = 0xC0,
  MidiStatus_ChannelPressure = 0xD0,
  MidiStatus_PitchBendChange = 0xE0,
  MidiStatus_System = 0xF0,
  */
      switch (message->status) {
        case MidiStatus_NoteOn: {
          u8 pad = message->note - 36;
          if (pad <= 9) {
            LoadPresetData data = {};
            data.preset_slot = PresetSlot_Primary;
            data.preset_name = (PresetNames)pad;
            inputevent_list_push(arena, &result, InputEvent_LoadPreset, &data);
          }
        } break;
        default: {
          printf("Unhandled midi message: %02x\n", message->status);
        } break;
      }
    }
  }

  return result;
}
