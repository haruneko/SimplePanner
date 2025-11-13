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
    // TODO: メンバー変数を追加（Task 1.4で実装）
    // - DelayLine mDelayLeft, mDelayRight
    // - ParameterSmoother instances
    // - Parameter values
};

} // namespace SimplePanner
} // namespace Steinberg
