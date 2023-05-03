#include "app/slimesee.h"

struct shader_1_uniforms {
  GLuint texture0;
  GLuint texture1;
  float speed_multiplier;
  int   wall_strategy;
  int   color_strategy;
  float random_steer_factor;
  float constant_steer_factor;
  float search_radius;
  float trail_strength;
  float vertex_radius;
  float search_angle;
  float time;
};

struct screenshot_data {
  String8 filename;
  int width;
  int height;
  BYTE *pixels;
  HANDLE thread;
  M_Arena *conflict;
};

// ThreadProc for writing screenshot data to a file
// NOTE(adam): This leaks the pixel buffer
DWORD WINAPI
slimesee_screenshot_thread(LPVOID lpParameter) {
  OS_ThreadContext tctx_memory = {};
  os_thread_init(&tctx_memory);
  M_BaseMemory *memory = tctx_memory.memory;

  M_Arena arena = m_make_arena_reserve(memory, MB(64));

  screenshot_data *data = (screenshot_data *)lpParameter;

  // TODO(adam): Use a different format that packs the data more
  // efficiently and maybe takes less time to process?
  String8List list = {};
  str8_list_pushf(&arena, &list, "P3\n%d %d\n255\n", data->width, data->height);
  for (int i = 0; i < data->width * data->height * 3; i += 3) {
    str8_list_pushf(&arena, &list, "%3d %3d %3d\n", data->pixels[i], data->pixels[i + 1], data->pixels[i + 2]);
  }
  // Write the screenshot data to a file
  os_file_write(data->filename, list);

  os_memory_release(memory, 0);

  return 0;
}

function void
slimesee_screenshot(M_Arena *arena, SlimeSee *slimesee) {
  // Save the current framebuffer to a file
  String8 filename = str8_pushf(arena, "screenshot_%d.ppm", slimesee->screenshot_count++);
  // Create a thread to write the file

  // NOTE(adam): This leaks the pixel buffer in the arena
  BYTE *pixels = push_array(arena, BYTE, slimesee->width * slimesee->height * 3);

  glReadPixels(0, 0, slimesee->width, slimesee->height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  screenshot_data *data = push_array(arena, screenshot_data, 1);
  data->filename = filename;
  data->pixels = pixels;
  data->width = slimesee->width;
  data->height = slimesee->height;
  data->conflict = arena;
  data->thread = CreateThread(NULL, 0, &slimesee_screenshot_thread, data, 0, NULL);
}

function void
slimesee_reset_points(M_Arena *arena, SlimeSee *slimesee) {
    pipeline_generate_initial_positions(arena, &slimesee->pipeline, &slimesee->preset);
}

function void
slimesee_clear_textures(SlimeSee *slimesee) {
  pipeline_create_target_textures(&slimesee->pipeline, slimesee->width, slimesee->height);
}

function void
draw_shader_1(Pipeline *pipeline, shader_1_uniforms *uniforms) {
  // Draw shader 1 to the framebuffer
  // bind textures on corresponding texture units
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pipeline->texture0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, pipeline->texture1);

  // Select shader 1 program
  glUseProgram(pipeline->shader1);

  glBindFramebuffer(GL_FRAMEBUFFER, pipeline->framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->target_textures[0], 0);
  glClear(GL_COLOR_BUFFER_BIT);

  // Pass the uniforms to the shader
  glUniform1i(pipeline->u_texture0_location_1, uniforms->texture0);
  glUniform1i(pipeline->u_texture1_location_1, uniforms->texture1);
  glUniform1f(pipeline->u_speed_multiplier_location_1, uniforms->speed_multiplier);
  glUniform1i(pipeline->u_wall_strategy_location_1, uniforms->wall_strategy);
  glUniform1i(pipeline->u_color_strategy_location_1, uniforms->color_strategy);
  glUniform1f(pipeline->u_random_steer_factor_location_1, uniforms->random_steer_factor);
  glUniform1f(pipeline->u_constant_steer_factor_location_1, uniforms->constant_steer_factor);
  glUniform1f(pipeline->u_search_radius_location_1, uniforms->search_radius);
  glUniform1f(pipeline->u_trail_strength_location_1, uniforms->trail_strength);
  glUniform1f(pipeline->u_vertex_radius_location_1, uniforms->vertex_radius);
  glUniform1f(pipeline->u_search_angle_location_1, uniforms->search_angle);
  glUniform1f(pipeline->u_time_location_1, uniforms->time);

  // Update points
  glBindBuffer(GL_ARRAY_BUFFER, pipeline->position_buffers[0]);
  glEnableVertexAttribArray(pipeline->a_position_location);
  glVertexAttribPointer(pipeline->a_position_location, 4, GL_FLOAT, GL_FALSE, 0, 0);

  // Save transformed output
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, pipeline->position_buffers[1]);
  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, pipeline->number_of_positions);
  glEndTransformFeedback();
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

  // Swap textures
  GLuint temp = pipeline->target_textures[0];
  pipeline->target_textures[0] = pipeline->texture0;
  pipeline->texture0 = temp;

  // Swap buffers
  temp = pipeline->position_buffers[0];
  pipeline->position_buffers[0] = pipeline->position_buffers[1];
  pipeline->position_buffers[1] = temp;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

struct shader_2_uniforms {
  GLuint texture0;
  GLuint texture1;
  float  fade_speed;
  float  blur_fraction;
  float  time;
};

function void
slimesee_set_resolution(SlimeSee *slimesee, int width, int height) {
  slimesee->width = width;
  slimesee->height = height;
  pipeline_set_resolution(&slimesee->pipeline, width, height);
}

function void
draw_shader_2(Pipeline *pipeline, shader_2_uniforms *uniforms) {
  // Draw shader2 to screen
  glUseProgram(pipeline->shader2);

  // Bind the textures passed as arguments
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pipeline->texture0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, pipeline->texture1);

  // Set the texture uniforms
  glUniform1i(pipeline->u_texture0_location_2, uniforms->texture0);
  glUniform1i(pipeline->u_texture1_location_2, uniforms->texture1);
  glUniform1f(pipeline->u_fade_speed_location_2, uniforms->fade_speed);
  glUniform1f(pipeline->u_blur_fraction_location_2, uniforms->blur_fraction);
  glUniform1f(pipeline->u_time_location_2, uniforms->time);
  //glUniform1f(pipeline->u_max_distance_location_2, uniforms->max_distance);

  glBindFramebuffer(GL_FRAMEBUFFER, pipeline->framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->target_textures[1], 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, pipeline->vertex_buffer);


  glEnableVertexAttribArray(pipeline->a_vertex_location);
  glVertexAttribPointer(pipeline->a_vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // Swap textures
  GLuint temp = pipeline->target_textures[1];
  pipeline->target_textures[1] = pipeline->texture1;
  pipeline->texture1 = temp;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Draw to the screen
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

function SlimeSee*
slimesee_init(M_Arena *arena, Preset *preset, int width, int height) {
  SlimeSee *slimesee = push_array(arena, SlimeSee, 1);
  slimesee->width = width;
  slimesee->height = height;
  slimesee->preset = *preset;
  slimesee->pipeline = *create_pipeline(arena, preset, width, height);
  return slimesee;
}

function void
slimesee_draw(SlimeSee *slimesee, float u_time) {
  shader_1_uniforms uniforms_1 = {};
  uniforms_1.time = u_time;
  uniforms_1.texture0 = 0;
  uniforms_1.texture1 = 1;
  uniforms_1.speed_multiplier = slimesee->preset.speed_multiplier;
  uniforms_1.vertex_radius = slimesee->preset.point_size;
  uniforms_1.random_steer_factor = slimesee->preset.random_steer_factor;
  uniforms_1.constant_steer_factor = slimesee->preset.constant_steer_factor;
  uniforms_1.trail_strength = slimesee->preset.trail_strength;
  uniforms_1.search_radius = slimesee->preset.search_radius;
  uniforms_1.wall_strategy = slimesee->preset.wall_strategy;
  uniforms_1.color_strategy = slimesee->preset.color_strategy;
  uniforms_1.search_angle = 0.2f;

  draw_shader_1(&slimesee->pipeline, &uniforms_1);

  shader_2_uniforms uniforms_2 = {};
  uniforms_2.texture0 = 0;
  uniforms_2.texture1 = 1;
  uniforms_2.time = u_time;
  uniforms_2.fade_speed = slimesee->preset.fade_speed;
  uniforms_2.blur_fraction = slimesee->preset.blurring;
  draw_shader_2(&slimesee->pipeline, &uniforms_2);
}

