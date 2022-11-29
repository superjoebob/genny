
#include "base.h"
#include "cpuid.h"

#include "fpu/adapt_fpu.h"
#include "sse41/adapt_sse41.h"

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

void FAR_API FAR_FN(adapt_ch1x2f32)(const float512* FAR_RESTRICT buf_in,
                                    size_t size_in,
                                    int sample_rate,
                                    float512* FAR_RESTRICT buf_out,
                                    size_t size_out) {
  if (size_out < size_in * 2)
    return;
#if defined(FAR_ENABLE_SSE)
#if !defined(FAR_ASSUME_SSE)
  if (FAR_FN(has_sse)())  // currently uses only SSE, not SSE4.1
#endif // !defined(FAR_ASSUME_SSE)
  {
    FAR_FN(adapt_ch1x2f32_sse41)
    ((const float128*)buf_in, size_in, sample_rate, (float128*)buf_out,
     size_out);
  }
#if !defined(FAR_ASSUME_SSE)
  else
#endif  // !defined(FAR_ASSUME_SSE)
#endif  // defined(FAR_ENABLE_SSE)
#if !defined(FAR_ASSUME_SSE)
  {
    FAR_FN(adapt_ch1x2f32_fpu)
    ((const float*)buf_in, size_in, sample_rate, (float*)buf_out, size_out);
  }
#endif  // !defined(FAR_ASSUME_SSE)
}

void FAR_API FAR_FN(adapt_ch2x1f32)(const float512* FAR_RESTRICT buf_in,
                                    size_t size_in,
                                    int sample_rate,
                                    float512* FAR_RESTRICT buf_out,
                                    size_t size_out) {
  if (size_out < size_in / 2)
    return;
#if defined(FAR_ENABLE_SSE3)
#if !defined(FAR_ASSUME_SSE3)
  if (FAR_FN(has_sse3)())  // currently uses only SSE3, not SSE4.1
#endif // !defined(FAR_ASSUME_SSE3)
  {
    FAR_FN(adapt_ch2x1f32_sse41)
    ((const float128*)buf_in, size_in, sample_rate, (float128*)buf_out,
     size_out);
  }
#if !defined(FAR_ASSUME_SSE3)
  else
#endif  // !defined(FAR_ASSUME_SSE3)
#endif  // defined(FAR_ENABLE_SSE3)
#if !defined(FAR_ASSUME_SSE3)
  {
    FAR_FN(adapt_ch2x1f32_fpu)
    ((const float*)buf_in, size_in, sample_rate, (float*)buf_out, size_out);
  }
#endif  // !defined(FAR_ASSUME_SSE3)
}

void FAR_API FAR_FN(adapt_ch1x2s16)(const short512* FAR_RESTRICT buf_in,
                                    size_t size_in,
                                    int sample_rate,
                                    short512* FAR_RESTRICT buf_out,
                                    size_t size_out) {
  if (size_out < size_in * 2)
    return;
#if defined(FAR_ENABLE_SSE)
#if !defined(FAR_ASSUME_SSE)
  if (FAR_FN(has_sse)())  // currently uses only SSE, not SSE4.1
#endif  // !defined(FAR_ASSUME_SSE)
  {
    FAR_FN(adapt_ch1x2s16_sse41)
    ((const short128*)buf_in, size_in, sample_rate, (short128*)buf_out,
     size_out);
  }
#if !defined(FAR_ASSUME_SSE)
  else
#endif  // !defined(FAR_ASSUME_SSE)
#endif  // defined(FAR_ENABLE_SSE)
#if !defined(FAR_ASSUME_SSE)
  {
    FAR_FN(adapt_ch1x2s16_fpu)
    ((const int16*)buf_in, size_in, sample_rate, (int16*)buf_out, size_out);
  }
#endif  // !defined(FAR_ASSUME_SSE)
}

void FAR_API FAR_FN(adapt_ch2x1s16)(const short512* FAR_RESTRICT buf_in,
                                    size_t size_in,
                                    int sample_rate,
                                    short512* FAR_RESTRICT buf_out,
                                    size_t size_out) {
  if (size_out < size_in / 2)
    return;
#if defined(FAR_ENABLE_SSE)
#if !defined(FAR_ASSUME_SSE)
  if (FAR_FN(has_sse)())  // currently uses only SSE, not SSE4.1
#endif  // !defined(FAR_ASSUME_SSE)
  {
    FAR_FN(adapt_ch2x1s16_sse41)
    ((const short128*)buf_in, size_in, sample_rate, (short128*)buf_out,
     size_out);
  }
#if !defined(FAR_ASSUME_SSE)
  else
#endif  // !defined(FAR_ASSUME_SSE)
#endif  // defined(FAR_ENABLE_SSE)
#if !defined(FAR_ASSUME_SSE)
  {
    FAR_FN(adapt_ch2x1s16_fpu)
    ((const int16*)buf_in, size_in, sample_rate, (int16*)buf_out, size_out);
  }
#endif  // !defined(FAR_ASSUME_SSE)
}

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus