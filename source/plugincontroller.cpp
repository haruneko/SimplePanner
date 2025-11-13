#include "plugincontroller.h"
#include "plugids.h"
#include "parameter_utils.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

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

    // Register parameters
    // Left Pan: -100 to +100
    parameters.addParameter(STR16("Left Pan"),
                           STR16(""),
                           0,  // stepCount (0 = continuous)
                           panToNormalized(ParamDefault::kLeftPan),
                           Vst::ParameterInfo::kCanAutomate,
                           kParamLeftPan);

    // Left Gain: -60dB to +6dB
    parameters.addParameter(STR16("Left Gain"),
                           STR16("dB"),
                           0,
                           dbToNormalized(ParamDefault::kLeftGain),
                           Vst::ParameterInfo::kCanAutomate,
                           kParamLeftGain);

    // Left Delay: 0ms to 100ms
    parameters.addParameter(STR16("Left Delay"),
                           STR16("ms"),
                           0,
                           delayMsToNormalized(ParamDefault::kLeftDelay),
                           Vst::ParameterInfo::kCanAutomate,
                           kParamLeftDelay);

    // Right Pan: -100 to +100
    parameters.addParameter(STR16("Right Pan"),
                           STR16(""),
                           0,
                           panToNormalized(ParamDefault::kRightPan),
                           Vst::ParameterInfo::kCanAutomate,
                           kParamRightPan);

    // Right Gain: -60dB to +6dB
    parameters.addParameter(STR16("Right Gain"),
                           STR16("dB"),
                           0,
                           dbToNormalized(ParamDefault::kRightGain),
                           Vst::ParameterInfo::kCanAutomate,
                           kParamRightGain);

    // Right Delay: 0ms to 100ms
    parameters.addParameter(STR16("Right Delay"),
                           STR16("ms"),
                           0,
                           delayMsToNormalized(ParamDefault::kRightDelay),
                           Vst::ParameterInfo::kCanAutomate,
                           kParamRightDelay);

    // Master Gain: -60dB to +6dB
    parameters.addParameter(STR16("Master Gain"),
                           STR16("dB"),
                           0,
                           dbToNormalized(ParamDefault::kMasterGain),
                           Vst::ParameterInfo::kCanAutomate,
                           kParamMasterGain);

    // Link L/R Gain: On/Off
    parameters.addParameter(STR16("Link L/R Gain"),
                           STR16(""),
                           1,  // stepCount (1 = on/off toggle)
                           ParamDefault::kLinkGain,
                           Vst::ParameterInfo::kCanAutomate,
                           kParamLinkGain);

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

    // Read version
    int32 version = 0;
    if (!streamer.readInt32(version))
        return kResultFalse;

    if (version != 1)
        return kResultFalse;

    // Read 8 parameter values (normalized)
    double leftPan, leftGain, leftDelay;
    double rightPan, rightGain, rightDelay;
    double masterGain, linkGain;

    if (!streamer.readDouble(leftPan)) return kResultFalse;
    if (!streamer.readDouble(leftGain)) return kResultFalse;
    if (!streamer.readDouble(leftDelay)) return kResultFalse;
    if (!streamer.readDouble(rightPan)) return kResultFalse;
    if (!streamer.readDouble(rightGain)) return kResultFalse;
    if (!streamer.readDouble(rightDelay)) return kResultFalse;
    if (!streamer.readDouble(masterGain)) return kResultFalse;
    if (!streamer.readDouble(linkGain)) return kResultFalse;

    // Set parameter values
    setParamNormalized(kParamLeftPan, leftPan);
    setParamNormalized(kParamLeftGain, leftGain);
    setParamNormalized(kParamLeftDelay, leftDelay);
    setParamNormalized(kParamRightPan, rightPan);
    setParamNormalized(kParamRightGain, rightGain);
    setParamNormalized(kParamRightDelay, rightDelay);
    setParamNormalized(kParamMasterGain, masterGain);
    setParamNormalized(kParamLinkGain, linkGain);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerController::setParamNormalized(Vst::ParamID tag, Vst::ParamValue value)
{
    // Handle Link Gain feature
    Vst::ParamValue linkGain = getParamNormalized(kParamLinkGain);
    bool isLinkEnabled = (linkGain >= 0.5);

    if (isLinkEnabled)
    {
        // If Link is enabled and Left Gain changes, also change Right Gain
        if (tag == kParamLeftGain)
        {
            EditController::setParamNormalized(kParamRightGain, value);
        }
        // If Link is enabled and Right Gain changes, also change Left Gain
        else if (tag == kParamRightGain)
        {
            EditController::setParamNormalized(kParamLeftGain, value);
        }
    }

    // Set the actual parameter value
    return EditController::setParamNormalized(tag, value);
}

} // namespace SimplePanner
} // namespace Steinberg
