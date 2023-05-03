#ifndef PIPELINE_H
#define PIPELINE_H

struct Pipeline {
  u32 number_of_positions;
  GLuint framebuffer;
  GLuint target_textures[2];
  GLuint position_buffers[2];
  GLuint texture0;
  GLuint texture1;
  GLuint shader1;
  GLint a_position_location;
  GLuint shader2;
  GLuint vertex_buffer;
  GLint a_vertex_location;

# define X(U) GLint U##_location_1;
# include "../data/shaders/shader1.uniforms"
# undef X

# define X(U) GLint U##_location_2;
# include "../data/shaders/shader2.uniforms"
# undef X

};

function Pipeline* create_pipeline(M_Arena *arena, Preset *preset, int width, int height);
function f32* generate_initial_positions(M_Arena *arena, Preset *preset);
function void pipeline_reset_target_textures(Pipeline *pipeline, int width, int height);
function void pipeline_set_resolution(Pipeline *pipeline, int width, int height);

#endif // PIPELINE_H
