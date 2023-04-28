#include <windows.h>
#include <stdio.h>
#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

#include <stdlib.h>

#include "base/base_memory_malloc.cpp"

#define GET_PROC_ADDRESS(v,m,n) (*(PROC*)(&(v))) = GetProcAddress((m),(n))

#define print_str8(x) printf("%.*s\n", (int)((x).size), (x).str)

#include "gl/gl_definitions.h"
#include "win32/win32_wgl_definitions.h"

#include "gl/gl.cpp"
#include "win32/win32_wgl.cpp"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
    case WM_SIZE: {
      // Call glViewport here
      int width = LOWORD(lParam);
      int height = HIWORD(lParam);
      glViewport(0, 0, width, height);
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

function f32*
generate_initial_positions(M_Arena *arena, StartingArrangement starting_arrangement, u64 n) {
    f32 speed_randomness = 0.1f;
    f32 initial_speed = 0.1f;
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
      switch (starting_arrangement) {
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
  ColorStrategy_Direction,
  ColorStrategy_Speed,
  ColorStrategy_Position,
  ColorStrategy_Grey,
  ColorStrategy_ShiftingPosition,
  ColorStrategy_Distance,
  ColorStrategy_Oscillation,
  ColorStrategy_COUNT,
};

struct shader_preset {
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

  W32_OpenGLWindow window = {};
  if (!error) {
    window = win32_create_opengl_window(Instance, &WndProc);
  }

  GLenum error_code = GL_NO_ERROR;

  // Initrialize a VAO
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  String8List feedback_varyings = {};
  GLuint shader1 = 0;
  if (window.window != 0) {
    ShowWindow(window.window, SW_SHOW);

    String8 data = str8_lit("gl_Position");
    str8_list_push(scratch, &feedback_varyings, data);
    String8 shader1_vs = os_file_read(scratch, str8_lit("../data/shaders/shader1.vs"));
    String8 shader1_fs = os_file_read(scratch, str8_lit("../data/shaders/shader1.fs"));
    //feedback_varyings = {};
    shader1 = gl_create_shader_program(scratch, shader1_vs, shader1_fs, feedback_varyings);
  }

  GLuint transform_feedback = 0;

# define X(U) GLint U##_location_1 = 0;
# include "../data/shaders/shader1.uniforms"
# undef X


# define power_of_two(x) (1 << (x))
  shader_preset Preset = {};
  Preset.number_of_points = power_of_two(10);
  Preset.starting_arrangement = StartingArrangement_Ring;
  Preset.average_starting_speed = 1.0f;
  Preset.starting_speed_spread = 0.1f;
  
  Preset.speed_multiplier = 1.0f;
  Preset.point_size = 1.0f;
  Preset.random_steer_factor = 0.10f;
  Preset.constant_steer_factor = 0.45f;
  Preset.trail_strength = 0.20f;
  Preset.search_radius = 0.05f;
  Preset.wall_strategy = WallStrategy_Wrap;
  Preset.color_strategy = ColorStrategy_Position;

  Preset.fade_speed = 0.07f;
  Preset.blurring = 1.0f;

  GLuint position_buffers[2];
  u64 number_of_positions = Preset.number_of_points;
  GLint a_position_location = 0;
  if (shader1 != 0) {
    //glEnable(GL_PROGRAM_POINT_SIZE);
    
    // Get location of the `a_position` attribute
    a_position_location = glGetAttribLocation(shader1, "a_position");
    glEnableVertexAttribArray(a_position_location);

    u64 seed =0;
    os_get_entropy(&seed, sizeof(seed));
    srand(seed);

    f32 *initial_positions = generate_initial_positions(scratch, Preset.starting_arrangement, number_of_positions);
    // Make buffers for holding position data, they will alternate
    glGenBuffers(2, position_buffers);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, number_of_positions*4*4, initial_positions, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, number_of_positions*4*4, initial_positions, GL_DYNAMIC_COPY);

    // Transform feedback
    glCreateTransformFeedbacks(1, &transform_feedback);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedback);

#   define X(U) U##_location_1 = glGetUniformLocation(shader1, #U);
#   include "../data/shaders/shader1.uniforms"
#   undef X

  }
  else {
    // Clear GL errors
    while ((error_code = glGetError()) != GL_NO_ERROR) {
      printf("error_code: %d\n", error_code);
    }
  }

# define X(U) GLint U##_location_2 = 0;
# include "../data/shaders/shader2.uniforms"
# undef X

  float vertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f,
    -1.0f,  1.0f,
  };

  GLuint vertex_buffer = 0;
  GLint a_vertex_location = 0;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  feedback_varyings = {};
  String8 shader2_vs = os_file_read(scratch, str8_lit("../data/shaders/shader2.vs"));
  String8 shader2_fs = os_file_read(scratch, str8_lit("../data/shaders/shader2.fs"));
  GLuint shader2 = gl_create_shader_program(scratch, shader2_vs, shader2_fs, feedback_varyings);

  if (shader2 != 0) {
#   define X(U) U##_location_2 = glGetUniformLocation(shader2, #U);
#   include "../data/shaders/shader2.uniforms"
#   undef X

    a_vertex_location = glGetAttribLocation(shader2, "a_vertex");

    // Set texture units
    glUniform1i(u_texture0_location_2, 0);
    glUniform1i(u_texture1_location_2, 1);

  } else {
    // Clear GL errors
    while ((error_code = glGetError()) != GL_NO_ERROR) {
      printf("error_code: %d\n", error_code);
    }
  }

  // Create framebuffer to draw to
  GLuint framebuffer = 0;
  glCreateFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  printf("framebuffer: %d\n", framebuffer);

  int width = 1280;
  int height = 720;
  // An array to store data

  while ((error_code = glGetError()) != GL_NO_ERROR) {
    printf("error_code: %d\n", error_code);
  }
  // Create a target texture and attach to our framebuffer
  GLuint target_texture[2];
  glGenTextures(2, target_texture);
  printf("target_texture[0,1]: [%d,%d]\n", target_texture[0], target_texture[1]);
  glBindTexture(GL_TEXTURE_2D, target_texture[0]);;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, target_texture[1]);;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_texture[0], 0);

  GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    printf("framebuffer_status: %x\n", framebuffer_status);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // A texture for storing the output of shader1
  GLuint texture1 = 0;
  glGenTextures(1, &texture1);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  // A texture for storing the output of shader2
  GLuint texture2 = 0;
  glGenTextures(1, &texture2);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  // Common uniforms
  float u_time = 0.f;

  // Shader 1
  float u_speed_multiplier = Preset.speed_multiplier;
  float u_vertex_radius = Preset.point_size;
  float u_random_steer_factor = Preset.random_steer_factor;
  float u_constant_steer_factor = Preset.constant_steer_factor;
  float u_trail_strength = Preset.trail_strength;
  float u_search_radius = Preset.search_radius;
  int   u_wall_strategy = Preset.wall_strategy;
  int   u_color_strategy = Preset.color_strategy;
  float u_search_angle = 0.2f;

  // Shader 2
  float u_fade_speed = Preset.fade_speed;
  float u_blur_fraction = Preset.blurring;
  float u_max_distance = 1.0f;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Enable debugging
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_debug_message_callback, 0);
  while ((error_code = glGetError()) != GL_NO_ERROR) {
    printf("error_code: %d\n", error_code);
  }

  for(;;) {
    MSG msg = {};
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    HDC dc = GetDC(window.window);
    win32_wglMakeCurrent(dc, window.glrc);

    // Draw shader 1 to the framebuffer
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // Select shader 1 program
    glUseProgram(shader1);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_texture[0], 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);
    // Clear the canvas

    // Pass the uniforms to the shader
    glUniform1i(u_texture0_location_1, 0);
    glUniform1i(u_texture1_location_1, 1);
    glUniform1f(u_speed_multiplier_location_1, u_speed_multiplier);
    glUniform1i(u_wall_strategy_location_1, u_wall_strategy);
    glUniform1i(u_color_strategy_location_1, u_color_strategy);
    glUniform1f(u_random_steer_factor_location_1, u_random_steer_factor);
    glUniform1f(u_constant_steer_factor_location_1, u_constant_steer_factor);
    glUniform1f(u_search_radius_location_1, u_search_radius);
    glUniform1f(u_trail_strength_location_1, u_trail_strength);
    glUniform1f(u_vertex_radius_location_1, u_vertex_radius);
    glUniform1f(u_search_angle_location_1, u_search_angle);
    glUniform1f(u_time_location_1, u_time);

    // Update points
    glBindBuffer(GL_ARRAY_BUFFER, position_buffers[0]);
    glEnableVertexAttribArray(a_position_location);
    glVertexAttribPointer(a_position_location, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Save transformed output
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, position_buffers[1]);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, number_of_positions);
    glEndTransformFeedback();
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    // Draw to screen
    //glDrawArrays(GL_POINTS, 0, number_of_positions);

    // Swap textures
    GLuint temp = target_texture[0];
    target_texture[0] = texture1;
    texture1 = temp;

    // Swap buffers
    temp = position_buffers[0];
    position_buffers[0] = position_buffers[1];
    position_buffers[1] = temp;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw shader2 to screen
    glUseProgram(shader2);

    // Bind the textures passed as arguments
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // Set the texture uniforms
    glUniform1i(u_texture0_location_2, 0);
    glUniform1i(u_texture1_location_2, 1);
    glUniform1f(u_fade_speed_location_2, u_fade_speed);
    glUniform1f(u_blur_fraction_location_2, u_blur_fraction);
    glUniform1f(u_time_location_2, u_time);
    //glUniform1f(u_max_distance_location_2, u_max_distance);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_texture[1], 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);


    glEnableVertexAttribArray(a_vertex_location);
    glVertexAttribPointer(a_vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Swap textures
    temp = target_texture[1];
    target_texture[1] = texture2;
    texture2 = temp;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Draw to the screen
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glFlush();

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
  }

  if (error.size != 0) {
    fprintf(stdout, "%.*s\n", str8_expand(error));
  } else {
    fprintf(stdout, "success\n");
  }

  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
