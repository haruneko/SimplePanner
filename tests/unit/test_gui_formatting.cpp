#include <gtest/gtest.h>
#include "plugineditor.h"
#include "plugincontroller.h"
#include "parameter_utils.h"

using namespace Steinberg;
using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------
// Test Helper Class - Expose protected methods for testing
// NOTE: We don't actually open the GUI window in tests, just test the format methods
//------------------------------------------------------------------------
class TestableEditor : public SimplePannerEditor
{
public:
    TestableEditor(void* controller) : SimplePannerEditor(controller) {}

    // Expose protected format methods for testing
    std::string testFormatPanValue(float normalized) { return formatPanValue(normalized); }
    std::string testFormatGainValue(float normalized) { return formatGainValue(normalized); }
    std::string testFormatDelayValue(float normalized) { return formatDelayValue(normalized); }

    // Override to prevent actual GUI initialization
    bool PLUGIN_API open(void* parent, const PlatformType& platformType) override
    {
        // For testing, we don't actually open the GUI
        return true;
    }

    void PLUGIN_API close() override
    {
        // For testing, we don't actually close the GUI
    }
};

//------------------------------------------------------------------------
// Test Fixture for GUI Formatting Functions
//------------------------------------------------------------------------
class GUIFormattingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a real controller for testing
        controller = new SimplePannerController();
        controller->initialize(nullptr);

        // Create editor with the controller
        editor = new TestableEditor(static_cast<void*>(controller));
    }

    void TearDown() override
    {
        if (editor)
        {
            delete editor;
            editor = nullptr;
        }

        if (controller)
        {
            controller->terminate();
            controller->release();
            controller = nullptr;
        }
    }

    SimplePannerController* controller = nullptr;
    TestableEditor* editor = nullptr;
};

//------------------------------------------------------------------------
// Task 5.3: Pan Slider Formatting Tests
//------------------------------------------------------------------------

TEST_F(GUIFormattingTest, FormatPanValue_Center)
{
    // Center position (normalized 0.5 for left pan, which maps to 0.0 in plain value)
    float normalized = panToNormalized(0.0f);
    std::string result = editor->testFormatPanValue(normalized);
    EXPECT_EQ(result, "C");
}

TEST_F(GUIFormattingTest, FormatPanValue_FullLeft)
{
    // Full left (-100)
    float normalized = panToNormalized(-100.0f);
    std::string result = editor->testFormatPanValue(normalized);
    EXPECT_EQ(result, "L100");
}

TEST_F(GUIFormattingTest, FormatPanValue_FullRight)
{
    // Full right (+100)
    float normalized = panToNormalized(100.0f);
    std::string result = editor->testFormatPanValue(normalized);
    EXPECT_EQ(result, "R100");
}

TEST_F(GUIFormattingTest, FormatPanValue_HalfLeft)
{
    // Half left (-50)
    float normalized = panToNormalized(-50.0f);
    std::string result = editor->testFormatPanValue(normalized);
    EXPECT_EQ(result, "L50");
}

TEST_F(GUIFormattingTest, FormatPanValue_HalfRight)
{
    // Half right (+50)
    float normalized = panToNormalized(50.0f);
    std::string result = editor->testFormatPanValue(normalized);
    EXPECT_EQ(result, "R50");
}

TEST_F(GUIFormattingTest, FormatPanValue_NearCenter)
{
    // Very close to center should still show "C"
    float normalized = panToNormalized(0.4f);
    std::string result = editor->testFormatPanValue(normalized);
    EXPECT_EQ(result, "C");

    normalized = panToNormalized(-0.4f);
    result = editor->testFormatPanValue(normalized);
    EXPECT_EQ(result, "C");
}

//------------------------------------------------------------------------
// Task 5.4: Gain Knob Formatting Tests
//------------------------------------------------------------------------

TEST_F(GUIFormattingTest, FormatGainValue_Unity)
{
    // 0dB (unity gain)
    float normalized = dbToNormalized(0.0f);
    std::string result = editor->testFormatGainValue(normalized);
    EXPECT_EQ(result, "+0.0 dB");
}

TEST_F(GUIFormattingTest, FormatGainValue_Mute)
{
    // -60dB (mute, displayed as -∞)
    float normalized = dbToNormalized(-60.0f);
    std::string result = editor->testFormatGainValue(normalized);
    EXPECT_EQ(result, "-∞ dB");
}

TEST_F(GUIFormattingTest, FormatGainValue_Positive)
{
    // +6dB (max)
    float normalized = dbToNormalized(6.0f);
    std::string result = editor->testFormatGainValue(normalized);
    EXPECT_EQ(result, "+6.0 dB");
}

TEST_F(GUIFormattingTest, FormatGainValue_Negative)
{
    // -12dB
    float normalized = dbToNormalized(-12.0f);
    std::string result = editor->testFormatGainValue(normalized);
    EXPECT_EQ(result, "-12.0 dB");
}

TEST_F(GUIFormattingTest, FormatGainValue_SlightlyAboveMute)
{
    // -59dB should not be treated as -∞
    float normalized = dbToNormalized(-59.0f);
    std::string result = editor->testFormatGainValue(normalized);
    EXPECT_EQ(result, "-59.0 dB");
}

TEST_F(GUIFormattingTest, FormatGainValue_Precision)
{
    // Test decimal precision
    float normalized = dbToNormalized(-3.5f);
    std::string result = editor->testFormatGainValue(normalized);
    EXPECT_EQ(result, "-3.5 dB");
}

//------------------------------------------------------------------------
// Task 5.5: Delay Knob Formatting Tests (TDD - write tests first)
//------------------------------------------------------------------------

TEST_F(GUIFormattingTest, FormatDelayValue_Zero)
{
    // 0ms (no delay)
    float normalized = delayMsToNormalized(0.0f);
    std::string result = editor->testFormatDelayValue(normalized);
    EXPECT_EQ(result, "0.0 ms");
}

TEST_F(GUIFormattingTest, FormatDelayValue_Max)
{
    // 100ms (max delay)
    float normalized = delayMsToNormalized(100.0f);
    std::string result = editor->testFormatDelayValue(normalized);
    EXPECT_EQ(result, "100.0 ms");
}

TEST_F(GUIFormattingTest, FormatDelayValue_Mid)
{
    // 50ms
    float normalized = delayMsToNormalized(50.0f);
    std::string result = editor->testFormatDelayValue(normalized);
    EXPECT_EQ(result, "50.0 ms");
}

TEST_F(GUIFormattingTest, FormatDelayValue_Precision)
{
    // 25.5ms - test decimal precision
    float normalized = delayMsToNormalized(25.5f);
    std::string result = editor->testFormatDelayValue(normalized);
    EXPECT_EQ(result, "25.5 ms");
}

TEST_F(GUIFormattingTest, FormatDelayValue_Small)
{
    // 0.5ms
    float normalized = delayMsToNormalized(0.5f);
    std::string result = editor->testFormatDelayValue(normalized);
    EXPECT_EQ(result, "0.5 ms");
}

//------------------------------------------------------------------------
// Boundary Tests
//------------------------------------------------------------------------

TEST_F(GUIFormattingTest, FormatValues_Boundaries)
{
    // Test normalized value boundaries (0.0 and 1.0)

    // Pan at 0.0 should be full left
    std::string pan = editor->testFormatPanValue(0.0f);
    EXPECT_EQ(pan, "L100");

    // Pan at 1.0 should be full right
    pan = editor->testFormatPanValue(1.0f);
    EXPECT_EQ(pan, "R100");

    // Gain at 0.0 should be -60dB (-∞)
    std::string gain = editor->testFormatGainValue(0.0f);
    EXPECT_EQ(gain, "-∞ dB");

    // Gain at 1.0 should be +6dB
    gain = editor->testFormatGainValue(1.0f);
    EXPECT_EQ(gain, "+6.0 dB");

    // Delay at 0.0 should be 0ms
    std::string delay = editor->testFormatDelayValue(0.0f);
    EXPECT_EQ(delay, "0.0 ms");

    // Delay at 1.0 should be 100ms
    delay = editor->testFormatDelayValue(1.0f);
    EXPECT_EQ(delay, "100.0 ms");
}
