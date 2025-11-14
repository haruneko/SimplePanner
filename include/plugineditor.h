#pragma once

#include "public.sdk/source/vst/vstguieditor.h"
#include "vstgui/vstgui.h"
#include "plugids.h"

namespace Steinberg {
namespace SimplePanner {

using namespace VSTGUI;

//------------------------------------------------------------------------
// SimplePannerEditor - Custom GUI Editor
//------------------------------------------------------------------------
class SimplePannerEditor : public Vst::VSTGUIEditor, public IControlListener
{
public:
    SimplePannerEditor(void* controller);
    virtual ~SimplePannerEditor();

    //--- from VSTGUIEditor ---------------
    bool PLUGIN_API open(void* parent, const PlatformType& platformType = PlatformType::kDefaultNative) SMTG_OVERRIDE;
    void PLUGIN_API close() SMTG_OVERRIDE;

    //--- IControlListener implementation -----------
    void valueChanged(CControl* control) SMTG_OVERRIDE;
    void controlBeginEdit(CControl* control) SMTG_OVERRIDE;
    void controlEndEdit(CControl* control) SMTG_OVERRIDE;

protected:
    //--- GUI Creation -------------------
    bool createUI();
    void updateValueDisplay(Vst::ParamID tag);

    //--- Value Formatting ---------------
    std::string formatPanValue(float normalized);
    std::string formatGainValue(float normalized);
    std::string formatDelayValue(float normalized);

private:
    //--- GUI Controls -------------------
    CSlider* mLeftPanSlider;
    CSlider* mRightPanSlider;
    CKnob* mLeftGainKnob;
    CKnob* mRightGainKnob;
    CKnob* mLeftDelayKnob;
    CKnob* mRightDelayKnob;
    CKnob* mMasterGainKnob;
    CTextButton* mLinkToggle;

    //--- Value Display Labels -----------
    CTextLabel* mLeftPanLabel;
    CTextLabel* mRightPanLabel;
    CTextLabel* mLeftGainLabel;
    CTextLabel* mRightGainLabel;
    CTextLabel* mLeftDelayLabel;
    CTextLabel* mRightDelayLabel;
    CTextLabel* mMasterGainLabel;
};

} // namespace SimplePanner
} // namespace Steinberg
