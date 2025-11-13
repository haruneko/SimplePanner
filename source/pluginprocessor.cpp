#include "pluginprocessor.h"
#include "plugids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// SimplePannerProcessor
//------------------------------------------------------------------------
SimplePannerProcessor::SimplePannerProcessor()
{
    setControllerClass(ControllerUID);
}

//------------------------------------------------------------------------
SimplePannerProcessor::~SimplePannerProcessor()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::initialize(FUnknown* context)
{
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk)
        return result;

    // ステレオ入出力を設定
    addAudioInput(STR16("Stereo In"), Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), Vst::SpeakerArr::kStereo);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::terminate()
{
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::setActive(TBool state)
{
    return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::process(Vst::ProcessData& data)
{
    // TODO: パラメータ処理とオーディオ処理を実装
    // 現在は単純なバイパス処理のみ

    if (data.numInputs == 0 || data.numOutputs == 0)
        return kResultOk;

    Vst::AudioBusBuffers& inputBus = data.inputs[0];
    Vst::AudioBusBuffers& outputBus = data.outputs[0];

    // ステレオ処理
    if (inputBus.numChannels >= 2 && outputBus.numChannels >= 2)
    {
        float* inL = inputBus.channelBuffers32[0];
        float* inR = inputBus.channelBuffers32[1];
        float* outL = outputBus.channelBuffers32[0];
        float* outR = outputBus.channelBuffers32[1];

        // シンプルなバイパス（入力をそのまま出力）
        for (int32 i = 0; i < data.numSamples; i++)
        {
            outL[i] = inL[i];
            outR[i] = inR[i];
        }
    }

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
{
    return AudioEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
    if (symbolicSampleSize == Vst::kSample32)
        return kResultTrue;

    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::setState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    // TODO: 状態の復元を実装

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::getState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    // TODO: 状態の保存を実装

    return kResultOk;
}

} // namespace SimplePanner
} // namespace Steinberg
