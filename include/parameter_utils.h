#pragma once

#include "plugids.h"
#include <cmath>
#include <algorithm>

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// Parameter Conversion Utilities
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Pan Conversion: Normalized (0.0-1.0) ↔ Plain (-100.0 to +100.0)
//------------------------------------------------------------------------

inline float normalizedToPan(float normalized) {
    return ParamRange::kPanMin + normalized * (ParamRange::kPanMax - ParamRange::kPanMin);
}

inline float panToNormalized(float pan) {
    return (pan - ParamRange::kPanMin) / (ParamRange::kPanMax - ParamRange::kPanMin);
}

inline float clampPan(float pan) {
    return std::clamp(pan, ParamRange::kPanMin, ParamRange::kPanMax);
}

//------------------------------------------------------------------------
// Gain Conversion: Normalized (0.0-1.0) ↔ dB (-60.0 to +6.0) ↔ Linear
//------------------------------------------------------------------------

inline float normalizedToDb(float normalized) {
    return ParamRange::kGainMin + normalized * (ParamRange::kGainMax - ParamRange::kGainMin);
}

inline float dbToNormalized(float db) {
    return (db - ParamRange::kGainMin) / (ParamRange::kGainMax - ParamRange::kGainMin);
}

inline float dbToLinear(float db) {
    // -60dB is treated as -∞ (complete mute)
    if (db <= -60.0f) return 0.0f;
    return std::pow(10.0f, db / 20.0f);
}

inline float linearToDb(float linear) {
    // 0.0 or negative is treated as -∞ (-60dB)
    if (linear <= 0.0f) return -60.0f;
    return 20.0f * std::log10(linear);
}

inline float clampGain(float gain) {
    return std::clamp(gain, ParamRange::kGainMin, ParamRange::kGainMax);
}

//------------------------------------------------------------------------
// Delay Conversion: Normalized (0.0-1.0) ↔ ms (0.0-100.0) ↔ Samples
//------------------------------------------------------------------------

inline float normalizedToDelayMs(float normalized) {
    return ParamRange::kDelayMin + normalized * (ParamRange::kDelayMax - ParamRange::kDelayMin);
}

inline float delayMsToNormalized(float ms) {
    return (ms - ParamRange::kDelayMin) / (ParamRange::kDelayMax - ParamRange::kDelayMin);
}

inline size_t delayMsToSamples(float ms, double sampleRate) {
    return static_cast<size_t>(std::round(ms * sampleRate / 1000.0));
}

inline float delaySamplesToMs(size_t samples, double sampleRate) {
    return static_cast<float>(samples * 1000.0 / sampleRate);
}

inline float clampDelay(float delay) {
    return std::clamp(delay, ParamRange::kDelayMin, ParamRange::kDelayMax);
}

//------------------------------------------------------------------------
// Convenience Functions
//------------------------------------------------------------------------

// Direct conversion: Normalized → Linear Gain
inline float normalizedToLinearGain(float normalized) {
    return dbToLinear(normalizedToDb(normalized));
}

// Direct conversion: Normalized → Delay Samples
inline size_t normalizedToDelaySamples(float normalized, double sampleRate) {
    return delayMsToSamples(normalizedToDelayMs(normalized), sampleRate);
}

} // namespace SimplePanner
} // namespace Steinberg
