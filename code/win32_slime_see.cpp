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

  if (shader1 != 0) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    // Get location of the `a_position` attribute
    GLint a_position_location = glGetAttribLocation(shader1, "a_position");
    glEnableVertexAttribArray(a_position_location);

    u64 seed =0;
    os_get_entropy(&seed, sizeof(seed));
    srand(seed);

    f32 speed_randomness = 0.1f;
    f32 initial_speed = 1.0f;
    u64 number_of_positions = 1000;
    f32 *initial_positions = push_array(scratch, f32, number_of_positions*4);
    // Ring arrangement
    for (u64 i = 0; i < number_of_positions; ++i) {
        f32 angle = (f32)i*pi_f32*2.0f/((f32)number_of_positions);
        f32 distance = 0.7f; // distance to center
        f32 x = sin(angle)*distance;
        f32 y = -cos(angle)*distance;
        f32 direction = 1 + (angle+pi_f32/2.f)/1000.f;
        f32 speed = (rand()/(f32)RAND_MAX*0.1f*speed_randomness + 0.01 * initial_speed)/1000.f; 
        //printf("%f %f %f %f\n", x, y, direction, speed);
        initial_positions[i*4 + 0] = x;
        initial_positions[i*4 + 1] = y;
        initial_positions[i*4 + 2] = direction;
        initial_positions[i*4 + 3] = speed;
    }

    // Make buffers for holding position data, they will alternate
    GLuint position_buffers[2];
    glGenBuffers(2, position_buffers);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_positions), initial_positions, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_positions), initial_positions, GL_DYNAMIC_COPY);

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
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  feedback_varyings = {};
  String8 shader2_vs = os_file_read(scratch, str8_lit("../data/shaders/shader2.vs"));
  String8 shader2_fs = os_file_read(scratch, str8_lit("../data/shaders/shader2.fs"));
  GLuint shader2 = gl_create_shader_program(scratch, shader2_vs, shader2_fs, feedback_varyings);

  GLuint texture = 0;
  if (shader2 != 0) {
#   define X(U) U##_location_2 = glGetUniformLocation(shader2, #U);
#   include "../data/shaders/shader2.uniforms"
#   undef X

    // Set texture units
    glUniform1i(u_texture0_location_2, 0);
    glUniform1i(u_texture1_location_2, 1);

    glGenTextures(1, &texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR

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

  u8 width = 1280;
  u8 height = 720;
  // An array to store data
  u8 *pixels = push_array(scratch, u8, width * height * 4);

  while ((error_code = glGetError()) != GL_NO_ERROR) {
    printf("error_code: %d\n", error_code);
  }
  // Create a target texture and attach to our framebuffer
  GLuint target_texture = 0;
  glGenTextures(1, &target_texture);
  printf("target_texture: %d\n", target_texture);
  glBindTexture(GL_TEXTURE_2D, target_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_texture, 0);

  GLint param = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &param);
  printf("object_name: %d\n", param);
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &param);
  printf("object_type: %x\n", param);
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &param);
  printf("texture_level: %d\n", param);
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, &param);
  printf("texture_cube_map_face: %d\n", param);
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &param);
  printf("object_type: %x\n", param);
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &param);
  printf("object_type: %x\n", param);

  /*
  // Create a depth buffer and attach to our framebuffer
  GLuint depth_buffer = 0;
  glGenRenderbuffers(1, &depth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
  */

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // A texture for storing the output of shader2
  GLuint texture2 = 0;
  glGenTextures(1, &texture2);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // GL_LINEAR
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

#if 0
  String8 vertexShaderSource = str8_lit("#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec4 aColor;\n"
      "out vec4 ourColor;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = vec4(aPos, 1.0);\n"
      "  ourColor = aColor;\n"
      "}");

  String8 fragmentShaderSource = str8_lit("#version 330 core\n"
      "out vec4 FragColor;\n"
      "in vec4 ourColor;\n"
      "void main()\n"
      "{\n"
      "  FragColor = ourColor;\n"
      "}\n");

  GLuint shaderProgram = gl_create_shader_program(scratch, vertexShaderSource, fragmentShaderSource, feedback_varyings);
  glUseProgram(shaderProgram);
#endif

  float u_time = 0.f;
  float u_speed_multiplier = 1.0f;

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

    // Clear the canvas
    glClearColor(0.0f, 0.0f, 00.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Pass the uniforms to the shader
    glUniform1f(u_time_location_1, u_time);
    glUniform1f(u_speed_multiplier_location_1, u_speed_multiplier);

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
