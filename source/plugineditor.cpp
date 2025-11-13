#include "plugineditor.h"
#include "parameter_utils.h"
#include <sstream>
#include <iomanip>

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// GUI Dimensions
//------------------------------------------------------------------------
constexpr int32 kEditorWidth = 600;
constexpr int32 kEditorHeight = 400;

//------------------------------------------------------------------------
// SimplePannerEditor - Constructor
//------------------------------------------------------------------------
SimplePannerEditor::SimplePannerEditor(void* controller)
: Vst::VSTGUIEditor(controller)
, mLeftPanSlider(nullptr)
, mRightPanSlider(nullptr)
, mLeftGainKnob(nullptr)
, mRightGainKnob(nullptr)
, mLeftDelayKnob(nullptr)
, mRightDelayKnob(nullptr)
, mMasterGainKnob(nullptr)
, mLinkToggle(nullptr)
, mLeftPanLabel(nullptr)
, mRightPanLabel(nullptr)
, mLeftGainLabel(nullptr)
, mRightGainLabel(nullptr)
, mLeftDelayLabel(nullptr)
, mRightDelayLabel(nullptr)
, mMasterGainLabel(nullptr)
{
    // Set editor size
    ViewRect viewRect(0, 0, kEditorWidth, kEditorHeight);
    setRect(viewRect);
}

//------------------------------------------------------------------------
// SimplePannerEditor - Destructor
//------------------------------------------------------------------------
SimplePannerEditor::~SimplePannerEditor()
{
    // Controls will be released by CFrame
}

//------------------------------------------------------------------------
// open
//------------------------------------------------------------------------
bool PLUGIN_API SimplePannerEditor::open(void* parent, const PlatformType& platformType)
{
    // Create CFrame (main window)
    CRect frameSize(0, 0, kEditorWidth, kEditorHeight);
    CFrame* newFrame = new CFrame(frameSize, this);

    if (!newFrame)
        return false;

    // Open the frame
    if (!newFrame->open(parent, platformType))
    {
        newFrame->forget();
        return false;
    }

    // Set the frame
    frame = newFrame;

    // Create the UI
    if (!createUI())
    {
        close();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------
// close
//------------------------------------------------------------------------
void PLUGIN_API SimplePannerEditor::close()
{
    // Clear control pointers (CFrame will handle deletion)
    mLeftPanSlider = nullptr;
    mRightPanSlider = nullptr;
    mLeftGainKnob = nullptr;
    mRightGainKnob = nullptr;
    mLeftDelayKnob = nullptr;
    mRightDelayKnob = nullptr;
    mMasterGainKnob = nullptr;
    mLinkToggle = nullptr;
    mLeftPanLabel = nullptr;
    mRightPanLabel = nullptr;
    mLeftGainLabel = nullptr;
    mRightGainLabel = nullptr;
    mLeftDelayLabel = nullptr;
    mRightDelayLabel = nullptr;
    mMasterGainLabel = nullptr;

    // Close and release the frame
    if (frame)
    {
        frame->close();
        frame->forget();
        frame = nullptr;
    }
}

//------------------------------------------------------------------------
// createUI
//------------------------------------------------------------------------
bool SimplePannerEditor::createUI()
{
    CFrame* frame = getFrame();
    if (!frame)
        return false;

    // Set background color
    frame->setBackgroundColor(CColor(44, 44, 44, 255)); // #2C2C2C

    // TODO: Add GUI controls in subsequent tasks

    return true;
}

//------------------------------------------------------------------------
// valueChanged - from IControlListener
//------------------------------------------------------------------------
void SimplePannerEditor::valueChanged(CControl* control)
{
    if (!control)
        return;

    // Get the parameter tag
    int32 tag = control->getTag();

    // Get the normalized value
    float value = control->getValueNormalized();

    // Update the controller through VSTGUIEditor's performEdit
    // This automatically handles beginEdit/performEdit/endEdit
    if (getController())
    {
        // VSTGUIEditor provides performEdit which handles the full edit cycle
        getController()->setParamNormalized(tag, value);
        getController()->performEdit(tag, value);
    }

    // Update value display
    updateValueDisplay(tag);
}

//------------------------------------------------------------------------
// updateValueDisplay
//------------------------------------------------------------------------
void SimplePannerEditor::updateValueDisplay(Vst::ParamID tag)
{
    if (!getController())
        return;

    float normalized = getController()->getParamNormalized(tag);
    std::string text;

    // TODO: Update specific labels based on tag

    switch (tag)
    {
        case kParamLeftPan:
            text = formatPanValue(normalized);
            if (mLeftPanLabel)
                mLeftPanLabel->setText(text.c_str());
            break;

        case kParamRightPan:
            text = formatPanValue(normalized);
            if (mRightPanLabel)
                mRightPanLabel->setText(text.c_str());
            break;

        case kParamLeftGain:
            text = formatGainValue(normalized);
            if (mLeftGainLabel)
                mLeftGainLabel->setText(text.c_str());
            break;

        case kParamRightGain:
            text = formatGainValue(normalized);
            if (mRightGainLabel)
                mRightGainLabel->setText(text.c_str());
            break;

        case kParamLeftDelay:
            text = formatDelayValue(normalized);
            if (mLeftDelayLabel)
                mLeftDelayLabel->setText(text.c_str());
            break;

        case kParamRightDelay:
            text = formatDelayValue(normalized);
            if (mRightDelayLabel)
                mRightDelayLabel->setText(text.c_str());
            break;

        case kParamMasterGain:
            text = formatGainValue(normalized);
            if (mMasterGainLabel)
                mMasterGainLabel->setText(text.c_str());
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------
// formatPanValue
//------------------------------------------------------------------------
std::string SimplePannerEditor::formatPanValue(float normalized)
{
    float pan = normalizedToPan(normalized);

    if (std::abs(pan) < 0.5f)
    {
        return "C";  // Center
    }
    else if (pan < 0)
    {
        int value = static_cast<int>(std::abs(pan) + 0.5f);
        return "L" + std::to_string(value);
    }
    else
    {
        int value = static_cast<int>(pan + 0.5f);
        return "R" + std::to_string(value);
    }
}

//------------------------------------------------------------------------
// formatGainValue
//------------------------------------------------------------------------
std::string SimplePannerEditor::formatGainValue(float normalized)
{
    float db = normalizedToDb(normalized);

    if (db <= -60.0f)
    {
        return "-∞ dB";
    }
    else
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1);
        if (db >= 0)
            oss << "+";
        oss << db << " dB";
        return oss.str();
    }
}

//------------------------------------------------------------------------
// formatDelayValue
//------------------------------------------------------------------------
std::string SimplePannerEditor::formatDelayValue(float normalized)
{
    float ms = normalizedToDelayMs(normalized);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << ms << " ms";
    return oss.str();
}

} // namespace SimplePanner
} // namespace Steinberg
