#include "app/pipeline.h"
#include "app/preset.h"

struct image_data {
  u32 	 width;
  u32 	 height;
  u32 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  u8  	 pixel_data[3];
};

#ifdef LOGOS
#define STUDIO143_LOGO 1
#include "logos/studio143.c"
#define BRONSON_LOGO 1
#include "logos/bronson.c"
#endif

typedef u8 Logos;
enum {
#ifdef STUDIO143_LOGO
  Logo_Studio143,
#endif
#ifdef BRONSON_LOGO
  Logo_Bronson,
#endif

  Logo_COUNT,
};

function f32*
generate_initial_positions(M_Arena *arena, Preset *preset) {
    f32 *result = 0;
    f32 speed_randomness = preset->starting_speed_spread;
    f32 initial_speed = preset->average_starting_speed;
    f32 frac_pi_2 = pi_f32/2.0f;
    switch(preset->starting_arrangement) {
      case StartingArrangement_Logo: {
        Logos logo = (Logos)random_range(0, Logo_COUNT);
        
        // We can use this to increase the density of the simulated pixels
        u32 points_per_pixel = 8;

        image_data *data = 0;
        switch(logo) {
#ifdef BRONSON_LOGO
          case Logo_Bronson: {
            data = (image_data *)&bronson_logo;
          } break;
#endif
#ifdef STUDIO143_LOGO
          case Logo_Studio143: {
            data = (image_data *)&studio143_logo;
          } break;
#endif
          case Logo_COUNT: {
            points_per_pixel = 512;
            image_data img = {};
            img.width = 1;
            img.height = 1;
            img.bytes_per_pixel = 1;
            img.pixel_data[0] = 255;
            img.pixel_data[1] = 255;
            img.pixel_data[2] = 255;
            data = &img;
          } break;
        }

        // This is the maximum number of points we could have from our image
        u32 max_points = data->width * data->height * points_per_pixel;
        u32 pixels = data->width * data->height * data->bytes_per_pixel;

        result = push_array(arena, f32, max_points*4);
        u32 position_index = 0;

        f32 screen_ratio = GlobalWindowWidth/(f32)GlobalWindowHeight;
        printf("Screen ratio: %f\n", screen_ratio);

        f32 pi_times_2_over_n = pi_f32*2.0f/(f32)max_points;
        for(u32 pixel_index = 0;
            pixel_index < pixels;
            pixel_index += data->bytes_per_pixel) {
          if (data->pixel_data[pixel_index] == 0 && data->pixel_data[pixel_index + 1] == 0 && data->pixel_data[pixel_index + 2] == 0) {
            continue;
          }
          
          f32 x;
          f32 y;

          if (max_points == points_per_pixel) {
            x = 0.0f;
            y = 0.0f;
          } else {
            x = (f32)((pixel_index / data->bytes_per_pixel) % data->width) / (f32)data->width * 2.0f - 1.0f;
            y = (f32)((pixel_index / data->bytes_per_pixel) / data->width) / (f32)data->height * -2.0f + 1.0f;
          }
          
          // Scale by screen ratio
          if (screen_ratio > 1.0f) {
            x /= screen_ratio;
          } else {
            y *= screen_ratio;
          }

          for (u32 i = 0; i < points_per_pixel; i++) {
            f32 angle = (f32)position_index/4 * pi_times_2_over_n;
            f32 direction = 1.0f + (angle+frac_pi_2) /1000.f;
            f32 speed = (rand()/(f32)RAND_MAX*0.01f*speed_randomness + 0.01f * initial_speed)/1000.f; 
            speed *= 0.2f;
            result[position_index++] = x;
            result[position_index++] = y;
            result[position_index++] = speed;
            result[position_index++] = direction;
          }
        }

        preset->number_of_points = position_index/4;
      } break;
      default: {
        u64 n = preset->number_of_points;
        result = push_array(arena, f32, n*4);
        f32 pi_times_2_over_n = pi_f32*2.0f/((f32)n);
        //printf("Starting arrangement: %d\n", preset->starting_arrangement);
        for (u64 i = 0; i < n; ++i) {
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
      } break;
    }

    return result;
}

function void
pipeline_create_target_textures(Pipeline *pipeline, int width, int height) {
  GLuint old_texture0 = pipeline->texture0;
  GLuint old_texture1 = pipeline->texture1;

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

  // Do this afterwards so glGenTextures doesn't give us back the same texture
  if (old_texture0) {
    glClearTexImage(old_texture0, 0, GL_RGBA, GL_FLOAT, 0);
    glDeleteTextures(1, &old_texture0);
  }
  if (old_texture1) {
    glClearTexImage(old_texture1, 0, GL_RGBA, GL_FLOAT, 0);
    glDeleteTextures(1, &old_texture1);
  }
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
  GLuint old_texture0 = pipeline->target_textures[0];
  GLuint old_texture1 = pipeline->target_textures[1];

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

  // Do this after generating new textures so opengl doesn't possibly give us the same ones
  if (old_texture1 != 0) {
    glClearTexImage(old_texture0, 0, GL_RGBA, GL_FLOAT, 0);
    glDeleteTextures(1, &old_texture0);
    glClearTexImage(old_texture1, 0, GL_RGBA, GL_FLOAT, 0);
    glDeleteTextures(1, &old_texture1);
  }
}

function Pipeline*
create_pipeline(M_Arena *arena, Preset *preset, int width, int height) {
  Pipeline *pipeline = push_array(arena, Pipeline, 1);
  MemoryZeroStruct(pipeline);

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

