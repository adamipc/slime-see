#include "os/os_media.h"

#include <mmdeviceapi.h>
//#include <setupapi.h>
//#include <initguid.h>
//#include <devpkey.h>
#include <functiondiscoverykeys.h>
#include <audioclient.h>

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

function String8List
os_media_list_audio_recording_devices(M_Arena *arena) {
  String8List result = {};
  if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) {
    IMMDeviceEnumerator *device_enumerator = NULL;
    if(SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
            CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator),
            (void **)&device_enumerator))) {
      IMMDeviceCollection *audio_capture_endpoints = NULL;

      if(SUCCEEDED(device_enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &audio_capture_endpoints))) {
        UINT count;
        if (SUCCEEDED(audio_capture_endpoints->GetCount(&count))) {
          for (ULONG i = 0; i < count; i++) {
            IMMDevice *endpoint = NULL;
            if (SUCCEEDED(audio_capture_endpoints->Item(i, &endpoint))) {
              IPropertyStore *props = NULL;
              if (SUCCEEDED(endpoint->OpenPropertyStore(STGM_READ, &props))) {
                PROPVARIANT friendly_name;
                PropVariantInit(&friendly_name);
                if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &friendly_name))) {
                  String8 device_name = str8_from_str16(arena, str16_cstring((u16*)friendly_name.pwszVal));
                  str8_list_push(arena, &result, device_name);

                  PropVariantClear(&friendly_name);
                  SAFE_RELEASE(props);
                  SAFE_RELEASE(endpoint);
                }
              }
            }
          }
          SAFE_RELEASE(audio_capture_endpoints);
          SAFE_RELEASE(device_enumerator);
        }
      }
    } 
  }

  return result;
}

#define REFTIMES_PER_SEC 10000000
#define REFTIEMS_PER_MILLISEC 10000

struct win32_audio_device {
  IAudioClient *audio_client;
  IAudioCaptureClient *capture_client;
  IMMDevice *endpoint;
  WAVEFORMATEX *wave_format;
};

function u8 *
os_media_audio_read(M_Arena *arena, os_audio_device *audio_device, u32 *bytes_read) {
  u8 *result = 0;

  win32_audio_device *w_audio_device = (win32_audio_device *)audio_device->platform_data;
  if (w_audio_device->capture_client) {
    u32 packet_length;

    IAudioCaptureClient *capture_client = w_audio_device->capture_client;
    if (FAILED(capture_client->GetNextPacketSize(&packet_length))) {
      // TODO(adam): Logging
    }

    if (packet_length != 0) {
      u8 *buffer;
      u32 frames_available;
      DWORD flags;

      if (FAILED(capture_client->GetBuffer(&buffer, &frames_available, &flags, 0, 0))) {
        // TODO(adam): Logging
      }

      if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
        buffer = 0;
      }

      u32 buffer_size = frames_available * w_audio_device->wave_format->nBlockAlign;
      result = push_array(arena, u8, buffer_size);

      if (buffer) {
        MemoryCopy(result, buffer, buffer_size);
      } else {
        MemoryZero(result, buffer_size);
      }

      *bytes_read = buffer_size;

      if (FAILED(capture_client->ReleaseBuffer(frames_available))) {
        // TODO(adam): Logging
      }
    } else {
      *bytes_read = 0;
    }
  }

  return result;
}

function os_audio_device*
os_media_audio_recording_open(M_Arena *arena, i32 device_id) {

  os_audio_device *result = push_array(arena, os_audio_device, 1);
  win32_audio_device *platform_data = push_array(arena, win32_audio_device, 1);

  IMMDevice *device = NULL;
  IMMDeviceEnumerator *device_enumerator = NULL;
  if(SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
          CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator),
          (void **)&device_enumerator))) {
    IMMDeviceCollection *audio_capture_endpoints = NULL;

    if(SUCCEEDED(device_enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &audio_capture_endpoints))) {
      if (FAILED(audio_capture_endpoints->Item(device_id, &device))) {
        // TODO(adam): Log error
      }

      SAFE_RELEASE(audio_capture_endpoints);
      SAFE_RELEASE(device_enumerator);
    }
  }

  IAudioClient *audio_client = NULL;
  if (device != 0) {
    if (FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                                   NULL, (void**)&audio_client))) {
      // TODO(adam): Log error
    }
  }

  WAVEFORMATEX *wave_format = NULL;
  if (audio_client != 0) {
    if (FAILED(audio_client->GetMixFormat(&wave_format))) {
      // TODO(adam): Log error
    }
  }

  // Request buffer of 1 second
  REFERENCE_TIME requested_duration = REFTIMES_PER_SEC;
  if (wave_format != 0) {
    if (SUCCEEDED(audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                        0,
                                        requested_duration,
                                        0,
                                        wave_format,
                                        0))) {
    }

    u32 buffer_frame_count;
    if (FAILED(audio_client->GetBufferSize(&buffer_frame_count))) {
      // TODO(adam): Log error
    }

    IAudioCaptureClient *capture_client = NULL;
    if (buffer_frame_count != 0) {
      if (FAILED(audio_client->GetService(__uuidof(IAudioCaptureClient), (void**)&capture_client))) {
        // TODO(adam): Log error
      }
    }

    if (capture_client != 0) {
      REFERENCE_TIME actual_duration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC * buffer_frame_count / wave_format->nSamplesPerSec);

      if (FAILED(audio_client->Start())) {
        // TODO(adam): Log error
      }

      platform_data->capture_client = capture_client;
      platform_data->audio_client = audio_client;
      platform_data->endpoint = device;
      platform_data->wave_format = wave_format;
      result->platform_data = (void*)platform_data;
      result->channels = (u8)wave_format->nChannels;
      result->samples_per_second = wave_format->nSamplesPerSec;
      result->bytes_per_sample = (u8)wave_format->wBitsPerSample / 8;
    }
  }

  return (os_audio_device*)result;
}

function String8List
os_media_list_midi_devices(M_Arena *arena) {
  String8List result = {};
  UINT numDevices = midiInGetNumDevs();
  for (UINT i = 0; i < numDevices; ++i) {
    MIDIINCAPS caps;
    midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
    String8 device_name = str8_push_copy(arena, str8_cstring((u8*)&caps.szPname));
    str8_list_push(arena, &result, device_name);
  }

  return result;
}

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg,
                         DWORD_PTR dwInstance,
                         DWORD_PTR dwParam1, DWORD_PTR dwParam2) {

  MidiDeviceHandle *handle = (MidiDeviceHandle*)dwInstance;
  switch (wMsg) {
    case MIM_OPEN: {
      printf("MIM_OPEN\n");
    } break;
    case MIM_CLOSE: {
      printf("MIM_CLOSE\n");
    } break;
    case MIM_DATA: {
      MidiMessage msg = {};
      msg.timestamp = dwParam2;
      msg.status = (dwParam1 & 0xF0);
      msg.channel = (dwParam1 & 0x0F);
      msg.byte1  = (dwParam1 >> 8) & 0xFF;
      msg.byte2 = (dwParam1 >> 16) & 0xFF;

      // Add message to circular buffer
      u32 next = (handle->buffer_write_index + 1) % handle->buffer_size;
      // NOTE(adam): If the buffer is full we drop the message
      if (next != handle->buffer_read_index) {
        handle->buffer[handle->buffer_write_index] = msg;
        handle->buffer_write_index = next;
      }
    } break;
    case MIM_LONGDATA: {
      printf("MIM_LONGDATA\n");
    } break;
    case MIM_ERROR: {
      printf("MIM_ERROR\n");
    } break;
    case MIM_LONGERROR: {
      printf("MIM_LONGERROR\n");
    } break;
    case MIM_MOREDATA: {
      printf("MIM_MOREDATA\n");
    } break;
    default: {
      printf("Unknown MIDI message\n");
    } break;
  }
}

function MidiDeviceHandle*
os_media_midi_open(M_Arena *arena, UINT device_id, MidiMessage *buffer, u32 buffer_size) {
  MidiDeviceHandle *result = push_array(arena, MidiDeviceHandle, 1);
  MMRESULT rv = midiInOpen(&result->handle, device_id, (DWORD_PTR)MidiInProc, (DWORD_PTR)result, CALLBACK_FUNCTION);
  if (rv != MMSYSERR_NOERROR) {
    printf("midiInOpen failed: rv=%d\n", rv);
  } else
  {
    result->buffer_size = buffer_size;
    result->buffer_write_index = 0;
    result->buffer_read_index = 0;
    result->buffer = buffer;
    // TODO(adam): Cleanup midi device at end
    midiInStart(result->handle);
  }

  return result;
}

function MidiMessage*
os_media_midi_read(M_Arena *arena, MidiDeviceHandle *handle) {
  MidiMessage *result = 0;
  // IMPORTANT(adam): This is not thread safe
  if (handle->buffer_read_index != handle->buffer_write_index) {
    result = push_array(arena, MidiMessage, 1);
    MemoryCopyStruct(result, &handle->buffer[handle->buffer_read_index]);
    handle->buffer_read_index = (handle->buffer_read_index + 1) % handle->buffer_size;
  }
  return result;
}
