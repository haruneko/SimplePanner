# SimplePanner - Design Document

## 1. Overview

本ドキュメントは SimplePanner VST3 プラグインの詳細設計を定義します。requirements.md で定義された要件を実装レベルに落とし込み、クラス構造、データフロー、アルゴリズム、スレッドセーフティを明確にします。

## 2. Architecture

### 2.1 High-Level Architecture

```
┌─────────────────────────────────────────────────────────┐
│                         DAW Host                         │
└─────────────┬───────────────────────────┬───────────────┘
              │                           │
              │ Audio Thread              │ UI Thread
              │                           │
    ┌─────────▼──────────┐      ┌────────▼─────────┐
    │ SimplePannerProcessor│      │SimplePannerController│
    │  (Audio Processing) │      │   (UI & Parameters)  │
    └─────────┬──────────┘      └────────┬─────────┘
              │                           │
              │ Parameter Changes         │
              └───────────────────────────┘
```

### 2.2 Component Responsibilities

| Component | Responsibility | Thread |
|-----------|----------------|--------|
| SimplePannerProcessor | オーディオ処理、DSP、状態管理 | Audio Thread |
| SimplePannerController | パラメータ管理、UI通信、状態同期 | UI Thread |
| DelayLine | 遅延バッファ管理 | Audio Thread |
| ParameterSmoother | パラメータ平滑化 | Audio Thread |

## 3. Class Design

### 3.1 Class Diagram

```
┌────────────────────────┐
│   Vst::AudioEffect     │
└───────────┬────────────┘
            │ inherits
┌───────────▼────────────────────────────────────┐
│        SimplePannerProcessor                   │
├────────────────────────────────────────────────┤
│ - DelayLine mDelayLeft                         │
│ - DelayLine mDelayRight                        │
│ - ParameterSmoother mLeftGainSmoother          │
│ - ParameterSmoother mRightGainSmoother         │
│ - ParameterSmoother mMasterGainSmoother        │
│ - ParameterSmoother mLeftPanSmoother           │
│ - ParameterSmoother mRightPanSmoother          │
│ - bool mLinkGain                                │
│ - double mSampleRate                           │
├────────────────────────────────────────────────┤
│ + initialize(context)                          │
│ + terminate()                                  │
│ + setActive(state)                             │
│ + process(data)                                │
│ + setupProcessing(setup)                       │
│ + setState(state)                              │
│ + getState(state)                              │
│ - processAudio(data)                           │
│ - updateParameters(data)                       │
└────────────────────────────────────────────────┘

┌────────────────────────┐
│  Vst::EditController   │
└───────────┬────────────┘
            │ inherits
┌───────────▼────────────────────────────────────┐
│       SimplePannerController                   │
├────────────────────────────────────────────────┤
│ (no additional state)                          │
├────────────────────────────────────────────────┤
│ + initialize(context)                          │
│ + terminate()                                  │
│ + setComponentState(state)                     │
│ + setParamNormalized(id, value)                │
│ + getParamStringByValue(id, value, string)     │
│ + getParamValueByString(id, string, value)     │
└────────────────────────────────────────────────┘

┌────────────────────────────────────────────────┐
│              DelayLine                         │
├────────────────────────────────────────────────┤
│ - std::vector<float> mBuffer                   │
│ - size_t mWriteIndex                           │
│ - size_t mMaxDelaySamples                      │
│ - size_t mCurrentDelaySamples                  │
├────────────────────────────────────────────────┤
│ + DelayLine(maxDelaySamples)                   │
│ + reset()                                      │
│ + setDelay(samples)                            │
│ + process(input) -> float                      │
│ + resize(newMaxDelay)                          │
└────────────────────────────────────────────────┘

┌────────────────────────────────────────────────┐
│          ParameterSmoother                     │
├────────────────────────────────────────────────┤
│ - float mCurrentValue                          │
│ - float mTargetValue                           │
│ - float mSmoothingCoeff                        │
│ - int mSmoothingTimeSamples                    │
├────────────────────────────────────────────────┤
│ + ParameterSmoother(smoothingTimeMs, sampleRate)│
│ + setTarget(value)                             │
│ + getNext() -> float                           │
│ + reset(value)                                 │
│ + isSmoothing() -> bool                        │
└────────────────────────────────────────────────┘
```

### 3.2 Parameter Definitions

```cpp
// include/plugids.h

namespace Steinberg {
namespace SimplePanner {

// Plugin UIDs
static const FUID ProcessorUID(0xA1B2C3D4, 0xE5F61234, 0x56789ABC, 0xDEF01234);
static const FUID ControllerUID(0x4D3C2B1A, 0x43216F5E, 0xCBA98765, 0x43210FED);

// Parameter IDs
enum ParameterID : Vst::ParamID {
    kParamLeftPan = 0,      // -100 to +100
    kParamLeftGain = 1,     // -60 to +6 dB
    kParamLeftDelay = 2,    // 0 to 100 ms
    kParamRightPan = 3,     // -100 to +100
    kParamRightGain = 4,    // -60 to +6 dB
    kParamRightDelay = 5,   // 0 to 100 ms
    kParamMasterGain = 6,   // -60 to +6 dB
    kParamLinkGain = 7,     // 0 (off) or 1 (on)
    kParamCount
};

// Parameter value ranges
namespace ParamRange {
    constexpr float kPanMin = -100.0f;
    constexpr float kPanMax = 100.0f;
    constexpr float kGainMin = -60.0f;
    constexpr float kGainMax = 6.0f;
    constexpr float kDelayMin = 0.0f;
    constexpr float kDelayMax = 100.0f;
}

// Default values (bypass/transparent state)
namespace ParamDefault {
    constexpr float kLeftPan = -100.0f;   // Full left
    constexpr float kLeftGain = 0.0f;     // Unity gain
    constexpr float kLeftDelay = 0.0f;    // No delay
    constexpr float kRightPan = 100.0f;   // Full right
    constexpr float kRightGain = 0.0f;    // Unity gain
    constexpr float kRightDelay = 0.0f;   // No delay
    constexpr float kMasterGain = 0.0f;   // Unity gain
    constexpr float kLinkGain = 0.0f;     // Off
}

} // namespace SimplePanner
} // namespace Steinberg
```

### 3.3 GUI Editor Design

#### 3.3.1 Editor Class Structure

```
┌────────────────────────┐
│  Vst::EditorView       │
└───────────┬────────────┘
            │ inherits
┌───────────▼────────────────────────────────────┐
│       SimplePannerEditor                       │
├────────────────────────────────────────────────┤
│ - VSTGUI::CFrame* frame                        │
│ - VSTGUI::CSlider* leftPanSlider               │
│ - VSTGUI::CSlider* rightPanSlider              │
│ - VSTGUI::CKnob* leftGainKnob                  │
│ - VSTGUI::CKnob* rightGainKnob                 │
│ - VSTGUI::CKnob* leftDelayKnob                 │
│ - VSTGUI::CKnob* rightDelayKnob                │
│ - VSTGUI::CKnob* masterGainKnob                │
│ - VSTGUI::CTextButton* linkToggle              │
│ - VSTGUI::CTextLabel* valueLabels[8]           │
│ - EditController* controller                   │
├────────────────────────────────────────────────┤
│ + SimplePannerEditor(controller)               │
│ + ~SimplePannerEditor()                        │
│ + open(parent)                                 │
│ + close()                                      │
│ + getRect(size)                                │
│ + valueChanged(control)                        │
│ + controlBeginEdit(tag)                        │
│ + controlEndEdit(tag)                          │
│ - createUI()                                   │
│ - updateValueDisplay(tag)                      │
│ - formatPanValue(value) -> string              │
│ - formatGainValue(value) -> string             │
│ - formatDelayValue(value) -> string            │
└────────────────────────────────────────────────┘
```

#### 3.3.2 VSTGUI Control Hierarchy

```
CFrame (600x400px)
 │
 ├─ CViewContainer (Left Channel Group, 280x180px)
 │   ├─ CTextLabel ("LEFT CHANNEL")
 │   ├─ CSlider (Pan, 200x15px, horizontal)
 │   ├─ CTextLabel (Pan value display, e.g., "L50")
 │   ├─ CKnob (Gain, 60x60px)
 │   ├─ CTextLabel (Gain value display, e.g., "-3.0 dB")
 │   ├─ CKnob (Delay, 60x60px)
 │   └─ CTextLabel (Delay value display, e.g., "10.0 ms")
 │
 ├─ CViewContainer (Right Channel Group, 280x180px)
 │   ├─ CTextLabel ("RIGHT CHANNEL")
 │   ├─ CSlider (Pan, 200x15px, horizontal)
 │   ├─ CTextLabel (Pan value display, e.g., "R75")
 │   ├─ CKnob (Gain, 60x60px)
 │   ├─ CTextLabel (Gain value display, e.g., "0.0 dB")
 │   ├─ CKnob (Delay, 60x60px)
 │   └─ CTextLabel (Delay value display, e.g., "0.0 ms")
 │
 └─ CViewContainer (Master Section, 560x120px)
     ├─ CTextLabel ("MASTER")
     ├─ CKnob (Master Gain, 60x60px)
     ├─ CTextLabel (Master Gain value display, e.g., "0.0 dB")
     ├─ CTextButton (Link L/R Gain toggle, 80x25px)
     └─ CTextLabel ("Link L/R Gain")
```

#### 3.3.3 GUI Implementation Details

##### Color Scheme

**Background Colors**:
- Frame background: `CColor(60, 60, 60, 255)` - #3C3C3C (Dark gray)
- Group boxes: `CColor(80, 80, 80, 255)` - #505050 (Medium gray)

**Control Colors** (視認性重視):
- Slider track: `CColor(50, 50, 50, 255)` - #323232 (Very dark, darker than background)
- Slider handle: `CColor(255, 255, 255, 255)` - #FFFFFF (White for high contrast)
- Slider frame: `CColor(40, 40, 40, 255)` - #282828 (Very dark border)
- Knob handle: `CColor(200, 200, 200, 255)` - #C8C8C8 (Light gray handle, high visibility)
- Knob corona: `CColor(120, 120, 120, 255)` - #787878 (Medium gray arc)

**Text Colors**:
- Title labels: `CColor(176, 176, 176, 255)` - #B0B0B0 (Light gray)
- Value text: `CColor(255, 255, 255, 255)` - #FFFFFF (White)
- L/R labels: `CColor(176, 176, 176, 255)` - #B0B0B0 (Light gray)

**Accent Colors**:
- Link button ON: `CColor(76, 175, 80, 255)` - #4CAF50 (Green)
- Link button OFF: `CColor(96, 96, 96, 255)` - #606060 (Gray)

**Design Rationale**:
- スライダートラックを背景(80,80,80)より暗い(50,50,50)にして境界を明確化
- スライダーハンドルを白(255,255,255)にして現在位置を明確に視認可能に
- ノブハンドルを明るい灰色(200,200,200)にして現在位置を明確化
- ノブコロナを中間灰色(120,120,120)にして値の範囲を表示
- すべてのコントロールが暗い環境でも識別可能

##### Control Configuration

**Pan Sliders** (視認性改善版):
```cpp
CSlider* slider = new CSlider(
    CRect(x, y, x+200, y+15),           // Position and size (200x15px)
    this,                                // Listener
    kParamLeftPan,                       // Tag (parameter ID)
    x, x+200,                            // Min/max X
    nullptr,                             // Handle bitmap (nullptr = default)
    nullptr                              // Background bitmap (nullptr = default)
);
slider->setDefaultValue(0.0f);           // Center for left, 1.0 for right
slider->setMin(0.0f);
slider->setMax(1.0f);
slider->setBackColor(CColor(50, 50, 50, 255));    // Track: Very dark gray
slider->setFrameColor(CColor(40, 40, 40, 255));   // Border: Even darker
slider->setValueColor(CColor(255, 255, 255, 255)); // Handle: White (high visibility)
```

**Gain/Delay Knobs** (視認性改善版):
```cpp
CKnob* knob = new CKnob(
    CRect(x, y, x+60, y+60),             // Position and size
    this,                                // Listener
    kParamLeftGain,                      // Tag (parameter ID)
    nullptr,                             // Background bitmap (nullptr = default draw)
    nullptr                              // Handle bitmap (nullptr = default)
);
knob->setMin(0.0f);
knob->setMax(1.0f);
knob->setDefaultValue(dbToNormalized(0.0f));  // 0dB
knob->setZoomFactor(10.0f);              // Shift+drag for fine adjustment
knob->setColorHandle(CColor(200, 200, 200, 255));  // Light gray handle (high visibility)
knob->setCoronaColor(CColor(120, 120, 120, 255));  // Medium gray corona
knob->setDrawStyle(CKnob::kHandleCircleDrawing | CKnob::kCoronaDrawing);  // Enable visual elements
```

**Link Toggle Button**:
```cpp
CTextButton* toggle = new CTextButton(
    CRect(x, y, x+80, y+25),
    this,
    kParamLinkGain,
    "LINKED",                            // Text when ON
    CTextButton::kOnOffStyle
);
toggle->setTextColorOff(CColor(200, 200, 200, 255));
toggle->setTextColorOn(CColor(255, 255, 255, 255));
toggle->setGradientOff(CColor(100, 100, 100, 255));
toggle->setGradientOn(CColor(76, 175, 80, 255));  // Green
```

##### Value Display Formatting

```cpp
std::string SimplePannerEditor::formatPanValue(float normalized) {
    float pan = normalizedToPan(normalized);
    if (std::abs(pan) < 0.5f) {
        return "C";  // Center
    } else if (pan < 0) {
        return "L" + std::to_string(static_cast<int>(std::abs(pan)));
    } else {
        return "R" + std::to_string(static_cast<int>(pan));
    }
}

std::string SimplePannerEditor::formatGainValue(float normalized) {
    float db = normalizedToDb(normalized);
    if (db <= -60.0f) {
        return "-∞ dB";
    } else {
        char buf[16];
        snprintf(buf, sizeof(buf), "%+.1f dB", db);
        return std::string(buf);
    }
}

std::string SimplePannerEditor::formatDelayValue(float normalized) {
    float ms = normalizedToDelayMs(normalized);
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f ms", ms);
    return std::string(buf);
}
```

#### 3.3.4 Parameter Update Flow

```
User interacts with control
        │
        ├─ valueChanged(control) called
        │       │
        │       ├─ Get normalized value from control
        │       ├─ controller->setParamNormalized(tag, value)
        │       │       │
        │       │       ├─ Controller validates and stores value
        │       │       ├─ Link L/R Gain logic applied (if enabled)
        │       │       └─ Controller notifies Processor via parameter queue
        │       │
        │       └─ updateValueDisplay(tag)
        │               │
        │               └─ Format and display value string
        │
        └─ controlEndEdit(tag) called
                │
                └─ End parameter automation recording (if active)
```

#### 3.3.5 Colors and Styling

```cpp
namespace Colors {
    const CColor kBackground(44, 44, 44, 255);          // #2C2C2C
    const CColor kGroupBox(60, 60, 60, 255);            // #3C3C3C
    const CColor kLabelText(204, 204, 204, 255);        // #CCCCCC
    const CColor kValueText(255, 255, 255, 255);        // #FFFFFF
    const CColor kKnobBody(80, 80, 80, 255);            // #505050
    const CColor kKnobIndicator(74, 144, 226, 255);     // #4A90E2
    const CColor kSliderTrack(64, 64, 64, 255);         // #404040
    const CColor kSliderHandle(74, 144, 226, 255);      // #4A90E2
    const CColor kToggleOn(76, 175, 80, 255);           // #4CAF50
    const CColor kToggleOff(96, 96, 96, 255);           // #606060
}
```

#### 3.3.6 Layout Coordinates

```cpp
// Window dimensions
constexpr int32 kEditorWidth = 600;
constexpr int32 kEditorHeight = 400;

// Margins and spacing
constexpr int32 kMargin = 20;
constexpr int32 kGroupPadding = 15;
constexpr int32 kControlSpacing = 20;

// Group boxes
constexpr CRect kLeftChannelBox(20, 40, 300, 220);
constexpr CRect kRightChannelBox(320, 40, 580, 220);
constexpr CRect kMasterBox(20, 240, 580, 360);

// Control sizes
constexpr int32 kSliderWidth = 200;
constexpr int32 kSliderHeight = 15;
constexpr int32 kKnobSize = 60;
constexpr int32 kToggleWidth = 80;
constexpr int32 kToggleHeight = 25;
```

## 4. Data Structures

### 4.1 Processor State

```cpp
// Stored in processor, serialized in setState/getState
struct ProcessorState {
    // Parameter values in plain units (not normalized)
    float leftPan;        // -100.0 to +100.0
    float leftGain;       // -60.0 to +6.0 dB
    float leftDelay;      // 0.0 to 100.0 ms
    float rightPan;       // -100.0 to +100.0
    float rightGain;      // -60.0 to +6.0 dB
    float rightDelay;     // 0.0 to 100.0 ms
    float masterGain;     // -60.0 to +6.0 dB
    bool linkGain;        // false or true
};
```

### 4.2 Audio Processing State

```cpp
// Runtime state in processor (not serialized)
struct AudioState {
    // Smoothed linear gain values
    float leftGainLinear;
    float rightGainLinear;
    float masterGainLinear;

    // Pan coefficients (pre-calculated)
    float leftPanL;   // Left input to left output gain
    float leftPanR;   // Left input to right output gain
    float rightPanL;  // Right input to left output gain
    float rightPanR;  // Right input to right output gain

    // Delay samples
    size_t leftDelaySamples;
    size_t rightDelaySamples;
};
```

## 5. Algorithm Details

### 5.1 Parameter Conversion

#### 5.1.1 Pan: Normalized ↔ Plain Value

```cpp
// Normalized (0.0 - 1.0) to Plain (-100.0 to +100.0)
float normalizedToPan(float normalized) {
    return ParamRange::kPanMin + normalized * (ParamRange::kPanMax - ParamRange::kPanMin);
}

// Plain (-100.0 to +100.0) to Normalized (0.0 - 1.0)
float panToNormalized(float pan) {
    return (pan - ParamRange::kPanMin) / (ParamRange::kPanMax - ParamRange::kPanMin);
}
```

#### 5.1.2 Gain: Normalized ↔ dB ↔ Linear

```cpp
// Normalized (0.0 - 1.0) to dB (-60.0 to +6.0)
float normalizedToDb(float normalized) {
    return ParamRange::kGainMin + normalized * (ParamRange::kGainMax - ParamRange::kGainMin);
}

// dB to Normalized
float dbToNormalized(float db) {
    return (db - ParamRange::kGainMin) / (ParamRange::kGainMax - ParamRange::kGainMin);
}

// dB to Linear gain
float dbToLinear(float db) {
    if (db <= -60.0f) return 0.0f;  // -∞ (mute)
    return std::pow(10.0f, db / 20.0f);
}

// Linear to dB
float linearToDb(float linear) {
    if (linear <= 0.0f) return -60.0f;  // -∞
    return 20.0f * std::log10(linear);
}
```

#### 5.1.3 Delay: Normalized ↔ ms ↔ Samples

```cpp
// Normalized (0.0 - 1.0) to ms (0.0 - 100.0)
float normalizedToDelayMs(float normalized) {
    return ParamRange::kDelayMin + normalized * (ParamRange::kDelayMax - ParamRange::kDelayMin);
}

// ms to Normalized
float delayMsToNormalized(float ms) {
    return (ms - ParamRange::kDelayMin) / (ParamRange::kDelayMax - ParamRange::kDelayMin);
}

// ms to Samples (sample-accurate, rounded)
size_t delayMsToSamples(float ms, double sampleRate) {
    return static_cast<size_t>(std::round(ms * sampleRate / 1000.0));
}

// Samples to ms
float delaySamplesToMs(size_t samples, double sampleRate) {
    return static_cast<float>(samples * 1000.0 / sampleRate);
}
```

### 5.2 Pan Calculation (Equal Power)

```cpp
// Calculate pan gains using equal power law
struct PanGains {
    float left;   // Gain to left output
    float right;  // Gain to right output
};

PanGains calculatePanGains(float pan) {
    // pan: -100.0 (full left) to +100.0 (full right)
    // Normalize to -1.0 to +1.0
    float normalizedPan = pan / 100.0f;

    // Map to angle: -π/4 to +π/4 radians
    float angle = normalizedPan * M_PI / 4.0f;

    // Equal power pan law
    PanGains gains;
    gains.left = std::cos(angle);   // 1.0 at -100, 0.707 at 0, 0.0 at +100
    gains.right = std::sin(angle);  // 0.0 at -100, 0.707 at 0, 1.0 at +100

    return gains;
}
```

### 5.3 Delay Line Implementation

```cpp
class DelayLine {
public:
    DelayLine(size_t maxDelaySamples)
        : mMaxDelaySamples(maxDelaySamples)
        , mCurrentDelaySamples(0)
        , mWriteIndex(0)
    {
        mBuffer.resize(maxDelaySamples, 0.0f);
    }

    void reset() {
        std::fill(mBuffer.begin(), mBuffer.end(), 0.0f);
        mWriteIndex = 0;
    }

    void setDelay(size_t samples) {
        mCurrentDelaySamples = std::min(samples, mMaxDelaySamples);
    }

    float process(float input) {
        // Write input to current position
        mBuffer[mWriteIndex] = input;

        // Calculate read position
        size_t readIndex = (mWriteIndex + mMaxDelaySamples - mCurrentDelaySamples) % mMaxDelaySamples;
        float output = mBuffer[readIndex];

        // Advance write position
        mWriteIndex = (mWriteIndex + 1) % mMaxDelaySamples;

        return output;
    }

    void resize(size_t newMaxDelay) {
        mMaxDelaySamples = newMaxDelay;
        mBuffer.resize(newMaxDelay, 0.0f);
        reset();
    }

private:
    std::vector<float> mBuffer;
    size_t mWriteIndex;
    size_t mMaxDelaySamples;
    size_t mCurrentDelaySamples;
};
```

### 5.4 Parameter Smoothing

```cpp
class ParameterSmoother {
public:
    ParameterSmoother(float smoothingTimeMs, double sampleRate)
        : mCurrentValue(0.0f)
        , mTargetValue(0.0f)
    {
        mSmoothingTimeSamples = static_cast<int>(smoothingTimeMs * sampleRate / 1000.0);
        // One-pole IIR coefficient: alpha = 1 - exp(-2π * fc * T)
        // Approximation for smoothing: alpha ≈ 1 / smoothingTimeSamples
        mSmoothingCoeff = 1.0f / static_cast<float>(mSmoothingTimeSamples);
    }

    void setTarget(float value) {
        mTargetValue = value;
    }

    float getNext() {
        // One-pole lowpass filter
        mCurrentValue += (mTargetValue - mCurrentValue) * mSmoothingCoeff;
        return mCurrentValue;
    }

    void reset(float value) {
        mCurrentValue = value;
        mTargetValue = value;
    }

    bool isSmoothing() const {
        return std::abs(mTargetValue - mCurrentValue) > 0.0001f;
    }

private:
    float mCurrentValue;
    float mTargetValue;
    float mSmoothingCoeff;
    int mSmoothingTimeSamples;
};
```

### 5.5 Audio Processing Flow

```cpp
tresult SimplePannerProcessor::process(Vst::ProcessData& data) {
    // 1. Update parameters from input parameter changes
    updateParameters(data);

    // 2. Process audio if present
    if (data.numInputs == 0 || data.numOutputs == 0)
        return kResultOk;

    processAudio(data);

    return kResultOk;
}

void SimplePannerProcessor::updateParameters(Vst::ProcessData& data) {
    if (!data.inputParameterChanges)
        return;

    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 i = 0; i < numParamsChanged; i++) {
        Vst::IParamValueQueue* queue = data.inputParameterChanges->getParameterData(i);
        if (!queue) continue;

        Vst::ParamValue value;
        int32 sampleOffset;
        int32 numPoints = queue->getPointCount();

        // Get last value in queue
        if (queue->getPoint(numPoints - 1, sampleOffset, value) != kResultTrue)
            continue;

        Vst::ParamID id = queue->getParameterId();

        switch (id) {
            case kParamLeftPan:
                mLeftPanSmoother.setTarget(normalizedToPan(value));
                break;
            case kParamLeftGain:
                mLeftGainSmoother.setTarget(dbToLinear(normalizedToDb(value)));
                break;
            case kParamLeftDelay:
                mDelayLeft.setDelay(delayMsToSamples(normalizedToDelayMs(value), mSampleRate));
                break;
            case kParamRightPan:
                mRightPanSmoother.setTarget(normalizedToPan(value));
                break;
            case kParamRightGain:
                mRightGainSmoother.setTarget(dbToLinear(normalizedToDb(value)));
                break;
            case kParamRightDelay:
                mDelayRight.setDelay(delayMsToSamples(normalizedToDelayMs(value), mSampleRate));
                break;
            case kParamMasterGain:
                mMasterGainSmoother.setTarget(dbToLinear(normalizedToDb(value)));
                break;
            case kParamLinkGain:
                mLinkGain = (value >= 0.5);
                break;
        }
    }
}

void SimplePannerProcessor::processAudio(Vst::ProcessData& data) {
    Vst::AudioBusBuffers& inputBus = data.inputs[0];
    Vst::AudioBusBuffers& outputBus = data.outputs[0];

    if (inputBus.numChannels < 2 || outputBus.numChannels < 2)
        return;

    float* inL = inputBus.channelBuffers32[0];
    float* inR = inputBus.channelBuffers32[1];
    float* outL = outputBus.channelBuffers32[0];
    float* outR = outputBus.channelBuffers32[1];

    for (int32 i = 0; i < data.numSamples; i++) {
        // Get smoothed parameter values
        float leftGain = mLeftGainSmoother.getNext();
        float rightGain = mRightGainSmoother.getNext();
        float masterGain = mMasterGainSmoother.getNext();
        float leftPan = mLeftPanSmoother.getNext();
        float rightPan = mRightPanSmoother.getNext();

        // Calculate pan gains
        PanGains leftPanGains = calculatePanGains(leftPan);
        PanGains rightPanGains = calculatePanGains(rightPan);

        // Apply delay
        float delayedLeft = mDelayLeft.process(inL[i]);
        float delayedRight = mDelayRight.process(inR[i]);

        // Apply individual channel gain
        float gainedLeft = delayedLeft * leftGain;
        float gainedRight = delayedRight * rightGain;

        // Apply panning
        float pannedLeft_toL = gainedLeft * leftPanGains.left;
        float pannedLeft_toR = gainedLeft * leftPanGains.right;
        float pannedRight_toL = gainedRight * rightPanGains.left;
        float pannedRight_toR = gainedRight * rightPanGains.right;

        // Mix
        float mixedL = pannedLeft_toL + pannedRight_toL;
        float mixedR = pannedLeft_toR + pannedRight_toR;

        // Apply master gain
        outL[i] = mixedL * masterGain;
        outR[i] = mixedR * masterGain;
    }
}
```

## 6. State Management

### 6.1 State Serialization

```cpp
tresult SimplePannerProcessor::setState(IBStream* state) {
    if (!state) return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    ProcessorState pState;

    // Read all parameters
    if (!streamer.readFloat(pState.leftPan)) return kResultFalse;
    if (!streamer.readFloat(pState.leftGain)) return kResultFalse;
    if (!streamer.readFloat(pState.leftDelay)) return kResultFalse;
    if (!streamer.readFloat(pState.rightPan)) return kResultFalse;
    if (!streamer.readFloat(pState.rightGain)) return kResultFalse;
    if (!streamer.readFloat(pState.rightDelay)) return kResultFalse;
    if (!streamer.readFloat(pState.masterGain)) return kResultFalse;

    int32 linkGainInt;
    if (!streamer.readInt32(linkGainInt)) return kResultFalse;
    pState.linkGain = (linkGainInt != 0);

    // Apply state
    applyState(pState);

    return kResultOk;
}

tresult SimplePannerProcessor::getState(IBStream* state) {
    if (!state) return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    // Write current parameter values
    streamer.writeFloat(getCurrentLeftPan());
    streamer.writeFloat(getCurrentLeftGain());
    streamer.writeFloat(getCurrentLeftDelay());
    streamer.writeFloat(getCurrentRightPan());
    streamer.writeFloat(getCurrentRightGain());
    streamer.writeFloat(getCurrentRightDelay());
    streamer.writeFloat(getCurrentMasterGain());
    streamer.writeInt32(mLinkGain ? 1 : 0);

    return kResultOk;
}
```

### 6.2 Controller State Sync

```cpp
tresult SimplePannerController::setComponentState(IBStream* state) {
    if (!state) return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    float leftPan, leftGain, leftDelay;
    float rightPan, rightGain, rightDelay;
    float masterGain;
    int32 linkGain;

    // Read state
    if (!streamer.readFloat(leftPan)) return kResultFalse;
    if (!streamer.readFloat(leftGain)) return kResultFalse;
    if (!streamer.readFloat(leftDelay)) return kResultFalse;
    if (!streamer.readFloat(rightPan)) return kResultFalse;
    if (!streamer.readFloat(rightGain)) return kResultFalse;
    if (!streamer.readFloat(rightDelay)) return kResultFalse;
    if (!streamer.readFloat(masterGain)) return kResultFalse;
    if (!streamer.readInt32(linkGain)) return kResultFalse;

    // Set normalized parameter values
    setParamNormalized(kParamLeftPan, panToNormalized(leftPan));
    setParamNormalized(kParamLeftGain, dbToNormalized(leftGain));
    setParamNormalized(kParamLeftDelay, delayMsToNormalized(leftDelay));
    setParamNormalized(kParamRightPan, panToNormalized(rightPan));
    setParamNormalized(kParamRightGain, dbToNormalized(rightGain));
    setParamNormalized(kParamRightDelay, delayMsToNormalized(rightDelay));
    setParamNormalized(kParamMasterGain, dbToNormalized(masterGain));
    setParamNormalized(kParamLinkGain, linkGain ? 1.0 : 0.0);

    return kResultOk;
}
```

## 7. Link L/R Gain Implementation

### 7.1 Controller-Side Implementation

```cpp
tresult SimplePannerController::setParamNormalized(Vst::ParamID id, Vst::ParamValue value) {
    tresult result = EditController::setParamNormalized(id, value);

    // If Link L/R Gain is enabled, synchronize gain parameters
    Vst::ParamValue linkValue = getParamNormalized(kParamLinkGain);
    bool isLinked = (linkValue >= 0.5);

    if (isLinked) {
        if (id == kParamLeftGain) {
            // Left gain changed, update right gain
            EditController::setParamNormalized(kParamRightGain, value);
        } else if (id == kParamRightGain) {
            // Right gain changed, update left gain
            EditController::setParamNormalized(kParamLeftGain, value);
        }
    }

    return result;
}
```

## 8. Thread Safety

### 8.1 Thread Boundaries

```
UI Thread                          Audio Thread
─────────────────────────────────────────────────────
Controller                         Processor
    │                                  │
    │ User changes parameter           │
    │ setParamNormalized()             │
    │                                  │
    │ performEdit()                    │
    ├──────────────────────────────────>
    │                                  │
    │                             Queue parameter
    │                                  │
    │                             process() called
    │                                  │
    │                             Read parameter queue
    │                                  │
    │                             Update smoothers
    │                                  │
    │                             Process audio
```

### 8.2 Synchronization Mechanisms

- **Parameter Changes**: VST3の `IParamValueQueue` を使用（ホストが提供）
- **Atomic Operations**: 不要（VST3フレームワークが保証）
- **Locks**: 不要（状態は各スレッドで独立）

### 8.3 Data Access Rules

| Data | Write Thread | Read Thread | Synchronization |
|------|--------------|-------------|-----------------|
| Parameter values (controller) | UI Thread | UI Thread | N/A (single thread) |
| Parameter queue | UI Thread | Audio Thread | VST3 queue |
| Smoother targets | Audio Thread | Audio Thread | N/A (single thread) |
| Delay buffers | Audio Thread | Audio Thread | N/A (single thread) |
| Audio state | Audio Thread | Audio Thread | N/A (single thread) |

## 9. Memory Management

### 9.1 Memory Allocation Strategy

| Component | Allocation Timing | Deallocation Timing |
|-----------|-------------------|---------------------|
| Delay buffers | `setActive(true)` or sample rate change | `setActive(false)` or destructor |
| Smoothers | Constructor | Destructor |
| Parameter objects | `initialize()` | `terminate()` |

### 9.2 Delay Buffer Sizing

```cpp
// Calculate maximum delay buffer size
size_t calculateMaxDelaySamples(double sampleRate) {
    // Max delay: 100ms
    // Add safety margin: 10%
    return static_cast<size_t>(std::ceil(0.1 * sampleRate * 1.1));
}

tresult SimplePannerProcessor::setActive(TBool state) {
    if (state) {
        // Allocate delay buffers
        size_t maxDelay = calculateMaxDelaySamples(mSampleRate);
        mDelayLeft.resize(maxDelay);
        mDelayRight.resize(maxDelay);
    } else {
        // Buffers will be cleared automatically
        mDelayLeft.reset();
        mDelayRight.reset();
    }

    return AudioEffect::setActive(state);
}
```

## 10. Error Handling

### 10.1 Error Categories

| Category | Handling Strategy |
|----------|-------------------|
| Invalid parameter values | Clamp to valid range |
| Null pointers | Return `kResultFalse` |
| Memory allocation failure | Return `kResultFalse`, cleanup resources |
| Invalid sample rate | Use default (44100 Hz), log warning |
| Buffer size mismatch | Skip processing, return `kResultOk` |

### 10.2 Validation Examples

```cpp
// Parameter validation
float clampPan(float pan) {
    return std::clamp(pan, ParamRange::kPanMin, ParamRange::kPanMax);
}

float clampGain(float gain) {
    return std::clamp(gain, ParamRange::kGainMin, ParamRange::kGainMax);
}

float clampDelay(float delay) {
    return std::clamp(delay, ParamRange::kDelayMin, ParamRange::kDelayMax);
}

// Sample rate validation
tresult SimplePannerProcessor::setupProcessing(Vst::ProcessSetup& setup) {
    if (setup.sampleRate <= 0.0) {
        // Invalid sample rate, use default
        mSampleRate = 44100.0;
        return kResultFalse;
    }

    mSampleRate = setup.sampleRate;

    // Resize delay buffers
    size_t maxDelay = calculateMaxDelaySamples(mSampleRate);
    mDelayLeft.resize(maxDelay);
    mDelayRight.resize(maxDelay);

    // Update smoothers
    mLeftGainSmoother = ParameterSmoother(10.0f, mSampleRate);
    // ... (initialize other smoothers)

    return AudioEffect::setupProcessing(setup);
}
```

## 11. Performance Considerations

### 11.1 Optimization Strategies

1. **Pre-calculate pan coefficients**: Update only when pan parameters change
2. **Avoid divisions**: Pre-calculate reciprocals where possible
3. **SIMD potential**: Consider vectorization for multi-channel processing
4. **Branch prediction**: Minimize conditionals in audio loop
5. **Cache locality**: Keep hot data structures compact

### 11.2 CPU Usage Estimates

| Component | Estimated CPU % (per instance, 44.1kHz, 512 samples) |
|-----------|------------------------------------------------------|
| Delay lines | < 0.1% |
| Gain calculations | < 0.05% |
| Pan calculations | < 0.1% |
| Smoothing | < 0.1% |
| Total | < 0.5% |

## 12. Testing Strategy

### 12.1 Unit Tests

- `DelayLine`: Verify exact delay amounts, boundary conditions
- `ParameterSmoother`: Verify smoothing behavior, convergence
- Parameter conversion functions: Verify round-trip conversions
- Pan calculation: Verify equal power law, boundary values

### 12.2 Integration Tests

- Default state bypass: Input = Output
- Extreme parameter values: No crashes, valid output
- State persistence: Save/load preserves all parameters
- Link L/R Gain: Synchronized behavior
- Sample rate changes: No crashes, correct delay times

### 12.3 Manual Tests

- Audio artifacts: Listen for clicks, pops, zipper noise
- DAW compatibility: Test in multiple DAWs
- Automation: Smooth parameter changes
- CPU usage: Monitor performance

## 13. Implementation Phases

### Phase 1: Core Infrastructure
- [ ] Project structure setup
- [ ] Parameter definitions
- [ ] Basic processor/controller skeleton
- [ ] State management (setState/getState)

### Phase 2: DSP Implementation
- [ ] DelayLine class
- [ ] ParameterSmoother class
- [ ] Parameter conversion utilities
- [ ] Pan calculation
- [ ] Basic audio processing loop

### Phase 3: Advanced Features
- [ ] Link L/R Gain implementation
- [ ] Master Gain integration
- [ ] Parameter smoothing integration
- [ ] Sample rate change handling

### Phase 4: Polish & Testing
- [✅] VST3 validator compliance
- [ ] Performance optimization
- [✅] Documentation
- [ ] DAW compatibility testing

### Phase 5: GUI Implementation
- [ ] SimplePannerEditor class skeleton
- [ ] VSTGUI integration and CFrame setup
- [ ] Pan sliders implementation (Left/Right)
- [ ] Gain knobs implementation (Left/Right/Master)
- [ ] Delay knobs implementation (Left/Right)
- [ ] Link L/R Gain toggle button
- [ ] Value display labels and formatting
- [ ] Color scheme and visual styling
- [ ] Controller integration (createView override)
- [ ] Parameter synchronization (GUI ↔ Controller)
- [ ] GUI testing in multiple DAWs

## 14. Open Questions / Design Decisions

1. **Smoothing time**: 10ms recommended, configurable?
2. **Delay buffer reallocation**: Clear or preserve content on sample rate change?
3. **Pan law alternative**: Allow linear pan as option?
4. **CPU optimization**: Implement SIMD? Profile first?
5. **Parameter precision**: Float vs double internally?

## 15. References

- VST3 SDK Documentation: https://steinbergmedia.github.io/vst3_doc/
- VST3 API: https://steinbergmedia.github.io/vst3_doc/vstsdk/
- Equal Power Panning: https://www.cs.cmu.edu/~music/icm-online/readings/panlaws/

---

**Document Version**: 1.1
**Last Updated**: 2025-11-13
**Status**: Core Complete, GUI Implementation Planned

**Revision History**:
- v1.0 (2025-11-13): Initial design document
- v1.1 (2025-11-13): Added Section 3.3 GUI Editor Design, Phase 5 GUI Implementation
