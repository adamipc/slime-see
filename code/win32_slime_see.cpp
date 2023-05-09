#include <windows.h>
#include <stdio.h>
#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

#include <stdlib.h>
#include <mmsystem.h>

// Use malloc for now
#include "base/base_memory_malloc.cpp"

#define GET_PROC_ADDRESS(v,m,n) (*(PROC*)(&(v))) = GetProcAddress((m),(n))
# define power_of_two(x) (1 << (x))
#define print_str8(x) printf("%.*s\n", (int)((x).size), (x).str)

#include "gl/gl_definitions.h"
#include "win32/win32_wgl_definitions.h"

#include "gl/gl.cpp"
#include "win32/win32_wgl.cpp"

#include "graphics/graphics_helper.cpp"

#include "audio/fft.cpp"

#include "app/preset.cpp"
#include "app/pipeline.cpp"
#include "app/slimesee.cpp"
#include "app/input.cpp"
#include "app/beat.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

function stbtt_fontinfo*
stbtt_font_init(M_Arena *arena) {
  stbtt_fontinfo *result = 0;

  M_Temp restore_point = m_begin_temp(arena);

  stbtt_fontinfo *font_info = push_array(arena, stbtt_fontinfo, 1);

  u64 bytes_read = 0;
  u8 *font_buffer = os_file_read_binary(arena, str8_lit("../data/fonts/FiraCode-SemiBold.ttf"), &bytes_read);

  if (bytes_read == 0 || !stbtt_InitFont(font_info, font_buffer, 0)) {
    m_end_temp(restore_point);
    printf("Failed to init font\n");
  } else {
    result = font_info;
  }

  return result;
}

global bool GlobalRunning = false;
global i32 WINDOW_WIDTH = 1280;
global i32 WINDOW_HEIGHT = 720;
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

function u32
read_audio(M_Arena *arena, os_audio_device *audio_device, f32 *audio_buffer, u32 audio_buffer_size, u32 *audio_buffer_running_sample_index) {
  M_Scratch scratch(arena);
  u32 bytes_read = 0;
  M_Temp temp = m_begin_temp(scratch);
  u8 *read_buffer = os_media_audio_read(scratch, audio_device, &bytes_read);

  u32 index_start = *audio_buffer_running_sample_index;
  while (bytes_read > 0) {
    Assert(bytes_read % 4*audio_device->channels == 0);
    // copy into audio_buffer
    // If 2 channel, downmix to mono
    f32 *samples = (f32*)read_buffer;
    u32 samples_read = bytes_read / 4;
    for (u32 i = 0; i < samples_read; i += audio_device->channels) {
      f32 sample1 = samples[i];
#if 0
      Assert(sample1 >= -1.0f && sample1 <= 1.0f);
#endif
      if (audio_device->channels == 2) {
        sample1 = (sample1 + samples[i+1]) * 0.5f;
      }

      audio_buffer[(*audio_buffer_running_sample_index)++ % audio_buffer_size] = sample1;
    }
    m_end_temp(temp);

    read_buffer = os_media_audio_read(scratch, audio_device, &bytes_read);
  }
  m_end_temp(temp);

  return *audio_buffer_running_sample_index - index_start;
}

function void
graph_frequencies(M_Arena *arena, os_audio_device *audio_device, f32 *audio_buffer, u32 audio_buffer_size) {
  M_Scratch scratch(arena);

  u32 sample_rate = audio_device->samples_per_second;
  u32 bandwidth_hz = sample_rate / 2;
  f32 duration_ms = (f32)audio_buffer_size / (f32)sample_rate * 1000.0f;
  f32 resolution_hz = (f32)sample_rate / (f32)audio_buffer_size;

  u32 number_of_bands = (u32)(bandwidth_hz / resolution_hz);
  f32 *bands = push_array(scratch, f32, number_of_bands);
  for (u32 i = 0; i < number_of_bands; i++) {
    bands[i] = i * resolution_hz;
  }

  f32 *energies = fft_energy_for_bands(scratch, audio_buffer, audio_buffer_size, bands, number_of_bands, sample_rate);

  u32 pixel_count = GlobalWindowWidth * GlobalWindowHeight;
  u8 *image = push_array(scratch, u8, pixel_count * 4);
  MemoryZero(image, pixel_count * 4);
  
  u32 pad_x = 0;
  u32 pad_y = 0;
  f32 c = (GlobalWindowWidth - (pad_x * 2)) / (f32)(number_of_bands/2.f);
  u32 max_line_height = GlobalWindowHeight - (pad_y * 2);
  f32 min_dB = -60.f;
  f32 max_dB = 0.f;
  for (u32 i = 0; i < number_of_bands/2; i += 1) {
    f32 energy = energies[i];
    f32 energy_dB = 10.f * log10f(energy);
    if (energy_dB >= min_dB) {
    u32 normalized_height = (u32)((energy_dB - min_dB) / (max_dB - min_dB) * max_line_height);

    u32 x = (u32)(pad_x + i * c);
    u32 y = pad_y;
    u32 w = (u32)(c/2);
    u32 color = 0xff0000ff;
    draw_rect(image, GlobalWindowWidth, GlobalWindowHeight, x, y, w, normalized_height, color);
    //printf("%d: band %0.2fhz: %f\n", i, i*resolution_hz, energy);
    }
  }

  // Draw the image to the screen using OpenGL
  glRasterPos2i(-1, -1);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.0f);
  glUseProgram(0);
  glDrawPixels(GlobalWindowWidth, GlobalWindowHeight, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glDisable(GL_ALPHA_TEST);

  /*
  for (u32 i = 0;
       i < number_of_bands;
       i++) {
    if (bands[i] >= 20.0f && bands[i] <= 20000.0f) {
      if (energies[i] > 50000.f) {
        printf("energy at %fHz: %f\n", bands[i], energies[i]);
      }
    }
  }
  */
}

function b32
ToggleFullscreen(HWND window, bool is_fullscreen) {
  M_Scratch scratch;
  if (is_fullscreen) {
    GlobalWindowWidth = WINDOW_WIDTH;
    GlobalWindowHeight = WINDOW_HEIGHT;
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
    HMONITOR primary_monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);
    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX info = {};
    info.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(monitor, &info);

    RECT window_rect = {info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right, info.rcMonitor.bottom};
    SetWindowLongPtr(window, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
    AdjustWindowRect(&window_rect, WS_POPUP, FALSE);
    SetWindowPos(window, HWND_TOP, window_rect.left, window_rect.top, window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top, SWP_FRAMECHANGED);
    RECT client_rect = {};
    GetClientRect(window, &client_rect);
    GlobalWindowWidth = (client_rect.right - client_rect.left);
    GlobalWindowHeight = (client_rect.bottom - client_rect.top);
  }

  GlobalResizeTriggered = true;

  return !is_fullscreen;
}

function void
draw_overlay(M_Arena *arena, u8* rgba_image, u32 image_width, u32 image_height, stbtt_fontinfo *font_info, os_audio_device *audio_device, SlimeSee *slimesee, b32 is_beat) {
  M_Scratch scratch;

  u32 pixel_count = image_width * image_height;
  MemoryZero(rgba_image, pixel_count*4);

  u32 pad_x = 8;
  u32 pad_y = 8;

  u32 width = 256;
  u32 height = 64;
  u8 *image = push_array(arena, u8, width * height);
  i32 line_height = 64;
  f32 scale = (f32)stbtt_ScaleForPixelHeight(font_info, (f32)line_height);

  String8 text = str8_lit("SlimeSee!");

  i32 ascent, descent, line_gap;
  stbtt_GetFontVMetrics(font_info, &ascent, &descent, &line_gap);

  ascent = (i32)roundf(ascent * scale);
  descent = (i32)roundf(descent * scale);

  i32 x = 0;

  for (i32 i = 0;
       i < text.size;
       ++i) {
    // how wide this character is
    i32 ax;
    i32 lsb;
    stbtt_GetCodepointHMetrics(font_info, text.str[i], &ax, &lsb);

    // monospaced
    //ax = 32;
    //lsb = 64;

    // get bounding box for character (may be offset to account for chars that dip above or below the line)
    i32 c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(font_info, text.str[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

    // compute y (different characters have different heights
    i32 y = ascent + c_y1;

    // render character (stride and offset is important here)
    i32 byte_offset = (pad_x + x + (i32)roundf(lsb * scale)) + ((pad_y + y) * width);
    stbtt_MakeCodepointBitmap(font_info, image + byte_offset, c_x2 - c_x1, c_y2 - c_y1, width, scale, scale, text.str[i]);

    // advance x
    x += (i32)roundf(ax * scale);

    // add kerning
    i32 kern = stbtt_GetCodepointKernAdvance(font_info, text.str[i], text.str[i + 1]);
    x += (i32)roundf(kern * scale);
  }
  
  for (u32 i = 0;
       i < width * height;
       ++i) {
    rgba_image[i * 4 + 0] = 255;
    rgba_image[i * 4 + 1] = 180;
    rgba_image[i * 4 + 2] = 99;
    rgba_image[i * 4 + 3] = image[i];
  }

  // Draw the image to the screen using OpenGL
  glRasterPos2i(-1, 1);
  glPixelZoom(1.0f, -1.0f);
  glEnable(GL_BLEND);
  // we have a one channel bitmap so values we want displayed are 255
  // ones we want transparent are 0
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(0);
  glDrawPixels(image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, rgba_image);
  glDisable(GL_BLEND);
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

  os_audio_device *audio_device = os_media_audio_recording_open(scratch, AudioDeviceID);

  BeatDetector *detector = beat_detector_init(scratch, audio_device->samples_per_second, OnsetDetectionMethod_SpectralFlux, PeakPickerType_ThresholdDecayDelay);

  u32 audio_buffer_size = detector->frame_size;
  u32 audio_buffer_running_sample_index = 0;
  f32 *audio_buffer = push_array(scratch, f32, audio_buffer_size);

#if 1
  win32_audio_device *w_audio_device = (win32_audio_device *)audio_device->platform_data;
  WAVEFORMATEXTENSIBLE *wave_format = (WAVEFORMATEXTENSIBLE *)w_audio_device->wave_format;
  Assert(wave_format->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE || wave_format->Format.wFormatTag == WAVE_FORMAT_PCM);
  Assert(wave_format->Format.nChannels <= 2);
  Assert(wave_format->SubFormat == KSDATAFORMAT_SUBTYPE_PCM || wave_format->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
  Assert(wave_format->Samples.wValidBitsPerSample == 32);
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

  SlimeSee *slimesee = slimesee_init(scratch, GlobalWindowWidth, GlobalWindowHeight);

  // Enable debugging and clear errors from initialization
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

  stbtt_fontinfo *font_info = stbtt_font_init(scratch);

  u8 *debug_buffer = 0;
  u64 debug_buffer_size = 0;

  b32 IsFullscreen = false;
  b32 AutomatePresets = false;
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
        case InputEvent_UpdateWindowGlitch: {
          GlitchWindowData *data = (GlitchWindowData *)node->data;
          switch (data->window_param)
          {
            case GlitchWindowParam_X: {
              glitchViewportX += data->glitch_value;
            } break;
            case GlitchWindowParam_Y: {
              glitchViewportY += data->glitch_value;
            } break;
            case GlitchWindowParam_Width: {
              glitchWindowWidth += data->glitch_value;
            } break;
            case GlitchWindowParam_Height: {
              glitchWindowHeight += data->glitch_value;
            } break;
          }
        } break;
        case InputEvent_LoadPreset: {
          PresetData *data = (PresetData *)node->data;
          Preset new_preset = get_preset(data->preset_name);
          switch (data->preset_slot) {
            case PresetSlot_Primary: {
              slimesee_transition_preset(slimesee, new_preset, data->preset_intensity);
            } break;
            case PresetSlot_Secondary: {
              slimesee_set_preset(slimesee, PresetSlot_Secondary, new_preset);
            } break;
            case PresetSlot_Beat: {
              slimesee_set_preset(slimesee, PresetSlot_Beat, new_preset);
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
              slimesee_transition_preset(slimesee, new_preset, data->preset_intensity);
            } break;
            case PresetSlot_Secondary: {
              slimesee_set_preset(slimesee, PresetSlot_Secondary, new_preset);
            } break;
            case PresetSlot_Beat: {
              slimesee_set_preset(slimesee, PresetSlot_Beat, new_preset);
            } break;
            default: {
              printf("Unhandled preset slot: %d\n", data->preset_slot);
            } break;
          }
        } break;
        case InputEvent_UpdateBeatTransitionRatio: {
          BeatTransitionRatioData *data = (BeatTransitionRatioData *)node->data;
          slimesee_set_beat_transition_ratio(slimesee, data->beat_transition_ratio);
        } break;
        case InputEvent_UpdateBeatSensitivity: {
          BeatSensitivityData *data = (BeatSensitivityData *)node->data;
          beat_detector_set_sensitivity(detector, data->beat_sensitivity);
        } break;
        case InputEvent_PanicAtTheDisco: {
          glitchViewportX = glitchViewportY = glitchWindowHeight = glitchWindowWidth = 0;
          slimesee_reset_points(slimesee);
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
          slimesee_set_blend_value(slimesee, data->blend_value);
        } break;
        case InputEvent_UpdateBeatTransitionTime: {
          BeatTransitionTimeData *data = (BeatTransitionTimeData *)node->data;
          slimesee_set_beat_transition_ms(slimesee, data->beat_transition_ms);
        } break;
        case InputEvent_UpdateColorSwap: {
          ColorSwapData *data = (ColorSwapData *)node->data;
          slimesee_set_color_swap(slimesee, data->color_swap);
        } break;
        case InputEvent_ToggleAutomation: {
          AutomatePresets = !AutomatePresets;
        } break;
        case InputEvent_ToggleFullscreen: {
        IsFullscreen = ToggleFullscreen(window.window, IsFullscreen);
        } break;
        case InputEvent_DumpState: {
          SlimeSeeState *state = slimesee_dump_state(scratch, slimesee);
          slimeseestate_write_to_file(state, str8_lit("slimesee.state"));
        } break;
        case InputEvent_LoadState: {
          SlimeSeeState *state = slimeseestate_read_from_file(scratch, str8_lit("slimesee.state"));
          if (state != 0) {
            slimesee_load_state(slimesee, state);
          }
        } break;
        default: {
          printf("Unhandled event: %02x\n", node->event);
        } break;
      }
    };

    // THis is here for debugging purposes but should be moved back before main drawing
    u32 samples_read = read_audio(scratch, audio_device, audio_buffer, audio_buffer_size, &audio_buffer_running_sample_index);
    f32 beat_intensity = beat_detector_process_audio(scratch, detector, audio_buffer, samples_read, audio_buffer_running_sample_index);

    if (beat_intensity > 0.0f) {
      printf("Beat intensity: %f\n", beat_intensity);
      slimesee_beat_transition(slimesee, beat_intensity);
    }

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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //graph_frequencies(scratch, audio_device, audio_buffer, audio_buffer_size);
    slimesee_draw(slimesee);

    // NOTE(adam): re-init buffer if it gets bigger, just change size if smaller
    if (debug_buffer == 0 || debug_buffer_size != 4*GlobalWindowWidth*GlobalWindowHeight) {
      u64 new_buffer_size = 4*GlobalWindowWidth*GlobalWindowHeight;
      if (new_buffer_size > debug_buffer_size) {
        debug_buffer = push_array(scratch, u8, debug_buffer_size);
      }
      debug_buffer_size = new_buffer_size;
    }

    //draw_overlay(scratch, debug_buffer, GlobalWindowWidth, GlobalWindowHeight, font_info, audio_device, slimesee, is_beat);

    error_code = glGetError();
    if (error_code != GL_NO_ERROR) {
      printf("ERROR::OPENGL::%d\n", error_code);
      break;
    } else 
    {
      //printf("no error\n");
    }

    SwapBuffers(dc);
    ReleaseDC(window.window, dc);

    u64 end_time = os_now_microseconds();

    u64 elapsed_time = end_time - last_time;
    last_time = end_time;
    slimesee_update_time(slimesee, elapsed_time, AutomatePresets);

    f32 frame_time_ms = elapsed_time / 1000.0f;
    f32 fps = 1000.0f / frame_time_ms;

    if ((frame % 61) == 0) {
      printf("frame time: %f ms, fps: %f\n", frame_time_ms, fps);
    }

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
