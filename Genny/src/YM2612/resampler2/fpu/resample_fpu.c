
#include "fpu/resample_fpu.h"

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

void FAR_FN(upsample2f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                              size_t size_in,
                              int sample_rate,
                              int num_channels,
                              byte4* FAR_RESTRICT buf_out,
                              size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float* FAR_RESTRICT)buf_in;
  float* dst = (float* FAR_RESTRICT)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    for (size_t i = 0; i < num_samples; ++i) {
      float value = *src++;
      *dst++ = value;  // x1
      *dst++ = value;  // x2
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
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

void FAR_FN(upsample3f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                              size_t size_in,
                              int sample_rate,
                              int num_channels,
                              byte4* FAR_RESTRICT buf_out,
                              size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float* FAR_RESTRICT)buf_in;
  float* dst = (float* FAR_RESTRICT)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    for (size_t i = 0; i < num_samples; ++i) {
      float value = *src++;
      *dst++ = value;  // x1
      *dst++ = value;  // x2
      *dst++ = value;  // x3
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
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

void FAR_FN(upsample4f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                              size_t size_in,
                              int sample_rate,
                              int num_channels,
                              byte4* FAR_RESTRICT buf_out,
                              size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float* FAR_RESTRICT)buf_in;
  float* dst = (float* FAR_RESTRICT)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    for (size_t i = 0; i < num_samples; ++i) {
      float value = *src++;
      *dst++ = value;  // x1
      *dst++ = value;  // x2
      *dst++ = value;  // x3
      *dst++ = value;  // x4
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
    for (size_t i = 0; i < num_samples; ++i) {
      float l = *src++;
      float r = *src++;
      *dst++ = l;  // x1
      *dst++ = r;  // x1
      *dst++ = l;  // x2
      *dst++ = r;  // x2
      *dst++ = l;  // x3
      *dst++ = r;  // x3
      *dst++ = l;  // x4
      *dst++ = r;  // x4
    }
  }
}

void FAR_FN(downsample2f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                int num_channels,
                                byte4* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float*)buf_in;
  float* dst = (float*)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    for (size_t i = 0; i < num_samples; i += 2) {
      float value = *src++;  // x1 (use)
      ++src;                 // x2 (discard)
      *dst++ = value;
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
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

void FAR_FN(downsample3f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                int num_channels,
                                byte4* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float*)buf_in;
  float* dst = (float*)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    for (size_t i = 0; i < num_samples; i += 3) {
      float value = *src++;  // x1 (use)
      ++src;                 // x2 (discard)
      ++src;                 // x3 (discard)
      *dst++ = value;
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
    for (size_t i = 0; i < num_samples; i += 3) {
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

void FAR_FN(downsample4f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                                size_t size_in,
                                int sample_rate,
                                int num_channels,
                                byte4* FAR_RESTRICT buf_out,
                                size_t size_out) {
  FAR_UNUSED(sample_rate);
  FAR_UNUSED(size_out);
  const float* src = (const float*)buf_in;
  float* dst = (float*)buf_out;
  if (num_channels == 1) {
    size_t num_samples = size_in / sizeof(float);
    for (size_t i = 0; i < num_samples; i += 4) {
      float value = *src++;  // x1 (use)
      ++src;                 // x2 (discard)
      ++src;                 // x3 (discard)
      ++src;                 // x4 (discard)
      *dst++ = value;
    }
  } else if (num_channels == 2) {
    size_t num_samples = size_in / (2 * sizeof(float));
    for (size_t i = 0; i < num_samples; i += 4) {
      float l = *src++;  // x1 (use)
      float r = *src++;  // x1 (use)
      ++src;             // x2 (discard)
      ++src;             // x2 (discard)
      ++src;             // x3 (discard)
      ++src;             // x3 (discard)
      ++src;             // x4 (discard)
      ++src;             // x4 (discard)
      *dst++ = l;
      *dst++ = r;
    }
  }
}

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus
