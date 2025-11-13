#include "pluginprocessor.h"
#include "plugids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <cmath>

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// SimplePannerProcessor
//------------------------------------------------------------------------
SimplePannerProcessor::SimplePannerProcessor()
    : mPan(0.0)
    , mGain(1.0)
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
    // パラメータ変更を処理
    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
        for (int32 i = 0; i < numParamsChanged; i++)
        {
            Vst::IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData(i);
            if (paramQueue)
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount();

                if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                {
                    switch (paramQueue->getParameterId())
                    {
                        case kParamPanId:
                            mPan = value * 2.0 - 1.0; // 0.0-1.0 を -1.0-1.0 に変換
                            break;
                        case kParamGainId:
                            mGain = value;
                            break;
                    }
                }
            }
        }
    }

    // オーディオ処理
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

        // パンニング係数を計算 (等パワーパンニング)
        float panRad = mPan * M_PI / 4.0f; // -45度から+45度
        float leftGain = std::cos(panRad) * mGain;
        float rightGain = std::sin(panRad) * mGain;

        for (int32 i = 0; i < data.numSamples; i++)
        {
            float mono = (inL[i] + inR[i]) * 0.5f;
            outL[i] = mono * leftGain;
            outR[i] = mono * rightGain;
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

    IBStreamer streamer(state, kLittleEndian);

    float savedPan = 0.0f;
    if (!streamer.readFloat(savedPan))
        return kResultFalse;
    mPan = savedPan;

    float savedGain = 1.0f;
    if (!streamer.readFloat(savedGain))
        return kResultFalse;
    mGain = savedGain;

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::getState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    streamer.writeFloat((float)mPan);
    streamer.writeFloat((float)mGain);

    return kResultOk;
}

} // namespace SimplePanner
} // namespace Steinberg
