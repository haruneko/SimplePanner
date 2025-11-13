#include "plugincontroller.h"
#include "plugids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// SimplePannerController
//------------------------------------------------------------------------
SimplePannerController::SimplePannerController()
{
}

//------------------------------------------------------------------------
SimplePannerController::~SimplePannerController()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerController::initialize(FUnknown* context)
{
    tresult result = EditController::initialize(context);
    if (result != kResultOk)
        return result;

    // パンニングパラメータを追加
    parameters.addParameter(STR16("Pan"),
                           STR16(""),
                           0,
                           0.5,  // デフォルト値 (中央)
                           Vst::ParameterInfo::kCanAutomate,
                           kParamPanId);

    // ゲインパラメータを追加
    parameters.addParameter(STR16("Gain"),
                           STR16("dB"),
                           0,
                           1.0,  // デフォルト値
                           Vst::ParameterInfo::kCanAutomate,
                           kParamGainId);

    return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerController::terminate()
{
    return EditController::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerController::setComponentState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    float savedPan = 0.0f;
    if (streamer.readFloat(savedPan))
    {
        setParamNormalized(kParamPanId, (savedPan + 1.0) / 2.0); // -1.0-1.0 を 0.0-1.0 に変換
    }

    float savedGain = 1.0f;
    if (streamer.readFloat(savedGain))
    {
        setParamNormalized(kParamGainId, savedGain);
    }

    return kResultOk;
}

} // namespace SimplePanner
} // namespace Steinberg
