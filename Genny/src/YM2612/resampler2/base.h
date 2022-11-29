#pragma once

//#include "libfar_config.h"
//#include "libfar_version.h"

// Standard headers
#if !defined(FAR_NO_STD_INCLUDES)
#ifdef __cplusplus
#include <cstdlib>
#else  // __cplusplus
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef __clang__
#include <stdalign.h>
// Should be defined in assert.h in C11, but clang-cl is missing it
#define static_assert(b, m) _Static_assert(b, m)
#endif  // _MSC_VER
#endif  // __cplusplus
#endif  // !defined(FAR_NO_STD_INCLUDES)

#define FAR_XCONCAT(a, b) a##b
#define FAR_CONCAT(a, b) FAR_XCONCAT(a, b)

// Default function naming strategy
// - In C++, use a namespace
// - In C, use a prefix
#if !defined(FAR_FN_PREFIX)
#ifdef __cplusplus
// No prefix, namespace instead
#define FAR_FN_PREFIX
#else  // __cplusplus
// Prefix to avoid global namespace collision
#define FAR_FN_PREFIX far_
#endif  // __cplusplus
#endif  // !defined(FAR_FN_PREFIX)
#define FAR_FN(x) FAR_CONCAT(FAR_FN_PREFIX, x)

// To allow shared library compiling
#if !defined(FAR_API)
#define FAR_API
#endif

// Boolean
#ifdef __cplusplus
using far_bool = bool;
constexpr bool far_true = true;
constexpr bool far_false = false;
#else  // __cplusplus
typedef int32_t far_bool;
#define far_true ((far_bool)-1)
#define far_false ((far_bool)0)
#endif  // __cplusplus

// Base types
typedef unsigned char uint8;
static_assert(sizeof(uint8) == 1, "");
typedef signed short int16;
static_assert(sizeof(int16) == 2, "");

// Aligned types for input and output buffers
#ifdef _MSC_VER
typedef struct byte4 {
  char __declspec(align(4)) c;
} byte4;
typedef struct byte8 {
  char __declspec(align(8)) c;
} byte8;
typedef struct byte16 {
  char __declspec(align(16)) c;
} byte16;
typedef struct byte32 {
  char __declspec(align(32)) c;
} byte32;
typedef struct byte64 {
  char __declspec(align(64)) c;
} byte64;
typedef struct __declspec(align(16)) short128 {
  int16 c[8];
} short128;
static_assert(sizeof(short128) == 16, "");
typedef struct __declspec(align(32)) short256 {
  int16 c[16];
} short256;
static_assert(sizeof(short256) == 32, "");
typedef struct __declspec(align(64)) short512 {
  int16 c[32];
} short512;
static_assert(sizeof(short512) == 64, "");
typedef struct __declspec(align(16)) float128 {
  float c[4];
} float128;
static_assert(sizeof(float128) == 16, "");
typedef struct __declspec(align(32)) float256 {
  float c[8];
} float256;
static_assert(sizeof(float256) == 32, "");
typedef struct __declspec(align(64)) float512 {
  float c[16];
} float512;
static_assert(sizeof(float512) == 64, "");
#else   // _MSC_VER
typedef char __attribute__((align(4))) byte4;
typedef char __attribute__((align(8))) byte4;
typedef char __attribute__((align(16)) byte16;
typedef char __attribute__((align(32)) byte32;
#endif  // _MSC_VER

// Aligned allocations for SIMD
#if !defined(FAR_CUSTOM_ALLOCATORS)
#ifdef _MSC_VER
#ifdef __cplusplus
namespace libfar {
inline void* malloc_align(size_t size, size_t align) {
  return _aligned_malloc(size, align);
}
inline void free_align(void* ptr) {
  _aligned_free(ptr);
}
}  // namespace libfar
#else  // __cplusplus
#define far_malloc_align(s, a) _aligned_malloc(s, a)
#define far_free_align(p) _aligned_free(p)
#endif  // __cplusplus
#else   // _MSC_VER
#if (defined(DEBUG) || defined(_DEBUG)) && !defined(NDEBUG)
inline void* FAR_FN(malloc_align)(size_t size, size_t align) {
  assert(size % align == 0);
  return aligned_alloc(align, size);
};
#else  // debug
#ifdef __cplusplus
#define malloc_align(s, a) aligned_alloc(a, s)
#else  // __cplusplus
#define far_malloc_align(s, a) aligned_alloc(a, s)
#endif  // __cplusplus
#endif  // debug
#ifdef __cplusplus
#define free_align(p) free(p)
#else  // __cplusplus
#define far_free_align(p) free(p)
#endif  // __cplusplus
#endif
#ifdef __cplusplus
namespace libfar {
#endif
inline byte16* FAR_FN(malloc_align16)(size_t size) {
  return (byte16*)FAR_FN(malloc_align)(size, 16);
}
inline void FAR_FN(free_align16)(byte16* ptr) {
  FAR_FN(free_align)((char*)ptr);
}
inline byte32* FAR_FN(malloc_align32)(size_t size) {
  return (byte32*)FAR_FN(malloc_align)(size, 32);
}
inline void FAR_FN(free_align32)(byte32* ptr) {
  FAR_FN(free_align)((char*)ptr);
}
inline byte64* FAR_FN(malloc_align64)(size_t size) {
  return (byte64*)FAR_FN(malloc_align)(size, 64);
}
inline void FAR_FN(free_align64)(byte64* ptr) {
  FAR_FN(free_align)((char*)ptr);
}
#ifdef __cplusplus
#include "memory_cxx.h"
}  // namespace libfar
#endif
#endif  //! defined(FAR_CUSTOM_ALLOCATORS)

// Pointer aliasing optimization
#define FAR_RESTRICT __restrict

// Unused parameter
#define FAR_UNUSED(x) (void)x

#ifdef _MSC_VER
#include <intrin.h>
#endif  // _MSC_VER
