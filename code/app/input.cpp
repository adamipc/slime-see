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
      //printf("Debug byte1: %02x, byte2: %02x\n", message->byte1, message->byte2);
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
          // The top 2 knobs on each page send absolute values from 0 to 127
          // The bottom 4 all send inc/dec values from -64 to 63 (or maybe -63 to 64)
          // or maybe even -64 to -1 and 1 to 64 since they wouldn't send 0
          // in most pratical cases its usually going to be -1 or 1 but if you turn
          // the knob fast enough you'll get larger absolute values
          //
          // Knobs have the controller value:
          // Page 1:
          //  3  9
          // 12 13
          // 14 15
          // Page 2:
          // 16 17
          // 18 19
          // 20 21
          // Page 3:
          // 22 23
          // 24 25
          // 26 27
          //
          // Debug knobs:
          // Page 1:
          // 28 29
          // 30 31
          // 32 33
          //
          switch (message->controller) {
            case 3:
            case 16:
            case 22: {
              event = InputEvent_UpdateBlendValue;
              data = push_array(arena, BlendValueData, 1);
              ((BlendValueData *)data)->blend_value = message->value/127.0f;
            } break;
            case 9:
            case 17:
            case 23: {
              event = InputEvent_UpdateColorSwap;
              data = push_array(arena, ColorSwapData, 1);
              ((ColorSwapData *)data)->color_swap = message->value/127.0f;
            } break;
            case 12:
            case 13:
            case 14:
            case 15:
            case 18:
            case 19:
            case 20:
            case 21:
            case 24:
            case 25:
            case 26:
            case 27: {
              event = InputEvent_UpdateWindowGlitch;
              data = push_array(arena, GlitchWindowData, 1);
              u32 window_param = (message->controller - 12) % 4;
              ((GlitchWindowData*)data)->window_param = (GlitchWindowParam)window_param;
              ((GlitchWindowData *)data)->glitch_value = (message->value < 64) ? (message->value) : (message->value - 128);
              ((GlitchWindowData *)data)->glitch_reset = false;

            } break;
            case 28: {
              event = InputEvent_UpdateBeatTransitionTime;
              data = push_array(arena, BeatTransitionTimeData, 1);
              printf("Beat transition time: %f ms\n", 150.0f * message->value/127.0f);
              ((BeatTransitionTimeData *)data)->beat_transition_ms = 75.0f * message->value/127.0f;
            } break;
            case 29: {
              event = InputEvent_UpdateBeatTransitionRatio;
              data = push_array(arena, BeatTransitionRatioData, 1);
              printf("Beat transition ratio: %f\n", message->value/127.0f);
              ((BeatTransitionRatioData *)data)->beat_transition_ratio = message->value/127.0f;
            } break;
            case 30: {
              // TODO(adam): Some other useful debug function!
            } break;
            case 31: {
              event = InputEvent_DEBUGUpdatePeakPickerMinThreshold;
              data = push_struct(arena, PeakPickerThresholdData);
              // send relative changes
              ((PeakPickerThresholdData *)data)->threshold_change = 1.0f * (message->value < 64 ? message->value : message->value - 128);
            } break;
            case 32: {
              event = InputEvent_DEBUGUpdatePeakPickerDecayFactor;
              data = push_struct(arena, PeakPickerDecayFactorData);
              // send relative changes
              ((PeakPickerDecayFactorData *)data)->decay_factor_change = 0.00001f * (message->value < 64 ? message->value : message->value - 128);
            } break;
            case 33: {
              event = InputEvent_DEBUGUpdatePeakPickerDelayMS;
              data = push_struct(arena, PeakPickerDelayMSData);
              // send relative changes
              ((PeakPickerDelayMSData *)data)->delay_ms_change = 1.f * (message->value < 64 ? message->value : message->value - 128);
            } break;
            /*
            case 29: {
              event = InputEvent_UpdateBeatSensitivity;
              data = push_array(arena, BeatSensitivityData, 1);
              ((BeatSensitivityData *)data)->beat_sensitivity = message->value/127.0f;
            } break;
            */
            default: {
              printf("Controller %d: %d\n", message->controller, message->value);
            } break;
          }

          if (event != InputEvent_None) {
            inputevent_list_push(arena, &result, event, data);
          }
        } break;
        case MidiStatus_NoteOn: {
          u8 pad = message->note - 36;

          /* Our pad layout is:
           *
           * page 1:
           * 12 13 14 15
           *  8  9 10 11
           *  4  5  6  7
           *  0  1  2  3
           *
           * page 2:
           * 28 29 30 31
           * 24 25 26 27
           * 20 21 22 23
           * 16 17 18 19
           *
           * page 3:
           * 44 45 46 47
           * 40 41 42 43
           * 36 37 38 39
           * 32 33 34 35
           *
           * Debug pads:
           * page 1:
           * 60 61 62 63
           * 56 57 58 59
           * 52 53 54 55
           * 48 49 50 51
           *
           * For our set we want the pads on each page to do the same thing, or
           * almost the same thing so if the page gets changed we don't have to worry
           * about people getting confused if it doesn't do the same thing as it did
           * before
           *
           * let's have the corners do very different functions while the inner 4 pads
           * have a similar effect and then the edges do slightly different things
           */

          data = push_struct(arena, LoadStateData);
          LoadStateData *load_state_data = (LoadStateData *)data;
          switch (pad) {
            case 0:
            case 16:
            case 32: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("color_theory_2"));
            } break;
            case 1:
            case 17:
            case 33: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("jellyfish_breaks_1"));
            } break;
            case 2:
            case 18:
            case 34: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("panache_1"));
            } break;
            case 3:
            case 19:
            case 35: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("watercolours_1"));
            } break;
            case 4:
            case 20:
            case 36: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("lava_lamp_1"));
            } break;
            case 5:
            case 21:
            case 37: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("ahha_1"));
            } break;
            case 6:
            case 22:
            case 38: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("pretty_lights_1"));
            } break;
            case 7:
            case 23:
            case 39: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("burning_chrome_1"));
            } break;
            case 8:
            case 24:
            case 40: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("shizzle_1"));
            } break;
            case 9:
            case 25:
            case 41: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("slimesee_013"));
            } break;
            case 10:
            case 26:
            case 42: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("warm_fire_1"));
            } break;
            case 11:
            case 27:
            case 43: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("particle_accelerator_1"));
            } break;
            case 12:
            case 28:
            case 44: {
              event = InputEvent_RandomizePreset;
              data = push_struct(arena, PresetData);
              ((PresetData *)data)->preset_slot = PresetSlot_Primary;
              ((PresetData *)data)->preset_intensity = message->velocity/127.0f;
              inputevent_list_push(arena, &result, event, data);
              data = push_struct(arena, PresetData);
              ((PresetData *)data)->preset_slot = PresetSlot_Secondary;
              ((PresetData *)data)->preset_intensity = message->velocity/127.0f;
              inputevent_list_push(arena, &result, event, data);
              data = push_struct(arena, PresetData);
              ((PresetData *)data)->preset_slot = PresetSlot_Beat;
              ((PresetData *)data)->preset_intensity = message->velocity/127.0f;
              // bottom of the loop will push this event
            } break;
                     // Yes
            case 13:
            case 29:
            case 45: {
              event = InputEvent_UpdateWindowGlitch;
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_X;
              ((GlitchWindowData *)data)->glitch_value = (i8)(rand()/((float)RAND_MAX) * 20 - 10);
              ((GlitchWindowData *)data)->glitch_reset = false;
              inputevent_list_push(arena, &result, event, data);
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_Y;
              ((GlitchWindowData *)data)->glitch_value = (i8)(rand()/((float)RAND_MAX) * 20 - 10);
              ((GlitchWindowData *)data)->glitch_reset = false;
              inputevent_list_push(arena, &result, event, data);
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_Width;
              ((GlitchWindowData *)data)->glitch_value = (i8)(rand()/((float)RAND_MAX) * 20 - 10);
              ((GlitchWindowData *)data)->glitch_reset = false;
              inputevent_list_push(arena, &result, event, data);
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_Height;
              ((GlitchWindowData *)data)->glitch_value = (i8)(rand()/((float)RAND_MAX) * 20 - 10);
              ((GlitchWindowData *)data)->glitch_reset = false;
            } break;
                     // No
            case 14:
            case 30:
            case 46: {
              event = InputEvent_UpdateWindowGlitch;
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_X;
              ((GlitchWindowData *)data)->glitch_value = 0;
              ((GlitchWindowData *)data)->glitch_reset = true;
              inputevent_list_push(arena, &result, event, data);
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_Y;
              ((GlitchWindowData *)data)->glitch_value = 0;
              ((GlitchWindowData *)data)->glitch_reset = true;
              inputevent_list_push(arena, &result, event, data);
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_Width;
              ((GlitchWindowData *)data)->glitch_value = 0;
              ((GlitchWindowData *)data)->glitch_reset = true;
              inputevent_list_push(arena, &result, event, data);
              data = push_array(arena, GlitchWindowData, 1);
              ((GlitchWindowData *)data)->window_param = GlitchWindowParam_Height;
              ((GlitchWindowData *)data)->glitch_value = 0;
              ((GlitchWindowData *)data)->glitch_reset = true;
            } break;
            case 15:
            case 31:
            case 47: {
              event = InputEvent_PanicAtTheDisco;
            } break;
            // Debug pads:
            case 48: {
              // Reset peak picker debug changes
              event = InputEvent_DEBUGPeakPickerReset;
            } break;
            case 49: {
              event = InputEvent_LoadState;
              load_state_data->state_filename = str8_push_copy(arena, str8_lit("logos_1"));
            } break;
          }

          if (event != InputEvent_None) {
            inputevent_list_push(arena, &result, event, data);
          }

          // If on page 2 or 3, randomize color swap and blend values for every pad
          // We have to send this after the pad event as it might overwrite this change
          u32 page = (pad / 16) + 1;
          if (page >= 2 && page <= 3) {
            event = InputEvent_UpdateColorSwap;
            data = push_array(arena, ColorSwapData, 1);
            ((ColorSwapData *)data)->color_swap = rand()/((float)RAND_MAX);
            inputevent_list_push(arena, &result, event, data);
            event = InputEvent_UpdateBlendValue;
            data = push_array(arena, BlendValueData, 1);
            ((BlendValueData *)data)->blend_value = rand()/((float)RAND_MAX);
            inputevent_list_push(arena, &result, event, data);
          }

          /* Old code
          data = push_array(arena, PresetData, 1);
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
          */
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
