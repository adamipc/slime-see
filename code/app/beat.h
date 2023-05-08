#ifndef BEAT_H
#define BEAT_H

typedef u8 OnsetDetectionMethod;
enum {
  OnsetDetectionMethod_HFC_Masri_Bateman,
  OnsetDetectionMethod_HFC_Jensen_Andersen,
  OnsetDetectionMethod_PMM_Hainsworth,
  OnsetDetectionMethod_PSF_Jensen,
  OnsetDetectionMethod_ELC_Collins,
  OnsetDetectionMethod_SpectralFlux,
  OnsetDetectionMethod_PhaseDeviation,
};

struct PhaseDeviationState {
  complex32 *third_last_frame;
  complex32 *second_last_frame;
  complex32 *last_frame;
};

struct SpectralFluxState {
  complex32 *second_last_frame;
  complex32 *last_frame;
};

typedef u8 PeakPickerType;
enum {
  PeakPickerType_LocalMaxima,
  PeakPickerType_Threshold,
  PeakPickerType_ThresholdDecay,
  PeakPickerType_ThresholdDecayDelay,
};

struct PeakPickerState {
  char v[256];
};

struct PeakPicker {
  PeakPickerType type;
  PeakPickerState *state;
};

struct PeakPickerState_ThresholdDecayDelay {
  f32 threshold;
  f32 min_threshold;
  f32 max_value;
  f32 decay_factor;
  f32 delay_ms;
  u32 last_peak_sample_index;
};

struct PeakPickerState_ThresholdDecay {
  f32 threshold;
  f32 decay_factor;
};

struct PeakPickerState_Threshold {
  f32 threshold;
};

struct PeakPickerState_LocalMaxima {
  f32 *buffer;
  u32 buffer_size;
  u32 buffer_index;
  u32 multiplier;
  u32 window_size;
  f32 threshold;
  f32 ewma_mean;
  f32 ewma_variance;
  f32 ewma_decay_factor;
};

struct BeatDetector {
  u32 sample_rate;
  OnsetDetectionMethod odm;
  u32 frame_size;
  u32 hop_size;
  u32 last_frame_index;
  f32 filter_frequency;
  complex32 *last_frame;
  complex32 *next_frame;
  PeakPicker *peak_picker;
  void* odm_state;
};

function BeatDetector* beat_detector_init(M_Arena *arena, u32 sample_rate, OnsetDetectionMethod odm, PeakPickerType ppt);
function f32 beat_detector_process_audio(M_Arena *arena, BeatDetector *detector, f32 *audio_buffer, u32 samples_read, u32 running_buffer_index);
function void draw_peaks_local_maxima(PeakPickerState_LocalMaxima *state, f32 normalized_value, f32 local_mean, b32 is_peak);
function b32 peak_picker_process(PeakPicker *peak_picker, f32 latest_value, u32 frame_index, u32 sample_rate, f32 *intensity_out);
function void beat_detector_set_sensitivity(BeatDetector *detector, f32 sensitivity);

#endif BEAT_H
