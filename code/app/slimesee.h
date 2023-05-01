#ifndef SLIMESEE_H
#define SLIMESEE_H

#include "app/pipeline.h"

struct SlimeSee {
  Preset preset;
  Pipeline pipeline;
};

function SlimeSee* slimesee_init(M_Arena *arena, Preset *preset, int width, int height);
function void slimesee_draw(SlimeSee *slimesee, float u_time);

#endif // SLIMESEE_H

