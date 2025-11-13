#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "delay_line.h"
#include "parameter_smoother.h"

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// SimplePannerProcessor
//------------------------------------------------------------------------
class SimplePannerProcessor : public Vst::AudioEffect
{
public:
    SimplePannerProcessor();
    ~SimplePannerProcessor() SMTG_OVERRIDE;

    // AudioEffect のオーバーライド
    tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;
    tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE;
    tresult PLUGIN_API process(Vst::ProcessData& data) SMTG_OVERRIDE;
    tresult PLUGIN_API setupProcessing(Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
    tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) SMTG_OVERRIDE;
    tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;
    tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;

    // Create function
    static FUnknown* createInstance(void* /*context*/)
    {
        return (Vst::IAudioProcessor*)new SimplePannerProcessor;
    }

protected:
    // Delay lines
    DelayLine mDelayLeft;
    DelayLine mDelayRight;

    // Parameter smoothers
    ParameterSmoother mLeftPanSmoother;
    ParameterSmoother mLeftGainSmoother;
    ParameterSmoother mRightPanSmoother;
    ParameterSmoother mRightGainSmoother;
    ParameterSmoother mMasterGainSmoother;

    // Current parameter values (normalized 0.0 - 1.0)
    double mLeftPan;
    double mLeftGain;
    double mLeftDelay;
    double mRightPan;
    double mRightGain;
    double mRightDelay;
    double mMasterGain;
    double mLinkGain;

    // Processing state
    double mSampleRate;
    bool mIsActive;
};

} // namespace SimplePanner
} // namespace Steinberg
