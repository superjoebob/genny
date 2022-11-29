
#include "sse41/adapt_sse41.h"

#include <smmintrin.h>  // SSE4.1

// Note - currently using only SSE3:
// - _mm_prefetch()   : SSE
// - _mm_load_ps()    : SSE
// - _mm_shuffle_ps() : SSE
// - _mm_stream_ps()  : SSE
// - _mm_hadd_ps      : SSE3

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

void FAR_FN(adapt_ch1x2f32_sse41)(const float128* FAR_RESTRICT buf_in,
                                  size_t size_in,
                                  int sample_rate,
                                  float128* FAR_RESTRICT buf_out,
                                  size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(float);
  const float* src = (const float* FAR_RESTRICT)buf_in;
  float* dst = (float* FAR_RESTRICT)buf_out;
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
}

void FAR_FN(adapt_ch2x1f32_sse41)(const float128* FAR_RESTRICT buf_in,
                                  size_t size_in,
                                  int sample_rate,
                                  float128* FAR_RESTRICT buf_out,
                                  size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(float);
  const float* src = (const float* FAR_RESTRICT)buf_in;
  float* dst = (float* FAR_RESTRICT)buf_out;
  // 128 blocks x4
  size_t num_blocks = (num_samples / 16);

  const __m128 half = _mm_set_ps1(0.5f);
  for (size_t i = 0; i < num_blocks; ++i) {
    _mm_prefetch((const char*)(src + 16), _MM_HINT_NTA);
    __m128 x0 = _mm_load_ps(src);
    __m128 x1 = _mm_load_ps(src + 4);
    __m128 x2 = _mm_load_ps(src + 8);
    __m128 x3 = _mm_load_ps(src + 12);
    __m128 x0x1 = _mm_hadd_ps(x0, x1);
    __m128 x2x3 = _mm_hadd_ps(x2, x3);
    __m128 x0x1half = _mm_mul_ps(x0x1, half);
    __m128 x2x3half = _mm_mul_ps(x2x3, half);
    _mm_stream_ps(dst, x0x1half);
    _mm_stream_ps(dst + 4, x2x3half);
    src += 16;
    dst += 8;
  }
  num_samples -= (num_blocks * 16);
  // Remain
  for (size_t i = 0; i < num_samples; i += 2) {
    float l = *src++;  // x1
    float r = *src++;  // x2
    *dst++ = (l + r) * 0.5f;
  }
}

void FAR_FN(adapt_ch1x2s16_sse41)(const short128* FAR_RESTRICT buf_in,
                                  size_t size_in,
                                  int sample_rate,
                                  short128* FAR_RESTRICT buf_out,
                                  size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(int16);
  const int16* src = (const int16* FAR_RESTRICT)buf_in;
  int16* dst = (int16 * FAR_RESTRICT) buf_out;
  // 128 blocks x4
  size_t num_blocks = (num_samples / 32);
  for (size_t i = 0; i < num_blocks; ++i) {
    _mm_prefetch((const char*)(src + 32), _MM_HINT_NTA);
    __m128i x0 = _mm_load_si128((const __m128i*)(src));
    __m128i x1 = _mm_load_si128((const __m128i*)(src + 8));
    __m128i x2 = _mm_load_si128((const __m128i*)(src + 16));
    __m128i x3 = _mm_load_si128((const __m128i*)(src + 24));
    __m128i x0lo1 = _mm_shufflelo_epi16(x0, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x0lo2 = _mm_shufflelo_epi16(x0, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x0hi1 = _mm_shufflehi_epi16(x0, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x0hi2 = _mm_shufflehi_epi16(x0, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x0lo2b = _mm_shuffle_epi32(x0lo2, _MM_SHUFFLE(1, 0, 1, 0));
    __m128i x0hi1b = _mm_shuffle_epi32(x0hi1, _MM_SHUFFLE(3, 2, 3, 2));
    __m128i x0lo = _mm_blend_epi16(x0lo1, x0lo2b, 0b11110000);
    __m128i x0hi = _mm_blend_epi16(x0hi1b, x0hi2, 0b11110000);
    __m128i x1lo1 = _mm_shufflelo_epi16(x1, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x1lo2 = _mm_shufflelo_epi16(x1, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x1hi1 = _mm_shufflehi_epi16(x1, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x1hi2 = _mm_shufflehi_epi16(x1, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x1lo2b = _mm_shuffle_epi32(x1lo2, _MM_SHUFFLE(1, 0, 1, 0));
    __m128i x1hi1b = _mm_shuffle_epi32(x1hi1, _MM_SHUFFLE(3, 2, 3, 2));
    __m128i x1lo = _mm_blend_epi16(x1lo1, x1lo2b, 0b11110000);
    __m128i x1hi = _mm_blend_epi16(x1hi1b, x1hi2, 0b11110000);
    __m128i x2lo1 = _mm_shufflelo_epi16(x2, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x2lo2 = _mm_shufflelo_epi16(x2, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x2hi1 = _mm_shufflehi_epi16(x2, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x2hi2 = _mm_shufflehi_epi16(x2, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x2lo2b = _mm_shuffle_epi32(x2lo2, _MM_SHUFFLE(1, 0, 1, 0));
    __m128i x2hi1b = _mm_shuffle_epi32(x2hi1, _MM_SHUFFLE(3, 2, 3, 2));
    __m128i x2lo = _mm_blend_epi16(x2lo1, x2lo2b, 0b11110000);
    __m128i x2hi = _mm_blend_epi16(x2hi1b, x2hi2, 0b11110000);
    __m128i x3lo1 = _mm_shufflelo_epi16(x3, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x3lo2 = _mm_shufflelo_epi16(x3, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x3hi1 = _mm_shufflehi_epi16(x3, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i x3hi2 = _mm_shufflehi_epi16(x3, _MM_SHUFFLE(3, 3, 2, 2));
    __m128i x3lo2b = _mm_shuffle_epi32(x3lo2, _MM_SHUFFLE(1, 0, 1, 0));
    __m128i x3hi1b = _mm_shuffle_epi32(x3hi1, _MM_SHUFFLE(3, 2, 3, 2));
    __m128i x3lo = _mm_blend_epi16(x3lo1, x3lo2b, 0b11110000);
    __m128i x3hi = _mm_blend_epi16(x3hi1b, x3hi2, 0b11110000);
    _mm_stream_si128((__m128i*)dst, x0lo);
    _mm_stream_si128((__m128i*)(dst + 8), x0hi);
    _mm_stream_si128((__m128i*)(dst + 16), x1lo);
    _mm_stream_si128((__m128i*)(dst + 24), x1hi);
    _mm_stream_si128((__m128i*)(dst + 32), x2lo);
    _mm_stream_si128((__m128i*)(dst + 40), x2hi);
    _mm_stream_si128((__m128i*)(dst + 48), x3lo);
    _mm_stream_si128((__m128i*)(dst + 56), x3hi);
    src += 32;
    dst += 64;
  }
  num_samples -= (num_blocks * 32);
  // Remain
  for (size_t i = 0; i < num_samples; ++i) {
    int16 value = *src++;
    *dst++ = value;  // x1
    *dst++ = value;  // x2
  }
}

void FAR_FN(adapt_ch2x1s16_sse41)(const short128* FAR_RESTRICT buf_in,
                                  size_t size_in,
                                  int sample_rate,
                                  short128* FAR_RESTRICT buf_out,
                                  size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(int16);
  const int16* src = (const int16* FAR_RESTRICT)buf_in;
  int16* dst = (int16 * FAR_RESTRICT) buf_out;
  // 128 blocks x4
  size_t num_blocks = (num_samples / 32);
  const __m128i offset =
      _mm_set1_epi16(-32768);  // use -offset to avoid truncation of +32768
  for (size_t i = 0; i < num_blocks; ++i) {
    _mm_prefetch((const char*)(src + 32), _MM_HINT_NTA);
    __m128i x0 = _mm_load_si128((const __m128i*)(src));
    __m128i x1 = _mm_load_si128((const __m128i*)(src + 8));
    __m128i x2 = _mm_load_si128((const __m128i*)(src + 16));
    __m128i x3 = _mm_load_si128((const __m128i*)(src + 24));
    x0 = _mm_sub_epi16(x0, offset);
    x1 = _mm_sub_epi16(x1, offset);
    x2 = _mm_sub_epi16(x2, offset);
    x3 = _mm_sub_epi16(x3, offset);
    __m128i x0l = _mm_shufflelo_epi16(x0, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x0h = _mm_shufflehi_epi16(x0, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x0b = _mm_blend_epi16(x0l, x0h, 0b11110000);
    __m128i x0s = _mm_shuffle_epi32(x0b, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x1l = _mm_shufflelo_epi16(x1, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x1h = _mm_shufflehi_epi16(x1, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x1b = _mm_blend_epi16(x1l, x1h, 0b11110000);
    __m128i x1s = _mm_shuffle_epi32(x1b, _MM_SHUFFLE(2, 0, 3, 1));
    __m128i x01lo = _mm_blend_epi16(x0s, x1s, 0b11110000);
    __m128i x01hiinv = _mm_blend_epi16(x0s, x1s, 0b00001111);
    __m128i x01hi = _mm_shuffle_epi32(x01hiinv, _MM_SHUFFLE(1, 0, 3, 2));
    __m128i x01avg = _mm_avg_epu16(x01lo, x01hi);
    __m128i x2l = _mm_shufflelo_epi16(x2, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x2h = _mm_shufflehi_epi16(x2, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x2b = _mm_blend_epi16(x2l, x2h, 0b11110000);
    __m128i x2s = _mm_shuffle_epi32(x2b, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x3l = _mm_shufflelo_epi16(x3, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x3h = _mm_shufflehi_epi16(x3, _MM_SHUFFLE(3, 1, 2, 0));
    __m128i x3b = _mm_blend_epi16(x3l, x3h, 0b11110000);
    __m128i x3s = _mm_shuffle_epi32(x3b, _MM_SHUFFLE(2, 0, 3, 1));
    __m128i x23lo = _mm_blend_epi16(x2s, x3s, 0b11110000);
    __m128i x23hiinv = _mm_blend_epi16(x2s, x3s, 0b00001111);
    __m128i x23hi = _mm_shuffle_epi32(x23hiinv, _MM_SHUFFLE(1, 0, 3, 2));
    __m128i x23avg = _mm_avg_epu16(x23lo, x23hi);
    x01avg = _mm_add_epi16(x01avg, offset);
    x23avg = _mm_add_epi16(x23avg, offset);
    _mm_stream_si128((__m128i*)dst, x01avg);
    _mm_stream_si128((__m128i*)(dst + 8), x23avg);
    src += 32;
    dst += 16;
  }
  num_samples -= (num_blocks * 32);
  // Remain
  for (size_t i = 0; i < num_samples; i += 2) {
    int16 value1 = *src++;  // x1
    int16 value2 = *src++;  // x2
    *dst++ = (value1 / 2) + (value2 / 2);
  }
}

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus
