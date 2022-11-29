
#include "libfar/base.h"
#include "libfar/cpuid.h"

#include "libfar/resample.h"

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

extern void FAR_FN(upsample2f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                       size_t size_in,
                                       int sample_rate,
                                       int num_channels,
                                       byte16* FAR_RESTRICT buf_out,
                                       size_t size_out);
extern void FAR_FN(upsample3f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                       size_t size_in,
                                       int sample_rate,
                                       int num_channels,
                                       byte16* FAR_RESTRICT buf_out,
                                       size_t size_out);
extern void FAR_FN(downsample2f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                         size_t size_in,
                                         int sample_rate,
                                         int num_channels,
                                         byte16* FAR_RESTRICT buf_out,
                                         size_t size_out);
extern void FAR_FN(downsample3f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                         size_t size_in,
                                         int sample_rate,
                                         int num_channels,
                                         byte16* FAR_RESTRICT buf_out,
                                         size_t size_out);

extern void FAR_FN(upsample2f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                                     size_t size_in,
                                     int sample_rate,
                                     int num_channels,
                                     byte4* FAR_RESTRICT buf_out,
                                     size_t size_out);
extern void FAR_FN(upsample3f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                                     size_t size_in,
                                     int sample_rate,
                                     int num_channels,
                                     byte4* FAR_RESTRICT buf_out,
                                     size_t size_out);
extern void FAR_FN(downsample2f32_fpu)(const byte4* FAR_RESTRICT buf_in,
                                       size_t size_in,
                                       int sample_rate,
                                       int num_channels,
                                       byte4* FAR_RESTRICT buf_out,
                                       size_t size_out);

void FAR_FN(upsample2f32)(const byte32* FAR_RESTRICT buf_in,
                          size_t size_in,
                          int sample_rate,
                          int num_channels,
                          byte32* FAR_RESTRICT buf_out,
                          size_t size_out) {
  if (size_out < size_in * 2)
    return;
  if (FAR_FN(has_sse)())  // currently uses only SSE, not SSE4.1
  {
    FAR_FN(upsample2f32_sse41)
    ((const byte16*)buf_in, size_in, sample_rate, num_channels,
     (byte16*)buf_out, size_out);
  } else {
    FAR_FN(upsample2f32_fpu)
    ((const byte4*)buf_in, size_in, sample_rate, num_channels, (byte4*)buf_out,
     size_out);
  }
}

void FAR_FN(upsample3f32)(const byte32* FAR_RESTRICT buf_in,
                          size_t size_in,
                          int sample_rate,
                          int num_channels,
                          byte32* FAR_RESTRICT buf_out,
                          size_t size_out) {
  if (size_out < size_in * 3)
    return;
  if (FAR_FN(has_sse)())  // currently uses only SSE, not SSE4.1
  {
    FAR_FN(upsample3f32_sse41)
    ((const byte16*)buf_in, size_in, sample_rate, num_channels,
     (byte16*)buf_out, size_out);
  } else {
    FAR_FN(upsample3f32_fpu)
    ((const byte4*)buf_in, size_in, sample_rate, num_channels, (byte4*)buf_out,
     size_out);
  }
}

void FAR_FN(downsample2f32)(const byte32* FAR_RESTRICT buf_in,
                            size_t size_in,
                            int sample_rate,
                            int num_channels,
                            byte32* FAR_RESTRICT buf_out,
                            size_t size_out) {
  if (size_out < size_in / 2)
    return;
  if (FAR_FN(has_sse)())  // currently uses only SSE, not SSE4.1
  {
    FAR_FN(downsample2f32_sse41)
    ((const byte16*)buf_in, size_in, sample_rate, num_channels,
     (byte16*)buf_out, size_out);
  } else {
    FAR_FN(downsample2f32_fpu)
    ((const byte4*)buf_in, size_in, sample_rate, num_channels, (byte4*)buf_out,
     size_out);
  }
}

void FAR_FN(downsample3f32)(const byte32* FAR_RESTRICT buf_in,
                            size_t size_in,
                            int sample_rate,
                            int num_channels,
                            byte32* FAR_RESTRICT buf_out,
                            size_t size_out) {
  if (size_out < size_in / 3)
    return;
  if (FAR_FN(has_sse)())  // currently uses only SSE, not SSE4.1
  {
    FAR_FN(downsample3f32_sse41)
    ((const byte16*)buf_in, size_in, sample_rate, num_channels,
     (byte16*)buf_out, size_out);
  } /* else {
     FAR_FN(downsample3f32_fpu)
     (buf_in, size_in, sample_rate, num_channels, buf_out, size_out);
   }*/
}

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus
