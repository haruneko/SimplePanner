// parameter_smoother.h
// One-pole IIR filter for smooth parameter transitions

#pragma once

#include <cmath>
#include <algorithm>

namespace Steinberg {
namespace SimplePanner {

/**
 * @brief One-pole IIR filter for smooth parameter transitions
 *
 * Provides smooth parameter changes to avoid clicks and pops in audio processing.
 * Uses a simple one-pole low-pass filter with configurable smoothing time.
 */
class ParameterSmoother {
public:
    /**
     * @brief Constructor
     * @param smoothingTimeMs Smoothing time constant in milliseconds (default: 10ms)
     *
     * The smoothing time is the time it takes to reach ~63% of the target value.
     */
    ParameterSmoother(float smoothingTimeMs = 10.0f)
        : mCurrentValue(0.0f)
        , mTargetValue(0.0f)
        , mSmoothingTimeMs(smoothingTimeMs)
        , mSampleRate(48000.0)
        , mAlpha(0.0f)
    {
        updateCoefficient();
    }

    /**
     * @brief Destructor
     */
    ~ParameterSmoother() = default;

    /**
     * @brief Set the sample rate
     * @param sampleRate Sample rate in Hz
     *
     * Must be called before processing audio.
     * Recalculates the smoothing coefficient based on sample rate.
     */
    void setSampleRate(double sampleRate) {
        mSampleRate = sampleRate;
        updateCoefficient();
    }

    /**
     * @brief Set smoothing time
     * @param smoothingTimeMs Smoothing time in milliseconds
     */
    void setSmoothingTime(float smoothingTimeMs) {
        mSmoothingTimeMs = smoothingTimeMs;
        updateCoefficient();
    }

    /**
     * @brief Set the target value
     * @param target Target value to smooth towards
     *
     * The smoother will gradually transition from current value to target.
     */
    void setTarget(float target) {
        mTargetValue = target;
    }

    /**
     * @brief Get the next smoothed value
     * @return Current smoothed value
     *
     * Call this once per audio sample to get the smoothed parameter value.
     */
    float getNext() {
        // One-pole IIR: y[n] = y[n-1] + alpha * (target - y[n-1])
        mCurrentValue += mAlpha * (mTargetValue - mCurrentValue);
        return mCurrentValue;
    }

    /**
     * @brief Reset to a specific value immediately
     * @param value Value to reset to
     *
     * Sets both current and target values, causing no smoothing.
     */
    void reset(float value) {
        mCurrentValue = value;
        mTargetValue = value;
    }

    /**
     * @brief Check if currently smoothing
     * @return True if current value differs from target
     *
     * Returns false when the smoother has converged to the target value.
     */
    bool isSmoothing() const {
        // Consider converged if difference is very small (0.1%)
        const float epsilon = 0.001f;
        return std::abs(mCurrentValue - mTargetValue) > epsilon;
    }

    /**
     * @brief Get current value without advancing
     * @return Current value
     */
    float getCurrentValue() const {
        return mCurrentValue;
    }

    /**
     * @brief Get target value
     * @return Target value
     */
    float getTargetValue() const {
        return mTargetValue;
    }

private:
    /**
     * @brief Update the smoothing coefficient
     *
     * Calculates alpha based on smoothing time and sample rate.
     * alpha = 1 - exp(-1 / (tau * sampleRate))
     * where tau is the time constant in seconds.
     */
    void updateCoefficient() {
        if (mSampleRate <= 0.0 || mSmoothingTimeMs <= 0.0f) {
            mAlpha = 1.0f;  // No smoothing
            return;
        }

        // Convert smoothing time to seconds
        float smoothingTimeSec = mSmoothingTimeMs / 1000.0f;

        // Calculate one-pole IIR coefficient
        // tau = time constant (time to reach ~63% of target)
        float tau = smoothingTimeSec;
        mAlpha = 1.0f - std::exp(-1.0f / (tau * static_cast<float>(mSampleRate)));

        // Clamp to valid range
        mAlpha = std::max(0.0f, std::min(1.0f, mAlpha));
    }

    float mCurrentValue;        ///< Current smoothed value
    float mTargetValue;         ///< Target value to smooth towards
    float mSmoothingTimeMs;     ///< Smoothing time in milliseconds
    double mSampleRate;         ///< Sample rate in Hz
    float mAlpha;               ///< Smoothing coefficient
};

} // namespace SimplePanner
} // namespace Steinberg
