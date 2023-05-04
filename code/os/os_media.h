#ifndef OS_MEDIA_H
#define OS_MEDIA_H

typedef u8 MidiStatus;
enum {
  MidiStatus_NoteOff = 0x80,
  MidiStatus_NoteOn = 0x90,
  MidiStatus_PolyphonicKeyPressure = 0xA0,
  MidiStatus_ControlChange = 0xB0,
  MidiStatus_ProgramChange = 0xC0,
  MidiStatus_ChannelPressure = 0xD0,
  MidiStatus_PitchBendChange = 0xE0,
  MidiStatus_System = 0xF0,
};

struct MidiMessage {
  MidiStatus status;
  u8 channel;
  union {
    struct {
      u8 byte1;
      u8 byte2;
    };
    struct {
      u8 note;
      u8 velocity;
    };
    struct {
      u8 controller;
      u8 value;
    };
    struct {
      u8 program;
    };
    struct {
      u8 pressure;
    };
    struct {
      u8 poly_note;
      u8 poly_pressure;
    };
    struct {
      u8 lsb;
      u8 msb;
    };
  };
  DWORD_PTR timestamp;
};

struct MidiDeviceHandle {
  HMIDIIN handle;
  MidiMessage *buffer;
  u32 buffer_write_index;
  u32 buffer_read_index;
  u32 buffer_size;
};

struct os_audio_device {
  u8 channels;
  u8 bytes_per_sample;
  u32 samples_per_second;
  void *platform_data;
};

function String8List os_media_list_midi_devices(M_Arena *arena);
function MidiDeviceHandle* os_media_midi_open(M_Arena *arena, UINT device_id, MidiMessage *buffer, u32 buffer_size);
function MidiMessage* os_media_midi_read(M_Arena *arena, MidiDeviceHandle *handle);

function String8List      os_media_list_audio_recording_devices(M_Arena *arena);
function os_audio_device* os_media_audio_recording_open(M_Arena *arena, i32 device_id);
function u8 *             os_media_audio_read(M_Arena *arena, os_audio_device *audio_device, u32 *bytes_read);

#endif // OS_MEDIA_H
