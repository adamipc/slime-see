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

global bool GlobalRunning = false;
global i32 GlobalWindowWidth = 1280;
global i32 GlobalWindowHeight = 720;
global i32 GlobalViewportX = 0;
global i32 GlobalViewportY = 0;
global b32 GlobalResizeTriggered = false;

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
    case WM_SIZE: {
      GlobalResizeTriggered = true;
    } break;
    case WM_DESTROY: {
      GlobalRunning = false;
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
  i32 MidiDeviceID = -1;
  i32 index = 0;
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
    midi_handle = os_media_midi_open(scratch, (UINT)MidiDeviceID, buffer, 256);
  }

  String8 error = {};

  error = win32_wgl_init(Instance);

  W32_OpenGLWindow window = {};
  if (!error) {
    window = win32_create_opengl_window(Instance, &window_proc, GlobalWindowWidth, GlobalWindowHeight);
  } else {
    print_str8(error);
  }

  if (window.window != 0) {
    ShowWindow(window.window, SW_SHOW);
  }

  GLenum error_code = GL_NO_ERROR;

  Preset preset = get_preset(PresetName_CollapsingBubble);

  SlimeSee *slimesee = slimesee_init(scratch, &preset, GlobalWindowWidth, GlobalWindowHeight);
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
  GlobalRunning = true;

  int FullscreenWidth = GetSystemMetrics(SM_CXSCREEN);
  int FullscreenHeight = GetSystemMetrics(SM_CYSCREEN);

  u64 seed =0;
  os_get_entropy(&seed, sizeof(seed));
  srand((u32)seed);

  int glitchViewportX = 0;
  int glitchViewportY = 0;
  int glitchWindowWidth = 0;
  int glitchWindowHeight = 0;

#if 0
  glitchViewportX = (rand() % 10) - 5;
  glitchViewportY = (rand() % 10) - 5;
  glitchWindowWidth = (rand() % 10) - 5;
  glitchWindowHeight = (rand() % 10) - 5;
#endif

  glViewport(GlobalViewportX+glitchViewportX, GlobalViewportY+glitchViewportY, GlobalWindowWidth+glitchWindowWidth, GlobalWindowHeight+glitchWindowHeight);
  b32 IsFullscreen = false;
  for(;;) {
    if (!GlobalRunning) {
      break;
    }

    WindowEventList window_events = win32_process_pending_messages(scratch);

    InputEventList input_events = app_process_input(scratch, midi_handle, &window_events);

    for (InputEventNode *node = input_events.first;
         node != 0;
         node = node->next) {
      switch (node->event) {
        case InputEvent_LoadPreset: {
          PresetData *data = (PresetData *)node->data;
          if (data->preset_slot == PresetSlot_Primary) {
            slimesee->preset = get_preset(data->preset_name);
          } else {
            printf("Unhandled preset slot: %d\n", data->preset_slot);
          }
        } break;
        case InputEvent_RandomizePreset: {
          PresetData *data = (PresetData *)node->data;
          if (data->preset_slot == PresetSlot_Primary) {
            slimesee->preset = randomize_preset(scratch, &preset);
          } else {
            printf("Unhandled preset slot: %d\n", data->preset_slot);
          }
        } break;
        case InputEvent_ResetPoints: {
          slimesee_reset_points(scratch, slimesee);
        } break;
        case InputEvent_ClearTextures: {
          slimesee_clear_textures(slimesee);
        } break;
        case InputEvent_TakeScreenshot: {
          slimesee_screenshot(scratch, slimesee);
        } break;
        case InputEvent_StopRunning: {
          GlobalRunning = false;
        } break;
        case InputEvent_ToggleFullscreen: {
        if (IsFullscreen) {
          GlobalWindowWidth = 1280;
          GlobalWindowHeight = 720;
          RECT window_rect = {50, 50, (LONG)GlobalWindowWidth+50, (LONG)GlobalWindowHeight+50};
          SetWindowLongPtr(window.window, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
          AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);
          SetWindowPos(window.window, HWND_TOP, window_rect.left, window_rect.top, window_rect.right - window_rect.left,
                       window_rect.bottom - window_rect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
          RECT client_rect = {};
          GetClientRect(window.window, &client_rect);
          GlobalWindowWidth = client_rect.right - client_rect.left;
          GlobalWindowHeight = client_rect.bottom - client_rect.top;
          slimesee_set_resolution(slimesee, GlobalWindowWidth+glitchWindowWidth, GlobalWindowHeight+glitchWindowHeight);
        } else {
          RECT window_rect = {0, 0, (LONG)FullscreenWidth, (LONG)FullscreenHeight};
          SetWindowLongPtr(window.window, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
          AdjustWindowRect(&window_rect, WS_POPUP, FALSE);
          SetWindowPos(window.window, HWND_TOP, 0, 0, FullscreenWidth, FullscreenHeight, SWP_FRAMECHANGED);
          RECT client_rect = {};
          GetClientRect(window.window, &client_rect);
          GlobalWindowWidth = (client_rect.right - client_rect.left);
          GlobalWindowHeight = (client_rect.bottom - client_rect.top);
      slimesee_set_resolution(slimesee, GlobalWindowWidth+glitchWindowWidth, GlobalWindowHeight+glitchWindowHeight);
          String8 debug_info = str8_pushf(scratch, "Fullscreen: %dx%d\n\0", GlobalWindowWidth, GlobalWindowHeight);
          printf("%.*s", str8_expand(debug_info));
        }
        IsFullscreen = !IsFullscreen;
        } break;
        default: {
          printf("Unhandled event: %02x\n", node->event);
        } break;
      }
    };

    if (GlobalResizeTriggered) {
      GlobalResizeTriggered = false;
      RECT client_rect = {};
      GetClientRect(window.window, &client_rect);
      GlobalWindowWidth = (client_rect.right - client_rect.left);
      GlobalWindowHeight = (client_rect.bottom - client_rect.top);
      slimesee_set_resolution(slimesee, GlobalWindowWidth+glitchWindowWidth, GlobalWindowHeight+glitchWindowHeight);
    }

    HDC dc = GetDC(window.window);
    win32_wglMakeCurrent(dc, window.glrc);

    glViewport(GlobalViewportX+glitchViewportX, GlobalViewportY+glitchViewportY, GlobalWindowWidth+glitchWindowWidth, GlobalWindowHeight+glitchWindowHeight);
#if 0
    glitchViewportX = (rand() % 10) - 5;
    glitchViewportY = (rand() % 10) - 5;
    glitchWindowWidth = (rand() % 10) - 5;
    glitchWindowHeight = (rand() % 10) - 5;
#endif

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

  // TODO(adam): Tear down window / gl context better?
  DestroyWindow(window.window);

  if (error.size != 0) {
    fprintf(stdout, "%.*s\n", str8_expand(error));
  } else {
    fprintf(stdout, "success\n");
  }

  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
