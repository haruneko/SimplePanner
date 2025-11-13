// test_delay_line.cpp
// Unit tests for DelayLine class

#include "delay_line.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------------
// Basic Operation Tests
//------------------------------------------------------------------------------

TEST(DelayLine, NoDelay_OutputEqualsInput) {
    DelayLine delay;
    delay.resize(1000);  // 1000 samples max
    delay.setDelay(0);   // No delay

    // Process samples with no delay
    EXPECT_FLOAT_EQ(delay.process(1.0f), 0.0f);  // First output is 0 (empty buffer)
    EXPECT_FLOAT_EQ(delay.process(2.0f), 1.0f);  // Second output is previous input
    EXPECT_FLOAT_EQ(delay.process(3.0f), 2.0f);
    EXPECT_FLOAT_EQ(delay.process(4.0f), 3.0f);
}

TEST(DelayLine, OneSampleDelay_OutputDelayedByOne) {
    DelayLine delay;
    delay.resize(1000);
    delay.setDelay(1);

    // With 1 sample delay, output should be delayed by 1 sample
    EXPECT_FLOAT_EQ(delay.process(1.0f), 0.0f);  // Initial: buffer empty
    EXPECT_FLOAT_EQ(delay.process(2.0f), 1.0f);  // Now we get first input (1 sample delay)
    EXPECT_FLOAT_EQ(delay.process(3.0f), 2.0f);
    EXPECT_FLOAT_EQ(delay.process(4.0f), 3.0f);
    EXPECT_FLOAT_EQ(delay.process(5.0f), 4.0f);
}

TEST(DelayLine, TenSampleDelay_OutputDelayedByTen) {
    DelayLine delay;
    delay.resize(1000);
    delay.setDelay(10);

    // Fill buffer with values
    for (int i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(delay.process(static_cast<float>(i + 1)), 0.0f);
    }

    // Now outputs should be delayed by 10 samples
    EXPECT_FLOAT_EQ(delay.process(11.0f), 1.0f);
    EXPECT_FLOAT_EQ(delay.process(12.0f), 2.0f);
    EXPECT_FLOAT_EQ(delay.process(13.0f), 3.0f);
}

TEST(DelayLine, MaxDelay_CanDelayUpToBufferSize) {
    size_t bufferSize = 100;
    DelayLine delay;
    delay.resize(bufferSize);
    delay.setDelay(bufferSize);

    // Fill entire buffer
    for (size_t i = 0; i < bufferSize; ++i) {
        EXPECT_FLOAT_EQ(delay.process(static_cast<float>(i + 1)), 0.0f);
    }

    // Now read back delayed values
    EXPECT_FLOAT_EQ(delay.process(101.0f), 1.0f);
    EXPECT_FLOAT_EQ(delay.process(102.0f), 2.0f);
}

//------------------------------------------------------------------------------
// Boundary Condition Tests
//------------------------------------------------------------------------------

TEST(DelayLine, EmptyBuffer_OutputsZero) {
    DelayLine delay;
    delay.resize(100);
    delay.setDelay(5);

    // Reading from empty buffer should return 0
    for (int i = 0; i < 5; ++i) {
        EXPECT_FLOAT_EQ(delay.process(0.0f), 0.0f);
    }
}

TEST(DelayLine, BufferOverflow_OldDataOverwritten) {
    DelayLine delay;
    delay.resize(5);  // Very small buffer
    delay.setDelay(5);

    // Fill buffer completely
    delay.process(1.0f);
    delay.process(2.0f);
    delay.process(3.0f);
    delay.process(4.0f);
    delay.process(5.0f);

    // Overflow - should overwrite oldest data
    delay.process(6.0f);
    delay.process(7.0f);

    // Read back - oldest values (1, 2) should be gone
    EXPECT_FLOAT_EQ(delay.process(8.0f), 3.0f);
    EXPECT_FLOAT_EQ(delay.process(9.0f), 4.0f);
}

TEST(DelayLine, DelayChange_ImmediateEffect) {
    DelayLine delay;
    delay.resize(100);
    delay.setDelay(5);

    // Process with 5 sample delay
    for (int i = 0; i < 10; ++i) {
        delay.process(static_cast<float>(i + 1));
    }

    // Change delay to 2 samples
    delay.setDelay(2);

    // Next output should reflect new delay
    // We wrote: 1,2,3,4,5,6,7,8,9,10
    // With delay=2, we read 2 samples back: should get 9
    float output = delay.process(11.0f);
    EXPECT_FLOAT_EQ(output, 9.0f);
}

TEST(DelayLine, ZeroDelayAfterNonZero_Works) {
    DelayLine delay;
    delay.resize(100);
    delay.setDelay(5);

    // Process with delay
    delay.process(1.0f);
    delay.process(2.0f);
    delay.process(3.0f);

    // Change to zero delay
    delay.setDelay(0);

    // Should get immediate output (previous input)
    EXPECT_FLOAT_EQ(delay.process(4.0f), 3.0f);
    EXPECT_FLOAT_EQ(delay.process(5.0f), 4.0f);
}

//------------------------------------------------------------------------------
// Circular Buffer Tests
//------------------------------------------------------------------------------

TEST(DelayLine, CircularBuffer_WrapsAround) {
    DelayLine delay;
    size_t bufferSize = 8;
    delay.resize(bufferSize);
    delay.setDelay(4);

    // Write more samples than buffer size to test wraparound
    for (int i = 0; i < 20; ++i) {
        float input = static_cast<float>(i + 1);
        float output = delay.process(input);

        // After initial 4 samples (delay amount), check delayed output
        if (i >= 4) {
            EXPECT_FLOAT_EQ(output, static_cast<float>(i - 3));
        }
    }
}

TEST(DelayLine, CircularBuffer_IndexManagement) {
    DelayLine delay;
    delay.resize(4);  // Very small for easy testing
    delay.setDelay(2);

    // Process multiple cycles through buffer
    EXPECT_FLOAT_EQ(delay.process(1.0f), 0.0f);  // [1, 0, 0, 0], read index wraps
    EXPECT_FLOAT_EQ(delay.process(2.0f), 0.0f);  // [1, 2, 0, 0]
    EXPECT_FLOAT_EQ(delay.process(3.0f), 1.0f);  // [1, 2, 3, 0], reading 1
    EXPECT_FLOAT_EQ(delay.process(4.0f), 2.0f);  // [1, 2, 3, 4], reading 2
    EXPECT_FLOAT_EQ(delay.process(5.0f), 3.0f);  // [5, 2, 3, 4], reading 3 (wrapped write)
    EXPECT_FLOAT_EQ(delay.process(6.0f), 4.0f);  // [5, 6, 3, 4], reading 4
}

//------------------------------------------------------------------------------
// Reset Tests
//------------------------------------------------------------------------------

TEST(DelayLine, Reset_ClearsBuffer) {
    DelayLine delay;
    delay.resize(100);
    delay.setDelay(5);

    // Fill with data
    for (int i = 0; i < 10; ++i) {
        delay.process(static_cast<float>(i + 1));
    }

    // Reset
    delay.reset();

    // Buffer should be cleared
    for (int i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(delay.process(0.0f), 0.0f);
    }
}

TEST(DelayLine, Reset_PreservesDelayAmount) {
    DelayLine delay;
    delay.resize(100);
    delay.setDelay(7);

    // Process and reset
    delay.process(1.0f);
    delay.reset();

    // Delay amount should be preserved, buffer cleared
    for (int i = 0; i < 7; ++i) {
        EXPECT_FLOAT_EQ(delay.process(0.0f), 0.0f);
    }

    // After filling delay again, should still be 7 samples
    for (int i = 0; i < 7; ++i) {
        delay.process(static_cast<float>(i + 1));
    }
    EXPECT_FLOAT_EQ(delay.process(8.0f), 1.0f);
}

TEST(DelayLine, Reset_CanProcessImmediately) {
    DelayLine delay;
    delay.resize(100);
    delay.setDelay(3);

    delay.process(1.0f);
    delay.process(2.0f);
    delay.reset();

    // Should be able to process immediately after reset
    EXPECT_FLOAT_EQ(delay.process(5.0f), 0.0f);
    EXPECT_FLOAT_EQ(delay.process(6.0f), 0.0f);
    EXPECT_FLOAT_EQ(delay.process(7.0f), 0.0f);
    EXPECT_FLOAT_EQ(delay.process(8.0f), 5.0f);
}

//------------------------------------------------------------------------------
// Edge Cases
//------------------------------------------------------------------------------

TEST(DelayLine, DelayLargerThanBuffer_ClampedToBufferSize) {
    DelayLine delay;
    size_t bufferSize = 10;
    delay.resize(bufferSize);
    delay.setDelay(1000);  // Much larger than buffer

    // Fill buffer
    for (size_t i = 0; i < bufferSize; ++i) {
        delay.process(static_cast<float>(i + 1));
    }

    // Should get output after buffer size worth of delay
    float output = delay.process(11.0f);
    EXPECT_FLOAT_EQ(output, 1.0f);
}

TEST(DelayLine, ResizeBuffer_PreservesDelay) {
    DelayLine delay;
    delay.resize(50);
    delay.setDelay(10);

    // Process some samples
    for (int i = 0; i < 15; ++i) {
        delay.process(static_cast<float>(i + 1));
    }

    // Resize to larger buffer
    delay.resize(200);

    // Delay setting should be preserved (buffer will be cleared though)
    // Continue processing
    for (int i = 0; i < 10; ++i) {
        delay.process(static_cast<float>(i + 100));
    }

    // Should still have 10 sample delay
    EXPECT_FLOAT_EQ(delay.process(200.0f), 100.0f);
}

TEST(DelayLine, MultipleResets_Stable) {
    DelayLine delay;
    delay.resize(100);
    delay.setDelay(5);

    // Reset multiple times
    for (int r = 0; r < 5; ++r) {
        delay.reset();

        // Process after each reset
        for (int i = 0; i < 10; ++i) {
            float input = static_cast<float>(i + 1);
            delay.process(input);
        }
    }

    // Should still work correctly
    EXPECT_FLOAT_EQ(delay.process(11.0f), 6.0f);
}

//------------------------------------------------------------------------------
// Signal Integrity Tests
//------------------------------------------------------------------------------

TEST(DelayLine, SignalIntegrity_SineWave) {
    DelayLine delay;
    delay.resize(1000);
    size_t delayAmount = 50;
    delay.setDelay(delayAmount);

    const float frequency = 440.0f;
    const float sampleRate = 48000.0f;
    const float omega = 2.0f * 3.14159265359f * frequency / sampleRate;

    // Process sine wave
    std::vector<float> inputs;
    std::vector<float> outputs;

    for (int i = 0; i < 200; ++i) {
        float input = std::sin(omega * i);
        float output = delay.process(input);
        inputs.push_back(input);
        outputs.push_back(output);
    }

    // After delay amount, output should match input from delay samples ago
    for (size_t i = delayAmount + 10; i < 150; ++i) {
        EXPECT_NEAR(outputs[i], inputs[i - delayAmount], 0.0001f);
    }
}

TEST(DelayLine, SignalIntegrity_ImpulseResponse) {
    DelayLine delay;
    delay.resize(100);
    size_t delayAmount = 20;
    delay.setDelay(delayAmount);

    // Send impulse
    delay.process(1.0f);

    // Send zeros
    for (size_t i = 0; i < delayAmount - 1; ++i) {
        float output = delay.process(0.0f);
        EXPECT_FLOAT_EQ(output, 0.0f);  // Should get zeros before impulse arrives
    }

    // Should get impulse at exact delay point
    float impulseOutput = delay.process(0.0f);
    EXPECT_FLOAT_EQ(impulseOutput, 1.0f);

    // After impulse, should get zeros again
    for (int i = 0; i < 10; ++i) {
        float output = delay.process(0.0f);
        EXPECT_FLOAT_EQ(output, 0.0f);
    }
}
