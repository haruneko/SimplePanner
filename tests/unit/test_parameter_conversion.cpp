#include <gtest/gtest.h>
#include "parameter_utils.h"

using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------
// Pan Conversion Tests
//------------------------------------------------------------------------

TEST(ParameterConversion, PanNormalizedToPlain_BoundaryMin) {
    EXPECT_FLOAT_EQ(normalizedToPan(0.0f), -100.0f);
}

TEST(ParameterConversion, PanNormalizedToPlain_BoundaryMax) {
    EXPECT_FLOAT_EQ(normalizedToPan(1.0f), 100.0f);
}

TEST(ParameterConversion, PanNormalizedToPlain_Center) {
    EXPECT_FLOAT_EQ(normalizedToPan(0.5f), 0.0f);
}

TEST(ParameterConversion, PanPlainToNormalized_BoundaryMin) {
    EXPECT_FLOAT_EQ(panToNormalized(-100.0f), 0.0f);
}

TEST(ParameterConversion, PanPlainToNormalized_BoundaryMax) {
    EXPECT_FLOAT_EQ(panToNormalized(100.0f), 1.0f);
}

TEST(ParameterConversion, PanPlainToNormalized_Center) {
    EXPECT_FLOAT_EQ(panToNormalized(0.0f), 0.5f);
}

TEST(ParameterConversion, PanRoundTrip) {
    float original = -50.0f;
    float normalized = panToNormalized(original);
    float result = normalizedToPan(normalized);
    EXPECT_NEAR(result, original, 0.001f);
}

//------------------------------------------------------------------------
// Gain Conversion Tests
//------------------------------------------------------------------------

TEST(ParameterConversion, GainNormalizedToDb_Min) {
    EXPECT_FLOAT_EQ(normalizedToDb(0.0f), -60.0f);
}

TEST(ParameterConversion, GainNormalizedToDb_Max) {
    EXPECT_FLOAT_EQ(normalizedToDb(1.0f), 6.0f);
}

TEST(ParameterConversion, GainNormalizedToDb_Unity) {
    // 0dB は範囲 -60~+6 の中で (60/66) の位置
    float normalized = dbToNormalized(0.0f);
    EXPECT_NEAR(normalized, 60.0f / 66.0f, 0.001f);
    EXPECT_FLOAT_EQ(normalizedToDb(normalized), 0.0f);
}

TEST(ParameterConversion, GainDbToLinear_Mute) {
    EXPECT_FLOAT_EQ(dbToLinear(-60.0f), 0.0f);
}

TEST(ParameterConversion, GainDbToLinear_Unity) {
    EXPECT_FLOAT_EQ(dbToLinear(0.0f), 1.0f);
}

TEST(ParameterConversion, GainDbToLinear_Plus6dB) {
    // +6dB ≈ 1.9953 (正確には 10^(6/20))
    EXPECT_NEAR(dbToLinear(6.0f), 1.9953f, 0.001f);
}

TEST(ParameterConversion, GainDbToLinear_Minus6dB) {
    // -6dB ≈ 0.5012 (正確には 10^(-6/20))
    EXPECT_NEAR(dbToLinear(-6.0f), 0.5012f, 0.001f);
}

TEST(ParameterConversion, GainLinearToDb_Mute) {
    EXPECT_FLOAT_EQ(linearToDb(0.0f), -60.0f);
}

TEST(ParameterConversion, GainLinearToDb_Unity) {
    EXPECT_FLOAT_EQ(linearToDb(1.0f), 0.0f);
}

TEST(ParameterConversion, GainRoundTrip_Db) {
    float original = -12.5f;
    float normalized = dbToNormalized(original);
    float result = normalizedToDb(normalized);
    EXPECT_NEAR(result, original, 0.001f);
}

TEST(ParameterConversion, GainRoundTrip_Linear) {
    float originalDb = -12.0f;
    float linear = dbToLinear(originalDb);
    float resultDb = linearToDb(linear);
    EXPECT_NEAR(resultDb, originalDb, 0.001f);
}

//------------------------------------------------------------------------
// Delay Conversion Tests
//------------------------------------------------------------------------

TEST(ParameterConversion, DelayNormalizedToMs_Min) {
    EXPECT_FLOAT_EQ(normalizedToDelayMs(0.0f), 0.0f);
}

TEST(ParameterConversion, DelayNormalizedToMs_Max) {
    EXPECT_FLOAT_EQ(normalizedToDelayMs(1.0f), 100.0f);
}

TEST(ParameterConversion, DelayNormalizedToMs_Half) {
    EXPECT_FLOAT_EQ(normalizedToDelayMs(0.5f), 50.0f);
}

TEST(ParameterConversion, DelayMsToSamples_44100Hz_0ms) {
    EXPECT_EQ(delayMsToSamples(0.0f, 44100.0), 0u);
}

TEST(ParameterConversion, DelayMsToSamples_44100Hz_10ms) {
    // 10ms @ 44.1kHz = 441 samples
    EXPECT_EQ(delayMsToSamples(10.0f, 44100.0), 441u);
}

TEST(ParameterConversion, DelayMsToSamples_44100Hz_5ms) {
    // 5ms @ 44.1kHz = 220.5 → 221 samples (rounded)
    EXPECT_EQ(delayMsToSamples(5.0f, 44100.0), 221u);
}

TEST(ParameterConversion, DelayMsToSamples_48000Hz_10ms) {
    // 10ms @ 48kHz = 480 samples
    EXPECT_EQ(delayMsToSamples(10.0f, 48000.0), 480u);
}

TEST(ParameterConversion, DelayMsToSamples_48000Hz_100ms) {
    // 100ms @ 48kHz = 4800 samples
    EXPECT_EQ(delayMsToSamples(100.0f, 48000.0), 4800u);
}

TEST(ParameterConversion, DelaySamplesToMs_44100Hz_441samples) {
    EXPECT_NEAR(delaySamplesToMs(441u, 44100.0), 10.0f, 0.001f);
}

TEST(ParameterConversion, DelayRoundTrip_Ms) {
    float original = 37.5f;
    float normalized = delayMsToNormalized(original);
    float result = normalizedToDelayMs(normalized);
    EXPECT_NEAR(result, original, 0.001f);
}

TEST(ParameterConversion, DelayRoundTrip_Samples) {
    float originalMs = 25.0f;
    double sampleRate = 44100.0;
    size_t samples = delayMsToSamples(originalMs, sampleRate);
    float resultMs = delaySamplesToMs(samples, sampleRate);
    // サンプル精度による丸め誤差を許容
    EXPECT_NEAR(resultMs, originalMs, 0.1f);
}

//------------------------------------------------------------------------
// Clamp Tests
//------------------------------------------------------------------------

TEST(ParameterConversion, ClampPan_WithinRange) {
    EXPECT_FLOAT_EQ(clampPan(50.0f), 50.0f);
}

TEST(ParameterConversion, ClampPan_Below) {
    EXPECT_FLOAT_EQ(clampPan(-150.0f), -100.0f);
}

TEST(ParameterConversion, ClampPan_Above) {
    EXPECT_FLOAT_EQ(clampPan(150.0f), 100.0f);
}

TEST(ParameterConversion, ClampGain_WithinRange) {
    EXPECT_FLOAT_EQ(clampGain(-10.0f), -10.0f);
}

TEST(ParameterConversion, ClampGain_Below) {
    EXPECT_FLOAT_EQ(clampGain(-100.0f), -60.0f);
}

TEST(ParameterConversion, ClampGain_Above) {
    EXPECT_FLOAT_EQ(clampGain(20.0f), 6.0f);
}

//------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
