#ifndef SLIMESEE_H
#define SLIMESEE_H

#include "app/pipeline.h"

struct SlimeSeeState {
  Preset primary_preset;
  Preset secondary_preset;
  Preset beat_preset;
  Preset old_preset;
  f32 blend_value;
  f32 beat_transition_ms;
  b32 beat_transition;
  f32 transition_start;
  f32 transition_length_ms;
  f32 beat_transition_ratio;
  f32 color_swap;
};

struct SlimeSee {
  Pipeline pipeline;
  int width;
  int height;
  int screenshot_count;
  f32 u_time_ms;
  SlimeSeeState *state;
  f32 beat_intensity;
  f32 stored_transition_start;
  f32 stored_transition_length_ms;
  b32 reset_points_on_beat;
  b32 clear_textures_on_beat;
};

typedef u8 PresetSlot;
enum {
  PresetSlot_Primary = 0,
  PresetSlot_Secondary = 1,
  PresetSlot_Beat = 2,
};

function SlimeSee* slimesee_init(M_Arena *arena, int width, int height);
function void slimesee_update_time(SlimeSee* slimesee, u64 elapsed_time_microseconds, b32 automate_presets);
function void slimesee_draw(SlimeSee *slimesee);
function void slimesee_clear_textures(SlimeSee *slimesee);
function void slimesee_reset_points(SlimeSee *slimesee);
function void slimesee_set_resolution(SlimeSee *slimesee, int width, int height);
function void slimesee_screenshot(M_Arena *arena, SlimeSee *slimesee);
function void slimesee_transition_preset(SlimeSee *slimesee, Preset new_preset, f32 intensity);
function void slimesee_beat_transition(SlimeSee *slimesee, f32 beat_intensity);
function void slimesee_set_beat_transition_ratio(SlimeSee *slimesee, f32 ratio);
function SlimeSeeState* slimesee_dump_state(M_Arena *arena, SlimeSee *slimesee);
function void slimesee_load_state(SlimeSee *slimesee, SlimeSeeState *state);
function void slimesee_set_preset(SlimeSee *slimesee, PresetSlot slot, Preset new_preset);
function void slimesee_set_beat_transition_ms(SlimeSee *slimesee, f32 beat_transition_ms);
function void slimesee_set_blend_value(SlimeSee *slimesee, f32 blend_value);
function void slimesee_set_color_swap(SlimeSee *slimesee, f32 color_swap);
function void slimeseestate_write_to_file(SlimeSeeState *state, String8 filename);
function SlimeSeeState* slimeseestate_read_from_file(M_Arena *arena, String8 filename);

#endif // SLIMESEE_H

