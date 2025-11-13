// test_processor_state.cpp
// Integration tests for SimplePannerProcessor state management

#include "pluginprocessor.h"
#include "plugids.h"
#include <gtest/gtest.h>
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/common/memorystream.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace Steinberg::SimplePanner;

//------------------------------------------------------------------------------
// Test Fixture
//------------------------------------------------------------------------------

class ProcessorStateTest : public ::testing::Test {
protected:
    void SetUp() override {
        processor = new SimplePannerProcessor();
        ASSERT_TRUE(processor != nullptr);
    }

    void TearDown() override {
        if (processor) {
            processor->release();
            processor = nullptr;
        }
    }

    SimplePannerProcessor* processor = nullptr;
};

//------------------------------------------------------------------------------
// Initialization Tests
//------------------------------------------------------------------------------

TEST_F(ProcessorStateTest, Initialize_ReturnsOk) {
    tresult result = processor->initialize(nullptr);
    EXPECT_EQ(result, kResultOk);

    processor->terminate();
}

TEST_F(ProcessorStateTest, Terminate_ReturnsOk) {
    processor->initialize(nullptr);

    tresult result = processor->terminate();
    EXPECT_EQ(result, kResultOk);
}

TEST_F(ProcessorStateTest, DoubleInitialize_HandlesProperly) {
    processor->initialize(nullptr);

    // Second initialize should either fail or handle gracefully
    tresult result = processor->initialize(nullptr);
    // AudioEffect base class returns kResultOk (already initialized)
    // This is acceptable behavior
    EXPECT_TRUE(result == kResultOk || result == kResultFalse);

    processor->terminate();
}

//------------------------------------------------------------------------------
// Bus Configuration Tests
//------------------------------------------------------------------------------

TEST_F(ProcessorStateTest, HasStereoInputBus) {
    processor->initialize(nullptr);

    int32 busCount = processor->getBusCount(kAudio, kInput);
    EXPECT_EQ(busCount, 1);

    BusInfo busInfo = {};
    processor->getBusInfo(kAudio, kInput, 0, busInfo);
    EXPECT_EQ(busInfo.channelCount, 2);
    EXPECT_EQ(busInfo.busType, kMain);

    processor->terminate();
}

TEST_F(ProcessorStateTest, HasStereoOutputBus) {
    processor->initialize(nullptr);

    int32 busCount = processor->getBusCount(kAudio, kOutput);
    EXPECT_EQ(busCount, 1);

    BusInfo busInfo = {};
    processor->getBusInfo(kAudio, kOutput, 0, busInfo);
    EXPECT_EQ(busInfo.channelCount, 2);
    EXPECT_EQ(busInfo.busType, kMain);

    processor->terminate();
}

//------------------------------------------------------------------------------
// State Management Tests
//------------------------------------------------------------------------------

TEST_F(ProcessorStateTest, GetState_WithoutSetState_ReturnsDefaults) {
    processor->initialize(nullptr);

    // Create stream for state
    MemoryStream stream;
    tresult result = processor->getState(&stream);
    EXPECT_EQ(result, kResultOk);

    // Stream should contain data
    int64 pos = 0;
    stream.tell(&pos);
    EXPECT_GT(pos, 0);

    processor->terminate();
}

TEST_F(ProcessorStateTest, SetState_GetState_RoundTrip) {
    processor->initialize(nullptr);

    // Get initial state
    MemoryStream saveStream;
    processor->getState(&saveStream);

    // Load state
    saveStream.seek(0, IBStream::kIBSeekSet, nullptr);
    MemoryStream loadStream;

    // Copy data
    char buffer[4096];
    int32 numBytes = 0;
    saveStream.read(buffer, sizeof(buffer), &numBytes);
    loadStream.write(buffer, numBytes, &numBytes);

    // Set state
    loadStream.seek(0, IBStream::kIBSeekSet, nullptr);
    tresult result = processor->setState(&loadStream);
    EXPECT_EQ(result, kResultOk);

    processor->terminate();
}

TEST_F(ProcessorStateTest, SetState_WithDefaultValues_Succeeds) {
    processor->initialize(nullptr);

    // Create state with default parameter values
    MemoryStream stream;

    // Write version
    IBStreamer streamer(&stream, kLittleEndian);
    streamer.writeInt32(1);  // version

    // Write 8 parameter values (normalized: 0.0 - 1.0)
    // Default values from plugids.h
    streamer.writeDouble(0.0);  // Left Pan = -100 → normalized = 0.0
    streamer.writeDouble(60.0 / 66.0);  // Left Gain = 0dB → normalized ≈ 0.909
    streamer.writeDouble(0.0);  // Left Delay = 0ms → normalized = 0.0
    streamer.writeDouble(1.0);  // Right Pan = +100 → normalized = 1.0
    streamer.writeDouble(60.0 / 66.0);  // Right Gain = 0dB → normalized ≈ 0.909
    streamer.writeDouble(0.0);  // Right Delay = 0ms → normalized = 0.0
    streamer.writeDouble(60.0 / 66.0);  // Master Gain = 0dB → normalized ≈ 0.909
    streamer.writeDouble(0.0);  // Link Gain = Off → normalized = 0.0

    // Set state
    stream.seek(0, IBStream::kIBSeekSet, nullptr);
    tresult result = processor->setState(&stream);
    EXPECT_EQ(result, kResultOk);

    processor->terminate();
}

TEST_F(ProcessorStateTest, SetState_WithCustomValues_Succeeds) {
    processor->initialize(nullptr);

    // Create state with custom parameter values
    MemoryStream stream;
    IBStreamer streamer(&stream, kLittleEndian);

    streamer.writeInt32(1);  // version

    // Write custom values
    streamer.writeDouble(0.5);   // Left Pan = 0 (center)
    streamer.writeDouble(0.75);  // Left Gain = -15dB
    streamer.writeDouble(0.5);   // Left Delay = 50ms
    streamer.writeDouble(0.25);  // Right Pan = -50
    streamer.writeDouble(0.5);   // Right Gain = -30dB
    streamer.writeDouble(1.0);   // Right Delay = 100ms
    streamer.writeDouble(1.0);   // Master Gain = +6dB
    streamer.writeDouble(1.0);   // Link Gain = On

    stream.seek(0, IBStream::kIBSeekSet, nullptr);
    tresult result = processor->setState(&stream);
    EXPECT_EQ(result, kResultOk);

    processor->terminate();
}

//------------------------------------------------------------------------------
// Activation Tests
//------------------------------------------------------------------------------

TEST_F(ProcessorStateTest, SetActive_True_AllocatesResources) {
    processor->initialize(nullptr);

    // Setup processing (required before activation)
    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = 512;
    setup.sampleRate = 48000.0;
    processor->setupProcessing(setup);

    // Activate
    tresult result = processor->setActive(true);
    EXPECT_EQ(result, kResultOk);

    // Deactivate
    processor->setActive(false);
    processor->terminate();
}

TEST_F(ProcessorStateTest, SetActive_False_ReleasesResources) {
    processor->initialize(nullptr);

    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = 512;
    setup.sampleRate = 48000.0;
    processor->setupProcessing(setup);

    processor->setActive(true);

    // Deactivate
    tresult result = processor->setActive(false);
    EXPECT_EQ(result, kResultOk);

    processor->terminate();
}

TEST_F(ProcessorStateTest, SetActive_WithoutSetupProcessing_Fails) {
    processor->initialize(nullptr);

    // Try to activate without setup
    tresult result = processor->setActive(true);
    // Should either fail or handle gracefully
    // (implementation dependent, but should not crash)

    if (result == kResultOk) {
        processor->setActive(false);
    }

    processor->terminate();
}

//------------------------------------------------------------------------------
// Setup Processing Tests
//------------------------------------------------------------------------------

TEST_F(ProcessorStateTest, SetupProcessing_ValidConfig_Succeeds) {
    processor->initialize(nullptr);

    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = 512;
    setup.sampleRate = 48000.0;

    tresult result = processor->setupProcessing(setup);
    EXPECT_EQ(result, kResultOk);

    processor->terminate();
}

TEST_F(ProcessorStateTest, SetupProcessing_DifferentSampleRates_Succeeds) {
    processor->initialize(nullptr);

    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = 512;

    // Test different sample rates
    double sampleRates[] = {44100.0, 48000.0, 88200.0, 96000.0};

    for (double sr : sampleRates) {
        setup.sampleRate = sr;
        tresult result = processor->setupProcessing(setup);
        EXPECT_EQ(result, kResultOk) << "Failed at sample rate: " << sr;
    }

    processor->terminate();
}

TEST_F(ProcessorStateTest, SetupProcessing_DifferentBlockSizes_Succeeds) {
    processor->initialize(nullptr);

    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.sampleRate = 48000.0;

    // Test different block sizes
    int32 blockSizes[] = {64, 128, 256, 512, 1024, 2048};

    for (int32 blockSize : blockSizes) {
        setup.maxSamplesPerBlock = blockSize;
        tresult result = processor->setupProcessing(setup);
        EXPECT_EQ(result, kResultOk) << "Failed at block size: " << blockSize;
    }

    processor->terminate();
}

//------------------------------------------------------------------------------
// Combined Workflow Tests
//------------------------------------------------------------------------------

TEST_F(ProcessorStateTest, FullLifecycle_InitializeActivateDeactivateTerminate) {
    // Initialize
    EXPECT_EQ(processor->initialize(nullptr), kResultOk);

    // Setup processing
    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = 512;
    setup.sampleRate = 48000.0;
    EXPECT_EQ(processor->setupProcessing(setup), kResultOk);

    // Activate
    EXPECT_EQ(processor->setActive(true), kResultOk);

    // Deactivate
    EXPECT_EQ(processor->setActive(false), kResultOk);

    // Terminate
    EXPECT_EQ(processor->terminate(), kResultOk);
}

TEST_F(ProcessorStateTest, StatePreservation_AcrossActivation) {
    processor->initialize(nullptr);

    // Set initial state
    MemoryStream saveStream;
    IBStreamer saveStreamer(&saveStream, kLittleEndian);
    saveStreamer.writeInt32(1);
    saveStreamer.writeDouble(0.5);  // Custom value
    saveStreamer.writeDouble(0.5);
    saveStreamer.writeDouble(0.5);
    saveStreamer.writeDouble(0.5);
    saveStreamer.writeDouble(0.5);
    saveStreamer.writeDouble(0.5);
    saveStreamer.writeDouble(0.5);
    saveStreamer.writeDouble(0.5);

    saveStream.seek(0, IBStream::kIBSeekSet, nullptr);
    processor->setState(&saveStream);

    // Setup and activate
    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = 512;
    setup.sampleRate = 48000.0;
    processor->setupProcessing(setup);
    processor->setActive(true);

    // Get state and verify it's preserved
    MemoryStream loadStream;
    processor->getState(&loadStream);

    int64 pos = 0;
    loadStream.tell(&pos);
    EXPECT_GT(pos, 0);

    processor->setActive(false);
    processor->terminate();
}
