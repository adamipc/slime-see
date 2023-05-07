
#include "graphics_helper.h"

function void
draw_rect(u8 *image, u32 image_width, u32 image_height, u32 loc_x, u32 loc_y, u32 width, u32 height, u32 color) {
  u32 *pixel = (u32 *)image + loc_y * image_width + loc_x;
  for (u32 y = 0; y < height; ++y) {
    for (u32 x = 0; x < width; ++x) {
      *pixel++ = color;
    }
    pixel += image_width - width;
  }
}
