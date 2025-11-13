#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// Plugin UIDs
// これらは一意である必要があります。本番環境では必ず変更してください。
//------------------------------------------------------------------------
static const FUID ProcessorUID(0xA1B2C3D4, 0xE5F61234, 0x56789ABC, 0xDEF01234);
static const FUID ControllerUID(0x4D3C2B1A, 0x43216F5E, 0xCBA98765, 0x43210FED);

//------------------------------------------------------------------------
// Parameter IDs
//------------------------------------------------------------------------
enum ParameterID : Vst::ParamID {
    kParamLeftPan = 0,      // Left channel pan: -100 to +100
    kParamLeftGain = 1,     // Left channel gain: -60 to +6 dB
    kParamLeftDelay = 2,    // Left channel delay: 0 to 100 ms
    kParamRightPan = 3,     // Right channel pan: -100 to +100
    kParamRightGain = 4,    // Right channel gain: -60 to +6 dB
    kParamRightDelay = 5,   // Right channel delay: 0 to 100 ms
    kParamMasterGain = 6,   // Master gain: -60 to +6 dB
    kParamLinkGain = 7,     // Link L/R gain: 0 (off) or 1 (on)
    kParamCount             // Total parameter count
};

//------------------------------------------------------------------------
// Parameter Value Ranges (Plain values, not normalized)
//------------------------------------------------------------------------
namespace ParamRange {
    constexpr float kPanMin = -100.0f;
    constexpr float kPanMax = 100.0f;
    constexpr float kGainMin = -60.0f;   // -60dB treated as -∞ (mute)
    constexpr float kGainMax = 6.0f;
    constexpr float kDelayMin = 0.0f;
    constexpr float kDelayMax = 100.0f;  // milliseconds
}

//------------------------------------------------------------------------
// Default Parameter Values (Transparent/Bypass state)
//------------------------------------------------------------------------
namespace ParamDefault {
    constexpr float kLeftPan = -100.0f;   // Full left
    constexpr float kLeftGain = 0.0f;     // Unity gain (0dB)
    constexpr float kLeftDelay = 0.0f;    // No delay
    constexpr float kRightPan = 100.0f;   // Full right
    constexpr float kRightGain = 0.0f;    // Unity gain (0dB)
    constexpr float kRightDelay = 0.0f;   // No delay
    constexpr float kMasterGain = 0.0f;   // Unity gain (0dB)
    constexpr float kLinkGain = 0.0f;     // Off
}

//------------------------------------------------------------------------
// Plugin Information
//------------------------------------------------------------------------
namespace PluginInfo {
    constexpr const char* kName = "SimplePanner";
    constexpr const char* kVendor = "Example Company";
    constexpr const char* kVersion = "1.0.0";
    constexpr const char* kEmail = "info@example.com";
    constexpr const char* kUrl = "https://www.example.com";
}

} // namespace SimplePanner
} // namespace Steinberg
