
#include "fpu/adapt_fpu.h"

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

void FAR_FN(adapt_ch1x2f32_fpu)(const float* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                float* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(float);
  for (size_t i = 0; i < num_samples; ++i) {
    float value = *buf_in++;
    *buf_out++ = value;  // x1
    *buf_out++ = value;  // x2
  }
}

void FAR_FN(adapt_ch2x1f32_fpu)(const float* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                float* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(float);
  for (size_t i = 0; i < num_samples; i += 2) {
    float value1 = *buf_in++; // x1
    float value2 = *buf_in++; // x2
    *buf_out++ = (value1 + value2) / 2.0f;
  }
}

void FAR_FN(adapt_ch1x2s16_fpu)(const int16* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                int16* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(int16);
  for (size_t i = 0; i < num_samples; ++i) {
    int16 value = *buf_in++;
    *buf_out++ = value;  // x1
    *buf_out++ = value;  // x2
  }
}

void FAR_FN(adapt_ch2x1s16_fpu)(const int16* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                int16* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  size_t num_samples = size_in / sizeof(int16);
  for (size_t i = 0; i < num_samples; i += 2) {
    int16 value1 = *buf_in++;  // x1
    int16 value2 = *buf_in++;  // x2
    *buf_out++ = (value1 / 2) + (value2 / 2);
  }
}

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus
