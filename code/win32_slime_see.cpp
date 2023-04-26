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

  if (window.window != 0) {
    ShowWindow(window.window, SW_SHOW);

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

    String8 data = str8_lit("gl_Position");
    String8List feedback_varyings = {};
    str8_list_push(scratch, &feedback_varyings, data);
    String8 shader1_vs = os_file_read(scratch, str8_lit("../data/shaders/shader1.vs"));
    String8 shader1_fs = os_file_read(scratch, str8_lit("../data/shaders/shader1.fs"));
    GLuint shader1 = gl_create_shader_program(scratch, shader1_vs, shader1_fs, feedback_varyings);
    
    // Get location of the `a_position` attribute
    GLint a_position_location = glGetAttribLocation(shader1, "a_position");
    glEnableVertexAttribArray(a_position_location);

    u64 number_of_positions = 1000;
    f32 *initial_positions = push_array(scratch, f32, number_of_positions*4);
    // Ring arrangement
    for (u64 i = 0; i < number_of_positions; ++i) {
        f32 angle = (f32)i*pi_f32*2.0f/((f32)number_of_positions);
        f32 distance = 0.7f; // distance to center
        f32 x = sin(angle)*distance;
        f32 y = -cos(angle)*distance;
        f32 direction = 1 + (angle+pi_f32/2.f)/1000.f;
        initial_positions[i*4 + 0] = x;
        initial_positions[i*4 + 1] = y;
        initial_positions[i*4 + 2] = direction;
        // TODO(adam): Calculate speed once we have random numbers
        initial_positions[i*4 + 3] = 0.1f;
    }

    os_file_write(str8_lit("foo.txt"), str8_lit("Bar baz text\n"));
    String8 my_file_data = os_file_read(scratch, str8_lit("foo.txt"));
    printf("%.*s\n", str8_expand(my_file_data));

    FileProperties properties = os_file_properties(str8_lit("."));
    printf("size: %llu\nis folder? %s\n", properties.size, properties.flags & FilePropertyFlag_Directory ? "yes" : "no");

    // Make buffers for holding position data, they will alternate
    GLuint position_buffers[2];
    glGenBuffers(2, position_buffers);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_positions), initial_positions, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_positions), initial_positions, GL_DYNAMIC_COPY);

    feedback_varyings = {};
    String8 shader2_vs = os_file_read(scratch, str8_lit("../data/shaders/shader2.vs"));
    String8 shader2_fs = os_file_read(scratch, str8_lit("../data/shaders/shader2.fs"));
    GLuint shader2 = gl_create_shader_program(scratch, shader2_vs, shader2_fs, feedback_varyings);

    GLuint shaderProgram = gl_create_shader_program(scratch, vertexShaderSource, fragmentShaderSource, feedback_varyings);

    printf("shaderProgram: %d\n", shaderProgram);
    float vertices[] = {
      // positions         // colors
      1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // top right
      1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
      -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top left
    };

    unsigned int indices[] = {
      0, 1, 3,
      1, 2, 3,
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLenum error_code = GL_NO_ERROR;
    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    float u_time = 0.f;

    for(;;) {
      MSG msg = {};
      while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      HDC dc = GetDC(window.window);
      win32_wglMakeCurrent(dc, window.glrc);

      glClearColor(0.9f, 0.3f, 0.f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT);

      float greenValue = (sin(u_time) / 2.0f) + 0.5f;
      //int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
      glUseProgram(shaderProgram);
      //glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
      glBindVertexArray(vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

      error_code = glGetError();
      if (error_code != GL_NO_ERROR) {
        printf("ERROR::OPENGL::%d\n", error_code);
        break;
      }

      u_time += 0.01f;

      SwapBuffers(dc);
      ReleaseDC(window.window, dc);
    }
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
