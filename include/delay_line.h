// delay_line.h
// Circular buffer based delay line for audio processing

#pragma once

#include <vector>
#include <cstddef>
#include <algorithm>

namespace Steinberg {
namespace SimplePanner {

/**
 * @brief Circular buffer based delay line
 *
 * Provides sample-accurate delay for audio signals using a circular buffer.
 * Delay amount can be changed in real-time without artifacts (no interpolation).
 */
class DelayLine {
public:
    /**
     * @brief Constructor
     * Initializes delay line with empty buffer
     */
    DelayLine()
        : mBuffer()
        , mWriteIndex(0)
        , mDelaySamples(0)
    {
    }

    /**
     * @brief Destructor
     */
    ~DelayLine() = default;

    /**
     * @brief Resize the internal buffer
     * @param size Maximum delay capacity in samples
     *
     * Clears the buffer and resets indices.
     * Must be called before processing audio.
     */
    void resize(size_t size) {
        mBuffer.resize(size, 0.0f);
        reset();
    }

    /**
     * @brief Set the delay amount
     * @param delaySamples Delay in samples (0 to buffer size)
     *
     * If delaySamples exceeds buffer size, it will be clamped.
     * Change takes effect immediately.
     */
    void setDelay(size_t delaySamples) {
        mDelaySamples = std::min(delaySamples, mBuffer.size());
    }

    /**
     * @brief Get current delay amount
     * @return Current delay in samples
     */
    size_t getDelay() const {
        return mDelaySamples;
    }

    /**
     * @brief Process a single sample
     * @param input Input sample
     * @return Delayed output sample
     *
     * Writes input to buffer and reads delayed sample.
     * Call this for every audio sample.
     */
    float process(float input) {
        if (mBuffer.empty()) {
            return 0.0f;
        }

        size_t bufferSize = mBuffer.size();

        // Calculate read position
        // Special handling for delay=0: read from 1 sample back (minimum delay)
        // For delay>0: read from delay samples back
        size_t effectiveDelay;
        if (mDelaySamples == 0) {
            effectiveDelay = 1;  // Minimum 1-sample delay
        } else {
            effectiveDelay = mDelaySamples;
        }

        size_t readIndex;
        if (mWriteIndex >= effectiveDelay) {
            readIndex = mWriteIndex - effectiveDelay;
        } else {
            readIndex = bufferSize - (effectiveDelay - mWriteIndex);
        }

        // Read delayed sample
        float output = mBuffer[readIndex];

        // Write input sample at current write position
        mBuffer[mWriteIndex] = input;

        // Advance write index with wraparound
        mWriteIndex = (mWriteIndex + 1) % bufferSize;

        return output;
    }

    /**
     * @brief Reset the delay line
     *
     * Clears all samples in the buffer to zero.
     * Preserves the delay amount and buffer size.
     */
    void reset() {
        std::fill(mBuffer.begin(), mBuffer.end(), 0.0f);
        mWriteIndex = 0;
    }

    /**
     * @brief Get buffer size
     * @return Current buffer size in samples
     */
    size_t getBufferSize() const {
        return mBuffer.size();
    }

private:
    std::vector<float> mBuffer;      ///< Circular buffer for delayed samples
    size_t mWriteIndex;              ///< Current write position
    size_t mDelaySamples;            ///< Current delay amount in samples
};

} // namespace SimplePanner
} // namespace Steinberg
