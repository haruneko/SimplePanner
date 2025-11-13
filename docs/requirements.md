# SimplePanner - Requirements Specification

## 1. Overview

SimplePannerは、ステレオ入力信号の各チャンネル（L/R）を独立して制御できるVST3オーディオプラグインです。パンニング、ゲイン調整、遅延を組み合わせることで、ステレオイメージの精密なコントロールとハース効果による定位調整を実現します。

## 2. Functional Requirements

### 2.1 Audio Processing

#### 2.1.1 Input/Output Configuration
- **Input**: Stereo (2 channels)
- **Output**: Stereo (2 channels)
- **Sample Rate**: DAWが提供するサンプルレートに対応（22.05kHz - 384kHz以上）
- **Bit Depth**: 32-bit floating point
- **Latency**: 遅延パラメータの最大値（100ms）に相当するレイテンシーを報告する

#### 2.1.2 Signal Flow
各入力チャンネル（L/R）は以下の処理を順次受ける：

1. **Delay**: 0-100msの遅延を適用
2. **Gain**: -60dB to +6dBのゲイン調整
3. **Pan**: -100 to +100の範囲で等パワーパンニング
4. **Master Gain**: 最終出力に対してマスターゲインを適用

```
Left Input  → [Delay] → [L Gain] → [Pan] → [Mix] → [Master Gain] → Left Output
Right Input → [Delay] → [R Gain] → [Pan] → [Mix] → [Master Gain] → Right Output
```

### 2.2 Parameters

#### 2.2.1 Parameter List

| ID | Name | Type | Range | Default | Unit | UI Control |
|----|------|------|-------|---------|------|------------|
| 0 | Left Pan | Float | -100 to +100 | -100 (Full Left) | - | Slider |
| 1 | Left Gain | Float | -60 to +6 | 0 | dB | Knob |
| 2 | Left Delay | Float | 0 to 100 | 0 | ms | Knob |
| 3 | Right Pan | Float | -100 to +100 | +100 (Full Right) | - | Slider |
| 4 | Right Gain | Float | -60 to +6 | 0 | dB | Knob |
| 5 | Right Delay | Float | 0 to 100 | 0 | ms | Knob |
| 6 | Master Gain | Float | -60 to +6 | 0 | dB | Knob |
| 7 | Link L/R Gain | Boolean | Off/On | Off | - | Toggle |

#### 2.2.2 Parameter Specifications

##### Pan Parameters (ID: 0, 3)
- **Range**: -100 (Full Left) to +100 (Full Right)
- **Resolution**: 0.01 steps (10,000 discrete values)
- **Display Format**:
  - `-100` displays as "L100"
  - `-50` displays as "L50"
  - `0` displays as "C" or "Center"
  - `+50` displays as "R50"
  - `+100` displays as "R100"
- **Pan Law**: Equal Power Panning (-3dB center)
  - Left Gain: `cos(pan * π/4)` where pan is normalized to -1.0 to +1.0
  - Right Gain: `sin(pan * π/4)` where pan is normalized to -1.0 to +1.0
- **Automation**: Supported

##### Gain Parameters (ID: 1, 4, 6)
- **Range**: -60dB to +6dB
- **Resolution**: 0.1dB steps
- **Special Value**: -60dB is treated as -∞ (complete mute, gain = 0.0)
- **Display Format**:
  - `-60.0 dB` displays as "-∞ dB"
  - `-6.0 dB` displays as "-6.0 dB"
  - `0.0 dB` displays as "0.0 dB"
  - `+6.0 dB` displays as "+6.0 dB"
- **Conversion**: dB to linear: `gain = pow(10, dB/20)`
- **Automation**: Supported

##### Delay Parameters (ID: 2, 5)
- **Range**: 0ms to 100ms
- **Resolution**: 0.1ms display resolution
- **Implementation**: Sample-accurate delay
  - Internal conversion: `samples = round(ms * sampleRate / 1000)`
  - Example at 44.1kHz: 10ms = 441 samples, 5ms = 221 samples
- **Interpolation**: None (sample-accurate only)
- **Buffer**: Maximum delay buffer size = `ceiling(0.1 * sampleRate)` samples
- **Display Format**: `10.0 ms`, `5.0 ms`, etc.
- **Automation**: Supported

##### Link L/R Gain (ID: 7)
- **Type**: Boolean toggle
- **Values**:
  - `0` = Off (L and R gains are independent)
  - `1` = On (L and R gains are linked)
- **Behavior**:
  - When enabled: Changing either Left Gain or Right Gain will set both to the same value
  - When disabled: Left Gain and Right Gain operate independently
  - Master Gain is always independent
- **Display**: Toggle switch or checkbox
- **Automation**: Supported

#### 2.2.3 Parameter Interaction

##### Master Gain Application
Master Gain is applied **after** individual channel processing:
```
Final_Left_Output = (Left_Processed_Signal) * MasterGain_Linear
Final_Right_Output = (Right_Processed_Signal) * MasterGain_Linear
```

Where:
- `MasterGain_Linear = pow(10, MasterGain_dB / 20)`
- Special case: If MasterGain_dB = -60, then MasterGain_Linear = 0.0

##### Link L/R Gain Behavior
When Link L/R Gain is **enabled**:
1. User adjusts Left Gain → Right Gain is set to the same value
2. User adjusts Right Gain → Left Gain is set to the same value
3. Both parameters always maintain the same value
4. Master Gain remains independent

When Link L/R Gain is **disabled**:
1. Left Gain and Right Gain can be set to different values
2. Each parameter operates independently

### 2.3 Audio Processing Details

#### 2.3.1 Delay Implementation
```cpp
// Pseudo-code
delaySamples_L = round(leftDelayMs * sampleRate / 1000.0)
delaySamples_R = round(rightDelayMs * sampleRate / 1000.0)

// Circular buffer implementation
delayBuffer[writePos] = inputSample
delayedSample = delayBuffer[(writePos - delaySamples + bufferSize) % bufferSize]
writePos = (writePos + 1) % bufferSize
```

#### 2.3.2 Gain Application
```cpp
// Pseudo-code
linearGain = (gainDb == -60.0) ? 0.0 : pow(10.0, gainDb / 20.0)
outputSample = inputSample * linearGain
```

#### 2.3.3 Pan Processing (Equal Power)
```cpp
// Pseudo-code
// Normalize pan from -100..+100 to -1..+1
normalizedPan = panValue / 100.0

// Equal power pan law
panAngle = normalizedPan * M_PI / 4.0  // -π/4 to +π/4 radians
leftGain = cos(panAngle)   // 1.0 at full left, 0.707 at center, 0.0 at full right
rightGain = sin(panAngle)  // 0.0 at full left, 0.707 at center, 1.0 at full right
```

#### 2.3.4 Complete Signal Path
```cpp
// Pseudo-code for one sample
// Left Channel
delayedLeft = delayBuffer_L.read(leftDelaySamples)
gainedLeft = delayedLeft * leftGainLinear
pannedLeft_toLeft = gainedLeft * cos(leftPanAngle)
pannedLeft_toRight = gainedLeft * sin(leftPanAngle)

// Right Channel
delayedRight = delayBuffer_R.read(rightDelaySamples)
gainedRight = delayedRight * rightGainLinear
pannedRight_toLeft = gainedRight * cos(rightPanAngle)
pannedRight_toRight = gainedRight * sin(rightPanAngle)

// Mix
mixedLeft = pannedLeft_toLeft + pannedRight_toLeft
mixedRight = pannedLeft_toRight + pannedRight_toRight

// Master Gain
outputLeft = mixedLeft * masterGainLinear
outputRight = mixedRight * masterGainLinear
```

### 2.4 Default State (Bypass Equivalent)

プラグインを読み込んだ直後の状態は、入力信号をそのまま出力する「透過状態」である必要があります：

| Parameter | Default Value | Note |
|-----------|---------------|------|
| Left Pan | -100 (Full Left) | 左入力は左出力へ |
| Left Gain | 0 dB | ゲイン変更なし |
| Left Delay | 0 ms | 遅延なし |
| Right Pan | +100 (Full Right) | 右入力は右出力へ |
| Right Gain | 0 dB | ゲイン変更なし |
| Right Delay | 0 ms | 遅延なし |
| Master Gain | 0 dB | ゲイン変更なし |
| Link L/R Gain | Off | 独立動作 |

この状態では：
```
Left Input → Left Output (no change)
Right Input → Right Output (no change)
```

## 3. Non-Functional Requirements

### 3.1 Performance
- **CPU Usage**: シングルスレッドで効率的に動作すること
- **Memory**: 最大遅延バッファサイズは `2 * ceiling(0.1 * maxSampleRate)` samples
  - 例：384kHz時、約77KBのメモリ（32-bit float）
- **Latency**: プラグインレイテンシーは遅延パラメータの最大値を報告
  - `reportedLatency = max(leftDelaySamples, rightDelaySamples)`

### 3.2 Compatibility
- **VST3 Standard**: VST 3.7.0以降に準拠
- **Platforms**:
  - Windows (x64)
  - macOS (x64, ARM64)
  - Linux (x64)
- **DAW Compatibility**: VST3をサポートする全てのDAW

### 3.3 User Interface

#### 3.3.1 GUI Framework
- **Technology**: VSTGUI 4.x (VST3 SDK付属)
- **Window Size**: 600px (width) × 400px (height)
- **Resizable**: No (fixed size)
- **Platform**: Native window on Windows/macOS/Linux

#### 3.3.2 Control Types

##### Pan Sliders (Left Pan, Right Pan)
- **Type**: Horizontal slider
- **Visual Style**: Linear slider with center detent
- **Dimensions**: 200px (width) × 30px (height)
- **Range Display**:
  - Left end: "L" label
  - Right end: "R" label
  - Center position: Visual indicator (detent)
- **Value Display**: Above slider (e.g., "L50", "C", "R75")
- **Interaction**:
  - Click + drag: Adjust value
  - Double-click: Reset to default
  - Shift + drag: Fine adjustment (10x slower)

##### Gain Knobs (Left Gain, Right Gain, Master Gain)
- **Type**: Rotary knob
- **Visual Style**: Circular knob with indicator line
- **Dimensions**: 60px diameter
- **Range**: -60dB to +6dB (270° rotation range)
- **Value Display**: Below knob (e.g., "-3.0 dB", "-∞ dB")
- **Interaction**:
  - Click + drag up/down: Adjust value
  - Double-click: Reset to 0dB
  - Shift + drag: Fine adjustment (10x slower)

##### Delay Knobs (Left Delay, Right Delay)
- **Type**: Rotary knob
- **Visual Style**: Circular knob with indicator line
- **Dimensions**: 60px diameter
- **Range**: 0ms to 100ms (270° rotation range)
- **Value Display**: Below knob (e.g., "10.0 ms")
- **Interaction**:
  - Click + drag up/down: Adjust value
  - Double-click: Reset to 0ms
  - Shift + drag: Fine adjustment (10x slower)

##### Link L/R Gain Toggle
- **Type**: Toggle button
- **Visual Style**: Button with LED indicator
- **Dimensions**: 80px (width) × 25px (height)
- **States**:
  - OFF: Gray background, text "UNLINKED"
  - ON: Green/highlighted background, text "LINKED"
- **Interaction**: Click to toggle

#### 3.3.3 Layout and Grouping

```
┌─────────────────────────────────────────────────────────────┐
│  SimplePanner                                               │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─── LEFT CHANNEL ────────────┐  ┌─── RIGHT CHANNEL ────┐ │
│  │                             │  │                       │ │
│  │  Pan:  [════●════] L50      │  │  Pan:  [════●════] R75│ │
│  │                             │  │                       │ │
│  │   Gain      Delay           │  │   Gain      Delay     │ │
│  │   ╔═╗       ╔═╗             │  │   ╔═╗       ╔═╗       │ │
│  │   ║ ║       ║ ║             │  │   ║ ║       ║ ║       │ │
│  │   ╚═╝       ╚═╝             │  │   ╚═╝       ╚═╝       │ │
│  │  -3.0dB    10.0ms           │  │  0.0dB     0.0ms      │ │
│  │                             │  │                       │ │
│  └─────────────────────────────┘  └───────────────────────┘ │
│                                                             │
│  ┌─── MASTER ──────────────────────────────────────────────┐│
│  │                                                         ││
│  │   Master Gain       [ UNLINKED ]                       ││
│  │      ╔═╗           Link L/R Gain                       ││
│  │      ║ ║                                               ││
│  │      ╚═╝                                               ││
│  │     0.0dB                                              ││
│  │                                                         ││
│  └─────────────────────────────────────────────────────────┘│
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### 3.3.4 Visual Design

##### Color Scheme
- **Background**: Dark gray (#2C2C2C)
- **Group Boxes**: Lighter gray (#3C3C3C) with subtle border
- **Text Labels**: Light gray (#CCCCCC)
- **Value Text**: White (#FFFFFF)
- **Knob Body**: Medium gray (#505050)
- **Knob Indicator**: Accent color (e.g., #4A90E2 blue)
- **Slider Track**: Dark gray (#404040)
- **Slider Handle**: Accent color (#4A90E2)
- **Toggle ON**: Green (#4CAF50) or accent color
- **Toggle OFF**: Gray (#606060)

##### Typography
- **Labels**: Sans-serif font (e.g., Arial, Helvetica)
  - Size: 12pt
  - Weight: Normal
- **Values**: Sans-serif font
  - Size: 11pt
  - Weight: Bold
- **Group Titles**: Sans-serif font
  - Size: 14pt
  - Weight: Bold

##### Spacing and Alignment
- **Margin**: 20px from window edges
- **Group Box Padding**: 15px internal padding
- **Control Spacing**: 20px between controls
- **Vertical Alignment**: Controls centered within group boxes

#### 3.3.5 Interaction and Feedback

##### Visual Feedback
- **Mouse Hover**: Control brightens slightly (10% lighter)
- **Active Drag**: Control highlights with accent color
- **Parameter Change**: Value text updates in real-time
- **Link Toggle**: Immediate visual state change + synchronized value update

##### Value Display Format
- **Pan Parameters**:
  - `-100` → "L100"
  - `-50` → "L50"
  - `0` → "C" or "CENTER"
  - `+50` → "R50"
  - `+100` → "R100"
- **Gain Parameters**:
  - `-60.0 dB` → "-∞ dB"
  - Other values → "±X.X dB" (1 decimal place)
- **Delay Parameters**:
  - "X.X ms" (1 decimal place)

##### Accessibility
- **Keyboard Support**:
  - Tab: Navigate between controls
  - Arrow keys: Adjust focused control
  - Enter/Space: Activate toggle button
- **Screen Reader**: Parameter names and values announced (if supported by VSTGUI)

#### 3.3.6 DAW Integration
- **Generic UI Fallback**: If custom GUI fails to load, DAW provides generic parameter list
- **Parameter Automation**: All controls respond to DAW automation in real-time
- **State Recall**: GUI reflects saved parameter values on project load

### 3.4 State Management
- **Parameter Persistence**: 全パラメータ値はプロジェクトと共に保存される
- **Automation**: 全パラメータがDAWオートメーションに対応
- **Preset System**: VST3標準のプリセット機能は使用しない（要件外）
- **Bypass**: VST3標準のバイパス機能は実装しない（DAW側で対応）

## 4. Out of Scope

以下の機能は実装対象外：

- ✗ カスタムプリセットシステム
- ✗ プラグイン内蔵のバイパス機能
- ✗ Mid/Side処理モード
- ✗ ステレオ幅コントロール
- ✗ フェーズ反転機能
- ✗ サブサンプル精度の遅延（補間）
- ✗ GUI スキン/テーマ変更機能
- ✗ メーター表示（レベルメーター、ゲインリダクションメーターなど）
- ✗ アナライザー（スペクトラム、位相など）

## 5. Testing Requirements

### 5.1 Functional Tests
- デフォルト状態でバイパス等価（入力=出力）であることを検証
- 各パラメータが指定範囲内で正しく動作することを検証
- Pan法則が等パワー（-3dB center）であることを検証
- Delay時間がサンプル精度で正確であることを検証
- Link L/R Gainが正しく動作することを検証
- Master Gainが全体に正しく適用されることを検証
- -60dBが完全ミュート（-∞）として動作することを検証

### 5.2 VST3 Validator
- VST3 SDK付属のvalidatorで全テストをパスすること

### 5.3 Performance Tests
- 複数インスタンスを同時実行してもCPU使用率が許容範囲内であること
- サンプルレート変更時にクラッシュしないこと
- パラメータ変更時にオーディオグリッチが発生しないこと

## 6. Implementation Notes

### 6.1 Parameter Smoothing
急激なパラメータ変更によるクリックノイズを防ぐため、以下のパラメータにはスムージングを適用することを推奨：
- Gain parameters (Left Gain, Right Gain, Master Gain)
- Pan parameters (Left Pan, Right Pan)

Delay parametersは物理的に瞬時変更不可能なため、スムージング不要（ただし、変更時のクリック対策は別途検討）。

### 6.2 Thread Safety
VST3の仕様に従い、以下の分離を保つ：
- UI thread: パラメータ変更、GUI更新
- Audio thread: オーディオ処理
- パラメータ変更はキューを介してオーディオスレッドに伝達

### 6.3 Sample Rate Changes
- サンプルレート変更時、遅延バッファをリアロケート
- 遅延時間（ms）は保持し、サンプル数を再計算
- バッファの内容はクリア（無音で初期化）

## 7. Acceptance Criteria

以下の全てを満たすこと：

### 7.1 Core Functionality
- [ ] VST3 validatorの全テストをパス
- [ ] デフォルト状態で入力信号が変更されずに出力される
- [ ] 全パラメータが仕様通りの範囲と挙動を示す
- [ ] パラメータオートメーションが正常に動作

### 7.2 GUI Requirements
- [ ] カスタムGUIが600×400pxで正しく表示される
- [ ] 全てのコントロール（スライダー、ノブ、トグル）が正常に動作する
- [ ] パラメータ値がリアルタイムで正しく表示される
- [ ] Panは "L100"/"C"/"R100" 形式、Gainは "-∞ dB"/"0.0 dB" 形式で表示される
- [ ] Link L/R Gainトグルが正しく連動する
- [ ] ダブルクリックでデフォルト値にリセットできる
- [ ] Shift+ドラッグで微調整できる
- [ ] GUIが開閉を繰り返しても正常に動作する

### 7.3 Platform and DAW Compatibility
- [ ] 主要DAW（Reaper, Ableton Live, FL Studio, Cubase等）で動作確認
- [ ] マルチプラットフォーム（Windows, macOS, Linux）でビルド・動作
- [ ] GUIが各プラットフォームで正しく描画される

### 7.4 Quality and Documentation
- [ ] ドキュメント（README, この仕様書）が整備されている
- [ ] クリティカルなメモリリークやクラッシュが存在しない

## 8. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-11-13 | - | Initial requirements specification |
| 1.1 | 2025-11-13 | - | Expanded Section 3.3 with detailed GUI specifications (framework, control types, layout, visual design, interactions) |
