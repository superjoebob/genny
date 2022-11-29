#pragma once

#include "base.h"

// Query CPU support for SIMD extensions at runtime.
#include "cpuid.h"

// Adapt channel count (convert mono/stereo).
#include "adapt.h"

// Resample audio signal (change sampling frequency).
#include "resample.h"
