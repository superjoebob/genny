#pragma once

#include "base.h"

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

/// Upsample from given rate to 2x the given rate (e.g. 24kHz -> 48kHz)
/// for float32 values in [-1:1]. Upsampling is done by duplicating the samples,
/// which introduces some high frequence ghost signals. For performance reasons
/// those signals are *not* filtered out (no low-pass filter is applied). If
/// this is an issue for your application, you should consider a quality-focused
/// DSP library instead.
void FAR_FN(upsample2f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                       size_t size_in,
                                       int sample_rate,
                                       int num_channels,
                                       byte16* FAR_RESTRICT buf_out,
                                       size_t size_out);

/// Upsample from given rate to 3x the given rate (e.g. 16kHz -> 48kHz)
/// for float32 values in [-1:1]. Upsampling is done by duplicating the samples,
/// which introduces some high frequence ghost signals. For performance reasons
/// those signals are *not* filtered out (no low-pass filter is applied). If
/// this is an issue for your application, you should consider a quality-focused
/// DSP library instead.
void FAR_FN(upsample3f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                       size_t size_in,
                                       int sample_rate,
                                       int num_channels,
                                       byte16* FAR_RESTRICT buf_out,
                                       size_t size_out);

/// Downsample from given rate to /2 the given rate (e.g. 48kHz -> 24kHz)
/// for float32 values in [-1:1]. Downsampling is done by dropping every other
/// sample group (series of N samples, where N is the number of channels). This
/// is a rough but fast approximation.
void FAR_FN(downsample2f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                         size_t size_in,
                                         int sample_rate,
                                         int num_channels,
                                         byte16* FAR_RESTRICT buf_out,
                                         size_t size_out);

/// Downsample from given rate to /3 the given rate (e.g. 48kHz -> 16kHz)
/// for float32 values in [-1:1]. Downsampling is done by dropping every second
/// and third sample group (series of N samples, where N is the number of
/// channels). This is a rough but fast approximation.
void FAR_FN(downsample3f32_sse41)(const byte16* FAR_RESTRICT buf_in,
                                         size_t size_in,
                                         int sample_rate,
                                         int num_channels,
                                         byte16* FAR_RESTRICT buf_out,
                                         size_t size_out);

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus
