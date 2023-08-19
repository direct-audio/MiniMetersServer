/* Copyright (C) 2007-2008 Jean-Marc Valin
   Copyright (C) 2008      Thorvald Natvig

   File: resample.c
   Arbitrary resampling code

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

   NOTE: The resampler was packed into a class and the integer mode removed
         as well as the function pointer system to call the correct resampling
         function. Original sourcefiles are arch.h, resample.c and speex_resampler.h
         from the speexdsp library.
         https://github.com/xiph/speexdsp
*/

#pragma once

#include <cmath>

namespace speexport {
  enum {
    RESAMPLER_ERR_SUCCESS = 0,
    RESAMPLER_ERR_ALLOC_FAILED = 1,
    RESAMPLER_ERR_BAD_STATE = 2,
    RESAMPLER_ERR_INVALID_ARG = 3,
    RESAMPLER_ERR_PTR_OVERLAP = 4,
    RESAMPLER_ERR_OVERFLOW = 5,

    RESAMPLER_ERR_MAX_ERROR
  };

  enum RESAMPLER_FUN {
    NONE,
    ZERO,
    DIRECT_SINGLE,
    DIRECT_DOUBLE,
    INTERP_DOUBLE,
    INTERP_SINGLE,
  };

  typedef float spx_mem_t;
  typedef float spx_coef_t;
  typedef float spx_lsp_t;
  typedef float spx_sig_t;
  typedef float spx_word16_t;
  typedef float spx_word32_t;
  typedef short spx_int16_t;
  typedef int spx_int32_t;
  typedef unsigned short spx_uint16_t;
  typedef unsigned int spx_uint32_t;

  static void* speex_alloc(int size) { return calloc(size, 1); }
  static void* speex_realloc(void* ptr, int size) { return realloc(ptr, size); }
  static void speex_free(void* ptr) { free(ptr); }

  /**
   * constexpr buther performance in debug mode for some reason
   * maybe the casts are wrong
   */
  #define MULT16_16(a,b)     ((spx_word32_t)(a)*(spx_word32_t)(b))
  //constexpr spx_word32_t MULT16_16(spx_word32_t a, spx_word32_t b) {
  //  return a * b;
  //}

  #define SATURATE32PSHR(x,shift,a) (x)
  //constexpr spx_word32_t SATURATE32PSHR(spx_word32_t x, spx_word32_t, spx_word32_t) {
  //  return x;
  //}

  #define PSHR32(a,shift) (a)
  //constexpr double PSHR32(double a, double shift) {
  //  return a;
  //}

  #define MULT16_32_Q15(a,b)     ((a)*(b))
  //constexpr spx_word16_t MULT16_32_Q15(spx_word16_t a, spx_word16_t b) {
  //  return a * b;
  //}

  #define SHR32(a,shift) (a)
  //constexpr spx_word32_t SHR32(spx_word32_t a, spx_word32_t) {
  //  return a;
  //}

  #ifndef M_PI
    #define M_PI 3.14159265358979323846
  #endif

  static const double kaiser12_table[68] = {
    0.99859849, 1.00000000, 0.99859849, 0.99440475, 0.98745105, 0.97779076,
    0.96549770, 0.95066529, 0.93340547, 0.91384741, 0.89213598, 0.86843014,
    0.84290116, 0.81573067, 0.78710866, 0.75723148, 0.72629970, 0.69451601,
    0.66208321, 0.62920216, 0.59606986, 0.56287762, 0.52980938, 0.49704014,
    0.46473455, 0.43304576, 0.40211431, 0.37206735, 0.34301800, 0.31506490,
    0.28829195, 0.26276832, 0.23854851, 0.21567274, 0.19416736, 0.17404546,
    0.15530766, 0.13794294, 0.12192957, 0.10723616, 0.09382272, 0.08164178,
    0.07063950, 0.06075685, 0.05193064, 0.04409466, 0.03718069, 0.03111947,
    0.02584161, 0.02127838, 0.01736250, 0.01402878, 0.01121463, 0.00886058,
    0.00691064, 0.00531256, 0.00401805, 0.00298291, 0.00216702, 0.00153438,
    0.00105297, 0.00069463, 0.00043489, 0.00025272, 0.00013031, 0.0000527734,
    0.00001000, 0.00000000
  };

  static const double kaiser10_table[36] = {
    0.99537781, 1.00000000, 0.99537781, 0.98162644, 0.95908712, 0.92831446,
    0.89005583, 0.84522401, 0.79486424, 0.74011713, 0.68217934, 0.62226347,
    0.56155915, 0.50119680, 0.44221549, 0.38553619, 0.33194107, 0.28205962,
    0.23636152, 0.19515633, 0.15859932, 0.12670280, 0.09935205, 0.07632451,
    0.05731132, 0.04193980, 0.02979584, 0.02044510, 0.01345224, 0.00839739,
    0.00488951, 0.00257636, 0.00115101, 0.00035515, 0.00000000, 0.00000000
  };

  static const double kaiser8_table[36] = {
    0.99635258, 1.00000000, 0.99635258, 0.98548012, 0.96759014, 0.94302200,
    0.91223751, 0.87580811, 0.83439927, 0.78875245, 0.73966538, 0.68797126,
    0.63451750, 0.58014482, 0.52566725, 0.47185369, 0.41941150, 0.36897272,
    0.32108304, 0.27619388, 0.23465776, 0.19672670, 0.16255380, 0.13219758,
    0.10562887, 0.08273982, 0.06335451, 0.04724088, 0.03412321, 0.02369490,
    0.01563093, 0.00959968, 0.00527363, 0.00233883, 0.00050000, 0.00000000
  };

  static const double kaiser6_table[36] = {
    0.99733006, 1.00000000, 0.99733006, 0.98935595, 0.97618418, 0.95799003,
    0.93501423, 0.90755855, 0.87598009, 0.84068475, 0.80211977, 0.76076565,
    0.71712752, 0.67172623, 0.62508937, 0.57774224, 0.53019925, 0.48295561,
    0.43647969, 0.39120616, 0.34752997, 0.30580127, 0.26632152, 0.22934058,
    0.19505503, 0.16360756, 0.13508755, 0.10953262, 0.08693120, 0.06722600,
    0.05031820, 0.03607231, 0.02432151, 0.01487334, 0.00752000, 0.00000000
  };

  struct FuncDef {
    const double* table;
    int oversample;
  };

  static const struct FuncDef kaiser12_funcdef = { kaiser12_table, 64 };
  #define KAISER12 (&kaiser12_funcdef)
  static const struct FuncDef kaiser10_funcdef = { kaiser10_table, 32 };
  #define KAISER10 (&kaiser10_funcdef)
  static const struct FuncDef kaiser8_funcdef = { kaiser8_table, 32 };
  #define KAISER8 (&kaiser8_funcdef)
  static const struct FuncDef kaiser6_funcdef = { kaiser6_table, 32 };
  #define KAISER6 (&kaiser6_funcdef)

  struct QualityMapping {
    int base_length;
    int oversample;
    float downsample_bandwidth;
    float upsample_bandwidth;
    const struct FuncDef* window_func;
  };

  static const struct QualityMapping quality_map[11] = {
    {  8,  4, 0.830f, 0.860f, KAISER6 }, /* Q0 */
    { 16,  4, 0.850f, 0.880f, KAISER6 }, /* Q1 */
    { 32,  4, 0.882f, 0.910f, KAISER6 }, /* Q2 */  /* 82.3% cutoff ( ~60 dB stop) 6  */
    { 48,  8, 0.895f, 0.917f, KAISER8 }, /* Q3 */  /* 84.9% cutoff ( ~80 dB stop) 8  */
    { 64,  8, 0.921f, 0.940f, KAISER8 }, /* Q4 */  /* 88.7% cutoff ( ~80 dB stop) 8  */
    { 80, 16, 0.922f, 0.940f, KAISER10}, /* Q5 */  /* 89.1% cutoff (~100 dB stop) 10 */
    { 96, 16, 0.940f, 0.945f, KAISER10}, /* Q6 */  /* 91.5% cutoff (~100 dB stop) 10 */
    {128, 16, 0.950f, 0.950f, KAISER10}, /* Q7 */  /* 93.1% cutoff (~100 dB stop) 10 */
    {160, 16, 0.960f, 0.960f, KAISER10}, /* Q8 */  /* 94.5% cutoff (~100 dB stop) 10 */
    {192, 32, 0.968f, 0.968f, KAISER12}, /* Q9 */  /* 95.5% cutoff (~100 dB stop) 10 */
    {256, 32, 0.975f, 0.975f, KAISER12}, /* Q10 */ /* 96.6% cutoff (~100 dB stop) 10 */
  };

  /*8,24,40,56,80,104,128,160,200,256,320*/
  static double compute_func(float x, const struct FuncDef* func) {
    float y, frac;
    double interp[4];
    int ind;
    y = x * func->oversample;
    ind = (int)floor(y);
    frac = (y - ind);
    /* CSE with handle the repeated powers */
    interp[3] = -0.1666666667 * frac + 0.1666666667 * (frac * frac * frac);
    interp[2] = frac + 0.5 * (frac * frac) - 0.5 * (frac * frac * frac);
    /*interp[2] = 1.f - 0.5f*frac - frac*frac + 0.5f*frac*frac*frac;*/
    interp[0] = -0.3333333333 * frac + 0.5 * (frac * frac) - 0.1666666667 * (frac * frac * frac);
    /* Just to make sure we don't have rounding problems */
    interp[1] = 1.f - interp[3] - interp[2] - interp[0];

    /*sum = frac*accum[1] + (1-frac)*accum[2];*/
    return interp[0] * func->table[ind] + interp[1] * func->table[ind + 1] + interp[2] * func->table[ind + 2] + interp[3] * func->table[ind + 3];
  }

  /* The slow way of computing a sinc for the table. Should improve that some day */
  static spx_word16_t sinc(float cutoff, float x, int N, const struct FuncDef* window_func) {
    /*fprintf (stderr, "%f ", x);*/
    float xx = x * cutoff;
    if (fabs(x) < 1e-6)
      return cutoff;
    else if (fabs(x) > .5 * N)
      return 0;
    /*FIXME: Can it really be any slower than this? */
    return cutoff * sin(M_PI * xx) / (M_PI * xx) * compute_func(fabs(2. * x / N), window_func);
  }

  static void cubic_coef(spx_word16_t frac, spx_word16_t interp[4]) {
    /* Compute interpolation coefficients. I'm not sure whether this corresponds to cubic interpolation
    but I know it's MMSE-optimal on a sinc */
    interp[0] = -0.16667f * frac + 0.16667f * frac * frac * frac;
    interp[1] = frac + 0.5f * frac * frac - 0.5f * frac * frac * frac;
    /*interp[2] = 1.f - 0.5f*frac - frac*frac + 0.5f*frac*frac*frac;*/
    interp[3] = -0.33333f * frac + 0.5f * frac * frac - 0.16667f * frac * frac * frac;
    /* Just to make sure we don't have rounding problems */
    interp[2] = 1. - interp[0] - interp[1] - interp[3];
  }

  static int multiply_frac(spx_uint32_t* result, spx_uint32_t value, spx_uint32_t num, spx_uint32_t den) {
    spx_uint32_t major = value / den;
    spx_uint32_t remain = value % den;
    /* TODO: Could use 64 bits operation to check for overflow. But only guaranteed in C99+ */
    if (remain > 4294967295U / num || major > 4294967295U / num
      || major * num > 4294967295U - remain * num / den)
      return RESAMPLER_ERR_OVERFLOW;
    *result = remain * num / den + major * num;
    return RESAMPLER_ERR_SUCCESS;
  }

  static spx_uint32_t compute_gcd(spx_uint32_t a, spx_uint32_t b) {
    while (b != 0)
    {
      spx_uint32_t temp = a;

      a = b;
      b = temp % b;
    }
    return a;
  }
//
//  const char* speex_resampler_strerror(int err) {
//    switch (err)
//    {
//    case RESAMPLER_ERR_SUCCESS:
//      return "Success.";
//    case RESAMPLER_ERR_ALLOC_FAILED:
//      return "Memory allocation failed.";
//    case RESAMPLER_ERR_BAD_STATE:
//      return "Bad resampler state.";
//    case RESAMPLER_ERR_INVALID_ARG:
//      return "Invalid argument.";
//    case RESAMPLER_ERR_PTR_OVERLAP:
//      return "Input and output buffers overlap.";
//    default:
//      return "Unknown error. Bad error code or strange version mismatch.";
//    }
//  }

  class SpeexResampler {
    RESAMPLER_FUN active_resampler = NONE;
    spx_uint32_t in_rate;
    spx_uint32_t out_rate;
    spx_uint32_t num_rate;
    spx_uint32_t den_rate;

    int    quality;
    spx_uint32_t nb_channels;
    spx_uint32_t filt_len;
    spx_uint32_t mem_alloc_size;
    spx_uint32_t buffer_size;
    int          int_advance;
    int          frac_advance;
    float  cutoff;
    spx_uint32_t oversample;
    int          initialised;
    int          started;

    /* These are per-channel */
    spx_int32_t* last_sample = nullptr;
    spx_uint32_t* samp_frac_num = nullptr;
    spx_uint32_t* magic_samples = nullptr;

    spx_word16_t* mem = nullptr;
    spx_word16_t* sinc_table = nullptr;
    spx_uint32_t sinc_table_length;
    int    in_stride;
    int    out_stride;

    int direct_single(spx_uint32_t channel_index, const spx_word16_t* in, spx_uint32_t* in_len, spx_word16_t* out, spx_uint32_t* out_len) {
      SpeexResampler* st = this;
      const int N = st->filt_len;
      int out_sample = 0;
      int last_sample = st->last_sample[channel_index];
      spx_uint32_t samp_frac_num = st->samp_frac_num[channel_index];
      const spx_word16_t* sinc_table = st->sinc_table;
      const int out_stride = st->out_stride;
      const int int_advance = st->int_advance;
      const int frac_advance = st->frac_advance;
      const spx_uint32_t den_rate = st->den_rate;
      spx_word32_t sum;

      while (!(last_sample >= (spx_int32_t)*in_len || out_sample >= (spx_int32_t)*out_len))
      {
        const spx_word16_t* sinct = &sinc_table[samp_frac_num * N];
        const spx_word16_t* iptr = &in[last_sample];

#ifndef OVERRIDE_INNER_PRODUCT_SINGLE
        int j;
        sum = 0;
        for (j = 0; j < N; j++) sum += MULT16_16(sinct[j], iptr[j]);

        sum = SATURATE32PSHR(sum, 15, 32767);
#else
        sum = inner_product_single(sinct, iptr, N);
#endif

        out[out_stride * out_sample++] = sum;
        last_sample += int_advance;
        samp_frac_num += frac_advance;
        if (samp_frac_num >= den_rate)
        {
          samp_frac_num -= den_rate;
          last_sample++;
        }
      }

      st->last_sample[channel_index] = last_sample;
      st->samp_frac_num[channel_index] = samp_frac_num;
      return out_sample;
    }

    /* This is the same as the previous function, except with a double-precision accumulator */
    int direct_double(spx_uint32_t channel_index, const spx_word16_t* in, spx_uint32_t* in_len, spx_word16_t* out, spx_uint32_t* out_len) {
      SpeexResampler* st = this;
      const int N = st->filt_len;
      int out_sample = 0;
      int last_sample = st->last_sample[channel_index];
      spx_uint32_t samp_frac_num = st->samp_frac_num[channel_index];
      const spx_word16_t* sinc_table = st->sinc_table;
      const int out_stride = st->out_stride;
      const int int_advance = st->int_advance;
      const int frac_advance = st->frac_advance;
      const spx_uint32_t den_rate = st->den_rate;
      double sum;

      while (!(last_sample >= (spx_int32_t)*in_len || out_sample >= (spx_int32_t)*out_len))
      {
        const spx_word16_t* sinct = &sinc_table[samp_frac_num * N];
        const spx_word16_t* iptr = &in[last_sample];

#ifndef OVERRIDE_INNER_PRODUCT_DOUBLE
        int j;
        double accum[4] = { 0,0,0,0 };

        for (j = 0; j < N; j += 4) {
          accum[0] += sinct[j] * iptr[j];
          accum[1] += sinct[j + 1] * iptr[j + 1];
          accum[2] += sinct[j + 2] * iptr[j + 2];
          accum[3] += sinct[j + 3] * iptr[j + 3];
        }
        sum = accum[0] + accum[1] + accum[2] + accum[3];
#else
        sum = inner_product_double(sinct, iptr, N);
#endif

        out[out_stride * out_sample++] = PSHR32(sum, 15);
        last_sample += int_advance;
        samp_frac_num += frac_advance;
        if (samp_frac_num >= den_rate)
        {
          samp_frac_num -= den_rate;
          last_sample++;
        }
      }

      st->last_sample[channel_index] = last_sample;
      st->samp_frac_num[channel_index] = samp_frac_num;
      return out_sample;
    }

    int interpolate_single(spx_uint32_t channel_index, const spx_word16_t* in, spx_uint32_t* in_len, spx_word16_t* out, spx_uint32_t* out_len) {
      SpeexResampler* st = this;
      const int N = st->filt_len;
      int out_sample = 0;
      int last_sample = st->last_sample[channel_index];
      spx_uint32_t samp_frac_num = st->samp_frac_num[channel_index];
      const int out_stride = st->out_stride;
      const int int_advance = st->int_advance;
      const int frac_advance = st->frac_advance;
      const spx_uint32_t den_rate = st->den_rate;
      spx_word32_t sum;

      while (!(last_sample >= (spx_int32_t)*in_len || out_sample >= (spx_int32_t)*out_len))
      {
        const spx_word16_t* iptr = &in[last_sample];

        const int offset = samp_frac_num * st->oversample / st->den_rate;
        const spx_word16_t frac = ((float)((samp_frac_num * st->oversample) % st->den_rate)) / st->den_rate;
        spx_word16_t interp[4];


#ifndef OVERRIDE_INTERPOLATE_PRODUCT_SINGLE
        int j;
        spx_word32_t accum[4] = { 0,0,0,0 };

        for (j = 0; j < N; j++) {
          const spx_word16_t curr_in = iptr[j];
          accum[0] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset - 2]);
          accum[1] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset - 1]);
          accum[2] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset]);
          accum[3] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset + 1]);
        }

        cubic_coef(frac, interp);
        sum = MULT16_32_Q15(interp[0], SHR32(accum[0], 1)) + MULT16_32_Q15(interp[1], SHR32(accum[1], 1)) + MULT16_32_Q15(interp[2], SHR32(accum[2], 1)) + MULT16_32_Q15(interp[3], SHR32(accum[3], 1));
        sum = SATURATE32PSHR(sum, 15, 32767);
#else
        cubic_coef(frac, interp);
        sum = interpolate_product_single(iptr, st->sinc_table + st->oversample + 4 - offset - 2, N, st->oversample, interp);
#endif

        out[out_stride * out_sample++] = sum;
        last_sample += int_advance;
        samp_frac_num += frac_advance;
        if (samp_frac_num >= den_rate)
        {
          samp_frac_num -= den_rate;
          last_sample++;
        }
      }

      st->last_sample[channel_index] = last_sample;
      st->samp_frac_num[channel_index] = samp_frac_num;
      return out_sample;
    }

    int interpolate_double(spx_uint32_t channel_index, const spx_word16_t* in, spx_uint32_t* in_len, spx_word16_t* out, spx_uint32_t* out_len) {
      SpeexResampler* st = this;
      const int N = st->filt_len;
      int out_sample = 0;
      int last_sample = st->last_sample[channel_index];
      spx_uint32_t samp_frac_num = st->samp_frac_num[channel_index];
      const int out_stride = st->out_stride;
      const int int_advance = st->int_advance;
      const int frac_advance = st->frac_advance;
      const spx_uint32_t den_rate = st->den_rate;
      spx_word32_t sum;

      while (!(last_sample >= (spx_int32_t)*in_len || out_sample >= (spx_int32_t)*out_len))
      {
        const spx_word16_t* iptr = &in[last_sample];

        const int offset = samp_frac_num * st->oversample / st->den_rate;
        const spx_word16_t frac = ((float)((samp_frac_num * st->oversample) % st->den_rate)) / st->den_rate;
        spx_word16_t interp[4];


#ifndef OVERRIDE_INTERPOLATE_PRODUCT_DOUBLE
        int j;
        double accum[4] = { 0,0,0,0 };

        for (j = 0; j < N; j++) {
          const double curr_in = iptr[j];
          accum[0] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset - 2]);
          accum[1] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset - 1]);
          accum[2] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset]);
          accum[3] += MULT16_16(curr_in, st->sinc_table[4 + (j + 1) * st->oversample - offset + 1]);
        }

        cubic_coef(frac, interp);
        sum = MULT16_32_Q15(interp[0], accum[0]) + MULT16_32_Q15(interp[1], accum[1]) + MULT16_32_Q15(interp[2], accum[2]) + MULT16_32_Q15(interp[3], accum[3]);
#else
        cubic_coef(frac, interp);
        sum = interpolate_product_double(iptr, st->sinc_table + st->oversample + 4 - offset - 2, N, st->oversample, interp);
#endif

        out[out_stride * out_sample++] = PSHR32(sum, 15);
        last_sample += int_advance;
        samp_frac_num += frac_advance;
        if (samp_frac_num >= den_rate)
        {
          samp_frac_num -= den_rate;
          last_sample++;
        }
      }

      st->last_sample[channel_index] = last_sample;
      st->samp_frac_num[channel_index] = samp_frac_num;
      return out_sample;
    }

    /* This resampler is used to produce zero output in situations where memory
       for the filter could not be allocated.  The expected numbers of input and
       output samples are still processed so that callers failing to check error
       codes are not surprised, possibly getting into infinite loops. */
    int basic_zero(spx_uint32_t channel_index, const spx_word16_t* in, spx_uint32_t* in_len, spx_word16_t* out, spx_uint32_t* out_len) {
      SpeexResampler* st = this;
      int out_sample = 0;
      int last_sample = st->last_sample[channel_index];
      spx_uint32_t samp_frac_num = st->samp_frac_num[channel_index];
      const int out_stride = st->out_stride;
      const int int_advance = st->int_advance;
      const int frac_advance = st->frac_advance;
      const spx_uint32_t den_rate = st->den_rate;

      (void)in;
      while (!(last_sample >= (spx_int32_t)*in_len || out_sample >= (spx_int32_t)*out_len))
      {
        out[out_stride * out_sample++] = 0;
        last_sample += int_advance;
        samp_frac_num += frac_advance;
        if (samp_frac_num >= den_rate)
        {
          samp_frac_num -= den_rate;
          last_sample++;
        }
      }

      st->last_sample[channel_index] = last_sample;
      st->samp_frac_num[channel_index] = samp_frac_num;
      return out_sample;
    }

    int update_filter() {
      SpeexResampler* st = this;
      spx_uint32_t old_length = st->filt_len;
      spx_uint32_t old_alloc_size = st->mem_alloc_size;
      int use_direct;
      spx_uint32_t min_sinc_table_length;
      spx_uint32_t min_alloc_size;

      st->int_advance = st->num_rate / st->den_rate;
      st->frac_advance = st->num_rate % st->den_rate;
      st->oversample = quality_map[st->quality].oversample;
      st->filt_len = quality_map[st->quality].base_length;

      if (st->num_rate > st->den_rate)
      {
        /* down-sampling */
        st->cutoff = quality_map[st->quality].downsample_bandwidth * st->den_rate / st->num_rate;
        if (multiply_frac(&st->filt_len, st->filt_len, st->num_rate, st->den_rate) != RESAMPLER_ERR_SUCCESS)
          goto fail;
        /* Round up to make sure we have a multiple of 8 for SSE */
        st->filt_len = ((st->filt_len - 1) & (~0x7)) + 8;
        if (2 * st->den_rate < st->num_rate)
          st->oversample >>= 1;
        if (4 * st->den_rate < st->num_rate)
          st->oversample >>= 1;
        if (8 * st->den_rate < st->num_rate)
          st->oversample >>= 1;
        if (16 * st->den_rate < st->num_rate)
          st->oversample >>= 1;
        if (st->oversample < 1)
          st->oversample = 1;
      }
      else {
        /* up-sampling */
        st->cutoff = quality_map[st->quality].upsample_bandwidth;
      }

#ifdef RESAMPLE_FULL_SINC_TABLE
      use_direct = 1;
      if (INT_MAX / sizeof(spx_word16_t) / st->den_rate < st->filt_len)
        goto fail;
#else
      /* Choose the resampling type that requires the least amount of memory */
      use_direct = st->filt_len * st->den_rate <= st->filt_len * st->oversample + 8
        && INT_MAX / sizeof(spx_word16_t) / st->den_rate >= st->filt_len;
#endif
      if (use_direct)
      {
        min_sinc_table_length = st->filt_len * st->den_rate;
      }
      else {
        if ((INT_MAX / sizeof(spx_word16_t) - 8) / st->oversample < st->filt_len)
          goto fail;

        min_sinc_table_length = st->filt_len * st->oversample + 8;
      }
      if (st->sinc_table_length < min_sinc_table_length)
      {
        spx_word16_t* sinc_table = (spx_word16_t*)speex_realloc(st->sinc_table, min_sinc_table_length * sizeof(spx_word16_t));
        if (!sinc_table)
          goto fail;

        st->sinc_table = sinc_table;
        st->sinc_table_length = min_sinc_table_length;
      }
      if (use_direct)
      {
        spx_uint32_t i;
        for (i = 0; i < st->den_rate; i++)
        {
          spx_int32_t j;
          for (j = 0; j < st->filt_len; j++)
          {
            st->sinc_table[i * st->filt_len + j] = sinc(st->cutoff, ((j - (spx_int32_t)st->filt_len / 2 + 1) - ((float)i) / st->den_rate), st->filt_len, quality_map[st->quality].window_func);
          }
        }
        if (st->quality > 8) {
          active_resampler = DIRECT_DOUBLE;
          
        } else {
          active_resampler = DIRECT_SINGLE;
        }
        /*fprintf (stderr, "resampler uses direct sinc table and normalised cutoff %f\n", cutoff);*/
      }
      else {
        spx_int32_t i;
        for (i = -4; i < (spx_int32_t)(st->oversample * st->filt_len + 4); i++)
          st->sinc_table[i + 4] = sinc(st->cutoff, (i / (float)st->oversample - st->filt_len / 2), st->filt_len, quality_map[st->quality].window_func);

        if (st->quality > 8) {
          active_resampler = INTERP_DOUBLE;
        } else {
          active_resampler = INTERP_SINGLE;
        }
      }

      /* Here's the place where we update the filter memory to take into account
         the change in filter length. It's probably the messiest part of the code
         due to handling of lots of corner cases. */

         /* Adding buffer_size to filt_len won't overflow here because filt_len
            could be multiplied by sizeof(spx_word16_t) above. */
      min_alloc_size = st->filt_len - 1 + st->buffer_size;
      if (min_alloc_size > st->mem_alloc_size)
      {
        spx_word16_t* mem;
        if (INT_MAX / sizeof(spx_word16_t) / st->nb_channels < min_alloc_size)
          goto fail;
        else if (!(mem = (spx_word16_t*)speex_realloc(st->mem, st->nb_channels * min_alloc_size * sizeof(*mem))))
          goto fail;

        st->mem = mem;
        st->mem_alloc_size = min_alloc_size;
      }
      if (!st->started)
      {
        spx_uint32_t i;
        for (i = 0; i < st->nb_channels * st->mem_alloc_size; i++)
          st->mem[i] = 0;
        /*speex_warning("reinit filter");*/
      }
      else if (st->filt_len > old_length)
      {
        spx_uint32_t i;
        /* Increase the filter length */
        /*speex_warning("increase filter size");*/
        for (i = st->nb_channels; i--;)
        {
          spx_uint32_t j;
          spx_uint32_t olen = old_length;
          /*if (st->magic_samples[i])*/
          {
            /* Try and remove the magic samples as if nothing had happened */

            /* FIXME: This is wrong but for now we need it to avoid going over the array bounds */
            olen = old_length + 2 * st->magic_samples[i];
            for (j = old_length - 1 + st->magic_samples[i]; j--;)
              st->mem[i * st->mem_alloc_size + j + st->magic_samples[i]] = st->mem[i * old_alloc_size + j];
            for (j = 0; j < st->magic_samples[i]; j++)
              st->mem[i * st->mem_alloc_size + j] = 0;
            st->magic_samples[i] = 0;
          }
          if (st->filt_len > olen)
          {
            /* If the new filter length is still bigger than the "augmented" length */
            /* Copy data going backward */
            for (j = 0; j < olen - 1; j++)
              st->mem[i * st->mem_alloc_size + (st->filt_len - 2 - j)] = st->mem[i * st->mem_alloc_size + (olen - 2 - j)];
            /* Then put zeros for lack of anything better */
            for (; j < st->filt_len - 1; j++)
              st->mem[i * st->mem_alloc_size + (st->filt_len - 2 - j)] = 0;
            /* Adjust last_sample */
            st->last_sample[i] += (st->filt_len - olen) / 2;
          }
          else {
            /* Put back some of the magic! */
            st->magic_samples[i] = (olen - st->filt_len) / 2;
            for (j = 0; j < st->filt_len - 1 + st->magic_samples[i]; j++)
              st->mem[i * st->mem_alloc_size + j] = st->mem[i * st->mem_alloc_size + j + st->magic_samples[i]];
          }
        }
      }
      else if (st->filt_len < old_length)
      {
        spx_uint32_t i;
        /* Reduce filter length, this a bit tricky. We need to store some of the memory as "magic"
           samples so they can be used directly as input the next time(s) */
        for (i = 0; i < st->nb_channels; i++)
        {
          spx_uint32_t j;
          spx_uint32_t old_magic = st->magic_samples[i];
          st->magic_samples[i] = (old_length - st->filt_len) / 2;
          /* We must copy some of the memory that's no longer used */
          /* Copy data going backward */
          for (j = 0; j < st->filt_len - 1 + st->magic_samples[i] + old_magic; j++)
            st->mem[i * st->mem_alloc_size + j] = st->mem[i * st->mem_alloc_size + j + st->magic_samples[i]];
          st->magic_samples[i] += old_magic;
        }
      }
      return RESAMPLER_ERR_SUCCESS;

    fail:
      active_resampler = ZERO;
      /* st->mem may still contain consumed input samples for the filter.
         Restore filt_len so that filt_len - 1 still points to the position after
         the last of these samples. */
      st->filt_len = old_length;
      return RESAMPLER_ERR_ALLOC_FAILED;
    }

    int process_native(spx_uint32_t channel_index, spx_uint32_t* in_len, spx_word16_t* out, spx_uint32_t* out_len) {
      SpeexResampler* st = this;
      int j = 0;
      const int N = st->filt_len;
      int out_sample = 0;
      spx_word16_t* mem = st->mem + channel_index * st->mem_alloc_size;
      spx_uint32_t ilen;

      st->started = 1;

      /* Call the right resampler */
      switch (active_resampler) {
      case DIRECT_SINGLE:
        out_sample = direct_single(channel_index, mem, in_len, out, out_len);
        break;
      case DIRECT_DOUBLE:
        out_sample = direct_double(channel_index, mem, in_len, out, out_len);
        break;
      case INTERP_SINGLE:
        out_sample = interpolate_single(channel_index, mem, in_len, out, out_len);
        break;
      case INTERP_DOUBLE:
        out_sample = interpolate_double(channel_index, mem, in_len, out, out_len);
        break;
      default:
        out_sample = basic_zero(channel_index, mem, in_len, out, out_len);
      }

      if (st->last_sample[channel_index] < (spx_int32_t)*in_len)
        *in_len = st->last_sample[channel_index];
      *out_len = out_sample;
      st->last_sample[channel_index] -= *in_len;

      ilen = *in_len;

      for (j = 0; j < N - 1; ++j)
        mem[j] = mem[j + ilen];

      return RESAMPLER_ERR_SUCCESS;
    }

    int magic(spx_uint32_t channel_index, spx_word16_t** out, spx_uint32_t out_len) {
      SpeexResampler* st = this;
      spx_uint32_t tmp_in_len = st->magic_samples[channel_index];
      spx_word16_t* mem = st->mem + channel_index * st->mem_alloc_size;
      const int N = st->filt_len;

      process_native(channel_index, &tmp_in_len, *out, &out_len);

      st->magic_samples[channel_index] -= tmp_in_len;

      /* If we couldn't process all "magic" input samples, save the rest for next time */
      if (st->magic_samples[channel_index])
      {
        spx_uint32_t i;
        for (i = 0; i < st->magic_samples[channel_index]; i++)
          mem[N - 1 + i] = mem[N - 1 + i + tmp_in_len];
      }
      *out += out_len * st->out_stride;
      return out_len;
    }

  public:
    SpeexResampler() {
      SpeexResampler* st = this;
      st->initialised = 0;
      st->started = 0;
      st->in_rate = 0;
      st->out_rate = 0;
      st->num_rate = 0;
      st->den_rate = 0;
      st->quality = -1;
      st->sinc_table_length = 0;
      st->mem_alloc_size = 0;
      st->filt_len = 0;
      st->mem = nullptr;

      st->cutoff = 1.f;
      st->in_stride = 1;
      st->out_stride = 1;
      st->buffer_size = 160;
    }

    ~SpeexResampler() {
      speex_free(mem);
      speex_free(sinc_table);
      speex_free(last_sample);
      speex_free(magic_samples);
      speex_free(samp_frac_num);
    }

    /** Create a new resampler with integer input and output rates.
     * @param nb_channels Number of channels to be processed
     * @param rate_in Input sampling rate (integer number of Hz).
     * @param rate_out Output sampling rate (integer number of Hz).
     * @param quality Resampling quality between 0 and 10, where 0 has poor quality
     * and 10 has very high quality.
     * @return Newly created resampler state
     * @retval NULL Error: not enough memory
     */
    int init(spx_uint32_t nb_channels, spx_uint32_t rate_in, spx_uint32_t rate_out, int quality, int* err) {
      int filter_err;
      SpeexResampler* st = this;
      st->nb_channels = nb_channels;


      /* Per channel data */
      if (!(st->last_sample = (spx_int32_t*)speex_alloc(nb_channels * sizeof(spx_int32_t))))
        return -1;
      if (!(st->magic_samples = (spx_uint32_t*)speex_alloc(nb_channels * sizeof(spx_uint32_t))))
        return -1;
      if (!(st->samp_frac_num = (spx_uint32_t*)speex_alloc(nb_channels * sizeof(spx_uint32_t))))
        return -1;

      set_quality(quality);

      set_rate_frac(rate_in, rate_out, rate_in, rate_out);

      filter_err = update_filter();
      if (filter_err == RESAMPLER_ERR_SUCCESS)
      {
        st->initialised = 1;
      }
      return filter_err;
    }

    /** Resample a float array. The input and output buffers must *not* overlap.
     * @param st Resampler state
     * @param channel_index Index of the channel to process for the multi-channel
     * base (0 otherwise)
     * @param in Input buffer
     * @param in_len Number of input samples in the input buffer. Returns the
     * number of samples processed
     * @param out Output buffer
     * @param out_len Size of the output buffer. Returns the number of samples written
     */
    int process(spx_uint32_t channel_index, const float* in, spx_uint32_t* in_len, float* out, spx_uint32_t* out_len) {
      SpeexResampler* st = this;
      int j;
      spx_uint32_t ilen = *in_len;
      spx_uint32_t olen = *out_len;
      spx_word16_t* x = st->mem + channel_index * st->mem_alloc_size;
      const int filt_offs = st->filt_len - 1;
      const spx_uint32_t xlen = st->mem_alloc_size - filt_offs;
      const int istride = st->in_stride;

      if (st->magic_samples[channel_index])
        olen -= magic(channel_index, &out, olen);
      if (!st->magic_samples[channel_index]) {
        while (ilen && olen) {
          spx_uint32_t ichunk = (ilen > xlen) ? xlen : ilen;
          spx_uint32_t ochunk = olen;

          if (in) {
            for (j = 0; j < ichunk; ++j)
              x[j + filt_offs] = in[j * istride];
          }
          else {
            for (j = 0; j < ichunk; ++j)
              x[j + filt_offs] = 0;
          }
          process_native(channel_index, &ichunk, out, &ochunk);
          ilen -= ichunk;
          olen -= ochunk;
          out += ochunk * st->out_stride;
          if (in)
            in += ichunk * istride;
        }
      }
      *in_len -= ilen;
      *out_len -= olen;
      return active_resampler == ZERO ? RESAMPLER_ERR_ALLOC_FAILED : RESAMPLER_ERR_SUCCESS;
    }

    /** Set (change) the input/output sampling rates (integer value).
     * @param st Resampler state
     * @param in_rate Input sampling rate (integer number of Hz).
     * @param out_rate Output sampling rate (integer number of Hz).
     */
    int set_rate(spx_uint32_t in_rate, spx_uint32_t out_rate) {
      return set_rate_frac(in_rate, out_rate, in_rate, out_rate);
    }

    /** Get the current input/output sampling rates (integer value).
     * @param st Resampler state
     * @param in_rate Input sampling rate (integer number of Hz) copied.
     * @param out_rate Output sampling rate (integer number of Hz) copied.
     */
    void get_rate(spx_uint32_t* in_rate, spx_uint32_t* out_rate) {
      SpeexResampler* st = this;
      *in_rate = st->in_rate;
      *out_rate = st->out_rate;
    }

    /** Set (change) the input/output sampling rates and resampling ratio
     * (fractional values in Hz supported).
     * @param st Resampler state
     * @param ratio_num Numerator of the sampling rate ratio
     * @param ratio_den Denominator of the sampling rate ratio
     * @param in_rate Input sampling rate rounded to the nearest integer (in Hz).
     * @param out_rate Output sampling rate rounded to the nearest integer (in Hz).
     */
    int set_rate_frac(spx_uint32_t ratio_num, spx_uint32_t ratio_den, spx_uint32_t in_rate, spx_uint32_t out_rate) {
      SpeexResampler* st = this;
      spx_uint32_t fact;
      spx_uint32_t old_den;
      spx_uint32_t i;

      if (ratio_num == 0 || ratio_den == 0)
        return RESAMPLER_ERR_INVALID_ARG;

      if (st->in_rate == in_rate && st->out_rate == out_rate && st->num_rate == ratio_num && st->den_rate == ratio_den)
        return RESAMPLER_ERR_SUCCESS;

      old_den = st->den_rate;
      st->in_rate = in_rate;
      st->out_rate = out_rate;
      st->num_rate = ratio_num;
      st->den_rate = ratio_den;

      fact = compute_gcd(st->num_rate, st->den_rate);

      st->num_rate /= fact;
      st->den_rate /= fact;

      if (old_den > 0)
      {
        for (i = 0; i < st->nb_channels; i++)
        {
          if (multiply_frac(&st->samp_frac_num[i], st->samp_frac_num[i], st->den_rate, old_den) != RESAMPLER_ERR_SUCCESS)
            return RESAMPLER_ERR_OVERFLOW;
          /* Safety net */
          if (st->samp_frac_num[i] >= st->den_rate)
            st->samp_frac_num[i] = st->den_rate - 1;
        }
      }

      if (st->initialised)
        return update_filter();
      return RESAMPLER_ERR_SUCCESS;
    }

    /** Get the current resampling ratio. This will be reduced to the least
     * common denominator.
     * @param st Resampler state
     * @param ratio_num Numerator of the sampling rate ratio copied
     * @param ratio_den Denominator of the sampling rate ratio copied
     */
    void get_ratio(spx_uint32_t* ratio_num, spx_uint32_t* ratio_den) {
      *ratio_num = num_rate;
      *ratio_den = den_rate;
    }

    /** Set (change) the conversion quality.
     * @param st Resampler state
     * @param quality Resampling quality between 0 and 10, where 0 has poor
     * quality and 10 has very high quality.
     */
    int set_quality(int pQuality) {
      if (pQuality > 10 || pQuality < 0)
        return RESAMPLER_ERR_INVALID_ARG;
      if (quality == pQuality)
        return RESAMPLER_ERR_SUCCESS;
      quality = pQuality;
      if (initialised)
        return update_filter();
      return RESAMPLER_ERR_SUCCESS;
    }

    /** Get the conversion quality.
     * @param st Resampler state
     * @param quality Resampling quality between 0 and 10, where 0 has poor
     * quality and 10 has very high quality.
     */
    void get_quality(int* pQuality) {
      *pQuality = quality;
    }

    /** Set (change) the input stride.
     * @param st Resampler state
     * @param stride Input stride
     */
    void set_input_stride(spx_uint32_t stride) {
      in_stride = stride;
    }

    /** Get the input stride.
     * @param st Resampler state
     * @param stride Input stride copied
     */
    void get_input_stride(spx_uint32_t* stride) {
      *stride = in_stride;
    }

    /** Set (change) the output stride.
     * @param st Resampler state
     * @param stride Output stride
     */
    void set_output_stride(spx_uint32_t stride) {
      out_stride = stride;
    }

    /** Get the output stride.
     * @param st Resampler state copied
     * @param stride Output stride
     */
    void get_output_stride(spx_uint32_t* stride) {
      *stride = out_stride;
    }

    /** Get the latency introduced by the resampler measured in input samples.
     * @param st Resampler state
     */
    int get_input_latency() {
      return filt_len / 2;
    }

    /** Get the latency introduced by the resampler measured in output samples.
     * @param st Resampler state
     */
    int get_output_latency() {
      return ((filt_len / 2) * den_rate + (num_rate >> 1)) / num_rate;
    }

    /** Make sure that the first samples to go out of the resamplers don't have
     * leading zeros. This is only useful before starting to use a newly created
     * resampler. It is recommended to use that when resampling an audio file, as
     * it will generate a file with the same length. For real-time processing,
     * it is probably easier not to use this call (so that the output duration
     * is the same for the first frame).
     * @param st Resampler state
     */
    int skip_zeros() {
      spx_uint32_t i;
      for (i = 0; i < nb_channels; i++)
        last_sample[i] = filt_len / 2;
      return RESAMPLER_ERR_SUCCESS;
    }

    /** Reset a resampler so a new (unrelated) stream can be processed.
     * @param st Resampler state
     */
    int reset_mem() {
      spx_uint32_t i;
      for (i = 0; i < nb_channels; i++) {
        last_sample[i] = 0;
        magic_samples[i] = 0;
        samp_frac_num[i] = 0;
      }
      for (i = 0; i < nb_channels * (filt_len - 1); i++)
        mem[i] = 0;
      return RESAMPLER_ERR_SUCCESS;
    }
  };
}
