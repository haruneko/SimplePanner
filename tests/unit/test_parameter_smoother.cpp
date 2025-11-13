// test_parameter_smoother.cpp
// Unit tests for ParameterSmoother class

#include "parameter_smoother.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------------
// Basic Operation Tests
//------------------------------------------------------------------------------

TEST(ParameterSmoother, InitialValue_IsZero) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);

    EXPECT_FLOAT_EQ(smoother.getNext(), 0.0f);
}

TEST(ParameterSmoother, SetTarget_GraduallyApproaches) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.setTarget(1.0f);

    // Get first few samples - should gradually increase
    float prev = 0.0f;
    for (int i = 0; i < 10; ++i) {
        float current = smoother.getNext();
        EXPECT_GT(current, prev);  // Should be increasing
        EXPECT_LT(current, 1.0f);  // Should not overshoot
        prev = current;
    }
}

TEST(ParameterSmoother, Convergence_ReachesTarget) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.setTarget(1.0f);

    // Process enough samples to converge (about 5x time constant)
    float value = 0.0f;
    for (int i = 0; i < 2400; ++i) {  // 50ms at 48kHz
        value = smoother.getNext();
    }

    // Should be very close to target (within 1%)
    EXPECT_NEAR(value, 1.0f, 0.01f);
}

TEST(ParameterSmoother, SameTarget_NoSmoothing) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(0.5f);  // Start at target value

    // Set same target
    smoother.setTarget(0.5f);
    float value = smoother.getNext();

    // Should stay at target (no change)
    EXPECT_NEAR(value, 0.5f, 0.0001f);
}

//------------------------------------------------------------------------------
// Reset Tests
//------------------------------------------------------------------------------

TEST(ParameterSmoother, Reset_ImmediateValue) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.setTarget(0.5f);

    // Process a few samples
    for (int i = 0; i < 10; ++i) {
        smoother.getNext();
    }

    // Reset to new value
    smoother.reset(0.8f);

    // Should immediately be at reset value
    EXPECT_FLOAT_EQ(smoother.getNext(), 0.8f);
}

TEST(ParameterSmoother, Reset_NoSmoothing) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(0.7f);

    // All subsequent calls should return reset value until new target
    for (int i = 0; i < 100; ++i) {
        EXPECT_FLOAT_EQ(smoother.getNext(), 0.7f);
    }
}

TEST(ParameterSmoother, ResetThenTarget_Smooths) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(0.0f);
    smoother.setTarget(1.0f);

    // Should start smoothing from 0 to 1
    float value = smoother.getNext();
    EXPECT_GT(value, 0.0f);
    EXPECT_LT(value, 1.0f);
}

//------------------------------------------------------------------------------
// Smoothing Time Tests
//------------------------------------------------------------------------------

TEST(ParameterSmoother, SmoothingTime_Approximately10ms) {
    ParameterSmoother smoother;
    double sampleRate = 48000.0;
    smoother.setSampleRate(sampleRate);
    smoother.setTarget(1.0f);

    // Check value after 10ms (480 samples at 48kHz)
    float value = 0.0f;
    for (int i = 0; i < 480; ++i) {
        value = smoother.getNext();
    }

    // After 10ms, should be at least 60% of the way to target (one pole filter)
    EXPECT_GT(value, 0.6f);
    // But not fully converged yet
    EXPECT_LT(value, 0.99f);
}

TEST(ParameterSmoother, DifferentSampleRates_ConsistentTime) {
    // Test at 44.1kHz
    ParameterSmoother smoother1;
    smoother1.setSampleRate(44100.0);
    smoother1.setTarget(1.0f);

    // Process 10ms worth of samples (441 samples)
    float value1 = 0.0f;
    for (int i = 0; i < 441; ++i) {
        value1 = smoother1.getNext();
    }

    // Test at 96kHz
    ParameterSmoother smoother2;
    smoother2.setSampleRate(96000.0);
    smoother2.setTarget(1.0f);

    // Process 10ms worth of samples (960 samples)
    float value2 = 0.0f;
    for (int i = 0; i < 960; ++i) {
        value2 = smoother2.getNext();
    }

    // Both should reach similar values after 10ms
    EXPECT_NEAR(value1, value2, 0.05f);
}

//------------------------------------------------------------------------------
// Boundary Condition Tests
//------------------------------------------------------------------------------

TEST(ParameterSmoother, NegativeValues_Work) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.setTarget(-1.0f);

    // Should smoothly approach negative target
    for (int i = 0; i < 10; ++i) {
        float value = smoother.getNext();
        EXPECT_LT(value, 0.0f);  // Should be negative
    }
}

TEST(ParameterSmoother, LargeJump_Smooths) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(0.0f);
    smoother.setTarget(100.0f);

    // Should gradually increase even for large jumps
    float prev = 0.0f;
    for (int i = 0; i < 20; ++i) {
        float current = smoother.getNext();
        EXPECT_GT(current, prev);
        EXPECT_LE(current, 100.0f);
        prev = current;
    }
}

TEST(ParameterSmoother, MultipleTargetChanges_Tracks) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(0.5f);

    // Change target
    smoother.setTarget(0.8f);
    for (int i = 0; i < 100; ++i) smoother.getNext();

    // Should be moving towards 0.8 (but not fully there yet)
    float value = smoother.getCurrentValue();
    EXPECT_GT(value, 0.5f);  // Should have moved from start
    EXPECT_LT(value, 0.8f);  // Should not have fully converged yet

    // Change to new target
    smoother.setTarget(0.3f);
    float newValue = smoother.getNext();

    // Should start moving towards 0.3
    EXPECT_LT(newValue, value);  // Should be decreasing
}

//------------------------------------------------------------------------------
// isSmoothing() Tests
//------------------------------------------------------------------------------

TEST(ParameterSmoother, IsSmoothing_InitiallyFalse) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);

    EXPECT_FALSE(smoother.isSmoothing());
}

TEST(ParameterSmoother, IsSmoothing_TrueWhenActive) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.setTarget(1.0f);

    // Should be smoothing right after setting target
    EXPECT_TRUE(smoother.isSmoothing());

    // Should still be smoothing after a few samples
    for (int i = 0; i < 10; ++i) {
        smoother.getNext();
    }
    EXPECT_TRUE(smoother.isSmoothing());
}

TEST(ParameterSmoother, IsSmoothing_FalseAfterConvergence) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.setTarget(1.0f);

    // Process enough samples to converge (about 7x time constant = 70ms)
    for (int i = 0; i < 3840; ++i) {  // 80ms at 48kHz
        smoother.getNext();
    }

    // Should no longer be smoothing
    EXPECT_FALSE(smoother.isSmoothing());
}

TEST(ParameterSmoother, IsSmoothing_FalseAfterReset) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.setTarget(1.0f);
    smoother.getNext();

    EXPECT_TRUE(smoother.isSmoothing());

    // Reset to target value
    smoother.reset(1.0f);

    EXPECT_FALSE(smoother.isSmoothing());
}

//------------------------------------------------------------------------------
// Edge Cases
//------------------------------------------------------------------------------

TEST(ParameterSmoother, ZeroToZero_NoSmoothing) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(0.0f);
    smoother.setTarget(0.0f);

    EXPECT_FALSE(smoother.isSmoothing());
    EXPECT_FLOAT_EQ(smoother.getNext(), 0.0f);
}

TEST(ParameterSmoother, VerySmallChange_Smooths) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(1.0f);
    smoother.setTarget(1.001f);

    // Should still smooth even tiny changes
    float value = smoother.getNext();
    EXPECT_GT(value, 1.0f);
    EXPECT_LT(value, 1.001f);
}

TEST(ParameterSmoother, RapidTargetChanges_Stable) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);

    // Rapidly change targets
    for (int iteration = 0; iteration < 100; ++iteration) {
        float target = (iteration % 2 == 0) ? 0.0f : 1.0f;
        smoother.setTarget(target);
        smoother.getNext();
    }

    // Should not crash or produce NaN
    float value = smoother.getNext();
    EXPECT_FALSE(std::isnan(value));
    EXPECT_FALSE(std::isinf(value));
}

//------------------------------------------------------------------------------
// getCurrentValue() Tests
//------------------------------------------------------------------------------

TEST(ParameterSmoother, GetCurrentValue_ReturnsLastValue) {
    ParameterSmoother smoother;
    smoother.setSampleRate(48000.0);
    smoother.reset(0.5f);

    EXPECT_FLOAT_EQ(smoother.getCurrentValue(), 0.5f);

    smoother.setTarget(1.0f);
    float value = smoother.getNext();

    EXPECT_FLOAT_EQ(smoother.getCurrentValue(), value);
}
