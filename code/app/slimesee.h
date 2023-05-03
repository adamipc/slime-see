#ifndef SLIMESEE_H
#define SLIMESEE_H

#include "app/pipeline.h"

struct SlimeSee {
  Preset preset;
  Pipeline pipeline;
  int width;
  int height;
  int screenshot_count;
};

function SlimeSee* slimesee_init(M_Arena *arena, Preset *preset, int width, int height);
function void slimesee_draw(SlimeSee *slimesee, float u_time);
function void slimesee_clear_textures(SlimeSee *slimesee);
function void slimesee_reset_points(M_Arena *arena, SlimeSee *slimesee);
function void slimesee_set_resolution(SlimeSee *slimesee, int width, int height);
function void slimesee_screenshot(M_Arena *arena, SlimeSee *slimesee);

#endif // SLIMESEE_H

