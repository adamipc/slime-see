#include "os/os_media.h"

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
os_media_midi_open(M_Arena *arena, u8 device_id, MidiMessage *buffer, u32 buffer_size) {
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
