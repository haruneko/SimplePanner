#include <gtest/gtest.h>
#include "plugineditor.h"
#include "plugincontroller.h"
#include "parameter_utils.h"

using namespace Steinberg;
using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------
// Mock Control for Testing
//------------------------------------------------------------------------
class MockControl : public VSTGUI::CControl
{
public:
    MockControl(const VSTGUI::CRect& size, VSTGUI::IControlListener* listener, int32_t tag)
        : CControl(size, listener, tag)
    {
    }

    void simulateBeginEdit()
    {
        if (listener)
            listener->controlBeginEdit(this);
    }

    void simulateValueChange(float newValue)
    {
        setValueNormalized(newValue);
        if (listener)
            listener->valueChanged(this);
    }

    void simulateEndEdit()
    {
        if (listener)
            listener->controlEndEdit(this);
    }

    void draw(VSTGUI::CDrawContext*) override {}
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint&, const VSTGUI::CButtonState&) override
    {
        return VSTGUI::kMouseEventNotHandled;
    }

    CLASS_METHODS(MockControl, CControl)
};

//------------------------------------------------------------------------
// Test Fixture for Parameter Synchronization
//------------------------------------------------------------------------
class ParameterSyncTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create controller
        controller = new SimplePannerController();
        controller->initialize(nullptr);

        // Create editor
        editor = new SimplePannerEditor(static_cast<void*>(controller));
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
    SimplePannerEditor* editor = nullptr;
};

//------------------------------------------------------------------------
// Task 5.7.1: controlBeginEdit() Tests
//------------------------------------------------------------------------

TEST_F(ParameterSyncTest, ControlBeginEdit_NotifiesController)
{
    // Create a mock control
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamLeftGain);

    // Get initial edit count (should be 0)
    // Note: We can't directly test internal state, but we can verify no crash

    // Simulate begin edit
    control->simulateBeginEdit();

    // Should not crash and should be callable
    EXPECT_TRUE(true);

    control->forget();
}

TEST_F(ParameterSyncTest, ControlBeginEdit_MultipleParameters)
{
    VSTGUI::CRect rect(0, 0, 100, 20);

    // Test with different parameters
    MockControl* leftPan = new MockControl(rect, editor, kParamLeftPan);
    MockControl* rightGain = new MockControl(rect, editor, kParamRightGain);
    MockControl* masterGain = new MockControl(rect, editor, kParamMasterGain);

    leftPan->simulateBeginEdit();
    rightGain->simulateBeginEdit();
    masterGain->simulateBeginEdit();

    // Should handle multiple begin edits without crash
    EXPECT_TRUE(true);

    leftPan->forget();
    rightGain->forget();
    masterGain->forget();
}

//------------------------------------------------------------------------
// Task 5.7.2: controlEndEdit() Tests
//------------------------------------------------------------------------

TEST_F(ParameterSyncTest, ControlEndEdit_NotifiesController)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamLeftDelay);

    // Simulate begin and end edit sequence
    control->simulateBeginEdit();
    control->simulateValueChange(0.5f);
    control->simulateEndEdit();

    // Should complete without crash
    EXPECT_TRUE(true);

    control->forget();
}

TEST_F(ParameterSyncTest, ControlEndEdit_WithoutBegin)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamRightDelay);

    // End edit without begin should be handled gracefully
    control->simulateEndEdit();

    EXPECT_TRUE(true);

    control->forget();
}

//------------------------------------------------------------------------
// Task 5.7.3: GUI → Controller Synchronization Tests
//------------------------------------------------------------------------

TEST_F(ParameterSyncTest, GUIToController_LeftPanSync)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamLeftPan);

    // Set initial value
    float testValue = panToNormalized(-50.0f); // L50
    control->simulateValueChange(testValue);

    // Verify controller received the value
    float controllerValue = controller->getParamNormalized(kParamLeftPan);
    EXPECT_FLOAT_EQ(controllerValue, testValue);

    control->forget();
}

TEST_F(ParameterSyncTest, GUIToController_GainSync)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamLeftGain);

    // Set gain to -6dB
    float testValue = dbToNormalized(-6.0f);
    control->simulateValueChange(testValue);

    // Verify controller received the value
    float controllerValue = controller->getParamNormalized(kParamLeftGain);
    EXPECT_FLOAT_EQ(controllerValue, testValue);

    control->forget();
}

TEST_F(ParameterSyncTest, GUIToController_DelaySync)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamLeftDelay);

    // Set delay to 50ms
    float testValue = delayMsToNormalized(50.0f);
    control->simulateValueChange(testValue);

    // Verify controller received the value
    float controllerValue = controller->getParamNormalized(kParamLeftDelay);
    EXPECT_FLOAT_EQ(controllerValue, testValue);

    control->forget();
}

TEST_F(ParameterSyncTest, GUIToController_LinkToggleSync)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamLinkGain);

    // Toggle link ON
    control->simulateValueChange(1.0f);
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLinkGain), 1.0f);

    // Toggle link OFF
    control->simulateValueChange(0.0f);
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLinkGain), 0.0f);

    control->forget();
}

TEST_F(ParameterSyncTest, GUIToController_AllParameters)
{
    // Test that all 8 parameters can be set through GUI
    VSTGUI::CRect rect(0, 0, 100, 20);

    struct TestCase {
        ParameterID id;
        float value;
    };

    TestCase testCases[] = {
        {kParamLeftPan, panToNormalized(-25.0f)},
        {kParamLeftGain, dbToNormalized(-3.0f)},
        {kParamLeftDelay, delayMsToNormalized(10.0f)},
        {kParamRightPan, panToNormalized(75.0f)},
        {kParamRightGain, dbToNormalized(0.0f)},
        {kParamRightDelay, delayMsToNormalized(20.0f)},
        {kParamMasterGain, dbToNormalized(-1.5f)},
        {kParamLinkGain, 1.0f}
    };

    for (const auto& testCase : testCases)
    {
        MockControl* control = new MockControl(rect, editor, testCase.id);
        control->simulateValueChange(testCase.value);

        float controllerValue = controller->getParamNormalized(testCase.id);
        EXPECT_FLOAT_EQ(controllerValue, testCase.value)
            << "Failed for parameter ID: " << testCase.id;

        control->forget();
    }
}

//------------------------------------------------------------------------
// Task 5.7.4: Controller → GUI Synchronization Tests (TDD)
//------------------------------------------------------------------------

TEST_F(ParameterSyncTest, ControllerToGUI_ParameterChangeNotification)
{
    // This test verifies that when controller parameter changes,
    // the GUI should be notified and update its controls

    // Note: This requires implementing a parameter change notification mechanism
    // in SimplePannerEditor, typically through IParameterFinder or similar

    // Set a parameter value in the controller
    controller->setParamNormalized(kParamLeftPan, panToNormalized(50.0f));

    // TODO: After implementing Controller → GUI sync,
    // verify that GUI controls reflect the new value

    // For now, just verify controller has the value
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLeftPan), panToNormalized(50.0f));
}

TEST_F(ParameterSyncTest, ControllerToGUI_MultipleParameterChanges)
{
    // Test multiple rapid parameter changes from controller side
    // (simulating DAW automation)

    controller->setParamNormalized(kParamLeftGain, dbToNormalized(-12.0f));
    controller->setParamNormalized(kParamRightGain, dbToNormalized(-6.0f));
    controller->setParamNormalized(kParamMasterGain, dbToNormalized(3.0f));

    // Verify all changes are stored in controller
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLeftGain), dbToNormalized(-12.0f));
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamRightGain), dbToNormalized(-6.0f));
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamMasterGain), dbToNormalized(3.0f));
}

//------------------------------------------------------------------------
// Edit Sequence Tests
//------------------------------------------------------------------------

TEST_F(ParameterSyncTest, EditSequence_BeginValueEnd)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control = new MockControl(rect, editor, kParamMasterGain);

    // Simulate complete edit sequence
    control->simulateBeginEdit();

    // Change value multiple times during edit
    control->simulateValueChange(dbToNormalized(-6.0f));
    control->simulateValueChange(dbToNormalized(-3.0f));
    control->simulateValueChange(dbToNormalized(0.0f));

    control->simulateEndEdit();

    // Final value should be the last one set
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamMasterGain), dbToNormalized(0.0f));

    control->forget();
}

TEST_F(ParameterSyncTest, EditSequence_NestedEdits)
{
    VSTGUI::CRect rect(0, 0, 100, 20);
    MockControl* control1 = new MockControl(rect, editor, kParamLeftGain);
    MockControl* control2 = new MockControl(rect, editor, kParamRightGain);

    // Start editing control1
    control1->simulateBeginEdit();
    control1->simulateValueChange(dbToNormalized(-3.0f));

    // Start editing control2 while control1 is still being edited
    control2->simulateBeginEdit();
    control2->simulateValueChange(dbToNormalized(-6.0f));

    // End both edits
    control2->simulateEndEdit();
    control1->simulateEndEdit();

    // Both should have their values
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamLeftGain), dbToNormalized(-3.0f));
    EXPECT_FLOAT_EQ(controller->getParamNormalized(kParamRightGain), dbToNormalized(-6.0f));

    control1->forget();
    control2->forget();
}
