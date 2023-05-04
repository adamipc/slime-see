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
  f32 beat_transition_time;
  f32 transition_start;
  f32 transition_length;
};

function SlimeSee* slimesee_init(M_Arena *arena, Preset *preset, int width, int height);
function void slimesee_draw(SlimeSee *slimesee, float u_time);
function void slimesee_clear_textures(SlimeSee *slimesee);
function void slimesee_reset_points(M_Arena *arena, SlimeSee *slimesee);
function void slimesee_set_resolution(SlimeSee *slimesee, int width, int height);
function void slimesee_screenshot(M_Arena *arena, SlimeSee *slimesee);
function void slimesee_transition_preset(SlimeSee *slimesee, Preset new_preset, f32 transition_start, f32 transition_length);

#endif // SLIMESEE_H

