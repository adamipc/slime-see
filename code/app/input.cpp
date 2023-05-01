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
  Assert(sizeof(((InputEventNode*)0)->data) >= sizeof(PresetData));

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
          InputEvents event = InputEvent_None;
          PresetData data = {};
          u8 pad = message->note - 36;
          if (pad <= 9) {
            event = InputEvent_LoadPreset;
            data.preset_slot = PresetSlot_Primary;
            data.preset_name = (PresetNames)(pad + 1);
          } else if (pad >= 16 && pad <=25) {
            event = InputEvent_LoadPreset;
            data.preset_slot = PresetSlot_Secondary;
            data.preset_name = (PresetNames)(pad - 15);
          } else if (pad >= 32 && pad <= 41) {
            event = InputEvent_LoadPreset;
            data.preset_slot = PresetSlot_Beat;
            data.preset_name = (PresetNames)(pad - 31);
          } else {
            switch (pad) {
              case 10: {
                event = InputEvent_ClearTextures;
              } break;
              case 11: {
                event = InputEvent_ResetPoints;
              } break;
              case 12: {
                event = InputEvent_RandomizePreset;
                data.preset_slot = PresetSlot_Primary;
              } break;
              case 13: {
                event = InputEvent_RandomizePreset;
                data.preset_slot = PresetSlot_Secondary;
              } break;
              case 14: {
                event = InputEvent_RandomizePreset;
                data.preset_slot = PresetSlot_Beat;
              } break;
              case 15: {
                event = InputEvent_ToggleAutomation;
              } break;
              default: {
                printf("Unknown pad: %d\n", pad);
              } break;
            }
          }

          if (event != InputEvent_None) {
            inputevent_list_push(arena, &result, event, &data);
          }
        } break;
        case MidiStatus_ChannelPressure: {
          // Ignore for now
        } break;
        case MidiStatus_NoteOff: {
          // Ignore for now
        } break;
        default: {
          printf("Unhandled midi message: %02x\n", message->status);
        } break;
      }
    }
  }

  return result;
}
