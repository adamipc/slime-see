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

  if (error.size > 0) {
    goto done;
  }

  W32_OpenGLWindow window = win32_create_opengl_window(Instance);

  if (window.window == 0) {
    goto done;
  }

  ShowWindow(window.window, SW_SHOW);

  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  for(;;) {
    MSG msg = {};
    BOOL get_message = GetMessage(&msg, 0, 0, 0);
    if (get_message == 0) {
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    HDC dc = GetDC(window.window);
    win32_wglMakeCurrent(dc, window.glrc);

    glClearColor(0.9f, 0.3f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    SwapBuffers(dc);
    ReleaseDC(window.window, dc);
  }

done:
  if (error.size != 0) {
    fprintf(stdout, "%.*s\n", str8_expand(error));
  } else {
    fprintf(stdout, "success\n");
  }

  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
