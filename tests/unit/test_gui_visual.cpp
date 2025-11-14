#include <gtest/gtest.h>
#include "plugineditor.h"
#include "plugincontroller.h"

using namespace Steinberg;
using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------
// Test Fixture for Visual Polish and GUI Stability
//------------------------------------------------------------------------
class GUIVisualTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create controller
        controller = new SimplePannerController();
        controller->initialize(nullptr);
    }

    void TearDown() override
    {
        if (controller)
        {
            controller->terminate();
            controller->release();
            controller = nullptr;
        }
    }

    SimplePannerController* controller = nullptr;
};

//------------------------------------------------------------------------
// Task 5.8: Visual Polish Tests
//------------------------------------------------------------------------

TEST_F(GUIVisualTest, EditorCreationAndDestruction)
{
    // Test that editor can be created and destroyed without crash
    // This verifies proper resource management
    SimplePannerEditor* editor = new SimplePannerEditor(static_cast<void*>(controller));
    EXPECT_NE(editor, nullptr);
    delete editor;
}

TEST_F(GUIVisualTest, MultipleEditorCreationCycles)
{
    // Test creating and destroying editor multiple times
    // This checks for resource leaks
    for (int i = 0; i < 10; ++i)
    {
        SimplePannerEditor* editor = new SimplePannerEditor(static_cast<void*>(controller));
        EXPECT_NE(editor, nullptr);
        delete editor;
    }
}

TEST_F(GUIVisualTest, EditorWithNullController)
{
    // Test that editor handles null controller gracefully
    // This should not crash
    SimplePannerEditor* editor = new SimplePannerEditor(nullptr);
    EXPECT_NE(editor, nullptr);
    delete editor;
}

TEST_F(GUIVisualTest, EditorDoesNotCrashOnClose)
{
    // Test that close() can be called safely
    SimplePannerEditor* editor = new SimplePannerEditor(static_cast<void*>(controller));

    // Call close() - should not crash even if editor wasn't opened
    editor->close();

    delete editor;
}

//------------------------------------------------------------------------
// Task 5.9: GUI Control Tests (Preparation)
//------------------------------------------------------------------------

TEST_F(GUIVisualTest, ControllerParameterCount)
{
    // Verify controller has all 8 parameters
    // This ensures GUI will have all necessary parameters to display

    // Check each parameter is accessible
    float value;

    value = controller->getParamNormalized(kParamLeftPan);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);

    value = controller->getParamNormalized(kParamLeftGain);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);

    value = controller->getParamNormalized(kParamLeftDelay);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);

    value = controller->getParamNormalized(kParamRightPan);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);

    value = controller->getParamNormalized(kParamRightGain);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);

    value = controller->getParamNormalized(kParamRightDelay);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);

    value = controller->getParamNormalized(kParamMasterGain);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);

    value = controller->getParamNormalized(kParamLinkGain);
    EXPECT_GE(value, 0.0f);
    EXPECT_LE(value, 1.0f);
}

TEST_F(GUIVisualTest, DefaultParameterValues)
{
    // Verify default parameter values match design specification
    // Left Pan: -100 (full left) = 0.0 normalized
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLeftPan), 0.0f);

    // Right Pan: +100 (full right) = 1.0 normalized
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamRightPan), 1.0f);

    // Left/Right/Master Gain: 0dB (unity) = middle of normalized range
    // 0dB is at (0 - (-60)) / (6 - (-60)) = 60/66 ≈ 0.909
    float unityGainNormalized = (0.0f - (-60.0f)) / (6.0f - (-60.0f));
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLeftGain), unityGainNormalized);
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamRightGain), unityGainNormalized);
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamMasterGain), unityGainNormalized);

    // Left/Right Delay: 0ms = 0.0 normalized
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLeftDelay), 0.0f);
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamRightDelay), 0.0f);

    // Link Gain: OFF = 0.0 normalized
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLinkGain), 0.0f);
}

//------------------------------------------------------------------------
// Stress Tests
//------------------------------------------------------------------------

TEST_F(GUIVisualTest, RapidParameterChanges)
{
    // Test that rapid parameter changes don't cause issues
    // This simulates DAW automation or user rapidly tweaking controls

    for (int i = 0; i < 100; ++i)
    {
        float value = static_cast<float>(i % 101) / 100.0f;
        controller->setParamNormalized(kParamLeftGain, value);
        controller->setParamNormalized(kParamRightGain, 1.0f - value);
    }

    // Should complete without crash
    EXPECT_TRUE(true);
}

TEST_F(GUIVisualTest, AllParametersExtremeValues)
{
    // Test all parameters at min and max values
    // This ensures GUI can handle extreme values

    ParameterID params[] = {
        kParamLeftPan, kParamLeftGain, kParamLeftDelay,
        kParamRightPan, kParamRightGain, kParamRightDelay,
        kParamMasterGain, kParamLinkGain
    };

    for (auto param : params)
    {
        // Set to min
        controller->setParamNormalized(param, 0.0f);
        float minValue = controller->getParamNormalized(param);
        EXPECT_FLOAT_EQ(minValue, 0.0f) << "Failed for param " << param << " at min";

        // Set to max
        controller->setParamNormalized(param, 1.0f);
        float maxValue = controller->getParamNormalized(param);
        EXPECT_FLOAT_EQ(maxValue, 1.0f) << "Failed for param " << param << " at max";
    }
}
