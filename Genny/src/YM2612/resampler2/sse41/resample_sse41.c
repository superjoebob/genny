
#include "sse41/resample_sse41.h"

#include <smmintrin.h>  // SSE 4.1

// Note - currently using only SSE:
// - _mm_prefetch()   : SSE
// - _mm_load_ps()    : SSE
// - _mm_shuffle_ps() : SSE
// - _mm_stream_ps()  : SSE

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

void FAR_FN(upsample2f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                int num_channels,
                                byte16* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float* FAR_RESTRICT)buf_in;
  float* dst = (float* FAR_RESTRICT)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    // 128 blocks x4
    size_t num_blocks = (num_samples / 16);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x2 = _mm_load_ps(src + 8);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x0lo = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(1, 1, 0, 0));
      __m128 x0hi = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(3, 3, 2, 2));
      __m128 x1lo = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(1, 1, 0, 0));
      __m128 x1hi = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(3, 3, 2, 2));
      __m128 x2lo = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(1, 1, 0, 0));
      __m128 x2hi = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(3, 3, 2, 2));
      __m128 x3lo = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(1, 1, 0, 0));
      __m128 x3hi = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(3, 3, 2, 2));
      _mm_stream_ps(dst, x0lo);
      _mm_stream_ps(dst + 4, x0hi);
      _mm_stream_ps(dst + 8, x1lo);
      _mm_stream_ps(dst + 12, x1hi);
      _mm_stream_ps(dst + 16, x2lo);
      _mm_stream_ps(dst + 20, x2hi);
      _mm_stream_ps(dst + 24, x3lo);
      _mm_stream_ps(dst + 28, x3hi);
      src += 16;
      dst += 32;
    }
    num_samples -= (num_blocks * 16);
    // Remain
    for (size_t i = 0; i < num_samples; ++i) {
      float value = *src++;
      *dst++ = value;  // x1
      *dst++ = value;  // x2
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
    // 128 blocks x4
    size_t num_blocks = (num_samples / 8);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x2 = _mm_load_ps(src + 8);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x0lo = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x0hi = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(3, 2, 3, 2));
      __m128 x1lo = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x1hi = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(3, 2, 3, 2));
      __m128 x2lo = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x2hi = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(3, 2, 3, 2));
      __m128 x3lo = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x3hi = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(3, 2, 3, 2));
      _mm_stream_ps(dst, x0lo);
      _mm_stream_ps(dst + 4, x0hi);
      _mm_stream_ps(dst + 8, x1lo);
      _mm_stream_ps(dst + 12, x1hi);
      _mm_stream_ps(dst + 16, x2lo);
      _mm_stream_ps(dst + 20, x2hi);
      _mm_stream_ps(dst + 24, x3lo);
      _mm_stream_ps(dst + 28, x3hi);
      src += 16;
      dst += 32;
    }
    num_samples -= (num_blocks * 8);
    for (size_t i = 0; i < num_samples; ++i) {
      float l = *src++;
      float r = *src++;
      *dst++ = l;  // x1
      *dst++ = r;  // x1
      *dst++ = l;  // x2
      *dst++ = r;  // x2
    }
  }
}

void FAR_FN(upsample3f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                int num_channels,
                                byte16* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float* FAR_RESTRICT)buf_in;
  float* dst = (float* FAR_RESTRICT)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    // 128 blocks x4
    size_t num_blocks = (num_samples / 16);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x2 = _mm_load_ps(src + 8);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x0a = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(1, 0, 0, 0));
      __m128 x0b = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(2, 2, 1, 1));
      __m128 x0c = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(3, 3, 3, 2));
      __m128 x1a = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(1, 0, 0, 0));
      __m128 x1b = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(2, 2, 1, 1));
      __m128 x1c = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(3, 3, 3, 2));
      __m128 x2a = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(1, 0, 0, 0));
      __m128 x2b = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(2, 2, 1, 1));
      __m128 x2c = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(3, 3, 3, 2));
      __m128 x3a = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(1, 0, 0, 0));
      __m128 x3b = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(2, 2, 1, 1));
      __m128 x3c = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(3, 3, 3, 2));
      _mm_stream_ps(dst, x0a);
      _mm_stream_ps(dst + 4, x0b);
      _mm_stream_ps(dst + 8, x0c);
      _mm_stream_ps(dst + 12, x1a);
      _mm_stream_ps(dst + 16, x1b);
      _mm_stream_ps(dst + 20, x1c);
      _mm_stream_ps(dst + 24, x2a);
      _mm_stream_ps(dst + 28, x2b);
      _mm_stream_ps(dst + 32, x2c);
      _mm_stream_ps(dst + 36, x3a);
      _mm_stream_ps(dst + 40, x3b);
      _mm_stream_ps(dst + 44, x3c);
      src += 16;
      dst += 48;
    }
    num_samples -= (num_blocks * 16);
    // Remain
    for (size_t i = 0; i < num_samples; ++i) {
      float value = *src++;
      *dst++ = value;  // x1
      *dst++ = value;  // x2
      *dst++ = value;  // x3
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
    // 128 blocks x4
    size_t num_blocks = (num_samples / 8);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x2 = _mm_load_ps(src + 8);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x0a = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x0b = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(3, 2, 1, 0));
      __m128 x0c = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(3, 2, 3, 2));
      __m128 x1a = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x1b = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(3, 2, 1, 0));
      __m128 x1c = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(3, 2, 3, 2));
      __m128 x2a = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x2b = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(3, 2, 1, 0));
      __m128 x2c = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(3, 2, 3, 2));
      __m128 x3a = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x3b = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(3, 2, 1, 0));
      __m128 x3c = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(3, 2, 3, 2));
      _mm_stream_ps(dst, x0a);
      _mm_stream_ps(dst + 4, x0b);
      _mm_stream_ps(dst + 8, x0c);
      _mm_stream_ps(dst + 12, x1a);
      _mm_stream_ps(dst + 16, x1b);
      _mm_stream_ps(dst + 20, x1c);
      _mm_stream_ps(dst + 24, x2a);
      _mm_stream_ps(dst + 28, x2b);
      _mm_stream_ps(dst + 32, x2c);
      _mm_stream_ps(dst + 36, x3a);
      _mm_stream_ps(dst + 40, x3b);
      _mm_stream_ps(dst + 44, x3c);
      src += 16;
      dst += 48;
    }
    num_samples -= (num_blocks * 8);
    for (size_t i = 0; i < num_samples; ++i) {
      float l = *src++;
      float r = *src++;
      *dst++ = l;  // x1
      *dst++ = r;  // x1
      *dst++ = l;  // x2
      *dst++ = r;  // x2
      *dst++ = l;  // x3
      *dst++ = r;  // x3
    }
  }
}

void FAR_FN(downsample2f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                  size_t size_in,
                                  int sample_rate,
                                  int num_channels,
                                  byte16* FAR_RESTRICT buf_out,
                                  size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float*)buf_in;
  float* dst = (float*)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    // 128 blocks x4
    size_t num_blocks = (num_samples / 16);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x2 = _mm_load_ps(src + 8);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x0x1 = _mm_shuffle_ps(x0, x1, _MM_SHUFFLE(2, 0, 2, 0));
      __m128 x2x3 = _mm_shuffle_ps(x2, x3, _MM_SHUFFLE(2, 0, 2, 0));
      _mm_stream_ps(dst, x0x1);
      _mm_stream_ps(dst + 4, x2x3);
      src += 16;
      dst += 8;
    }
    num_samples -= (num_blocks * 16);
    // Remain
    for (size_t i = 0; i < num_samples; i += 2) {
      float value = *src++;  // x1 (use)
      ++src;                 // x2 (discard)
      *dst++ = value;
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
    // 128 blocks x4
    size_t num_blocks = (num_samples / 8);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x2 = _mm_load_ps(src + 8);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x0x1 = _mm_shuffle_ps(x0, x1, _MM_SHUFFLE(1, 0, 1, 0));
      __m128 x2x3 = _mm_shuffle_ps(x2, x3, _MM_SHUFFLE(1, 0, 1, 0));
      _mm_stream_ps(dst, x0x1);
      _mm_stream_ps(dst + 4, x2x3);
      src += 16;
      dst += 8;
    }
    num_samples -= (num_blocks * 8);
    for (size_t i = 0; i < num_samples; i += 2) {
      float l = *src++;  // x1 (use)
      float r = *src++;  // x1 (use)
      ++src;             // x2 (discard)
      ++src;             // x2 (discard)
      *dst++ = l;
      *dst++ = r;
    }
  }
}

void FAR_FN(downsample3f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                  size_t size_in,
                                  int sample_rate,
                                  int num_channels,
                                  byte16* FAR_RESTRICT buf_out,
                                  size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float*)buf_in;
  float* dst = (float*)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    // 128 blocks x4
    size_t num_blocks = (num_samples / 24);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 24), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x2 = _mm_load_ps(src + 8);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x4 = _mm_load_ps(src + 16);
      __m128 x5 = _mm_load_ps(src + 20);
      __m128 x12 = _mm_shuffle_ps(x1, x2, _MM_SHUFFLE(0, 1, 0, 2));
      __m128 x012 = _mm_shuffle_ps(x0, x12, _MM_SHUFFLE(2, 0, 3, 0));
      __m128 x45 = _mm_shuffle_ps(x4, x5, _MM_SHUFFLE(0, 1, 0, 2));
      __m128 x345 = _mm_shuffle_ps(x3, x45, _MM_SHUFFLE(2, 0, 3, 0));
      _mm_stream_ps(dst, x012);
      _mm_stream_ps(dst + 4, x345);
      src += 24;
      dst += 8;
    }
    num_samples -= (num_blocks * 24);
    // Remain
    for (size_t i = 0; i < num_samples; i += 2) {
      float value = *src++;  // x1 (use)
      ++src;                 // x2 (discard)
      ++src;                 // x3 (discard)
      *dst++ = value;
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
    // 128 blocks x4
    size_t num_blocks = (num_samples / 12);
    for (size_t i = 0; i < num_blocks; ++i) {
      _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
      __m128 x0 = _mm_load_ps(src);
      __m128 x1 = _mm_load_ps(src + 4);
      __m128 x3 = _mm_load_ps(src + 12);
      __m128 x4 = _mm_load_ps(src + 16);
      __m128 x01 = _mm_shuffle_ps(x0, x1, _MM_SHUFFLE(3, 2, 1, 0));
      __m128 x34 = _mm_shuffle_ps(x3, x4, _MM_SHUFFLE(3, 2, 1, 0));
      _mm_stream_ps(dst, x01);
      _mm_stream_ps(dst + 4, x34);
      src += 24;
      dst += 8;
    }
    num_samples -= (num_blocks * 12);
    for (size_t i = 0; i < num_samples; i += 2) {
      float l = *src++;  // x1 (use)
      float r = *src++;  // x1 (use)
      ++src;             // x2 (discard)
      ++src;             // x2 (discard)
      ++src;             // x3 (discard)
      ++src;             // x3 (discard)
      *dst++ = l;
      *dst++ = r;
    }
  }
}

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus
