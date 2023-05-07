#include "fft.h"
#include <math.h>

// https://literateprograms.org/cooley-tukey_fft_algorithm__c_.html

function complex32 
complex_from_polar(f32 r, f32 theta_radians) {
  complex32 result;
  result.re = r * cosf(theta_radians);
  result.im = r * sinf(theta_radians);
  return result;
}

function f32       
complex_magnitude(complex32 c) {
  return sqrtf(c.re*c.re + c.im*c.im);
}

function complex32 
complex_add(complex32 left, complex32 right) {
  complex32 result;
  result.re = left.re + right.re;
  result.im = left.im + right.im;
  return result;
}

function complex32 
complex_sub(complex32 left, complex32 right) {
  complex32 result;
  result.re = left.re - right.re;
  result.im = left.im - right.im;
  return result;
}

function complex32
complex_mult(complex32 left, complex32 right) {
  complex32 result;
  result.re = left.re*right.re - left.im*right.im;
  result.im = left.re*right.im + left.im*right.re;
  return result;
}

function f32 
complex_phase(complex32 c) {
  f32 result = atan2f(c.im, c.re);
  return result;
}

function void
fft_calculate(complex32 *x, u32 N, complex32 *X, complex32 *scratch, complex32* twiddles) {
  bool even_iteration = N & 0x55555555;

  if (N == 1) {
    X[0] = x[0];
    return;
  }

  complex32 *E = x;

  for (u32 n = 1;
       n < N;
       n *= 2) {
    complex32 *Xstart = even_iteration? scratch : X;
    u32 skip = N/(2 * n);
    /* each of D and E is of length n, and each element of each D and E is
     * separated by 2*skip. The Es begin at E[0] to E[skip - 1] and the Ds
     * begin at E[skip] to E[2*skip -1] */
    complex32 *Xp = Xstart;
    complex32 *Xp2 = Xstart + N/2;
    for (u32 k = 0;
         k != n;
         k++) {
      f32 tim = twiddles[k * skip].im;
      f32 tre = twiddles[k * skip].re;
      for (u32 m = 0;
           m != skip;
           ++m) {
        complex32* D = E + skip;
        /* twiddle *D to get dre and dim */
        f32 dre = D->re * tre - D->im * tim;
        f32 dim = D->re * tim + D->im * tre;
        Xp->re = E->re + dre;
        Xp->im = E->im + dim;
        Xp2->re = E->re - dre;
        Xp2->im = E->im - dim;
        ++Xp;
        ++Xp2;
        ++E;
      }
      E += skip;
    }
    E = Xstart;
    even_iteration = !even_iteration;
  }
}

function complex32*
fft_get_twiddle_factors(M_Arena *arena, u32 n) {
  complex32 *twiddles = push_array(arena, complex32, n);
  for (u32 k = 0;
       k != n;
       ++k) {
    twiddles[k] = complex_from_polar(1.0f, -2.0f*pi_f32*k/n);
  }
  return twiddles;
}

function void
fft_simple(M_Arena *arena, complex32 *x, complex32 *out, u32 n) {
  M_Temp restore_point = m_begin_temp(arena);
  complex32 *scratch = push_array(arena, complex32, n);
  complex32 *twiddles = fft_get_twiddle_factors(arena, n);

  fft_calculate(x, n, out, scratch,twiddles);

  m_end_temp(restore_point);
}

function f32*
fft_energy_for_bands(M_Arena *arena, f32 *input, u32 len, f32 *frequency_bands, u32 number_of_bands, u32 sample_rate) {
  f32 *results = push_array(arena, f32, number_of_bands);

  M_Temp restore_point = m_begin_temp(arena);
  complex32 *xin = push_array(arena, complex32, len);

  for (u32 i = 0;
       i < len;
       i++) {
    xin[i].re = input[i];
    xin[i].im = 0.0f;
  }

  complex32 *xout = push_array(arena, complex32, len);
  fft_simple(arena, xin, xout, len);

  for (u32 i = 0;
       i < number_of_bands;
       i++) {
    u32 band = (u32) floor(frequency_bands[i] * len / sample_rate + 0.5f);

    f32 magnitude = complex_magnitude(xout[band]);

    f32 scaled_magnitude = magnitude / len;

    results[i] = scaled_magnitude * scaled_magnitude;
  }

  m_end_temp(restore_point);

  return results;
}

