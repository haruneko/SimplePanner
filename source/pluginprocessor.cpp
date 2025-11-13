#include "pluginprocessor.h"
#include "plugids.h"
#include "parameter_utils.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// SimplePannerProcessor
//------------------------------------------------------------------------
SimplePannerProcessor::SimplePannerProcessor()
    : mSampleRate(48000.0)
    , mIsActive(false)
{
    setControllerClass(ControllerUID);

    // Initialize parameter values to defaults (normalized 0.0 - 1.0)
    mLeftPan = panToNormalized(ParamDefault::kLeftPan);
    mLeftGain = dbToNormalized(ParamDefault::kLeftGain);
    mLeftDelay = delayMsToNormalized(ParamDefault::kLeftDelay);
    mRightPan = panToNormalized(ParamDefault::kRightPan);
    mRightGain = dbToNormalized(ParamDefault::kRightGain);
    mRightDelay = delayMsToNormalized(ParamDefault::kRightDelay);
    mMasterGain = dbToNormalized(ParamDefault::kMasterGain);
    mLinkGain = ParamDefault::kLinkGain;
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
    if (state)
    {
        // Activate: allocate delay line buffers
        // Calculate max delay samples: 100ms at current sample rate
        size_t maxDelaySamples = static_cast<size_t>(0.1 * mSampleRate);  // 100ms
        mDelayLeft.resize(maxDelaySamples + 1);
        mDelayRight.resize(maxDelaySamples + 1);

        // Reset delay lines to zero
        mDelayLeft.reset();
        mDelayRight.reset();

        // Initialize parameter smoothers with current sample rate
        mLeftPanSmoother.setSampleRate(mSampleRate);
        mLeftGainSmoother.setSampleRate(mSampleRate);
        mRightPanSmoother.setSampleRate(mSampleRate);
        mRightGainSmoother.setSampleRate(mSampleRate);
        mMasterGainSmoother.setSampleRate(mSampleRate);

        // Reset smoothers to current parameter values
        mLeftPanSmoother.reset(static_cast<float>(mLeftPan));
        mLeftGainSmoother.reset(static_cast<float>(mLeftGain));
        mRightPanSmoother.reset(static_cast<float>(mRightPan));
        mRightGainSmoother.reset(static_cast<float>(mRightGain));
        mMasterGainSmoother.reset(static_cast<float>(mMasterGain));

        mIsActive = true;
    }
    else
    {
        // Deactivate: resources will be freed by destructors
        mIsActive = false;
    }

    return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::process(Vst::ProcessData& data)
{
    // TODO: パラメータ処理とオーディオ処理を実装 (Task 2.2)
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
    // Save sample rate
    mSampleRate = newSetup.sampleRate;

    // Update parameter smoothers if active
    if (mIsActive)
    {
        mLeftPanSmoother.setSampleRate(mSampleRate);
        mLeftGainSmoother.setSampleRate(mSampleRate);
        mRightPanSmoother.setSampleRate(mSampleRate);
        mRightGainSmoother.setSampleRate(mSampleRate);
        mMasterGainSmoother.setSampleRate(mSampleRate);

        // Resize delay lines for new sample rate
        size_t maxDelaySamples = static_cast<size_t>(0.1 * mSampleRate);  // 100ms
        mDelayLeft.resize(maxDelaySamples + 1);
        mDelayRight.resize(maxDelaySamples + 1);

        // Update current delay amounts
        size_t leftDelaySamples = delayMsToSamples(normalizedToDelayMs(mLeftDelay), mSampleRate);
        size_t rightDelaySamples = delayMsToSamples(normalizedToDelayMs(mRightDelay), mSampleRate);
        mDelayLeft.setDelay(leftDelaySamples);
        mDelayRight.setDelay(rightDelaySamples);
    }

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

    // Read version
    int32 version = 0;
    if (!streamer.readInt32(version))
        return kResultFalse;

    if (version != 1)
        return kResultFalse;

    // Read 8 parameter values (normalized)
    if (!streamer.readDouble(mLeftPan)) return kResultFalse;
    if (!streamer.readDouble(mLeftGain)) return kResultFalse;
    if (!streamer.readDouble(mLeftDelay)) return kResultFalse;
    if (!streamer.readDouble(mRightPan)) return kResultFalse;
    if (!streamer.readDouble(mRightGain)) return kResultFalse;
    if (!streamer.readDouble(mRightDelay)) return kResultFalse;
    if (!streamer.readDouble(mMasterGain)) return kResultFalse;
    if (!streamer.readDouble(mLinkGain)) return kResultFalse;

    // Update delay lines if active
    if (mIsActive)
    {
        size_t leftDelaySamples = delayMsToSamples(normalizedToDelayMs(mLeftDelay), mSampleRate);
        size_t rightDelaySamples = delayMsToSamples(normalizedToDelayMs(mRightDelay), mSampleRate);
        mDelayLeft.setDelay(leftDelaySamples);
        mDelayRight.setDelay(rightDelaySamples);
    }

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerProcessor::getState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    // Write version
    streamer.writeInt32(1);

    // Write 8 parameter values (normalized)
    streamer.writeDouble(mLeftPan);
    streamer.writeDouble(mLeftGain);
    streamer.writeDouble(mLeftDelay);
    streamer.writeDouble(mRightPan);
    streamer.writeDouble(mRightGain);
    streamer.writeDouble(mRightDelay);
    streamer.writeDouble(mMasterGain);
    streamer.writeDouble(mLinkGain);

    return kResultOk;
}

} // namespace SimplePanner
} // namespace Steinberg
