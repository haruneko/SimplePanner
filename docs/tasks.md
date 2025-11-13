# SimplePanner - Implementation Tasks

## Overview

本ドキュメントは SimplePanner の実装タスクを定義します。各タスクはテスト駆動開発（TDD）のアプローチに従い、**テスト作成 → 実装 → テスト実行**の順序で進めます。

## Task Status Legend

- [ ] TODO: 未着手
- [🔄] IN PROGRESS: 作業中
- [✅] DONE: 完了
- [⏸️] BLOCKED: ブロック中（依存タスクが未完了）

## Implementation Phases

### Phase 0: Setup and Preparation

#### Task 0.1: Project Cleanup and Preparation
**Priority**: P0 (Critical)
**Estimated Time**: 30 minutes

**Subtasks**:
- [ ] 0.1.1 既存のプレースホルダーコード（pluginprocessor.cpp等）をバックアップ
- [ ] 0.1.2 include/plugids.h を requirements に従って更新
  - 新しいパラメータID（8個）を定義
  - プラグインUID（ProcessorUID, ControllerUID）を生成
  - デフォルト値、範囲の定数を定義
- [ ] 0.1.3 テスト用ディレクトリ構造を作成
  ```
  tests/
  ├── unit/           # ユニットテスト
  ├── integration/    # 統合テスト
  └── manual/         # マニュアルテスト手順
  ```

**Completion Criteria**:
- plugids.h が requirements.md のパラメータ定義と一致している
- テストディレクトリが作成されている
- プロジェクトがビルドエラーなくコンパイルできる

---

### Phase 1: Core Infrastructure

#### Task 1.1: Parameter Conversion Utilities
**Priority**: P0 (Critical)
**Estimated Time**: 2 hours
**Dependencies**: Task 0.1

**Test Tasks**:
- [ ] 1.1.1 **TEST**: `tests/unit/test_parameter_conversion.cpp` を作成
  - Pan変換のテスト（normalized ↔ plain）
    - 境界値: 0.0 → -100, 1.0 → +100
    - 中間値: 0.5 → 0
    - ラウンドトリップ: plain → normalized → plain
  - Gain変換のテスト（normalized ↔ dB ↔ linear）
    - -60dB → 0.0 (mute)
    - 0dB → 1.0 (unity gain)
    - +6dB → 約1.995
    - ラウンドトリップ
  - Delay変換のテスト（normalized ↔ ms ↔ samples）
    - 0ms @ 44.1kHz → 0 samples
    - 10ms @ 44.1kHz → 441 samples
    - 100ms @ 48kHz → 4800 samples
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 1.1.2 **IMPL**: `include/parameter_utils.h` を作成
  - `normalizedToPan()`, `panToNormalized()`
  - `normalizedToDb()`, `dbToNormalized()`, `dbToLinear()`, `linearToDb()`
  - `normalizedToDelayMs()`, `delayMsToNormalized()`, `delayMsToSamples()`, `delaySamplesToMs()`
  - 境界値チェックとクランプ処理を含める

**Verification Tasks**:
- [ ] 1.1.3 **VERIFY**: ユニットテストを実行
  ```bash
  cd build && ctest -R test_parameter_conversion
  ```
- [ ] 1.1.4 **VERIFY**: すべてのテストケースがパス

**Completion Criteria**:
- すべての変換関数が実装されている
- ユニットテストが100%パス
- 境界値で未定義動作が発生しない

---

#### Task 1.2: DelayLine Class
**Priority**: P0 (Critical)
**Estimated Time**: 3 hours
**Dependencies**: None

**Test Tasks**:
- [ ] 1.2.1 **TEST**: `tests/unit/test_delay_line.cpp` を作成
  - 基本動作テスト
    - 遅延なし（0サンプル）: 入力 = 出力
    - 1サンプル遅延: 正しく1サンプル遅れる
    - 最大遅延: バッファサイズまで遅延可能
  - 境界条件テスト
    - 空バッファ: 0が出力される
    - バッファ満杯後: 正しく上書きされる
    - 遅延量変更: 即座に反映される
  - サーキュラーバッファテスト
    - ラップアラウンド: インデックスが正しく循環する
  - リセットテスト
    - reset()後: すべてゼロになる
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 1.2.2 **IMPL**: `include/delay_line.h` を作成
  - クラス定義: `DelayLine`
  - メンバー変数: `std::vector<float> mBuffer`, `size_t mWriteIndex`, など
  - メソッド: `process()`, `reset()`, `setDelay()`, `resize()`
- [ ] 1.2.3 **IMPL**: `source/delay_line.cpp` を作成
  - サーキュラーバッファロジック実装
  - 境界チェック

**Verification Tasks**:
- [ ] 1.2.4 **VERIFY**: ユニットテストを実行
  ```bash
  cd build && ctest -R test_delay_line
  ```
- [ ] 1.2.5 **VERIFY**: すべてのテストケースがパス
- [ ] 1.2.6 **VERIFY**: メモリリークチェック（valgrind等）

**Completion Criteria**:
- DelayLine クラスが完全に実装されている
- ユニットテストが100%パス
- メモリリークが存在しない

---

#### Task 1.3: ParameterSmoother Class
**Priority**: P0 (Critical)
**Estimated Time**: 2 hours
**Dependencies**: None

**Test Tasks**:
- [ ] 1.3.1 **TEST**: `tests/unit/test_parameter_smoother.cpp` を作成
  - 基本動作テスト
    - 目標値設定: `setTarget()` 後、`getNext()` で徐々に近づく
    - 収束テスト: 十分な時間後、目標値に収束する
  - リセットテスト
    - `reset()`: 即座に値がセットされる
  - スムージング時間テスト
    - 10ms設定: 約10ms（サンプル数）で収束
  - 境界条件テスト
    - 即座に同じ値を設定: スムージング不要
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 1.3.2 **IMPL**: `include/parameter_smoother.h` を作成
  - クラス定義: `ParameterSmoother`
  - One-pole IIR フィルタ実装
  - メソッド: `setTarget()`, `getNext()`, `reset()`, `isSmoothing()`
- [ ] 1.3.3 **IMPL**: `source/parameter_smoother.cpp` を作成
  - スムージング係数の計算
  - 収束判定ロジック

**Verification Tasks**:
- [ ] 1.3.4 **VERIFY**: ユニットテストを実行
  ```bash
  cd build && ctest -R test_parameter_smoother
  ```
- [ ] 1.3.5 **VERIFY**: すべてのテストケースがパス
- [ ] 1.3.6 **VERIFY**: 収束時間が仕様通り（10ms程度）

**Completion Criteria**:
- ParameterSmoother クラスが完全に実装されている
- ユニットテストが100%パス
- スムージング動作が期待通り

---

#### Task 1.4: Processor Skeleton with State Management
**Priority**: P0 (Critical)
**Estimated Time**: 4 hours
**Dependencies**: Task 1.1

**Test Tasks**:
- [ ] 1.4.1 **TEST**: `tests/integration/test_processor_state.cpp` を作成
  - 初期化テスト
    - `initialize()` → `kResultOk`
    - `terminate()` → `kResultOk`
  - 状態管理テスト
    - `setState()` / `getState()` ラウンドトリップ
    - デフォルト状態の保存・復元
    - 任意のパラメータ値の保存・復元
  - アクティブ化テスト
    - `setActive(true)` → バッファ確保
    - `setActive(false)` → リソース解放
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 1.4.2 **IMPL**: `include/pluginprocessor.h` を更新
  - メンバー変数を追加
    - `DelayLine mDelayLeft, mDelayRight`
    - `ParameterSmoother` x5 (L/R gain, master gain, L/R pan)
    - `bool mLinkGain`
    - `double mSampleRate`
    - パラメータ値保持用変数
  - メソッドを宣言
    - `setState()`, `getState()`
    - `initialize()`, `terminate()`, `setActive()`
    - `setupProcessing()`
- [ ] 1.4.3 **IMPL**: `source/pluginprocessor.cpp` を更新
  - コンストラクタ: デフォルト値で初期化
  - `initialize()`: 入出力バス設定
  - `setState()` / `getState()`: シリアライゼーション実装
  - `setActive()`: DelayLine の確保/解放
  - `setupProcessing()`: サンプルレート保存、バッファリサイズ

**Verification Tasks**:
- [ ] 1.4.4 **VERIFY**: 統合テストを実行
  ```bash
  cd build && ctest -R test_processor_state
  ```
- [ ] 1.4.5 **VERIFY**: VST3 Validator 実行（基本チェック）
  ```bash
  cd build && bin/Release/validator VST3/Release/SimplePanner.vst3
  ```
- [ ] 1.4.6 **VERIFY**: Validator の "Scan Parameters", "Valid State Transition" がパス

**Completion Criteria**:
- Processor の基本構造が実装されている
- 状態管理が正しく動作する
- VST3 Validator の初期テストがパス

---

#### Task 1.5: Controller with Parameter Definitions
**Priority**: P0 (Critical)
**Estimated Time**: 3 hours
**Dependencies**: Task 1.1, 1.4

**Test Tasks**:
- [ ] 1.5.1 **TEST**: `tests/integration/test_controller_parameters.cpp` を作成
  - パラメータ登録テスト
    - 8つのパラメータが正しく登録されている
    - 各パラメータのデフォルト値が正しい
  - パラメータ文字列変換テスト
    - `getParamStringByValue()`: 値 → 文字列（例: 0.5 → "C", -60dB → "-∞ dB"）
    - `getParamValueByString()`: 文字列 → 値
  - 状態同期テスト
    - `setComponentState()`: Processor状態をControllerに反映
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 1.5.2 **IMPL**: `include/plugincontroller.h` を更新
  - メソッド宣言
    - `initialize()`, `terminate()`
    - `setComponentState()`
    - `getParamStringByValue()`, `getParamValueByString()`
- [ ] 1.5.3 **IMPL**: `source/plugincontroller.cpp` を更新
  - `initialize()`: 8つのパラメータを登録
    - Pan parameters: -100 to +100, スライダー
    - Gain parameters: -60dB to +6dB, ノブ, "-∞ dB" 表記
    - Delay parameters: 0ms to 100ms, ノブ
    - Link Gain: On/Off, トグル
  - `setComponentState()`: Processor状態からパラメータ値を復元
  - 文字列変換実装: Pan("L100", "C", "R50"), Gain("-∞ dB", "-3.0 dB"), Delay("10.0 ms")

**Verification Tasks**:
- [ ] 1.5.4 **VERIFY**: 統合テストを実行
  ```bash
  cd build && ctest -R test_controller_parameters
  ```
- [ ] 1.5.5 **VERIFY**: VST3 Validator 実行
  ```bash
  cd build && bin/Release/validator VST3/Release/SimplePanner.vst3
  ```
- [ ] 1.5.6 **VERIFY**: Validator の "Scan Parameters", "Scan Editor Classes" がパス

**Completion Criteria**:
- Controller の基本構造が実装されている
- 8つのパラメータがすべて登録されている
- 文字列変換が正しく動作する
- VST3 Validator のパラメータテストがパス

---

### Phase 2: DSP Implementation

#### Task 2.1: Pan Calculation Implementation
**Priority**: P0 (Critical)
**Estimated Time**: 2 hours
**Dependencies**: Task 1.1

**Test Tasks**:
- [ ] 2.1.1 **TEST**: `tests/unit/test_pan_calculation.cpp` を作成
  - 等パワーパンニングテスト
    - Pan = -100: left=1.0, right=0.0
    - Pan = 0: left≈0.707, right≈0.707
    - Pan = +100: left=0.0, right=1.0
  - パワー保存テスト
    - すべてのPan値で: left² + right² ≈ 1.0
  - 中間値テスト
    - Pan = -50, +50 で適切な値
  - **Expected Result**: すべてのテストがパス、パワーが保存されている

**Implementation Tasks**:
- [ ] 2.1.2 **IMPL**: `include/pan_calculator.h` を作成
  - 構造体 `PanGains { float left; float right; }`
  - 関数 `PanGains calculatePanGains(float pan)`
  - 等パワーパンニング: cos/sin法
- [ ] 2.1.3 **IMPL**: `source/pan_calculator.cpp` を作成
  - cos/sin 実装
  - 角度計算: `pan/100 * π/4`

**Verification Tasks**:
- [ ] 2.1.4 **VERIFY**: ユニットテストを実行
  ```bash
  cd build && ctest -R test_pan_calculation
  ```
- [ ] 2.1.5 **VERIFY**: すべてのテストケースがパス
- [ ] 2.1.6 **VERIFY**: パワー保存の誤差が1%以内

**Completion Criteria**:
- Pan計算関数が実装されている
- ユニットテストが100%パス
- 等パワー法則が正しく実装されている

---

#### Task 2.2: Basic Audio Processing Loop (No Smoothing)
**Priority**: P0 (Critical)
**Estimated Time**: 4 hours
**Dependencies**: Task 1.2, 1.4, 2.1

**Test Tasks**:
- [ ] 2.2.1 **TEST**: `tests/integration/test_audio_processing_basic.cpp` を作成
  - バイパステスト（デフォルト状態）
    - Left In → Left Out (no change)
    - Right In → Right Out (no change)
  - 遅延テスト
    - 10ms遅延設定 → 正確に10ms遅れる
  - ゲインテスト
    - 0dB: unity gain
    - -6dB: 約0.5倍
    - -60dB: 完全ミュート
  - パンテスト
    - Pan=-100: 左出力のみ
    - Pan=+100: 右出力のみ
    - Pan=0: 両方に等パワー分配
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 2.2.2 **IMPL**: `source/pluginprocessor.cpp` の `process()` を実装
  - `updateParameters()`: パラメータキューから値を読み取る
    - スムージング無し（直接値を使用）
    - 遅延サンプル数を計算
  - `processAudio()`: 基本的なDSPループ
    - Delay → Gain → Pan → Mix → Master Gain
    - サンプルごとの処理
- [ ] 2.2.3 **IMPL**: ステレオ処理の実装
  - 入力チャンネル数チェック
  - 出力バッファへの書き込み

**Verification Tasks**:
- [ ] 2.2.4 **VERIFY**: 統合テストを実行
  ```bash
  cd build && ctest -R test_audio_processing_basic
  ```
- [ ] 2.2.5 **VERIFY**: VST3 Validator 実行
  ```bash
  cd build && bin/Release/validator VST3/Release/SimplePanner.vst3
  ```
- [ ] 2.2.6 **VERIFY**: Validator の "Process Test", "Silence Processing" がパス
- [ ] 2.2.7 **VERIFY**: Manual audio test
  - テスト用オーディオファイル（ピンクノイズ）を用意
  - DAW（Reaper等）でプラグインを読み込む
  - デフォルト状態で音が変わらないことを確認
  - Pan, Gain, Delay を動かして音が変化することを確認

**Completion Criteria**:
- 基本的なオーディオ処理が動作する
- デフォルト状態でバイパス等価
- VST3 Validator の処理テストがパス
- 実際のオーディオで動作確認済み

---

#### Task 2.3: Parameter Smoothing Integration
**Priority**: P1 (High)
**Estimated Time**: 3 hours
**Dependencies**: Task 1.3, 2.2

**Test Tasks**:
- [ ] 2.3.1 **TEST**: `tests/integration/test_parameter_smoothing.cpp` を作成
  - スムージング動作テスト
    - 急激なゲイン変更: クリックノイズが発生しない
    - 急激なPan変更: クリックノイズが発生しない
  - 収束テスト
    - パラメータ変更後、約10msで目標値に到達
  - 複数パラメータ同時変更テスト
    - すべてのパラメータが独立してスムージングされる
  - **Expected Result**: すべてのテストがパス、クリックノイズなし

**Implementation Tasks**:
- [ ] 2.3.2 **IMPL**: `source/pluginprocessor.cpp` の `updateParameters()` を更新
  - ParameterSmoother を使用
  - 各パラメータの目標値を smoother にセット
- [ ] 2.3.3 **IMPL**: `source/pluginprocessor.cpp` の `processAudio()` を更新
  - smoother から値を取得: `getNext()`
  - サンプルごとにスムージングされた値を使用

**Verification Tasks**:
- [ ] 2.3.4 **VERIFY**: 統合テストを実行
  ```bash
  cd build && ctest -R test_parameter_smoothing
  ```
- [ ] 2.3.5 **VERIFY**: VST3 Validator 実行
  ```bash
  cd build && bin/Release/validator VST3/Release/SimplePanner.vst3
  ```
- [ ] 2.3.6 **VERIFY**: Manual audio test
  - DAWでプラグインを読み込む
  - ゲインを急激に変更（0dB → -20dB → 0dB）
  - クリックノイズが聞こえないことを確認
  - Panを急激に変更（L100 → R100 → L100）
  - クリックノイズが聞こえないことを確認

**Completion Criteria**:
- パラメータスムージングが統合されている
- 急激なパラメータ変更でクリックノイズが発生しない
- VST3 Validator の処理テストがパス
- 実際のオーディオでスムージング動作確認済み

---

### Phase 3: Advanced Features

#### Task 3.1: Link L/R Gain Implementation
**Priority**: P1 (High)
**Estimated Time**: 2 hours
**Dependencies**: Task 1.5, 2.2

**Test Tasks**:
- [ ] 3.1.1 **TEST**: `tests/integration/test_link_gain.cpp` を作成
  - Link無効時
    - Left Gain変更 → Right Gainは変わらない
    - Right Gain変更 → Left Gainは変わらない
  - Link有効時
    - Left Gain変更 → Right Gainも同じ値になる
    - Right Gain変更 → Left Gainも同じ値になる
  - Link状態変更テスト
    - Off→On: 現在の値を保持
    - On→Off: 独立動作に戻る
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 3.1.2 **IMPL**: `source/plugincontroller.cpp` の `setParamNormalized()` をオーバーライド
  - Link Gainパラメータの状態をチェック
  - Link有効時: Left/Right Gain の同期処理
  - `performEdit()` で相手側のパラメータも更新
- [ ] 3.1.3 **IMPL**: Processor側での Link Gain 処理
  - パラメータキューから Link Gain 値を読み取る
  - 必要に応じて同期（Controller側で処理済みなので不要かも）

**Verification Tasks**:
- [ ] 3.1.4 **VERIFY**: 統合テストを実行
  ```bash
  cd build && ctest -R test_link_gain
  ```
- [ ] 3.1.5 **VERIFY**: VST3 Validator 実行
- [ ] 3.1.6 **VERIFY**: Manual UI test
  - DAWでプラグインを読み込む
  - Link L/R Gain をOFF
    - Left Gainを変更 → Right Gainは変わらないことを確認
  - Link L/R Gain をON
    - Left Gainを変更 → Right Gainも変わることを確認
    - Right Gainを変更 → Left Gainも変わることを確認

**Completion Criteria**:
- Link L/R Gain が正しく動作する
- 統合テストがパス
- 実際のUIで同期動作確認済み

---

#### Task 3.2: Master Gain Implementation
**Priority**: P1 (High)
**Estimated Time**: 1.5 hours
**Dependencies**: Task 2.2

**Test Tasks**:
- [ ] 3.2.1 **TEST**: `tests/integration/test_master_gain.cpp` を作成
  - Master Gain 動作テスト
    - 0dB: 最終出力が変わらない
    - -6dB: 最終出力が約0.5倍
    - +6dB: 最終出力が約2倍
    - -60dB: 完全ミュート
  - 個別Gainとの相互作用テスト
    - Left Gain=-3dB, Master Gain=-3dB → 合計-6dB相当
  - **Expected Result**: すべてのテストがパス

**Implementation Tasks**:
- [ ] 3.2.2 **IMPL**: `source/pluginprocessor.cpp` の `processAudio()` を更新
  - Master Gain を最終段で適用（既に実装済みの可能性）
  - スムージング適用

**Verification Tasks**:
- [ ] 3.2.3 **VERIFY**: 統合テストを実行
  ```bash
  cd build && ctest -R test_master_gain
  ```
- [ ] 3.2.4 **VERIFY**: Manual audio test
  - DAWでプラグインを読み込む
  - Master Gain を変更して音量が変わることを確認
  - 個別Gainと組み合わせて正しく動作することを確認

**Completion Criteria**:
- Master Gain が正しく動作する
- 統合テストがパス
- 実際のオーディオで動作確認済み

---

#### Task 3.3: Sample Rate Change Handling
**Priority**: P2 (Medium)
**Estimated Time**: 2 hours
**Dependencies**: Task 2.2

**Test Tasks**:
- [ ] 3.3.1 **TEST**: `tests/integration/test_sample_rate_change.cpp` を作成
  - サンプルレート変更テスト
    - 44.1kHz → 48kHz: 遅延時間(ms)が保持される
    - 48kHz → 96kHz: バッファサイズが正しく変更される
  - 遅延サンプル数再計算テスト
    - 10ms @ 44.1kHz = 441 samples
    - 10ms @ 48kHz = 480 samples
  - クラッシュ防止テスト
    - 無効なサンプルレート（0, 負数）: デフォルト値を使用
  - **Expected Result**: すべてのテストがパス、クラッシュなし

**Implementation Tasks**:
- [ ] 3.3.2 **IMPL**: `source/pluginprocessor.cpp` の `setupProcessing()` を更新
  - サンプルレート検証
  - DelayLine のリサイズ
  - ParameterSmoother の再初期化
  - 現在の遅延時間(ms)を保持したままサンプル数を再計算

**Verification Tasks**:
- [ ] 3.3.3 **VERIFY**: 統合テストを実行
  ```bash
  cd build && ctest -R test_sample_rate_change
  ```
- [ ] 3.3.4 **VERIFY**: Manual stress test
  - DAWでプラグインを読み込む
  - サンプルレートを変更（44.1kHz ↔ 48kHz ↔ 96kHz）
  - クラッシュしないことを確認
  - 遅延時間が保持されることを確認

**Completion Criteria**:
- サンプルレート変更が正しく処理される
- 統合テストがパス
- 実際のDAWで動作確認済み

---

### Phase 4: Polish & Testing

#### Task 4.1: Full VST3 Validator Compliance
**Priority**: P0 (Critical)
**Estimated Time**: 3 hours
**Dependencies**: All Phase 1-3 tasks

**Test Tasks**:
- [ ] 4.1.1 **TEST**: VST3 Validator の全テストを実行
  ```bash
  cd build && bin/Release/validator VST3/Release/SimplePanner.vst3 > validator_report.txt
  ```
- [ ] 4.1.2 **VERIFY**: 以下のテストカテゴリがすべてパス
  - [ ] General Tests
    - [ ] Scan Editor Classes
    - [ ] Scan Buses
    - [ ] Scan Parameters (8個のパラメータ)
    - [ ] Terminate/Initialize
    - [ ] Valid State Transition
    - [ ] Bus Consistency
    - [ ] Bus Activation
    - [ ] Check Audio Bus Arrangement
  - [ ] Single Precision Tests
    - [ ] Process Test
    - [ ] Silence Processing
    - [ ] Silence Flags
    - [ ] Parameters Flush
    - [ ] Variable Block Size
    - [ ] Process Format
    - [ ] Speaker Arrangement (Stereo)
  - [ ] 0 failed tests

**Fix Tasks**:
- [ ] 4.1.3 **FIX**: Validator で失敗したテストを修正
  - 失敗したテストごとに原因を特定
  - 必要に応じてコードを修正
  - 再テスト

**Completion Criteria**:
- VST3 Validator のすべてのテストがパス
- validator_report.txt に "0 failed tests" が表示される

---

#### Task 4.2: Performance Optimization
**Priority**: P2 (Medium)
**Estimated Time**: 2 hours
**Dependencies**: Task 4.1

**Test Tasks**:
- [ ] 4.2.1 **TEST**: CPU使用率測定
  - DAWで複数インスタンス（10個）を起動
  - CPU使用率を測定
  - 目標: 1インスタンスあたり < 1% (44.1kHz, 512 samples)

**Optimization Tasks**:
- [ ] 4.2.2 **OPT**: プロファイリング
  - perf / Instruments / Visual Studio Profiler を使用
  - ホットスポットを特定
- [ ] 4.2.3 **OPT**: 最適化実施（必要な場合のみ）
  - Pan係数の事前計算（パラメータ変更時のみ）
  - 不要な計算の削除
  - キャッシュ効率の改善

**Verification Tasks**:
- [ ] 4.2.4 **VERIFY**: CPU使用率を再測定
- [ ] 4.2.5 **VERIFY**: 目標値以下であることを確認
- [ ] 4.2.6 **VERIFY**: 音質が劣化していないことを確認

**Completion Criteria**:
- CPU使用率が許容範囲内
- 音質に影響がない

---

#### Task 4.3: DAW Compatibility Testing
**Priority**: P1 (High)
**Estimated Time**: 4 hours
**Dependencies**: Task 4.1

**Test Tasks**:
- [ ] 4.3.1 **TEST**: Reaper でのテスト
  - プラグイン読み込み
  - パラメータ操作
  - オートメーション
  - プロジェクト保存・読み込み
- [ ] 4.3.2 **TEST**: Ableton Live でのテスト（可能であれば）
  - 同上
- [ ] 4.3.3 **TEST**: FL Studio でのテスト（可能であれば）
  - 同上
- [ ] 4.3.4 **TEST**: Cubase でのテスト（可能であれば）
  - 同上

**Fix Tasks**:
- [ ] 4.3.5 **FIX**: DAW固有の問題を修正
  - 問題が見つかった場合、原因を特定して修正

**Completion Criteria**:
- 少なくとも1つのDAWで完全に動作する（必須: Reaper）
- 可能であれば複数のDAWで動作確認

---

#### Task 4.4: Documentation Update
**Priority**: P1 (High)
**Estimated Time**: 2 hours
**Dependencies**: Task 4.1

**Documentation Tasks**:
- [ ] 4.4.1 **DOC**: README.md を更新
  - 機能説明を requirements に合わせて更新
  - パラメータリスト（8個）を記載
  - ビルド手順の確認
- [ ] 4.4.2 **DOC**: User manual を作成
  - `docs/user_manual.md`
  - 各パラメータの説明
  - 使用例（ハース効果など）
  - スクリーンショット（可能であれば）
- [ ] 4.4.3 **DOC**: Developer documentation を更新
  - `docs/development.md`
  - ビルド手順
  - テスト実行方法
  - コード構造の説明
- [ ] 4.4.4 **DOC**: CHANGELOG.md を作成
  - バージョン 1.0.0 のリリースノート

**Completion Criteria**:
- すべてのドキュメントが最新
- ユーザーがREADMEを読んで使用できる
- 開発者がドキュメントを読んで貢献できる

---

#### Task 4.5: Final Integration Test
**Priority**: P0 (Critical)
**Estimated Time**: 2 hours
**Dependencies**: All tasks

**Test Tasks**:
- [ ] 4.5.1 **TEST**: エンドツーエンドテスト
  - クリーンビルド
    ```bash
    rm -rf build && mkdir build && cd build
    cmake .. && cmake --build .
    ```
  - VST3 Validator 実行
  - すべてのユニットテスト実行
    ```bash
    ctest --output-on-failure
    ```
  - すべての統合テスト実行
- [ ] 4.5.2 **TEST**: マニュアルテストシナリオ
  - シナリオ1: デフォルト状態
    - プラグイン読み込み → 音が変わらない
  - シナリオ2: パンニング
    - Left Pan=R50, Right Pan=L50 → LRが入れ替わる
  - シナリオ3: ハース効果
    - Left Delay=0ms, Right Delay=10ms → 左に定位
  - シナリオ4: Link Gain
    - Link ON → L/R Gain が連動
  - シナリオ5: Master Gain
    - Master Gain=-6dB → 全体が-6dB

**Verification Tasks**:
- [ ] 4.5.3 **VERIFY**: すべての自動テストがパス
- [ ] 4.5.4 **VERIFY**: すべてのマニュアルテストシナリオが成功
- [ ] 4.5.5 **VERIFY**: Acceptance criteria（requirements.md）をすべて満たす

**Completion Criteria**:
- すべてのテストがパス
- すべてのAcceptance criteriaを満たす
- プロダクション準備完了

---

## Test Infrastructure Setup

### CMake Test Configuration

以下を `CMakeLists.txt` に追加（テスト実行用）:

```cmake
# テストを有効化
enable_testing()

# Google Test を使用する場合（オプション）
# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/release-1.11.0.zip
# )
# FetchContent_MakeAvailable(googletest)

# ユニットテストの追加
# add_executable(test_parameter_conversion tests/unit/test_parameter_conversion.cpp)
# target_link_libraries(test_parameter_conversion PRIVATE gtest_main)
# add_test(NAME test_parameter_conversion COMMAND test_parameter_conversion)

# 統合テストの追加
# add_executable(test_processor_state tests/integration/test_processor_state.cpp)
# target_link_libraries(test_processor_state PRIVATE SimplePanner sdk gtest_main)
# add_test(NAME test_processor_state COMMAND test_processor_state)
```

### Manual Test Setup

`tests/manual/test_procedure.md` を作成し、マニュアルテスト手順を記載:

```markdown
# SimplePanner Manual Test Procedures

## Setup
1. Build the plugin
2. Copy to VST3 folder
3. Launch DAW (Reaper recommended)
4. Create audio track with test signal

## Test Cases
... (詳細なテスト手順)
```

---

## Task Tracking

進捗管理は以下の方法で行います:

1. **GitHub Issues**: 各タスクをIssueとして作成
2. **This Document**: タスクの完了状態を更新
3. **Git Commits**: 各タスク完了時にコミット
   - コミットメッセージに Task ID を含める（例: `[Task 1.1] Implement parameter conversion utilities`）

---

## Priority Legend

- **P0 (Critical)**: 必須機能、これがないと動作しない
- **P1 (High)**: 重要機能、ユーザー体験に大きく影響
- **P2 (Medium)**: 有用な機能、あると良い
- **P3 (Low)**: Nice to have、後回しでも良い

---

## Estimated Total Time

| Phase | Estimated Time |
|-------|----------------|
| Phase 0: Setup | 0.5 hours |
| Phase 1: Core Infrastructure | 14 hours |
| Phase 2: DSP Implementation | 9 hours |
| Phase 3: Advanced Features | 5.5 hours |
| Phase 4: Polish & Testing | 11 hours |
| **Total** | **40 hours** |

---

**Document Version**: 1.0
**Last Updated**: 2025-11-13
**Status**: Ready to Start
