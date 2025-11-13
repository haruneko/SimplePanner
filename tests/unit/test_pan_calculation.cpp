// test_pan_calculation.cpp
// Unit tests for pan calculation (equal power panning)

#include "pan_calculator.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------------
// Equal Power Panning Tests
//------------------------------------------------------------------------------

TEST(PanCalculation, FullLeft_LeftOneRightZero) {
    PanGains gains = calculatePanGains(-100.0f);

    EXPECT_NEAR(gains.left, 1.0f, 0.0001f);
    EXPECT_NEAR(gains.right, 0.0f, 0.0001f);
}

TEST(PanCalculation, FullRight_LeftZeroRightOne) {
    PanGains gains = calculatePanGains(100.0f);

    EXPECT_NEAR(gains.left, 0.0f, 0.0001f);
    EXPECT_NEAR(gains.right, 1.0f, 0.0001f);
}

TEST(PanCalculation, Center_EqualPower) {
    PanGains gains = calculatePanGains(0.0f);

    // Center should be approximately 0.707 (1/sqrt(2)) for both
    EXPECT_NEAR(gains.left, 0.7071f, 0.001f);
    EXPECT_NEAR(gains.right, 0.7071f, 0.001f);
}

TEST(PanCalculation, HalfLeft_CorrectGains) {
    PanGains gains = calculatePanGains(-50.0f);

    // At -50, left should be stronger than right
    EXPECT_GT(gains.left, gains.right);
    EXPECT_GT(gains.left, 0.7f);
    EXPECT_LT(gains.right, 0.7f);
}

TEST(PanCalculation, HalfRight_CorrectGains) {
    PanGains gains = calculatePanGains(50.0f);

    // At +50, right should be stronger than left
    EXPECT_GT(gains.right, gains.left);
    EXPECT_GT(gains.right, 0.7f);
    EXPECT_LT(gains.left, 0.7f);
}

//------------------------------------------------------------------------------
// Power Preservation Tests
//------------------------------------------------------------------------------

TEST(PanCalculation, PowerConservation_FullLeft) {
    PanGains gains = calculatePanGains(-100.0f);
    float power = gains.left * gains.left + gains.right * gains.right;

    EXPECT_NEAR(power, 1.0f, 0.01f);
}

TEST(PanCalculation, PowerConservation_FullRight) {
    PanGains gains = calculatePanGains(100.0f);
    float power = gains.left * gains.left + gains.right * gains.right;

    EXPECT_NEAR(power, 1.0f, 0.01f);
}

TEST(PanCalculation, PowerConservation_Center) {
    PanGains gains = calculatePanGains(0.0f);
    float power = gains.left * gains.left + gains.right * gains.right;

    EXPECT_NEAR(power, 1.0f, 0.01f);
}

TEST(PanCalculation, PowerConservation_AllPositions) {
    // Test power conservation at many pan positions
    for (float pan = -100.0f; pan <= 100.0f; pan += 5.0f) {
        PanGains gains = calculatePanGains(pan);
        float power = gains.left * gains.left + gains.right * gains.right;

        EXPECT_NEAR(power, 1.0f, 0.01f) << "Failed at pan = " << pan;
    }
}

//------------------------------------------------------------------------------
// Intermediate Values Tests
//------------------------------------------------------------------------------

TEST(PanCalculation, QuarterLeft_CorrectRatio) {
    PanGains gains = calculatePanGains(-25.0f);

    EXPECT_GT(gains.left, 0.8f);
    EXPECT_LT(gains.left, 0.95f);
    EXPECT_GT(gains.right, 0.3f);
    EXPECT_LT(gains.right, 0.6f);
}

TEST(PanCalculation, QuarterRight_CorrectRatio) {
    PanGains gains = calculatePanGains(25.0f);

    EXPECT_GT(gains.right, 0.8f);
    EXPECT_LT(gains.right, 0.95f);
    EXPECT_GT(gains.left, 0.3f);
    EXPECT_LT(gains.left, 0.6f);
}

//------------------------------------------------------------------------------
// Symmetry Tests
//------------------------------------------------------------------------------

TEST(PanCalculation, Symmetry_OppositePositions) {
    PanGains gainsLeft = calculatePanGains(-30.0f);
    PanGains gainsRight = calculatePanGains(30.0f);

    // Left pan at -30 should have same gain values as right pan at +30, but swapped
    EXPECT_NEAR(gainsLeft.left, gainsRight.right, 0.0001f);
    EXPECT_NEAR(gainsLeft.right, gainsRight.left, 0.0001f);
}

TEST(PanCalculation, Symmetry_AroundCenter) {
    for (float offset = 0.0f; offset <= 100.0f; offset += 10.0f) {
        PanGains gainsLeft = calculatePanGains(-offset);
        PanGains gainsRight = calculatePanGains(offset);

        EXPECT_NEAR(gainsLeft.left, gainsRight.right, 0.0001f)
            << "Failed at offset = " << offset;
        EXPECT_NEAR(gainsLeft.right, gainsRight.left, 0.0001f)
            << "Failed at offset = " << offset;
    }
}

//------------------------------------------------------------------------------
// Edge Cases
//------------------------------------------------------------------------------

TEST(PanCalculation, BeyondRange_ClampsToFullLeft) {
    PanGains gains = calculatePanGains(-150.0f);

    // Should be clamped to full left
    EXPECT_NEAR(gains.left, 1.0f, 0.0001f);
    EXPECT_NEAR(gains.right, 0.0f, 0.0001f);
}

TEST(PanCalculation, BeyondRange_ClampsToFullRight) {
    PanGains gains = calculatePanGains(150.0f);

    // Should be clamped to full right
    EXPECT_NEAR(gains.left, 0.0f, 0.0001f);
    EXPECT_NEAR(gains.right, 1.0f, 0.0001f);
}

//------------------------------------------------------------------------------
// Monotonicity Tests
//------------------------------------------------------------------------------

TEST(PanCalculation, Monotonic_LeftGainDecreases) {
    // As pan moves from left to right, left gain should decrease monotonically
    float prevLeftGain = 2.0f;

    for (float pan = -100.0f; pan <= 100.0f; pan += 1.0f) {
        PanGains gains = calculatePanGains(pan);
        EXPECT_LE(gains.left, prevLeftGain + 0.0001f)
            << "Left gain increased at pan = " << pan;
        prevLeftGain = gains.left;
    }
}

TEST(PanCalculation, Monotonic_RightGainIncreases) {
    // As pan moves from left to right, right gain should increase monotonically
    float prevRightGain = -1.0f;

    for (float pan = -100.0f; pan <= 100.0f; pan += 1.0f) {
        PanGains gains = calculatePanGains(pan);
        EXPECT_GE(gains.right, prevRightGain - 0.0001f)
            << "Right gain decreased at pan = " << pan;
        prevRightGain = gains.right;
    }
}
