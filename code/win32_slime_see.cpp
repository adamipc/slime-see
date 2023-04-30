#include <windows.h>
#include <stdio.h>
#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

#include <stdlib.h>

#include "base/base_memory_malloc.cpp"

#define GET_PROC_ADDRESS(v,m,n) (*(PROC*)(&(v))) = GetProcAddress((m),(n))
# define power_of_two(x) (1 << (x))
#define print_str8(x) printf("%.*s\n", (int)((x).size), (x).str)

#include "gl/gl_definitions.h"
#include "win32/win32_wgl_definitions.h"

#include "gl/gl.cpp"
#include "win32/win32_wgl.cpp"

global bool running = false;

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
    case WM_SIZE: {
      // Call glViewport here
      int width = LOWORD(lParam);
      int height = HIWORD(lParam);
      glViewport(0, 0, width, height);
    } break;
    case WM_DESTROY: {
      running = false;
      PostQuitMessage(0);
      return 0;
    } break;
    case WM_CREATE: {
    } break;
    default: {
      return DefWindowProc(hWnd, message, wParam, lParam);
    } break;
  }

  return 0;
}

void APIENTRY
gl_debug_message_callback(GLenum source,
                          GLenum type,
                          GLuint id,
                          GLenum severity,
                          GLsizei length,
                          const GLchar *message,
                          const void *userParam) {
  printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
         (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
         type, severity, message);
}

typedef u8 StartingArrangement;
enum{
  StartingArrangement_Ring,
  StartingArrangement_Random,
  StartingArrangement_Origin,
  StartingArrangement_COUNT,
};

typedef u8 WallStrategy;
enum{
  WallStrategy_None = 0,
  WallStrategy_Wrap = 1,
  WallStrategy_Bounce = 2,
  WallStrategy_BounceRandom = 3,
  WallStrategy_SlowAndReverse = 4,
  WallStrategy_COUNT = 5,
};

typedef u8 ColorStrategy;
enum{
  ColorStrategy_Direction = 0,
  ColorStrategy_Speed = 1,
  ColorStrategy_Position = 2,
  ColorStrategy_Grey = 3,
  ColorStrategy_ShiftingHue = 4,
  ColorStrategy_Distance = 5,
  ColorStrategy_Oscillation = 6,
  ColorStrategy_COUNT,
};

struct Preset {
  u64                   number_of_points;
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

typedef u8 PresetNames;
enum{
  PresetName_GreenSlime,
  PresetName_CollapsingBubble,
  PresetName_SlimeRing,
  PresetName_ShiftingWeb,
  PresetName_Waves,
  PresetName_Flower,
  PresetName_ChristmasChaos,
  PresetName_Explode,
  PresetName_Tartan,
  PresetName_Globe,
};

function Preset
get_preset(PresetNames preset_name) {
  Preset result = {};

  switch (preset_name) {
    case PresetName_GreenSlime: {
      result.number_of_points = power_of_two(22);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.0f;
      result.starting_speed_spread = 0.3f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.1f;
      result.trail_strength = 0.01f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_Position;

      result.fade_speed = 0.01f;
      result.blurring = 1.0f;
    } break;
    case PresetName_CollapsingBubble: {
      result.number_of_points = power_of_two(11);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 0.5f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.5f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.5f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.005f;
      result.blurring = 1.0f;
    } break;
    case PresetName_SlimeRing: {
      result.number_of_points = power_of_two(21);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 0.1f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.4f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Grey;

      result.fade_speed = 0.05f;
      result.blurring = 1.0f;
    } break;
    case PresetName_ShiftingWeb: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 1.0f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.45f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.05f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Position;

      result.fade_speed = 0.07f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Waves: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 1.0f;
      result.starting_speed_spread = 0.0f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.04f;
      result.constant_steer_factor = 0.07f;
      result.trail_strength = 0.1f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.04f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Flower: {
      result.number_of_points = power_of_two(15);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.0f;
      result.starting_speed_spread = 0.8f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.02f;
      result.constant_steer_factor = 0.04f;
      result.trail_strength = 0.5f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.02f;
      result.blurring = 1.0f;
    } break;
    case PresetName_ChristmasChaos: {
      result.number_of_points = power_of_two(12);
      result.starting_arrangement = StartingArrangement_Random;
      result.average_starting_speed = 0.9f;
      result.starting_speed_spread = 0.0f;

      result.speed_multiplier = 1.0f;
      result.point_size = 3.0f;
      result.random_steer_factor = 0.10f;
      result.constant_steer_factor = 4.00f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.02f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Explode: {
      result.number_of_points = power_of_two(18);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.4f;
      result.starting_speed_spread = 0.3f;

      result.speed_multiplier = 1.0f;
      result.point_size = 2.0f;
      result.random_steer_factor = 0.05f;
      result.constant_steer_factor = 0.10f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_None;
      result.color_strategy = ColorStrategy_Grey;

      result.fade_speed = 0.00f;
      result.blurring = 0.0f;
    } break;
    case PresetName_Tartan: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.8f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.05f;
      result.constant_steer_factor = 0.01f;
      result.trail_strength = 0.01f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.01f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Globe: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 0.0f;
      result.starting_speed_spread = 0.3f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.005f;
      result.constant_steer_factor = 0.00f;
      result.trail_strength = 0.20f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_ShiftingHue;

      result.fade_speed = 0.117f;
      result.blurring = 1.0f;
    } break;
  }

  return result;
}

function f32*
generate_initial_positions(M_Arena *arena, Preset *preset) {
    f32 speed_randomness = preset->starting_speed_spread;
    f32 initial_speed = preset->average_starting_speed;
    u64 n = preset->number_of_points;
    f32 *result = push_array(arena, f32, n*4);
    // Ring arrangement
    for (u64 i = 0; i < n; ++i) {
      f32 pi_times_2_over_n = pi_f32*2.0f/((f32)n);
      f32 frac_pi_2 = pi_f32/2.0f;
      f32 angle = (f32)i * pi_times_2_over_n;
      f32 distance = 0.7f; // distance to center
      f32 x;
      f32 y;
      f32 direction;
      f32 speed = (rand()/(f32)RAND_MAX*0.01f*speed_randomness + 0.01 * initial_speed)/1000.f; 
      switch (preset->starting_arrangement) {
        case StartingArrangement_Ring: {
          x = sin(angle)*distance;
          y = -cos(angle)*distance;
          direction = 1.0 + (angle+frac_pi_2) /1000.f;
        } break;
        case StartingArrangement_Random: {
          // x and y between -1.0 and 1.0, direction between 0.0 and 1.0
          x = rand()/(f32)RAND_MAX*2.0f - 1.0f;
          y = rand()/(f32)RAND_MAX*2.0f - 1.0f;
          direction = rand()/(f32)RAND_MAX;
       } break;
        case StartingArrangement_Origin: {
          x = 0.0f;
          y = 0.0f;
          direction = 1.0 + (angle+frac_pi_2) /1000.f;
       } break;
      }
      // print a sample
      //if (i % 1000 == 0) {
        //printf("%f %f %f %f\n", x, y, speed, direction);
      //}
      result[i*4 + 0] = x;
      result[i*4 + 1] = y;
      result[i*4 + 2] = speed;
      result[i*4 + 3] = direction;
    }

    return result;
}

struct Pipeline {
  u64 number_of_positions;
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

function Pipeline*
create_pipeline(M_Arena *arena, Preset *preset, int width, int height) {
  Pipeline *pipeline = push_array(arena, Pipeline, 1);

  GLenum error_code = GL_NO_ERROR;

  // Initrialize a VAO
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  String8List feedback_varyings = {};
  String8 data = str8_lit("gl_Position");
  str8_list_push(arena, &feedback_varyings, data);
  String8 shader1_vs = os_file_read(arena, str8_lit("../data/shaders/shader1.vs"));
  String8 shader1_fs = os_file_read(arena, str8_lit("../data/shaders/shader1.fs"));
  //feedback_varyings = {};
  pipeline->shader1 = gl_create_shader_program(arena, shader1_vs, shader1_fs, feedback_varyings);

  GLuint transform_feedback = 0;

  pipeline->number_of_positions = preset->number_of_points;
  if (pipeline->shader1 != 0) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    // Get location of the `a_position` attribute
    pipeline->a_position_location = glGetAttribLocation(pipeline->shader1, "a_position");
    glEnableVertexAttribArray(pipeline->a_position_location);

    u64 seed =0;
    os_get_entropy(&seed, sizeof(seed));
    srand(seed);

    f32 *initial_positions = generate_initial_positions(arena, preset);
    // Make buffers for holding position data, they will alternate
    glGenBuffers(2, pipeline->position_buffers);
    glBindBuffer(GL_ARRAY_BUFFER, pipeline->position_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, pipeline->number_of_positions*4*4, initial_positions, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, pipeline->position_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, pipeline->number_of_positions*4*4, initial_positions, GL_DYNAMIC_COPY);

    // Transform feedback
    glGenTransformFeedbacks(1, &transform_feedback);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedback);

#   define X(U) pipeline->##U##_location_1 = glGetUniformLocation(pipeline->shader1, #U);
#   include "../data/shaders/shader1.uniforms"
#   undef X

  }
  else {
    // Clear GL errors
    while ((error_code = glGetError()) != GL_NO_ERROR) {
      printf("error_code: %d\n", error_code);
    }
  }

  float vertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f,
    -1.0f,  1.0f,
  };

  glGenBuffers(1, &pipeline->vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, pipeline->vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  feedback_varyings = {};
  String8 shader2_vs = os_file_read(arena, str8_lit("../data/shaders/shader2.vs"));
  String8 shader2_fs = os_file_read(arena, str8_lit("../data/shaders/shader2.fs"));
  pipeline->shader2 = gl_create_shader_program(arena, shader2_vs, shader2_fs, feedback_varyings);

  if (pipeline->shader2 != 0) {
#   define X(U) pipeline->##U##_location_2 = glGetUniformLocation(pipeline->shader2, #U);
#   include "../data/shaders/shader2.uniforms"
#   undef X

    pipeline->a_vertex_location = glGetAttribLocation(pipeline->shader2, "a_vertex");

    // Set texture units
    glUniform1i(pipeline->u_texture0_location_2, 0);
    glUniform1i(pipeline->u_texture1_location_2, 1);

  } else {
    // Clear GL errors
    while ((error_code = glGetError()) != GL_NO_ERROR) {
      printf("error_code: %d\n", error_code);
    }
  }

  // Create framebuffer to draw to
  glGenFramebuffers(1, &pipeline->framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, pipeline->framebuffer);
  printf("framebuffer: %d\n", pipeline->framebuffer);

  while ((error_code = glGetError()) != GL_NO_ERROR) {
    printf("error_code: %d\n", error_code);
  }

  // Create a target texture and attach to our framebuffer
  glGenTextures(2, pipeline->target_textures);
  printf("target_textures[0,1]: [%d,%d]\n", pipeline->target_textures[0], pipeline->target_textures[1]);
  glBindTexture(GL_TEXTURE_2D, pipeline->target_textures[0]);;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, pipeline->target_textures[1]);;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->target_textures[0], 0);

  GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    printf("framebuffer_status: %x\n", framebuffer_status);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // A texture for storing the output of shader1
  glGenTextures(1, &pipeline->texture0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pipeline->texture0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  // A texture for storing the output of shader2
  glGenTextures(1, &pipeline->texture1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, pipeline->texture1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  return pipeline;
}

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

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode) {
  BOOL bResult = AllocConsole();
  if (!bResult) {
    MessageBoxA(0, "AllocConsole failed", "Error", MB_OK);
  }
  FILE *fDummy;
  freopen_s(&fDummy, "CONOUT$", "w", stdout);
  freopen_s(&fDummy, "CONOUT$", "w", stderr);
  freopen_s(&fDummy, "CONIN$", "r", stdin);

  // Main code
  os_init();

  OS_ThreadContext tctx_memory = {};
  os_thread_init(&tctx_memory);

  M_Scratch scratch;

  String8 error = {};

  error = win32_wgl_init(Instance);

  int width = 1280;
  int height = 720;

  W32_OpenGLWindow window = {};
  if (!error) {
    window = win32_create_opengl_window(Instance, &window_proc, width, height);
  } else {
    print_str8(error);
  }

  if (window.window != 0) {
    ShowWindow(window.window, SW_SHOW);
  }

  GLenum error_code = GL_NO_ERROR;

  Preset preset = get_preset(PresetName_ShiftingWeb);

  Pipeline *pipeline = create_pipeline(scratch, &preset, width, height);
  // Common uniforms
  float u_time = 0.f;

  // Shader 1
  float u_speed_multiplier = preset.speed_multiplier;
  float u_vertex_radius = preset.point_size;
  float u_random_steer_factor = preset.random_steer_factor;
  float u_constant_steer_factor = preset.constant_steer_factor;
  float u_trail_strength = preset.trail_strength;
  float u_search_radius = preset.search_radius;
  int   u_wall_strategy = preset.wall_strategy;
  int   u_color_strategy = preset.color_strategy;
  float u_search_angle = 0.2f;

  // Shader 2
  float u_fade_speed = preset.fade_speed;
  float u_blur_fraction = preset.blurring;
  float u_max_distance = 1.0f;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Enable debugging
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_debug_message_callback, 0);
  while ((error_code = glGetError()) != GL_NO_ERROR) {
    printf("error_code: %d\n", error_code);
  }

  u64 last_time = os_now_microseconds();
  u64 frame = 0;
  running = true;
  for(;;) {
    if (!running) {
      break;
    }
    MSG msg = {};
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    HDC dc = GetDC(window.window);
    win32_wglMakeCurrent(dc, window.glrc);

    shader_1_uniforms uniforms_1 = {};
    uniforms_1.time = u_time;
    uniforms_1.texture0 = 0;
    uniforms_1.texture1 = 1;
    uniforms_1.speed_multiplier = u_speed_multiplier;
    uniforms_1.vertex_radius = u_vertex_radius;
    uniforms_1.random_steer_factor = u_random_steer_factor;
    uniforms_1.constant_steer_factor = u_constant_steer_factor;
    uniforms_1.trail_strength = u_trail_strength;
    uniforms_1.search_radius = u_search_radius;
    uniforms_1.wall_strategy = u_wall_strategy;
    uniforms_1.color_strategy = u_color_strategy;
    uniforms_1.search_angle = u_search_angle;

    glViewport(0, 0, width, height);
    draw_shader_1(pipeline, &uniforms_1);

    shader_2_uniforms uniforms_2 = {};
    uniforms_2.texture0 = 0;
    uniforms_2.texture1 = 1;
    uniforms_2.time = u_time;
    uniforms_2.fade_speed = u_fade_speed;
    uniforms_2.blur_fraction = u_blur_fraction;
    draw_shader_2(pipeline, &uniforms_2);

    error_code = glGetError();
    if (error_code != GL_NO_ERROR) {
      printf("ERROR::OPENGL::%d\n", error_code);
      break;
    } else 
    {
      //printf("no error\n");
    }

    u_time += 0.01f;

    SwapBuffers(dc);
    ReleaseDC(window.window, dc);

    u64 end_time = os_now_microseconds();

    u64 elapsed_time = end_time - last_time;

    f32 frame_time_ms = elapsed_time / 1000.0f;
    f32 fps = 1000.0f / frame_time_ms;

    if ((frame % 61) == 0) {
      printf("frame time: %f ms, fps: %f\n", frame_time_ms, fps);
    }

    last_time = end_time;
    frame++;
  }

dbl_break:
  ;

  if (error.size != 0) {
    fprintf(stdout, "%.*s\n", str8_expand(error));
  } else {
    fprintf(stdout, "success\n");
  }

  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
