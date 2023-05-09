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
  float transition_time;
  int   color_strategy_old;
  float color_swap;
};

struct screenshot_data {
  String8 filename;
  int width;
  int height;
  BYTE *pixels;
  HANDLE thread;
  M_Arena *conflict;
};

function SlimeSeeState* 
slimesee_dump_state(M_Arena *arena, SlimeSee *slimesee) {
  // Copy the state and return it
  SlimeSeeState *results = push_struct(arena, SlimeSeeState);

  MemoryCopyStruct(results, slimesee->state);

  return results;
}

function void slimesee_set_beat_transition_ms(SlimeSee *slimesee, f32 beat_transition_ms) {
  slimesee->state->beat_transition_ms = beat_transition_ms;
}

function void slimesee_set_blend_value(SlimeSee *slimesee, f32 blend_value) {
  slimesee->state->blend_value = blend_value;
}
function void slimesee_set_color_swap(SlimeSee *slimesee, f32 color_swap) {
  slimesee->state->color_swap = color_swap;
}

function void slimeseestate_write_to_file(SlimeSeeState *state, String8 filename) {
  M_Scratch scratch;
  String8List list = {};

  // Write the size of the state structure as header for versioning
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", sizeof(SlimeSeeState)));

  // Primary Preset
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->primary_preset.number_of_points));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->primary_preset.starting_arrangement));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.average_starting_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.starting_speed_spread));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.speed_multiplier));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.point_size));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.random_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.constant_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.trail_strength));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.search_radius));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->primary_preset.wall_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->primary_preset.color_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.fade_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->primary_preset.blurring));

  // Secondary Preset
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->secondary_preset.number_of_points));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->secondary_preset.starting_arrangement));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.average_starting_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.starting_speed_spread));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.speed_multiplier));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.point_size));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.random_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.constant_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.trail_strength));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.search_radius));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->secondary_preset.wall_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->secondary_preset.color_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.fade_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->secondary_preset.blurring));

  // Beat Preset
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->beat_preset.number_of_points));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->beat_preset.starting_arrangement));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.average_starting_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.starting_speed_spread));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.speed_multiplier));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.point_size));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.random_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.constant_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.trail_strength));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.search_radius));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->beat_preset.wall_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->beat_preset.color_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.fade_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_preset.blurring));

  // Old Preset
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->old_preset.number_of_points));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->old_preset.starting_arrangement));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.average_starting_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.starting_speed_spread));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.speed_multiplier));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.point_size));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.random_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.constant_steer_factor));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.trail_strength));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.search_radius));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->old_preset.wall_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->old_preset.color_strategy));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.fade_speed));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->old_preset.blurring));

  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->blend_value));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_transition_ms));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%d\n", state->beat_transition));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->transition_start));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->transition_length_ms));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->beat_transition_ratio));
  str8_list_push(scratch, &list, str8_pushf(scratch, "%f\n", state->color_swap));

  b32 result = os_file_write(filename, list);

  if (!result) {
    printf("Failed to write to file %.*s\n", str8_expand(filename));
  }
}

function u32 str8_to_u32(String8 str) {
  // NOTE(adam): use atoi for now
  return atoi((char*)str.str);
}

function f32 str8_to_f32(String8 str) {
  // NOTE(adam): use atof for now
  return (f32)atof((char*)str.str);
}

function SlimeSeeState* slimeseestate_read_from_file(M_Arena *arena, String8 filename) {
  M_Scratch scratch(arena);
  M_Temp restore_point = m_begin_temp(arena);
  SlimeSeeState *results = push_struct(arena, SlimeSeeState);

  String8 file_contents = os_file_read(scratch, filename);

  if (file_contents.size > 0) {
    String8List lines = str8_split(scratch, file_contents, (u8*)"\n", 1);

    String8Node *node = lines.first;

    // Check that our "header" matches the size of SlimeSeeState
    if (str8_match(node->string, str8_pushf(scratch, "%d", sizeof(SlimeSeeState)), 0)) {
      node = node->next;
      printf("File %.*s is a valid SlimeSeeState file\n", str8_expand(filename));

      /*
struct Preset {
  u32                   number_of_points;
  StartingArrangement   starting_arrangement;
  float                 average_starting_speed;
  float                 starting_speed_spread;

  float                 speed_multiplier;
  float                 point_size;
  float                 random_steer_factor;
  float                 constant_steer_factor;
  float                 trail_strength;
  float                 search_radius;
  WallStrategy          wall_strategy;
  ColorStrategy         color_strategy;

  float                 fade_speed;
  float                 blurring;
};
*/
      // Read Primary Preset
      results->primary_preset.number_of_points = str8_to_u32(node->string); node = node->next;
      results->primary_preset.starting_arrangement = (StartingArrangement)str8_to_u32(node->string); node = node->next;
      results->primary_preset.average_starting_speed = str8_to_f32(node->string); node = node->next;
      results->primary_preset.starting_speed_spread = str8_to_f32(node->string); node = node->next;
      results->primary_preset.speed_multiplier = str8_to_f32(node->string); node = node->next;
      results->primary_preset.point_size = str8_to_f32(node->string); node = node->next;
      results->primary_preset.random_steer_factor = str8_to_f32(node->string); node = node->next;
      results->primary_preset.constant_steer_factor = str8_to_f32(node->string); node = node->next;
      results->primary_preset.trail_strength = str8_to_f32(node->string); node = node->next;
      results->primary_preset.search_radius = str8_to_f32(node->string); node = node->next;
      results->primary_preset.wall_strategy = (WallStrategy)str8_to_u32(node->string); node = node->next;
      results->primary_preset.color_strategy = (ColorStrategy)str8_to_u32(node->string); node = node->next;
      results->primary_preset.fade_speed = str8_to_f32(node->string); node = node->next;
      results->primary_preset.blurring = str8_to_f32(node->string); node = node->next;

      // Read Secondary Preset
      results->secondary_preset.number_of_points = str8_to_u32(node->string); node = node->next;
      results->secondary_preset.starting_arrangement = (StartingArrangement)str8_to_u32(node->string); node = node->next;
      results->secondary_preset.average_starting_speed = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.starting_speed_spread = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.speed_multiplier = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.point_size = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.random_steer_factor = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.constant_steer_factor = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.trail_strength = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.search_radius = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.wall_strategy = (WallStrategy)str8_to_u32(node->string); node = node->next;
      results->secondary_preset.color_strategy = (ColorStrategy)str8_to_u32(node->string); node = node->next;
      results->secondary_preset.fade_speed = str8_to_f32(node->string); node = node->next;
      results->secondary_preset.blurring = str8_to_f32(node->string); node = node->next;

      // Read Beat Preset
      results->beat_preset.number_of_points = str8_to_u32(node->string); node = node->next;
      results->beat_preset.starting_arrangement = (StartingArrangement)str8_to_u32(node->string); node = node->next;
      results->beat_preset.average_starting_speed = str8_to_f32(node->string); node = node->next;
      results->beat_preset.starting_speed_spread = str8_to_f32(node->string); node = node->next;
      results->beat_preset.speed_multiplier = str8_to_f32(node->string); node = node->next;
      results->beat_preset.point_size = str8_to_f32(node->string); node = node->next;
      results->beat_preset.random_steer_factor = str8_to_f32(node->string); node = node->next;
      results->beat_preset.constant_steer_factor = str8_to_f32(node->string); node = node->next;
      results->beat_preset.trail_strength = str8_to_f32(node->string); node = node->next;
      results->beat_preset.search_radius = str8_to_f32(node->string); node = node->next;
      results->beat_preset.wall_strategy = (WallStrategy)str8_to_u32(node->string); node = node->next;
      results->beat_preset.color_strategy = (ColorStrategy)str8_to_u32(node->string); node = node->next;
      results->beat_preset.fade_speed = str8_to_f32(node->string); node = node->next;
      results->beat_preset.blurring = str8_to_f32(node->string); node = node->next;

      // Read Old Preset
      results->old_preset.number_of_points = str8_to_u32(node->string); node = node->next;
      results->old_preset.starting_arrangement = (StartingArrangement)str8_to_u32(node->string); node = node->next;
      results->old_preset.average_starting_speed = str8_to_f32(node->string); node = node->next;
      results->old_preset.starting_speed_spread = str8_to_f32(node->string); node = node->next;
      results->old_preset.speed_multiplier = str8_to_f32(node->string); node = node->next;
      results->old_preset.point_size = str8_to_f32(node->string); node = node->next;
      results->old_preset.random_steer_factor = str8_to_f32(node->string); node = node->next;
      results->old_preset.constant_steer_factor = str8_to_f32(node->string); node = node->next;
      results->old_preset.trail_strength = str8_to_f32(node->string); node = node->next;
      results->old_preset.search_radius = str8_to_f32(node->string); node = node->next;
      results->old_preset.wall_strategy = (WallStrategy)str8_to_u32(node->string); node = node->next;
      results->old_preset.color_strategy = (ColorStrategy)str8_to_u32(node->string); node = node->next;
      results->old_preset.fade_speed = str8_to_f32(node->string); node = node->next;
      results->old_preset.blurring = str8_to_f32(node->string); node = node->next;

      // Read other state values
      results->blend_value = str8_to_f32(node->string); node = node->next;
      results->beat_transition_ms = str8_to_f32(node->string); node = node->next;
      results->beat_transition = str8_to_u32(node->string); node = node->next;
      results->transition_start = str8_to_f32(node->string); node = node->next;
      results->transition_length_ms = str8_to_f32(node->string); node = node->next;
      results->beat_transition_ratio = str8_to_f32(node->string); node = node->next;
      results->color_swap = str8_to_f32(node->string); node = node->next;
    } else {
      printf("File %.*s is not a valid SlimeSeeState file\n", str8_expand(filename));
      results = 0;
    }
  } else {
    results = 0;
    m_end_temp(restore_point);
  }

  return results;
}

function void slimesee_set_preset(SlimeSee *slimesee, PresetSlot slot, Preset new_preset) {
  switch(slot) {
    case PresetSlot_Primary: {
      slimesee->state->primary_preset = new_preset;
    } break;
    case PresetSlot_Secondary: {
      slimesee->state->secondary_preset = new_preset;
    } break;
    case PresetSlot_Beat: {
      slimesee->state->beat_preset = new_preset;
    } break;
  }
}

function void 
slimesee_load_state(SlimeSee *slimesee, SlimeSeeState *state) {
  Assert(state != 0);
  slimesee->state = state;
  slimesee_clear_textures(slimesee);
  slimesee_reset_points(slimesee);
}

function void
slimesee_transition_preset(SlimeSee *slimesee, Preset new_preset, f32 intensity) {
  // TODO(adam): Fix this so it sets old_preset to the same logic as used when drawing
  slimesee->state->old_preset = lerp_preset(slimesee->state->primary_preset, slimesee->state->secondary_preset, slimesee->state->blend_value);
  slimesee->state->primary_preset = new_preset;
  slimesee->state->transition_start = slimesee->u_time_ms;
  // at intensity 1.0 we should transition immediately, 
  // at 0.0 we should take full transition time of 1 second
  slimesee->state->transition_length_ms = lerp_f32(1000.0f, intensity, 0.0f);
  // above 0.5 intensity we should reset the textures
  if (intensity > 0.5f) {
    slimesee_clear_textures(slimesee);
    slimesee->state->blend_value = rand()/((float)RAND_MAX);
  }

  // at intensity of 1.0 we should regenerate the points immediately
  if (intensity == 1.0f) {
    slimesee_reset_points(slimesee);
    slimesee->state->blend_value = 0.f;
    slimesee->state->color_swap = rand()/((float)RAND_MAX);
  }
}

function void 
slimesee_set_beat_transition_ratio(SlimeSee *slimesee, f32 ratio) {
  slimesee->state->beat_transition_ratio = ratio;
}

function void 
slimesee_beat_transition(SlimeSee *slimesee, f32 beat_intensity) {
  //printf("beat transition start: %0.2f\n", slimesee->u_time_ms);
  // save transition state to restore after beat transition
  if (!slimesee->state->beat_transition) {
    slimesee->stored_transition_start = slimesee->state->transition_start;
    slimesee->stored_transition_length_ms = slimesee->state->transition_length_ms;
  }

  // TODO(adam): Use the intensity
  slimesee->state->transition_start = slimesee->u_time_ms;
  slimesee->state->transition_length_ms = slimesee->state->beat_transition_ms;
  slimesee->beat_intensity = beat_intensity;
  slimesee->state->beat_transition = true;
  if (slimesee->reset_points_on_beat) {
    slimesee_reset_points(slimesee);
    slimesee->reset_points_on_beat = false;
  }
  if (slimesee->clear_textures_on_beat) {
    slimesee_clear_textures(slimesee);
    slimesee->clear_textures_on_beat = false;
  }
}

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
slimesee_reset_points(SlimeSee *slimesee) {
  // TODO(adam): make this use the transition preset?
  pipeline_generate_initial_positions(&slimesee->pipeline, &slimesee->state->primary_preset);
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
  glUniform1f(pipeline->u_transition_time_location_1, uniforms->transition_time);
  glUniform1f(pipeline->u_color_swap_location_1, uniforms->color_swap);
  glUniform1i(pipeline->u_color_strategy_old_location_1, uniforms->color_strategy_old);


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
  glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
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
  //glClear(GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisable(GL_BLEND);
}

function SlimeSee*
slimesee_init(M_Arena *arena, int width, int height) {
  SlimeSee *slimesee = push_struct(arena, SlimeSee);
  slimesee->width = width;
  slimesee->height = height;
  slimesee->state = push_struct(arena, SlimeSeeState);
  slimesee->state->primary_preset = randomize_preset();
  slimesee->state->secondary_preset = randomize_preset();
  slimesee->state->beat_preset = randomize_preset();
  slimesee->state->old_preset = slimesee->state->primary_preset;
  slimesee->state->transition_start = 0.0f;
  slimesee->state->transition_length_ms = 0.0f;
  slimesee->state->beat_transition_ms = 37.5f; // sixteenth beat at 120bpm
  slimesee->state->beat_transition = false;
  slimesee->state->beat_transition_ratio = 0.0f;
  slimesee->u_time_ms = 0.f;
  slimesee->state->color_swap = 0.f;
  slimesee->pipeline = *create_pipeline(arena, &slimesee->state->primary_preset, width, height);
  return slimesee;
}

function void 
slimesee_update_time(SlimeSee* slimesee, u64 elapsed_time_microseconds, b32 automate_presets) {
  slimesee->last_time_ms = slimesee->u_time_ms;
  slimesee->u_time_ms += (f32)elapsed_time_microseconds / 10000.0f;

  if (automate_presets) {
    if (slimesee->u_time_ms > slimesee->state->transition_start + slimesee->state->transition_length_ms) {
      slimesee->state->old_preset = lerp_preset(slimesee->state->primary_preset, slimesee->state->secondary_preset, slimesee->state->blend_value);
      slimesee->state->primary_preset = slimesee->state->secondary_preset;
      slimesee->state->secondary_preset = randomize_preset();
      //slimesee->state->beat_preset = randomize_preset();
      slimesee->state->transition_start = slimesee->u_time_ms;
      slimesee->state->transition_length_ms = (rand()/((f32)RAND_MAX) * 20000.0f) + 2000.0f;
      slimesee->state->color_swap = rand()/((f32)RAND_MAX);
      if (rand()/((f32)RAND_MAX) < 0.05f) {
        slimesee->reset_points_on_beat = true;
      }
      if (rand()/((f32)RAND_MAX) < 0.05f) {
        slimesee->state->beat_preset = randomize_preset();
      }
      if (rand()/((f32)RAND_MAX) < 0.8f) {
        slimesee->clear_textures_on_beat = true;
      }
    }
  }
}

function void
slimesee_draw(SlimeSee *slimesee) {
  f32 transition_now = abs_f32(slimesee->u_time_ms - slimesee->state->transition_start);
  b32 transition_in_progress = transition_now < slimesee->state->transition_length_ms;

  b32 beat_transition = slimesee->state->beat_transition;

  // Update these before setting draw_preset
  // by default Blend between primary and secondary preset
  Preset transition_source = slimesee->state->primary_preset;
  Preset transition_destination = slimesee->state->secondary_preset;
  f32 transition_progress = slimesee->state->blend_value;

  Preset draw_preset = lerp_preset(transition_source, transition_destination, transition_progress);

  if (transition_in_progress) {
    transition_progress = transition_now / slimesee->state->transition_length_ms;
    if (beat_transition) {
      // NOTE(adam): slimesee->state->beat_transition_ratio is a value from 0.0 to 1.0
      // we will use it to decide how much of the beat transition should be from
      // the draw_preset to the beat preset and how much should be from the beat_preset
      // back to the draw_preset
      // 0.0 means the transition starts at the beat preset and spends the entire transition_length_ms
      // going back to the beat preset
      // 1.0 means it spends the entire transition_length_ms going from the draw_preset to the beat preset
      // 0.5 means it spends half the transition_length_ms going from the draw_preset to the beat preset
      // and the other half going from the beat preset back to the draw_preset

      if (transition_progress < slimesee->state->beat_transition_ratio) {
        transition_source = draw_preset;
        transition_destination = slimesee->state->beat_preset;
        transition_progress = transition_progress / slimesee->state->beat_transition_ratio * slimesee->beat_intensity;
      } else {
        transition_progress = (transition_progress - slimesee->state->beat_transition_ratio) / (1.0f - slimesee->state->beat_transition_ratio);
        transition_progress *= slimesee->beat_intensity;

        transition_source = slimesee->state->beat_preset;
        transition_destination = draw_preset;
      }
    } else {
      transition_source = slimesee->state->old_preset;
      transition_destination = draw_preset;
    }
  } else {
    if (beat_transition) {
      slimesee->state->transition_start = slimesee->stored_transition_start;
      slimesee->state->transition_length_ms = slimesee->stored_transition_length_ms;
      slimesee->state->beat_transition = false;
    }
  }

  draw_preset = lerp_preset(transition_source, transition_destination, transition_progress);


  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  f32 u_time = slimesee->u_time_ms - slimesee->last_time_ms;

  shader_1_uniforms uniforms_1 = {};
  uniforms_1.time = u_time;
  uniforms_1.texture0 = 0;
  uniforms_1.texture1 = 1;
  uniforms_1.speed_multiplier = draw_preset.speed_multiplier;
  uniforms_1.vertex_radius = draw_preset.point_size;
  uniforms_1.random_steer_factor = draw_preset.random_steer_factor;
  uniforms_1.constant_steer_factor = draw_preset.constant_steer_factor;
  uniforms_1.trail_strength = draw_preset.trail_strength;
  uniforms_1.search_radius = draw_preset.search_radius;
  uniforms_1.wall_strategy = draw_preset.wall_strategy;
  uniforms_1.search_angle = 0.2f;
  
  // for color transitions
  uniforms_1.color_strategy = transition_destination.color_strategy;
  uniforms_1.color_strategy_old = transition_source.color_strategy;
  uniforms_1.transition_time = transition_progress;
  uniforms_1.color_swap = slimesee->state->color_swap;

  draw_shader_1(&slimesee->pipeline, &uniforms_1);

  shader_2_uniforms uniforms_2 = {};
  uniforms_2.texture0 = 0;
  uniforms_2.texture1 = 1;
  uniforms_2.time = slimesee->u_time_ms;
  uniforms_2.fade_speed = draw_preset.fade_speed;
  uniforms_2.blur_fraction = draw_preset.blurring;
  draw_shader_2(&slimesee->pipeline, &uniforms_2);
}

