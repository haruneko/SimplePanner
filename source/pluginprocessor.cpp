#include "pluginprocessor.h"
#include "plugids.h"
#include "parameter_utils.h"
#include "pan_calculator.h"

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
    // Process parameter changes
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

                // Get last point and update smoothers
                if (numPoints > 0 && paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                {
                    switch (paramQueue->getParameterId())
                    {
                        case kParamLeftPan:
                            mLeftPan = value;
                            mLeftPanSmoother.setTarget(static_cast<float>(value));
                            break;
                        case kParamLeftGain:
                            mLeftGain = value;
                            mLeftGainSmoother.setTarget(static_cast<float>(value));
                            break;
                        case kParamLeftDelay:
                            mLeftDelay = value;
                            if (mIsActive)
                            {
                                size_t delaySamples = delayMsToSamples(normalizedToDelayMs(value), mSampleRate);
                                mDelayLeft.setDelay(delaySamples);
                            }
                            break;
                        case kParamRightPan:
                            mRightPan = value;
                            mRightPanSmoother.setTarget(static_cast<float>(value));
                            break;
                        case kParamRightGain:
                            mRightGain = value;
                            mRightGainSmoother.setTarget(static_cast<float>(value));
                            break;
                        case kParamRightDelay:
                            mRightDelay = value;
                            if (mIsActive)
                            {
                                size_t delaySamples = delayMsToSamples(normalizedToDelayMs(value), mSampleRate);
                                mDelayRight.setDelay(delaySamples);
                            }
                            break;
                        case kParamMasterGain:
                            mMasterGain = value;
                            mMasterGainSmoother.setTarget(static_cast<float>(value));
                            break;
                        case kParamLinkGain:
                            mLinkGain = value;
                            break;
                    }
                }
            }
        }
    }

    // Check for valid I/O
    if (data.numInputs == 0 || data.numOutputs == 0)
        return kResultOk;

    Vst::AudioBusBuffers& inputBus = data.inputs[0];
    Vst::AudioBusBuffers& outputBus = data.outputs[0];

    // Process stereo audio
    if (inputBus.numChannels >= 2 && outputBus.numChannels >= 2)
    {
        float* inL = inputBus.channelBuffers32[0];
        float* inR = inputBus.channelBuffers32[1];
        float* outL = outputBus.channelBuffers32[0];
        float* outR = outputBus.channelBuffers32[1];

        // Process each sample
        for (int32 i = 0; i < data.numSamples; i++)
        {
            // Get smoothed parameter values (per-sample)
            float leftPanNorm = mLeftPanSmoother.getNext();
            float leftGainNorm = mLeftGainSmoother.getNext();
            float rightPanNorm = mRightPanSmoother.getNext();
            float rightGainNorm = mRightGainSmoother.getNext();
            float masterGainNorm = mMasterGainSmoother.getNext();

            // Convert normalized parameters to usable values
            float leftPanValue = normalizedToPan(leftPanNorm);
            float leftGainDb = normalizedToDb(leftGainNorm);
            float leftGainLinear = dbToLinear(leftGainDb);

            float rightPanValue = normalizedToPan(rightPanNorm);
            float rightGainDb = normalizedToDb(rightGainNorm);
            float rightGainLinear = dbToLinear(rightGainDb);

            float masterGainDb = normalizedToDb(masterGainNorm);
            float masterGainLinear = dbToLinear(masterGainDb);

            // Calculate pan gains
            PanGains leftPanGains = calculatePanGains(leftPanValue);
            PanGains rightPanGains = calculatePanGains(rightPanValue);

            // Read input samples
            float inLeftSample = inL[i];
            float inRightSample = inR[i];

            // Apply delay
            float delayedLeft = mDelayLeft.process(inLeftSample);
            float delayedRight = mDelayRight.process(inRightSample);

            // Apply gain
            float gainedLeft = delayedLeft * leftGainLinear;
            float gainedRight = delayedRight * rightGainLinear;

            // Apply panning and mix
            float mixedLeft = gainedLeft * leftPanGains.left + gainedRight * rightPanGains.left;
            float mixedRight = gainedLeft * leftPanGains.right + gainedRight * rightPanGains.right;

            // Apply master gain
            outL[i] = mixedLeft * masterGainLinear;
            outR[i] = mixedRight * masterGainLinear;
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
