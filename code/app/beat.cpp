#include "beat.h"
#include "audio/fft.h"

function f32
hamming_window(u32 n, u32 N) {
  f32 result = 0.5f * (1.f - cosf(2.f * pi_f32 * n / (N - 1.f)));
  return result;
}

function b32 peak_picker_local_maxima(PeakPickerState_LocalMaxima *state, f32 latest_value) {
  b32 result = false;

  f32 ewma_diff = latest_value - state->ewma_mean;
  state->ewma_mean     += state->ewma_decay_factor * ewma_diff;
  state->ewma_variance += state->ewma_decay_factor * (ewma_diff*ewma_diff - state->ewma_variance);

  f32 ewma_stddev = sqrtf(state->ewma_variance);

  f32 normalized_value = (latest_value - state->ewma_mean) / ewma_stddev;

  state->buffer[state->buffer_index++] = normalized_value;
  state->buffer_index %= state->buffer_size;

  f32 local_mean = 0.f;
  for (u32 i = 0;
       i < state->buffer_size;
       i++) {
    local_mean += state->buffer[i];
  }
  local_mean /= state->buffer_size;

  u32 starting_index = (state->buffer_size + (state->buffer_index - (state->window_size * 2 + 1))) % state->buffer_size;
  u32 center_window_index = starting_index + state->window_size;
  u32 end_index = starting_index + state->window_size * 2;
  //printf("buffer_index: %u, starting_index: %u, center_window_index: %u, end_index: %u\n", state->buffer_index, starting_index % state->buffer_size, center_window_index % state->buffer_size, end_index % state->buffer_size);
  if (normalized_value > local_mean) {
    f32 center_window = state->buffer[center_window_index % state->buffer_size];
    b32 is_local_maximum = true;
    for (u32 i = starting_index;
         i <= end_index;
         i++) {
      u32 index = i % state->buffer_size;
      if (index == center_window_index % state->buffer_size) {
        continue;
      }
      if (state->buffer[index] > center_window) {
        is_local_maximum = false;
        break;
      }
    }

    if (is_local_maximum && center_window > local_mean + state->threshold) {
      result = true;
    }
  }

  draw_peaks_local_maxima(state, normalized_value, local_mean, result);

  return result;
}

function void
draw_peaks_local_maxima(PeakPickerState_LocalMaxima *state, f32 normalized_value, f32 local_mean, b32 is_peak) {
  M_Scratch scratch;
  u32 image_width = 512;
  u32 image_height = 256;

  u32 pixel_count = image_width * image_height;
  u8 *image = push_array(scratch, u8, pixel_count * 4);
  MemoryZero(image, pixel_count * 4);

  u32 pad_x = 8;
  u32 pad_y = 8;

  f32 c = (image_width - (pad_x * 2)) / (f32)state->buffer_size;
  f32 r = (image_height - (pad_y * 2)) / 2.f;


  for (u32 i = state->buffer_index;
       i < state->buffer_size + state->buffer_index;
       i++) {
    u32 index = i % state->buffer_size;
    u32 x = (u32)(pad_x + (i - state->buffer_index) * c);
    u32 y = pad_y;
    u32 width = (u32)(c/2);
    Assert(state->buffer[index] >= -1.f && state->buffer[index] <= 1.f);
    u32 height = (u32)((state->buffer[index] * r) + ((image_height / 2.f) - pad_y));

    u32 color = 0xff00ff00;

    // current index we are checking to see if it is the local maximum
    if (state->buffer[index] == normalized_value) {
      color = 0xffff0000;
    } else if (is_peak && state->buffer[index] >= local_mean + state->threshold) {
      color = 0xff0000ff;
    }

    /*
    printf("%d: x: %d, y: %d, width: %d, height: %d, value: %f, color: %08x %s %s\n",
        index, x, y, width, height, state->buffer[index], color,
        (state->buffer[index] == normalized_value) ? "(current index)" : "", is_peak ? " (peak)" : "");
        */

    draw_rect(image, image_width, image_height, x, y, width, height, color);
  }

  // Draw horizontal line at the local_mean + state->threshold
  {
    u32 x = 0;
    u32 y = (u32)(((local_mean + state->threshold) * r) + ((image_height / 2.f) - pad_y));
    u32 width = image_width;
    u32 height = 2;
    u32 color = 0xff0ff0ff;
    draw_rect(image, image_width, image_height, x, y, width, height, color);
  }
  // Draw horizontal line at the local_mean
  {
    u32 x = 0;
    u32 y = (u32)((local_mean * r) + ((image_height / 2.f) - pad_y));
    u32 width = image_width;
    u32 height = 2;
    u32 color = 0xffffffff;
    draw_rect(image, image_width, image_height, x, y, width, height, color);
  }
  // Draw the image to the screen using OpenGL
  glRasterPos2i(0, 0);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.0f);
  glUseProgram(0);
  glDrawPixels(image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glDisable(GL_ALPHA_TEST);
}


function f32
onset_detection_phase_deviation(M_Arena *arena, PhaseDeviationState *state, complex32 **next_frame, u32 frame_size, u32 max_bin) {
  complex32 *temp = state->third_last_frame;
  state->third_last_frame = state->second_last_frame;
  state->second_last_frame = state->last_frame;
  state->last_frame = *next_frame;
  *next_frame = temp;

  f32 phase_deviation = 0.f;
  for (u32 k = 0;
       k < max_bin;
       k++) {
    complex32 last_frame_value = state->last_frame[k];
    complex32 second_last_frame_value = state->second_last_frame[k];
    complex32 third_last_frame_value = state->third_last_frame[k];

    f32 last_frame_phase = atan2f(last_frame_value.im, last_frame_value.re);
    f32 second_last_frame_phase = atan2f(second_last_frame_value.im, second_last_frame_value.re);
    f32 third_last_frame_phase = atan2f(third_last_frame_value.im, third_last_frame_value.re);

    f32 last_phase_difference = last_frame_phase - second_last_frame_phase;
    f32 second_last_phase_difference = second_last_frame_phase - third_last_frame_phase;

    phase_deviation += abs_f32(last_phase_difference - second_last_phase_difference);
  }
  phase_deviation /= max_bin;

  return phase_deviation;
}

function f32
onset_detection_spectral_flux(M_Arena *arena, SpectralFluxState *state, complex32 **next_frame, u32 frame_size, u32 max_bin) {
  f32 result = 0.f;
  complex32 *temp = state->second_last_frame;
  state->second_last_frame = state->last_frame;
  state->last_frame = *next_frame;
  *next_frame = temp;

  for (u32 k = 0;
       k < max_bin;
       k++) {
    f32 second_last_frame_magnitude = complex_magnitude(state->second_last_frame[k]);
    f32 last_frame_magnitude = complex_magnitude(state->last_frame[k]);

    f32 diff = last_frame_magnitude - second_last_frame_magnitude;
    if (diff > 0.f) {
      result += diff;
    }
  }

  return result;
}

function b32 peak_picker_threshold(PeakPickerState_Threshold *state, f32 latest_value) {
  b32 result = false;
  if (latest_value > state->threshold) {
    printf("%0.4f %0.4f\n", latest_value, state->threshold);
    result = true;
  }
  return result;
}

function b32 peak_picker_threshold_decay_delay(PeakPickerState_ThresholdDecayDelay *state, f32 latest_value, u32 sample_index, u32 sample_rate) {
  b32 result = false;

  u32 samples_since_last_peak = sample_index - state->last_peak_sample_index; 
  f32 ms_since_last_peak = (f32)samples_since_last_peak / (f32)sample_rate * 1000.f;

  if (ms_since_last_peak >= state->delay_ms) {
    //printf("last_peak_sample_index: %u, sample_index: %u, samples_since_last_peak: %u, ms_since_last_peak: %0.4f\n", state->last_peak_sample_index, sample_index, samples_since_last_peak, ms_since_last_peak);
    //printf("latest_value: %0.4f, threshold: %0.4f\n", latest_value, state->threshold);
    if (latest_value > state->threshold) {
      //printf("latest_value: %0.4f, threshold: %0.4f ms since last peak: %0.4f\n", latest_value, state->threshold, ms_since_last_peak);
      result = true;

      if (latest_value > state->max_value) {
        state->max_value = latest_value;
      } else {
        state->max_value *= state->decay_factor;
        if (state->max_value < state->min_threshold) {
          state->max_value = state->min_threshold;
        }
      }

      if (latest_value > state->threshold) {
        state->threshold = latest_value;
      }

      state->last_peak_sample_index = sample_index;
    }
  }

  if (!result) {
    state->threshold *= state->decay_factor;
    if (state->threshold < state->min_threshold) {
      state->threshold = state->min_threshold;
    }
  }
  
  return result;
}

function b32 peak_picker_threshold_decay(PeakPickerState_ThresholdDecay *state, f32 latest_value) {
  b32 result = false;
  if (latest_value > state->threshold) {
    result = true;
    state->threshold = latest_value;
  } else {
    state->threshold *= state->decay_factor;
  }
  return result;
}

function b32 
peak_picker_process(PeakPicker *peak_picker, f32 latest_value, u32 frame_index, u32 sample_rate, f32 *intensity_out) {
  b32 result = false;
  switch(peak_picker->type) {
    case PeakPickerType_LocalMaxima: {
      result = peak_picker_local_maxima((PeakPickerState_LocalMaxima*)peak_picker->state, latest_value);
    } break;
    case PeakPickerType_ThresholdDecayDelay: {
      PeakPickerState_ThresholdDecayDelay *state = (PeakPickerState_ThresholdDecayDelay*)peak_picker->state;
      f32 original_threshold = state->min_threshold;
      result = peak_picker_threshold_decay_delay(state, latest_value, frame_index, sample_rate);
      if (result) {
        f32 threshold_to_max = state->max_value - original_threshold;
        f32 above_threshold = latest_value - original_threshold;
        *intensity_out = above_threshold / threshold_to_max;
        printf("above_threshold: %f, threshold_to_max: %f, intensity: %f\n", above_threshold, threshold_to_max, *intensity_out);
      }
    } break;
    case PeakPickerType_ThresholdDecay: {
      result = peak_picker_threshold_decay((PeakPickerState_ThresholdDecay*)peak_picker->state, latest_value);
    } break;
    case PeakPickerType_Threshold: {
      result = peak_picker_threshold((PeakPickerState_Threshold*)peak_picker->state, latest_value);
    } break;
    default: {
      Assert(false);
    } break;
  }

  return result;
}

function f32
beat_detector_process_audio(M_Arena *arena, BeatDetector *detector, f32 *audio_buffer, u32 samples_read, u32 running_buffer_index) {
  f32 result = 0.f;

  M_Scratch scratch(arena);
  u32 sample_rate = detector->sample_rate;
  u32 frame_size = detector->frame_size;
  u32 bandwidth_hz = sample_rate / 2;
  f32 duration_ms = (f32)frame_size / (f32)sample_rate * 1000.0f;
  f32 resolution_hz = (f32)sample_rate / (f32)frame_size;

  u32 number_of_bands = (u32)(bandwidth_hz / resolution_hz);
  f32 *bands = push_array(scratch, f32, number_of_bands);
  for (u32 i = 0; i < number_of_bands; i++) {
    bands[i] = i * resolution_hz;
  }

  u32 new_samples = running_buffer_index - detector->last_frame_index;

  //printf("new_samples: %d, hop_size: %d, frame_size: %d, running_buffer_index: %d, last_frame_index: %d\n",
  //       new_samples, detector->hop_size, frame_size, running_buffer_index, detector->last_frame_index);
  while (new_samples >= detector->hop_size) {
    f32 *windowed_frame = push_array(scratch, f32, frame_size);
    for (u32 i = 0; i < frame_size; i++) {
      u32 window_index = (detector->last_frame_index + i) % frame_size;
      windowed_frame[i] = audio_buffer[window_index] * hamming_window(i, frame_size);
    }

    complex32 *xin = push_array(scratch, complex32, frame_size);

    for (u32 i = 0;
        i < frame_size;
        i++) {
      xin[i].re =  windowed_frame[i];
      xin[i].im = 0.0f;
    }

    fft_simple(arena, xin, detector->next_frame, frame_size);

    u32 max_bin = frame_size / 2;
    if (detector->filter_frequency != 0.0f) {
      max_bin = (u32)(detector->filter_frequency / (detector->sample_rate / frame_size));
    }

    detector->last_frame_index += detector->hop_size;

    b32 is_beat = false;

    // The particular detection method is responsible for saving however many frames
    // it needs and leaving detector->next_frame pointing to memory where the next
    // frame can be filled
    f32 latest_value = 0.0f;
    switch (detector->odm) {
      case OnsetDetectionMethod_PhaseDeviation: {
        PhaseDeviationState *state = (PhaseDeviationState*)detector->odm_state;
        latest_value = onset_detection_phase_deviation(arena, state, &detector->next_frame, frame_size, max_bin);
      } break;
      case OnsetDetectionMethod_SpectralFlux: {
        SpectralFluxState *state = (SpectralFluxState*)detector->odm_state;
        latest_value = onset_detection_spectral_flux(arena, state, &detector->next_frame, frame_size, max_bin);
      } break;
      default: {
        Assert(false);
      } break;
    }

    f32 intensity = 1.f;
    is_beat = peak_picker_process(detector->peak_picker, latest_value, detector->last_frame_index, detector->sample_rate, &intensity);

    if (is_beat) {
      // map from 0 to 1 with 0 being just at threshold and 1 being the max value
      result = intensity;
    }

    new_samples = running_buffer_index - detector->last_frame_index;
    //printf("...new_samples: %d, hop_size: %d, frame_size: %d, running_buffer_index: %d, last_frame_index: %d\n",
    //    new_samples, detector->hop_size, frame_size, running_buffer_index, detector->last_frame_index);
  }

  return result;
}

function void
peak_picker_init(M_Arena *arena, PeakPicker *peak_picker, PeakPickerType type) {
  peak_picker->type = type;
  switch (type) {
    case PeakPickerType_LocalMaxima: {
      PeakPickerState_LocalMaxima *state = push_array(arena, PeakPickerState_LocalMaxima, 1);
      state->threshold = 0.80f;
      state->ewma_decay_factor = 0.38f;
      state->multiplier = 3;
      state->window_size = 3;
      state->buffer_index = 0;
      state->buffer_size = (state->multiplier * state->window_size) + 1;
      state->buffer = push_array(arena, f32, state->buffer_size);
      state->ewma_mean = 0.f;
      state->ewma_variance = 1.f;
      peak_picker->state = (PeakPickerState*)state;
    } break;
    case PeakPickerType_ThresholdDecayDelay: {
      PeakPickerState_ThresholdDecayDelay *state = push_array(arena, PeakPickerState_ThresholdDecayDelay, 1);
      state->threshold = 150.f;
      state->min_threshold = 20.f;
      state->decay_factor = 0.992f;
      state->delay_ms = 300.f;
      state->last_peak_sample_index = 0;
      state->max_value = 0.f;
      peak_picker->state = (PeakPickerState*)state;
    } break;
    case PeakPickerType_ThresholdDecay: {
      PeakPickerState_ThresholdDecay *state = push_array(arena, PeakPickerState_ThresholdDecay, 1);
      state->threshold = 140.f;
      state->decay_factor = 0.99f;
      peak_picker->state = (PeakPickerState*)state;
    } break;
    case PeakPickerType_Threshold: {
      PeakPickerState_Threshold *state = push_array(arena, PeakPickerState_Threshold, 1);
      state->threshold = 140.f;
      peak_picker->state = (PeakPickerState*)state;
    } break;
    default: {
      Assert(false);
    } break;
  }
}

function void 
beat_detector_set_sensitivity(BeatDetector *detector, f32 sensitivity) {
  PeakPicker *peak_picker = detector->peak_picker;

  // sensitivity is in range 0.0 to 1.0
  // 0.0 is least sensitive, 1.0 is most sensitive for the
  // peak picker threshold / delay / decay method we have
  // 4 parameters we can change to adjust sensitivity
  // threshold, min_threshold, decay_factor, delay_ms
  // our default values are 150.f, 20.f, 0.992f, 300.f
  // we'll use a linear mapping from sensitivity to each parameter
  // so sensitivity 0.0 maps to the minimum value, and 1.0 maps to the maximum value
  // which for these parameters the minimum is the least sensitive and the maximum is the most sensitive

  Assert(peak_picker->type == PeakPickerType_ThresholdDecayDelay);
  PeakPickerState_ThresholdDecayDelay *state = (PeakPickerState_ThresholdDecayDelay*)peak_picker->state;
  state->threshold = lerp_f32(50.f, sensitivity, 300.f);
  state->min_threshold = lerp_f32(0.f, sensitivity, 200.f);
  state->decay_factor = lerp_f32(0.985f, sensitivity, 0.999f);
  state->delay_ms = lerp_f32(50.f, sensitivity, 750.f);


  // For the onset detector, we have 1 parameter we can change to adjust sensitivity
  // the filter frequency, default is 180Hz.
  // we'll use a linear mapping from sensitivity to the filter frequency
  // minimum sensitivity maps to our max frequency, and maximum sensitivity maps to our min frequency
  // assuming for 44100Hz sample rate and a frame size of 2048 our max frequency is 22050Hz
  // and our frequency resolution is 44100Hz / 2048 = 21.533Hz
  // we want max sensitivity to still allow very low frequencies through, so we'll set our min frequency to 45Hz
  // and our max we really don't care about the entire spectrum as we are always going to have most musical content
  // having fundamental frequencies below 1000Hz, so we'll set our max frequency to 1000Hz

  detector->filter_frequency = lerp_f32(1000.f, sensitivity, 45.f);
}

function BeatDetector*
beat_detector_init(M_Arena *arena, u32 sample_rate, OnsetDetectionMethod odm, PeakPickerType ppt) {
  BeatDetector *detector = push_array(arena, BeatDetector, 1);
  detector->sample_rate = sample_rate;
  detector->odm = odm;
  detector->frame_size = 2048;
  detector->hop_size = 1024;
  detector->filter_frequency = 0.f;
  PeakPicker *peak_picker = push_array(arena, PeakPicker, 1);
  peak_picker_init(arena, peak_picker, ppt);
  // Can set custom frame and hop sizes for the detection method that is used
  Assert(sample_rate == 44100);
  detector->frame_size = 2048; // 46ms;
  detector->hop_size = 441;    // 10ms (78.5% overlap)
  switch (odm) {
    case OnsetDetectionMethod_SpectralFlux: {
      SpectralFluxState *state = push_array(arena, SpectralFluxState, 1);
      state->last_frame = push_array(arena, complex32, detector->frame_size);
      state->second_last_frame = push_array(arena, complex32, detector->frame_size);
      detector->odm_state = state;
      detector->filter_frequency = 180.f;
    } break;
    case OnsetDetectionMethod_PhaseDeviation: {
      PhaseDeviationState *state = push_array(arena, PhaseDeviationState, 1);
      state->last_frame = push_array(arena, complex32, detector->frame_size);
      state->second_last_frame = push_array(arena, complex32, detector->frame_size);
      state->third_last_frame = push_array(arena, complex32, detector->frame_size);
      detector->odm_state = state;
      detector->filter_frequency = 95.f;
    } break;
  }

  detector->peak_picker = peak_picker;

  detector->next_frame = push_array(arena, complex32, detector->frame_size);
  detector->last_frame_index = 0;
  return detector;
}
