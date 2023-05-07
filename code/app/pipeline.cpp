#include "app/pipeline.h"
#include "app/preset.h"

function f32*
generate_initial_positions(M_Arena *arena, Preset *preset) {
    f32 speed_randomness = preset->starting_speed_spread;
    f32 initial_speed = preset->average_starting_speed;
    u64 n = preset->number_of_points;
    f32 *result = push_array(arena, f32, n*4);
    printf("Starting arrangement: %d\n", preset->starting_arrangement);
    for (u64 i = 0; i < n; ++i) {
      f32 pi_times_2_over_n = pi_f32*2.0f/((f32)n);
      f32 frac_pi_2 = pi_f32/2.0f;
      f32 angle = (f32)i * pi_times_2_over_n;
      f32 distance = 0.7f; // distance to center
      f32 x = 0.f;
      f32 y = 0.f;
      f32 direction = 1.f;
      f32 speed = (rand()/(f32)RAND_MAX*0.01f*speed_randomness + 0.01f * initial_speed)/1000.f; 
      switch (preset->starting_arrangement) {
        case StartingArrangement_Ring: {
          x = (f32)sin(angle)*distance;
          y = -(f32)cos(angle)*distance;
          direction = 1.0f + (angle+frac_pi_2) /1000.f;
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
           direction = 1.0f + (angle+frac_pi_2) /1000.f;
        } break;
        default: {
          printf("Unknown starting arrangement: %d\n", preset->starting_arrangement);
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

function void
pipeline_create_target_textures(Pipeline *pipeline, int width, int height) {
  if (pipeline->texture0) {
    glDeleteTextures(1, &pipeline->texture0);
  }
  if (pipeline->texture1) {
    glDeleteTextures(1, &pipeline->texture1);
  }

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
}

function void
pipeline_generate_initial_positions(Pipeline *pipeline, Preset *preset) {
  M_Scratch scratch;
  f32 *initial_positions = generate_initial_positions(scratch, preset);
  if (pipeline->position_buffers[1] != 0) {
    glDeleteBuffers(2, pipeline->position_buffers);
  }

  pipeline->number_of_positions = preset->number_of_points;
  // Make buffers for holding position data, they will alternate
  glGenBuffers(2, pipeline->position_buffers);
  glBindBuffer(GL_ARRAY_BUFFER, pipeline->position_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, pipeline->number_of_positions*4*4, initial_positions, GL_DYNAMIC_COPY);
  glBindBuffer(GL_ARRAY_BUFFER, pipeline->position_buffers[1]);
  glBufferData(GL_ARRAY_BUFFER, pipeline->number_of_positions*4*4, initial_positions, GL_DYNAMIC_COPY);
}

function void
pipeline_set_resolution(Pipeline *pipeline, int width, int height) {
  pipeline_create_target_textures(pipeline, width, height);
  pipeline_reset_target_textures(pipeline, width, height);
}

function void
pipeline_reset_target_textures(Pipeline *pipeline, int width, int height) {
  if (pipeline->target_textures[1] != 0) {
    glDeleteTextures(2, pipeline->target_textures);
  }
  // Create a target texture and attach to our framebuffer
  glGenTextures(2, pipeline->target_textures);
  printf("target_textures[0,1]: [%d,%d]\n", pipeline->target_textures[0], pipeline->target_textures[1]);
  glBindTexture(GL_TEXTURE_2D, pipeline->target_textures[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, pipeline->target_textures[1]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

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

  if (pipeline->shader1 != 0) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    // Get location of the `a_position` attribute
    pipeline->a_position_location = glGetAttribLocation(pipeline->shader1, "a_position");
    glEnableVertexAttribArray(pipeline->a_position_location);

    pipeline_generate_initial_positions(pipeline, preset);

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

  pipeline_reset_target_textures(pipeline, width, height);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->target_textures[0], 0);

  GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    printf("framebuffer_status: %x\n", framebuffer_status);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  pipeline_create_target_textures(pipeline, width, height);

  return pipeline;
}

