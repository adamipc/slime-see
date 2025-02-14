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
  InputEvent_LoadState,
  InputEvent_ClearTextures,
  InputEvent_PanicAtTheDisco,
  InputEvent_TakeScreenshot,
  InputEvent_UpdateWindowGlitch,
  InputEvent_UpdateBeatSensitivity,
  InputEvent_UpdateBeatTransitionRatio,
  InputEvent_UpdateColorSwap,

  InputEvent_DEBUGUpdatePeakPickerThreshold,
  InputEvent_DEBUGUpdatePeakPickerMinThreshold,
  InputEvent_DEBUGUpdatePeakPickerDecayFactor,
  InputEvent_DEBUGUpdatePeakPickerDelayMS,
  InputEvent_DEBUGPeakPickerReset,
  InputEvent_DEBUGLoadStudio143Logo,
  InputEvent_DEBUGSetActiveDJLogo,

  InputEvent_COUNT,
};

typedef u8 DJLogo;
enum {
  DJLogo_None = 0,
  DJLogo_TheLSDJ,
  DJLogo_CameronWade,
  DJLogo_OuttaPhase,

  DJLogo_COUNT,
};

struct DEBUGActiveDJLogoData {
  DJLogo active_dj_logo;
};

struct LoadStateData {
  String8 state_filename;
};

struct PresetData {
  PresetSlot preset_slot;
  PresetNames preset_name;
  f32 preset_intensity;
};

struct BeatTransitionRatioData {
  f32 beat_transition_ratio;
};

struct BeatSensitivityData {
  f32 beat_sensitivity;
};

struct BlendValueData {
  f32 blend_value;
};

struct BeatTransitionTimeData {
  f32 beat_transition_ms;
};

struct ColorSwapData {
  f32 color_swap;
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

typedef u8 GlitchWindowParam;
enum {
  GlitchWindowParam_X = 0,
  GlitchWindowParam_Y = 1,
  GlitchWindowParam_Width = 2,
  GlitchWindowParam_Height = 3,
};

struct GlitchWindowData {
  i8 glitch_value;
  GlitchWindowParam window_param;
  b32 glitch_reset;
};

struct PeakPickerThresholdData {
  f32 threshold_change;
};

struct PeakPickerDecayFactorData {
  f32 decay_factor_change;
};

struct PeakPickerDelayMSData {
  f32 delay_ms_change;
};

typedef u8 WindowEvents;
enum {
  WindowEvent_None = 0,
  WindowEvent_CloseRequested,
  WindowEvent_KeyboardInput,

  WindowEvent_COUNT,
};

struct KeyboardData {
  u32 VKCode;
  b32 WasDown;
  b32 IsDown;
  b32 AltKeyWasDown;
};

struct WindowEventNode {
  WindowEventNode *next;
  WindowEvents event;
  void *data;
};

struct WindowEventList {
  WindowEventNode *first;
  WindowEventNode *last;
  u64 node_count;
};

function InputEventList app_process_input(M_Arena *arena, MidiDeviceHandle *midi_handle, WindowEventList *window_events);
function void inputevent_list_push(M_Arena *arena, InputEventList *list, InputEvents event, void* data);

function void windowevent_list_push(M_Arena *arena, WindowEventList *list, WindowEvents event, void* data);
function WindowEventList win32_process_pending_messages(M_Arena *arena);

#endif // APP_INPUT_H

