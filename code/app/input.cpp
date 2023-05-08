#include "app/input.h"

function void
windowevent_list_push(M_Arena *arena, WindowEventList *list, WindowEvents event, void* data) {
  WindowEventNode *node = push_array(arena, WindowEventNode, 1);
  node->event = event;
  node->data = data;

  SLLQueuePush(list->first, list->last, node);
  list->node_count += 1;
}

function WindowEventList
win32_process_pending_messages(M_Arena *arena) {
  WindowEventList result = {};
  result.node_count = 0;

  MSG message;
  while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
  {
    WindowEvents event = WindowEvent_None;
    KeyboardData *data = push_array(arena, KeyboardData, 1);
    switch(message.message) {
      case WM_QUIT: {
        event = WindowEvent_CloseRequested;
      } break;
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP: {
        event = WindowEvent_KeyboardInput;
        data->VKCode = (u32)message.wParam;
        data->WasDown = ((message.lParam & (1 << 30)) != 0);
        data->IsDown = ((message.lParam & (1 << 31)) == 0);
        data->AltKeyWasDown = (message.lParam & (1 << 29));
       } break;
      default: {
        TranslateMessage(&message);
        DispatchMessageA(&message);
      } break;
    }
    if (event != WindowEvent_None) {
      windowevent_list_push(arena, &result, event, data);
    }
  }

  return result;
}

function void
inputevent_list_push(M_Arena *arena, InputEventList *list, InputEvents event, void* data) {
  InputEventNode *node = push_array(arena, InputEventNode, 1);
  node->event = event;
  node->data = data;

  SLLQueuePush(list->first, list->last, node);
  list->node_count += 1;
}

function InputEventList
app_process_input(M_Arena *arena, MidiDeviceHandle *midi_handle, WindowEventList *window_events) {
  Assert(sizeof(((InputEventNode*)0)->data) >= sizeof(PresetData));

  InputEventList result = {};
  result.node_count = 0;

  // Read window events
  for (WindowEventNode *node = window_events->first;
       node != 0;
       node = node->next) {
    switch (node->event) {
      case WindowEvent_KeyboardInput: {
        KeyboardData *data = (KeyboardData*)node->data;
        // Diable key repeat
        if (data->IsDown && !data->WasDown) {
          if (data->VKCode == VK_ESCAPE || (data->VKCode == VK_F4 && data->AltKeyWasDown)) {
            inputevent_list_push(arena, &result, InputEvent_StopRunning, 0);
          } else if (data->VKCode == VK_RETURN) {
            inputevent_list_push(arena, &result, InputEvent_ToggleFullscreen, 0);
          } else if (data->VKCode == 'R') {
            PresetData *preset_data = push_array(arena, PresetData, 1);
            preset_data->preset_slot = PresetSlot_Primary;
            preset_data->preset_intensity = 1.0f;
            inputevent_list_push(arena, &result, InputEvent_RandomizePreset, preset_data);
          } else if (data->VKCode == 'P') {
            inputevent_list_push(arena, &result, InputEvent_PanicAtTheDisco, 0);
          } else if (data->VKCode == 'C') {
            inputevent_list_push(arena, &result, InputEvent_ClearTextures, 0);
          } else if (data->VKCode == 'S') {
            inputevent_list_push(arena, &result, InputEvent_DumpState, 0);
          } else if (data->VKCode == 'L') {
            inputevent_list_push(arena, &result, InputEvent_LoadState, 0);
          } else if (data->VKCode == 'A') {
            inputevent_list_push(arena, &result, InputEvent_ToggleAutomation, 0);
          } else if (data->VKCode == VK_BACK) {
            inputevent_list_push(arena, &result, InputEvent_TakeScreenshot, 0);
          } else if (data->VKCode >= '0' && data->VKCode <= '9') {
            int preset_index = data->VKCode - '0';
            PresetData *preset_data = push_array(arena, PresetData, 1);
            preset_data->preset_slot = PresetSlot_Primary;
            preset_data->preset_name = (PresetNames)preset_index + 1;
            preset_data->preset_intensity = 1.0f;
            inputevent_list_push(arena, &result, InputEvent_LoadPreset, preset_data);
          }
        }
      } break;
      case WindowEvent_CloseRequested: {
        inputevent_list_push(arena, &result, InputEvent_StopRunning, 0);
      } break;
    }
  }

  // Read midi
  if (midi_handle != 0) {
    MidiMessage *message = 0;
    while ((message = os_media_midi_read(arena, midi_handle)) != 0) {
      InputEvents event = InputEvent_None;
      void *data = 0;
      switch (message->status) {
        case MidiStatus_ProgramChange: {
          //printf("Program change: %d\n", message->program);
          // Ignore for now
        } break;
        case MidiStatus_PolyphonicKeyPressure: {
          //printf("Polyphonic key pressure: %d %d\n", message->poly_note, message->poly_pressure);
          // Ignore for now
        } break;
        case MidiStatus_ControlChange: {
          //printf("Control change: %d %d\n", message->controller, message->value);
          //printf("Debug byte1: %02x, byte2: %02x\n", message->byte1, message->byte2);
          switch (message->controller) {
            case 3: {
              event = InputEvent_UpdateBlendValue;
              data = push_array(arena, BlendValueData, 1);
              ((BlendValueData *)data)->blend_value = message->value/127.0f;
            } break;
            case 9:
            case 17:
            case 23: {
              event = InputEvent_UpdateBeatTransitionTime;
              data = push_array(arena, BeatTransitionTimeData, 1);
              ((BeatTransitionTimeData *)data)->beat_transition_ms = 125.0f * message->value/127.0f;
            } break;
            case 12:
            case 13:
            case 14:
            case 15: {
              event = InputEvent_UpdateWindowGlitch;
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = (GlitchWindowParam)(message->controller - 12);
              // Our controller sends values from 0 to 127, but we want to map it to -64 to 64
              // We want to map 0 to 0, 127 through 64 to -1 through -64 and 1 through 63 to 1 through 63

              ((GlitchWindowData *)data)->glitch_value = (message->value < 64) ? (message->value) : (message->value - 128);
            } break;
            case 16: {
              event = InputEvent_UpdateColorSwap;
              data = push_array(arena, ColorSwapData, 1);
              ((ColorSwapData *)data)->color_swap = message->value/127.0f;
            } break;
            case 22: {
              event = InputEvent_UpdateBeatSensitivity;
              data = push_array(arena, BeatSensitivityData, 1);
              ((BeatSensitivityData *)data)->beat_sensitivity = message->value/127.0f;
            } break;
            case 24: {
              event = InputEvent_UpdateBeatTransitionRatio;
              data = push_array(arena, BeatTransitionRatioData, 1);
              ((BeatTransitionRatioData *)data)->beat_transition_ratio = message->value/127.0f;
            } break;
            default: {
              printf("Controller %d: %d\n", message->controller, message->value);
            } break;
          }

          if (event != InputEvent_None) {
            inputevent_list_push(arena, &result, event, data);
          }
        } break;
        case MidiStatus_NoteOn: {
          data = push_array(arena, PresetData, 1);
          u8 pad = message->note - 36;
          ((PresetData *)data)->preset_intensity = message->velocity/127.0f;

          if (pad <= 9) {
            event = InputEvent_LoadPreset;
            ((PresetData *)data)->preset_slot = PresetSlot_Primary;
            ((PresetData *)data)->preset_name = (PresetNames)(pad + 1);
          } else if (pad >= 16 && pad <=25) {
            event = InputEvent_LoadPreset;
            ((PresetData *)data)->preset_slot = PresetSlot_Secondary;
            ((PresetData *)data)->preset_name = (PresetNames)(pad - 15);
          } else if (pad >= 32 && pad <= 41) {
            event = InputEvent_LoadPreset;
            ((PresetData *)data)->preset_slot = PresetSlot_Beat;
            ((PresetData *)data)->preset_name = (PresetNames)(pad - 31);
          } else {
            switch (pad) {
              case 10: {
                event = InputEvent_ClearTextures;
              } break;
              case 11: {
                event = InputEvent_PanicAtTheDisco;
              } break;
              case 12: {
                event = InputEvent_RandomizePreset;
                ((PresetData *)data)->preset_slot = PresetSlot_Primary;
              } break;
              case 13: {
                event = InputEvent_RandomizePreset;
                ((PresetData *)data)->preset_slot = PresetSlot_Secondary;
              } break;
              case 14: {
                event = InputEvent_RandomizePreset;
                ((PresetData *)data)->preset_slot = PresetSlot_Beat;
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
            inputevent_list_push(arena, &result, event, data);
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
