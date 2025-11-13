#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"

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
    Vst::ParamValue mPan;   // -1.0 (左) to 1.0 (右)
    Vst::ParamValue mGain;  // 0.0 to 1.0
};

} // namespace SimplePanner
} // namespace Steinberg
