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
    CFrame* frm = getFrame();
    if (!frm)
        return false;

    // Set background color
    frm->setBackgroundColor(CColor(44, 44, 44, 255)); // #2C2C2C

    // === Left Channel Group ===
    CRect leftGroupRect(20, 40, 300, 220);
    CViewContainer* leftGroup = new CViewContainer(leftGroupRect);
    leftGroup->setBackgroundColor(CColor(60, 60, 60, 255)); // #3C3C3C
    frm->addView(leftGroup);

    // Left Channel Title
    CRect leftTitleRect(0, 5, 280, 25);
    CTextLabel* leftTitle = new CTextLabel(leftTitleRect, "LEFT CHANNEL");
    leftTitle->setFont(kNormalFontBig);
    leftTitle->setFontColor(CColor(204, 204, 204, 255)); // #CCCCCC
    leftTitle->setBackColor(CColor(60, 60, 60, 0)); // Transparent
    leftTitle->setFrameColor(CColor(0, 0, 0, 0)); // No frame
    leftTitle->setStyle(CTextLabel::kNoDrawStyle);
    leftTitle->setTextTruncateMode(CTextLabel::kTruncateNone);
    leftTitle->sizeToFit();
    leftTitle->setHoriAlign(CHoriTxtAlign::kCenterText);
    leftGroup->addView(leftTitle);

    // Left Pan Label
    CRect leftPanLabelRect(40, 40, 240, 55);
    mLeftPanLabel = new CTextLabel(leftPanLabelRect, "C");
    mLeftPanLabel->setFont(kNormalFont);
    mLeftPanLabel->setFontColor(CColor(255, 255, 255, 255));
    mLeftPanLabel->setBackColor(CColor(60, 60, 60, 0));
    mLeftPanLabel->setFrameColor(CColor(0, 0, 0, 0));
    mLeftPanLabel->setStyle(CTextLabel::kNoDrawStyle);
    mLeftPanLabel->setHoriAlign(CHoriTxtAlign::kCenterText);
    leftGroup->addView(mLeftPanLabel);

    // Left Pan Slider
    CRect leftPanSliderRect(40, 60, 240, 75);
    mLeftPanSlider = new CSlider(leftPanSliderRect, this, kParamLeftPan, 40, 240, nullptr, nullptr);
    mLeftPanSlider->setMin(0.0f);
    mLeftPanSlider->setMax(1.0f);
    mLeftPanSlider->setDefaultValue(0.0f); // Full Left = -100
    leftGroup->addView(mLeftPanSlider);

    // Initialize Left Pan Label
    if (getController())
    {
        float panValue = getController()->getParamNormalized(kParamLeftPan);
        mLeftPanSlider->setValue(panValue);
        mLeftPanLabel->setText(formatPanValue(panValue).c_str());
    }

    // Left Gain Label
    CRect leftGainLabelRect(110, 85, 170, 100);
    mLeftGainLabel = new CTextLabel(leftGainLabelRect, "0.0 dB");
    mLeftGainLabel->setFont(kNormalFont);
    mLeftGainLabel->setFontColor(CColor(255, 255, 255, 255));
    mLeftGainLabel->setBackColor(CColor(60, 60, 60, 0));
    mLeftGainLabel->setFrameColor(CColor(0, 0, 0, 0));
    mLeftGainLabel->setStyle(CTextLabel::kNoDrawStyle);
    mLeftGainLabel->setHoriAlign(CHoriTxtAlign::kCenterText);
    leftGroup->addView(mLeftGainLabel);

    // Left Gain Knob (60x60px)
    CRect leftGainKnobRect(110, 105, 170, 165);
    mLeftGainKnob = new CKnob(leftGainKnobRect, this, kParamLeftGain, nullptr, nullptr);
    mLeftGainKnob->setMin(0.0f);
    mLeftGainKnob->setMax(1.0f);
    mLeftGainKnob->setDefaultValue(dbToNormalized(0.0f)); // 0dB default
    mLeftGainKnob->setZoomFactor(10.0f); // Shift+drag for fine adjustment
    leftGroup->addView(mLeftGainKnob);

    // Initialize Left Gain Label
    if (getController())
    {
        float gainValue = getController()->getParamNormalized(kParamLeftGain);
        mLeftGainKnob->setValue(gainValue);
        mLeftGainLabel->setText(formatGainValue(gainValue).c_str());
    }

    // === Right Channel Group ===
    CRect rightGroupRect(320, 40, 580, 220);
    CViewContainer* rightGroup = new CViewContainer(rightGroupRect);
    rightGroup->setBackgroundColor(CColor(60, 60, 60, 255)); // #3C3C3C
    frm->addView(rightGroup);

    // Right Channel Title
    CRect rightTitleRect(0, 5, 260, 25);
    CTextLabel* rightTitle = new CTextLabel(rightTitleRect, "RIGHT CHANNEL");
    rightTitle->setFont(kNormalFontBig);
    rightTitle->setFontColor(CColor(204, 204, 204, 255)); // #CCCCCC
    rightTitle->setBackColor(CColor(60, 60, 60, 0)); // Transparent
    rightTitle->setFrameColor(CColor(0, 0, 0, 0)); // No frame
    rightTitle->setStyle(CTextLabel::kNoDrawStyle);
    rightTitle->setTextTruncateMode(CTextLabel::kTruncateNone);
    rightTitle->sizeToFit();
    rightTitle->setHoriAlign(CHoriTxtAlign::kCenterText);
    rightGroup->addView(rightTitle);

    // Right Pan Label
    CRect rightPanLabelRect(30, 40, 230, 55);
    mRightPanLabel = new CTextLabel(rightPanLabelRect, "C");
    mRightPanLabel->setFont(kNormalFont);
    mRightPanLabel->setFontColor(CColor(255, 255, 255, 255));
    mRightPanLabel->setBackColor(CColor(60, 60, 60, 0));
    mRightPanLabel->setFrameColor(CColor(0, 0, 0, 0));
    mRightPanLabel->setStyle(CTextLabel::kNoDrawStyle);
    mRightPanLabel->setHoriAlign(CHoriTxtAlign::kCenterText);
    rightGroup->addView(mRightPanLabel);

    // Right Pan Slider
    CRect rightPanSliderRect(30, 60, 230, 75);
    mRightPanSlider = new CSlider(rightPanSliderRect, this, kParamRightPan, 30, 230, nullptr, nullptr);
    mRightPanSlider->setMin(0.0f);
    mRightPanSlider->setMax(1.0f);
    mRightPanSlider->setDefaultValue(1.0f); // Full Right = +100
    rightGroup->addView(mRightPanSlider);

    // Initialize Right Pan Label
    if (getController())
    {
        float panValue = getController()->getParamNormalized(kParamRightPan);
        mRightPanSlider->setValue(panValue);
        mRightPanLabel->setText(formatPanValue(panValue).c_str());
    }

    // Right Gain Label
    CRect rightGainLabelRect(100, 85, 160, 100);
    mRightGainLabel = new CTextLabel(rightGainLabelRect, "0.0 dB");
    mRightGainLabel->setFont(kNormalFont);
    mRightGainLabel->setFontColor(CColor(255, 255, 255, 255));
    mRightGainLabel->setBackColor(CColor(60, 60, 60, 0));
    mRightGainLabel->setFrameColor(CColor(0, 0, 0, 0));
    mRightGainLabel->setStyle(CTextLabel::kNoDrawStyle);
    mRightGainLabel->setHoriAlign(CHoriTxtAlign::kCenterText);
    rightGroup->addView(mRightGainLabel);

    // Right Gain Knob (60x60px)
    CRect rightGainKnobRect(100, 105, 160, 165);
    mRightGainKnob = new CKnob(rightGainKnobRect, this, kParamRightGain, nullptr, nullptr);
    mRightGainKnob->setMin(0.0f);
    mRightGainKnob->setMax(1.0f);
    mRightGainKnob->setDefaultValue(dbToNormalized(0.0f)); // 0dB default
    mRightGainKnob->setZoomFactor(10.0f); // Shift+drag for fine adjustment
    rightGroup->addView(mRightGainKnob);

    // Initialize Right Gain Label
    if (getController())
    {
        float gainValue = getController()->getParamNormalized(kParamRightGain);
        mRightGainKnob->setValue(gainValue);
        mRightGainLabel->setText(formatGainValue(gainValue).c_str());
    }

    // === Master Section ===
    CRect masterRect(20, 240, 580, 360);
    CViewContainer* masterGroup = new CViewContainer(masterRect);
    masterGroup->setBackgroundColor(CColor(60, 60, 60, 255)); // #3C3C3C
    frm->addView(masterGroup);

    // Master Title
    CRect masterTitleRect(0, 5, 560, 25);
    CTextLabel* masterTitle = new CTextLabel(masterTitleRect, "MASTER");
    masterTitle->setFont(kNormalFontBig);
    masterTitle->setFontColor(CColor(204, 204, 204, 255)); // #CCCCCC
    masterTitle->setBackColor(CColor(60, 60, 60, 0)); // Transparent
    masterTitle->setFrameColor(CColor(0, 0, 0, 0)); // No frame
    masterTitle->setStyle(CTextLabel::kNoDrawStyle);
    masterTitle->setTextTruncateMode(CTextLabel::kTruncateNone);
    masterTitle->sizeToFit();
    masterTitle->setHoriAlign(CHoriTxtAlign::kCenterText);
    masterGroup->addView(masterTitle);

    // Master Gain Label
    CRect masterGainLabelRect(250, 35, 310, 50);
    mMasterGainLabel = new CTextLabel(masterGainLabelRect, "0.0 dB");
    mMasterGainLabel->setFont(kNormalFont);
    mMasterGainLabel->setFontColor(CColor(255, 255, 255, 255));
    mMasterGainLabel->setBackColor(CColor(60, 60, 60, 0));
    mMasterGainLabel->setFrameColor(CColor(0, 0, 0, 0));
    mMasterGainLabel->setStyle(CTextLabel::kNoDrawStyle);
    mMasterGainLabel->setHoriAlign(CHoriTxtAlign::kCenterText);
    masterGroup->addView(mMasterGainLabel);

    // Master Gain Knob (60x60px)
    CRect masterGainKnobRect(250, 55, 310, 115);
    mMasterGainKnob = new CKnob(masterGainKnobRect, this, kParamMasterGain, nullptr, nullptr);
    mMasterGainKnob->setMin(0.0f);
    mMasterGainKnob->setMax(1.0f);
    mMasterGainKnob->setDefaultValue(dbToNormalized(0.0f)); // 0dB default
    mMasterGainKnob->setZoomFactor(10.0f); // Shift+drag for fine adjustment
    masterGroup->addView(mMasterGainKnob);

    // Initialize Master Gain Label
    if (getController())
    {
        float gainValue = getController()->getParamNormalized(kParamMasterGain);
        mMasterGainKnob->setValue(gainValue);
        mMasterGainLabel->setText(formatGainValue(gainValue).c_str());
    }

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
