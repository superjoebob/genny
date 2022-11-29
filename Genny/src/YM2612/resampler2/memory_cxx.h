#pragma once

#include "base.h"

#ifdef __cplusplus
namespace libfar {

template <typename T>
T* malloc_align(size_t size);

template <>
inline float128* malloc_align(size_t size) {
  return reinterpret_cast<float128*>(malloc_align16(size));
}

template <>
inline float256* malloc_align(size_t size) {
  return reinterpret_cast<float256*>(malloc_align32(size));
}

template <>
inline float512* malloc_align(size_t size) {
  return reinterpret_cast<float512*>(malloc_align64(size));
}

template <>
inline short128* malloc_align(size_t size) {
  return reinterpret_cast<short128*>(malloc_align16(size));
}

template <>
inline short256* malloc_align(size_t size) {
  return reinterpret_cast<short256*>(malloc_align32(size));
}

template <>
inline short512* malloc_align(size_t size) {
  return reinterpret_cast<short512*>(malloc_align64(size));
}

}  // namespace libfar
#endif  // __cplusplus
