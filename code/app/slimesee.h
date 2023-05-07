#ifndef SLIMESEE_H
#define SLIMESEE_H

#include "app/pipeline.h"

struct SlimeSee {
  Preset primary_preset;
  Preset secondary_preset;
  Preset beat_preset;
  Preset old_preset;
  Pipeline pipeline;
  int width;
  int height;
  int screenshot_count;
  f32 blend_value;
  f32 beat_transition_ms;
  b32 beat_transition;
  f32 transition_start;
  f32 transition_length_ms;
  f32 beat_transition_ratio;
  f32 color_swap;
  f32 u_time_ms;
};

function SlimeSee* slimesee_init(M_Arena *arena, int width, int height);
function void slimesee_update_time(SlimeSee* slimesee, u64 elapsed_time_microseconds);
function void slimesee_draw(SlimeSee *slimesee);
function void slimesee_clear_textures(SlimeSee *slimesee);
function void slimesee_reset_points(SlimeSee *slimesee);
function void slimesee_set_resolution(SlimeSee *slimesee, int width, int height);
function void slimesee_screenshot(M_Arena *arena, SlimeSee *slimesee);
function void slimesee_transition_preset(SlimeSee *slimesee, Preset new_preset, f32 intensity);
function void slimesee_beat_transition(SlimeSee *slimesee);
function void slimesee_set_beat_transition_ratio(SlimeSee *slimesee, f32 ratio);

#endif // SLIMESEE_H

