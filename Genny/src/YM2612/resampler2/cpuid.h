#pragma once

#include "base.h"

#ifdef __cplusplus
namespace libfar {
#endif  // __cplusplus

/// Query the CPU for hardware support. The result is cached internally such
/// that subsequent calls are no-op.
void FAR_API FAR_FN(cpuid_init)();

/// Check for SSE support. This will call |cpuid_init| if not already called.
far_bool FAR_API FAR_FN(has_sse)();

/// Check for SSE2 support. This will call |cpuid_init| if not already called.
far_bool FAR_API FAR_FN(has_sse2)();

/// Check for SSE3 support. This will call |cpuid_init| if not already called.
far_bool FAR_API FAR_FN(has_sse3)();

/// Check for SSSE3 support. This will call |cpuid_init| if not already called.
far_bool FAR_API FAR_FN(has_ssse3)();

/// Check for SSE4.1 support. This will call |cpuid_init| if not already called.
far_bool FAR_API FAR_FN(has_sse41)();

/// Check for SSE4.2 support. This will call |cpuid_init| if not already called.
far_bool FAR_API FAR_FN(has_sse42)();

#ifdef __cplusplus
}  // namespace libfar
#endif  // __cplusplus
