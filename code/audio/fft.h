#ifndef FFT_H
#define FFT_H

// https://literateprograms.org/cooley-tukey_fft_algorithm__c_.html

typedef struct complex32_t {
  f32 re;
  f32 im;
} complex32;

function complex32 complex_from_polar(f32 r, f32 theta_radians);
function f32       complex_magnitude(complex32 c);
function complex32 complex_add(complex32 left, complex32 right);
function complex32 complex_sub(complex32 left, complex32 right);
function complex32 complex_mult(complex32 left, complex32 right);
function f32       complex_phase(complex32 c);

function void fft_simple(M_Arena *arena, complex32 *x, complex32 *xout, u32 n);
function complex32* fft_get_twiddle_factors(M_Arena *arena, u32 n);
function void fft_calculate(complex32 *x, u32 n, complex32 *X, complex32 *scratch, complex32* twiddles);
function f32* fft_energy_for_bands(M_Arena *arena, f32 *input, u32 len, f32 *frequency_bands, u32 number_of_bands, u32 sample_rate);

#endif FFT_H
