#include <windows.h>
#include <stdio.h>
#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

#include <stdlib.h>
#include <mmsystem.h>

#include "base/base_memory_malloc.cpp"

#define GET_PROC_ADDRESS(v,m,n) (*(PROC*)(&(v))) = GetProcAddress((m),(n))
# define power_of_two(x) (1 << (x))
#define print_str8(x) printf("%.*s\n", (int)((x).size), (x).str)

#include "gl/gl_definitions.h"
#include "win32/win32_wgl_definitions.h"

#include "gl/gl.cpp"
#include "win32/win32_wgl.cpp"

#include "app/preset.cpp"
#include "app/pipeline.cpp"
#include "app/slimesee.cpp"
#include "app/input.cpp"

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

  String8List midi_devices = os_media_list_midi_devices(scratch);

  String8 device_name = str8_lit("MPD218");
  DWORD MidiDeviceID = -1;
  u8 index = 0;
  for (String8Node *node = midi_devices.first;
       node != 0;
       node = node->next) {
    if (str8_match(node->string, device_name, 0)) {
      MidiDeviceID = index;
      break;
    }
    ++index;
  }

  MidiDeviceHandle *midi_handle = 0;
  if (MidiDeviceID >= 0) {
    MidiMessage *buffer = push_array(scratch, MidiMessage, 256);
    midi_handle = os_media_midi_open(scratch, MidiDeviceID, buffer, 256);
  }

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

  Preset preset = get_preset(PresetName_CollapsingBubble);

  SlimeSee *slimesee = slimesee_init(scratch, &preset, width, height);
  // Common uniforms
  float u_time = 0.f;

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

    InputEventList input_events = app_process_input(scratch, midi_handle);

    for (InputEventNode *node = input_events.first;
         node != 0;
         node = node->next) {
      switch (node->event) {
        case InputEvent_LoadPreset: {
          LoadPresetData *data = (LoadPresetData *)node->data;
          preset = get_preset(data->preset_name);
        } break;
      }
    };

    HDC dc = GetDC(window.window);
    win32_wglMakeCurrent(dc, window.glrc);

    glViewport(0, 0, width, height);

    slimesee->preset = preset;
    slimesee_draw(slimesee, u_time);

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
