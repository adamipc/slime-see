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
global b32 GlobalResizeTriggered = false;
global i32 glitchViewportX = 0;
global i32 glitchViewportY = 0;
global i32 glitchWindowWidth = 0;
global i32 glitchWindowHeight = 0;


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

function b32
ToggleFullscreen(HWND window, bool is_fullscreen) {
  M_Scratch scratch;
  if (is_fullscreen) {
    GlobalWindowWidth = 1280;
    GlobalWindowHeight = 720;
    RECT window_rect = {50, 50, (LONG)GlobalWindowWidth+50, (LONG)GlobalWindowHeight+50};
    SetWindowLongPtr(window, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);
    SetWindowPos(window, HWND_TOP, window_rect.left, window_rect.top, window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
    RECT client_rect = {};
    GetClientRect(window, &client_rect);
    GlobalWindowWidth = client_rect.right - client_rect.left;
    GlobalWindowHeight = client_rect.bottom - client_rect.top;
  } else {
    int FullscreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int FullscreenHeight = GetSystemMetrics(SM_CYSCREEN);

    RECT window_rect = {0, 0, (LONG)FullscreenWidth, (LONG)FullscreenHeight};
    SetWindowLongPtr(window, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
    AdjustWindowRect(&window_rect, WS_POPUP, FALSE);
    SetWindowPos(window, HWND_TOP, 0, 0, FullscreenWidth, FullscreenHeight, SWP_FRAMECHANGED);
    RECT client_rect = {};
    GetClientRect(window, &client_rect);
    GlobalWindowWidth = (client_rect.right - client_rect.left);
    GlobalWindowHeight = (client_rect.bottom - client_rect.top);
  }

  GlobalResizeTriggered = true;

  return !is_fullscreen;
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

  String8 audio_device_name = str8_lit("VoiceMeeter Aux Output (VB-Audio VoiceMeeter AUX VAIO)");
  String8List audio_devices = os_media_list_audio_recording_devices(scratch);
  i32 AudioDeviceID = -1;
  i32 index = 0;
  for (String8Node *node = audio_devices.first;
       node != 0;
       node = node->next) {
    if (str8_match(node->string, audio_device_name, 0)) {
      printf("Found audio device: %.*s\n", str8_expand(node->string));
      AudioDeviceID = index;
      break;
    }
    index++;
  }

  u32 audio_buffer_size = 4096;
  u32 audio_buffer_running_sample_index = 0;
  f32 *audio_buffer = push_array(scratch, f32, audio_buffer_size);
  os_audio_device *audio_device = os_media_audio_recording_open(scratch, AudioDeviceID);

#if 1
  win32_audio_device *w_audio_device = (win32_audio_device *)audio_device->platform_data;
  WAVEFORMATEXTENSIBLE *wave_format = (WAVEFORMATEXTENSIBLE *)w_audio_device->wave_format;
  Assert(wave_format->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE || wave_format->Format.wFormatTag == WAVE_FORMAT_PCM);
  Assert(wave_format->Format.nChannels <= 2);
  Assert(wave_format->SubFormat == KSDATAFORMAT_SUBTYPE_PCM || wave_format->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
#endif

  String8List midi_devices = os_media_list_midi_devices(scratch);

  String8 midi_device_name = str8_lit("MPD218");
  i32 MidiDeviceID = -1;
  index = 0;
  for (String8Node *node = midi_devices.first;
       node != 0;
       node = node->next) {
    if (str8_match(node->string, midi_device_name, 0)) {
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

  u64 seed =0;
  os_get_entropy(&seed, sizeof(seed));
  srand((u32)seed);

#if 0
  glitchViewportX = (rand() % 10) - 5;
  glitchViewportY = (rand() % 10) - 5;
  glitchWindowWidth = (rand() % 10) - 5;
  glitchWindowHeight = (rand() % 10) - 5;
#endif

  glViewport(glitchViewportX, glitchViewportY, GlobalWindowWidth+glitchWindowWidth, GlobalWindowHeight+glitchWindowHeight);
  b32 IsFullscreen = false;
  b32 AutomatePresets = false;
  f32 TransitionLength = 1.0;
  for(;;) {
    if (!GlobalRunning) {
      break;
    }

    u32 bytes_read = 0;
    M_Temp read_temp = m_begin_temp(scratch);
    u8 *read_buffer = os_media_audio_read(scratch, audio_device, &bytes_read);
    
    u32 index_start = audio_buffer_running_sample_index;
    while (bytes_read > 0) {
      Assert(bytes_read % 4*audio_device->channels == 0);
      // copy into audio_buffer
      // If 2 channel, downmix to mono
      f32 *samples = (f32*)read_buffer;
      u32 samples_read = bytes_read / 4;
      for (u32 i = 0; i < samples_read; i += audio_device->channels) {
        f32 sample1 = samples[i];
        if (audio_device->channels == 2) {
          sample1 = (sample1 + samples[i+1]) * 0.5f;
        }

        audio_buffer[audio_buffer_running_sample_index++ % audio_buffer_size] = sample1;
      }
      m_end_temp(read_temp);

      read_buffer = os_media_audio_read(scratch, audio_device, &bytes_read);
    }
    m_end_temp(read_temp);

    u32 samples_read = audio_buffer_running_sample_index - index_start;
    printf("samples_read: %d\n", samples_read);

    WindowEventList window_events = win32_process_pending_messages(scratch);

    InputEventList input_events = app_process_input(scratch, midi_handle, &window_events);

    for (InputEventNode *node = input_events.first;
         node != 0;
         node = node->next) {
      switch (node->event) {
        case InputEvent_LoadPreset: {
          PresetData *data = (PresetData *)node->data;
          Preset new_preset = get_preset(data->preset_name);
          switch (data->preset_slot) {
            case PresetSlot_Primary: {
              slimesee_transition_preset(slimesee, new_preset, u_time, TransitionLength);
            } break;
            case PresetSlot_Secondary: {
              slimesee->secondary_preset = new_preset;
            } break;
            case PresetSlot_Beat: {
              slimesee->beat_preset = new_preset;
            } break;
            default: {
              printf("Unhandled preset slot: %d\n", data->preset_slot);
            } break;
          }
        } break;
        case InputEvent_RandomizePreset: {
          PresetData *data = (PresetData *)node->data;
          Preset new_preset = randomize_preset();
          switch (data->preset_slot) {
            case PresetSlot_Primary: {
              slimesee_transition_preset(slimesee, new_preset, u_time, TransitionLength);
            } break;
            case PresetSlot_Secondary: {
              slimesee->secondary_preset = new_preset;
            } break;
            case PresetSlot_Beat: {
              slimesee->beat_preset = new_preset;
            } break;
            default: {
              printf("Unhandled preset slot: %d\n", data->preset_slot);
            } break;
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
        case InputEvent_UpdateBlendValue: {
          BlendValueData *data = (BlendValueData *)node->data;
          slimesee->blend_value = data->blend_value;
        } break;
        case InputEvent_UpdateBeatTransitionTime: {
          BeatTransitionTimeData *data = (BeatTransitionTimeData *)node->data;
          slimesee->beat_transition_time = data->beat_transition_time;
        } break;
        case InputEvent_ToggleAutomation: {
          AutomatePresets = !AutomatePresets;
        } break;
        case InputEvent_ToggleFullscreen: {
        IsFullscreen = ToggleFullscreen(window.window, IsFullscreen);
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

    glViewport(glitchViewportX, glitchViewportY, GlobalWindowWidth+glitchWindowWidth, GlobalWindowHeight+glitchWindowHeight);
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

    u_time += 0.002f;

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
