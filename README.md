# SimplePanner

[![Build and Test](https://github.com/haruneko/SimplePanner/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/haruneko/SimplePanner/actions/workflows/build-and-test.yml)

ステレオイメージとハース効果を精密にコントロールするVST3プラグイン

## 概要

SimplePannerは、ステレオ入力信号の各チャンネル（L/R）を独立して制御できるVST3オーディオプラグインです。パンニング、ゲイン調整、遅延を組み合わせることで、ステレオイメージの精密なコントロールとハース効果による定位調整を実現します。

## 機能

### コアフィーチャー
- **独立したL/Rチャンネル制御**: 左右の入力を完全に独立して処理
- **精密なパンニング**: -100（L）〜+100（R）の範囲で各チャンネルの定位を調整
- **柔軟なゲイン調整**: -60dB（-∞）〜+6dBの範囲で各チャンネルと全体の音量を制御
- **ハース効果対応**: 0〜100msの遅延で自然な定位感を創出
- **等パワーパンニング**: 音響的に自然な-3dBセンター特性

### パラメータ（8つ）
1. **Left Pan**: 左入力の定位（-100 〜 +100）
2. **Left Gain**: 左入力の音量（-60dB 〜 +6dB）
3. **Left Delay**: 左入力の遅延時間（0 〜 100ms）
4. **Right Pan**: 右入力の定位（-100 〜 +100）
5. **Right Gain**: 右入力の音量（-60dB 〜 +6dB）
6. **Right Delay**: 右入力の遅延時間（0 〜 100ms）
7. **Master Gain**: 最終出力の全体音量（-60dB 〜 +6dB）
8. **Link L/R Gain**: L/Rゲインの連動ON/OFF

### 使用例
- **ワイドなステレオイメージ**: 左入力をL100、右入力をR100に配置
- **LRスワップ**: 左入力をR100、右入力をL100に配置
- **ハース効果**: 片方のチャンネルを10-30ms遅延させて定位を調整
- **モノラル化**: 両チャンネルをCenter（0）に配置

## ビルド手順

### 必要環境

- CMake 3.14以上
- C++17対応のコンパイラ
  - Windows: Visual Studio 2017以上
  - macOS: Xcode 9以上
  - Linux: GCC 7以上またはClang 5以上
- Git

### ビルド方法

1. リポジトリをクローン:
```bash
git clone <このリポジトリのURL>
cd SimplePanner
```

2. ビルドディレクトリを作成:
```bash
mkdir build
cd build
```

3. CMakeを実行:
```bash
cmake ..
```

4. ビルド:
```bash
cmake --build .
```

VST3 SDKは初回ビルド時に自動的にクローンされます。

### インストール

ビルドが完了すると、`build`ディレクトリ内に`SimplePanner.vst3`が生成されます。
これを各プラットフォームのVST3フォルダにコピーしてください:

- Windows: `C:\Program Files\Common Files\VST3\`
- macOS: `/Library/Audio/Plug-Ins/VST3/` または `~/Library/Audio/Plug-Ins/VST3/`
- Linux: `~/.vst3/`

## 使用方法

### 基本的な使い方

1. DAW（Digital Audio Workstation）でステレオトラックにプラグインを挿入
2. デフォルト状態では入力信号がそのまま出力されます（透過状態）
3. GUIでパラメータを調整してステレオイメージをコントロール

### GUI 操作

SimplePannerは直感的なグラフィカルインターフェイスを提供します（600x400ピクセル）：

#### レイアウト
- **左チャンネルセクション**（左上）: Left Pan、Left Gain、Left Delay
- **右チャンネルセクション**（右上）: Right Pan、Right Gain、Right Delay
- **マスターセクション**（下部）: Master Gain、Link L/R Gain

#### コントロール操作

**スライダー（Pan）**:
- ドラッグ: 値を変更
- **Shift + ドラッグ**: 微調整モード（10倍精密）
- **ダブルクリック**: デフォルト値にリセット
- 両側に L/R ラベル表示

**ノブ（Gain、Delay）**:
- ドラッグ: 円形に動かして値を変更
- **Shift + ドラッグ**: 微調整モード（10倍精密）
- **ダブルクリック**: デフォルト値にリセット

**トグルボタン（Link L/R Gain）**:
- クリック: ON/OFF切り替え
- **OFF状態**: グレー表示
- **ON状態**: グリーン表示（L/Rゲイン連動）

#### 値の表示
- **Pan**: "L100"（左端）、"C"（中央）、"R50"（右50）
- **Gain**: "-∞ dB"（ミュート）、"0.0 dB"、"+3.0 dB"
- **Delay**: "0.0 ms"、"10.5 ms"、"100.0 ms"

### パラメータ詳細

#### Left/Right Pan（スライダー）
- 範囲: -100（Full Left）〜 +100（Full Right）
- デフォルト: Left=-100, Right=+100
- 各入力チャンネルの定位を調整します
- Center（0）では等パワー分配（-3dB）

#### Left/Right Gain（ノブ）
- 範囲: -60dB（ミュート）〜 +6dB
- デフォルト: 0dB（Unity Gain）
- 各入力チャンネルの音量を調整します
- -60dBは完全ミュート（-∞）として動作

#### Left/Right Delay（ノブ）
- 範囲: 0ms 〜 100ms
- デフォルト: 0ms
- ハース効果により定位感を調整できます
- 10-30msの遅延で自然な広がりが得られます

#### Master Gain（ノブ）
- 範囲: -60dB（ミュート）〜 +6dB
- デフォルト: 0dB
- 最終出力の全体音量を調整します
- 個別チャンネルのゲインに加算されます

#### Link L/R Gain（トグル）
- ON: Left/Right Gainが連動して変更されます
- OFF: 独立して調整できます
- 全体の音量を簡単に調整したい場合に便利

## プロジェクト構成

```
SimplePanner/
├── CMakeLists.txt          # CMakeビルド設定
├── LICENSE                 # ライセンスファイル
├── README.md               # このファイル
├── docs/                   # ドキュメント
│   ├── requirements.md     # 要件定義
│   ├── design.md          # 詳細設計
│   └── tasks.md           # 実装タスク
├── include/                # ヘッダーファイル
│   ├── plugids.h          # プラグインID/パラメータ定義
│   ├── pluginprocessor.h  # オーディオ処理クラス
│   ├── plugincontroller.h # UIコントロールクラス
│   ├── plugineditor.h     # GUIエディタクラス
│   ├── delay_line.h       # 遅延バッファクラス
│   ├── parameter_smoother.h # パラメータ平滑化
│   ├── parameter_utils.h  # パラメータ変換ユーティリティ
│   └── pan_calculator.h   # パンニング計算
├── source/                 # ソースファイル
│   ├── pluginprocessor.cpp
│   ├── plugincontroller.cpp
│   ├── plugineditor.cpp   # GUI実装
│   ├── pluginfactory.cpp
│   ├── delay_line.cpp
│   ├── parameter_smoother.cpp
│   ├── parameter_utils.cpp
│   └── pan_calculator.cpp
├── tests/                  # テストコード
│   ├── unit/              # ユニットテスト（107 tests）
│   │   ├── test_gui_formatting.cpp      # GUI値フォーマット
│   │   ├── test_gui_parameter_sync.cpp  # GUIパラメータ同期
│   │   ├── test_gui_visual.cpp          # GUI安定性
│   │   └── ...
│   ├── integration/       # 統合テスト（17 tests）
│   └── manual/            # マニュアルテスト手順
│       ├── test_procedure.md           # オーディオ機能テスト
│       └── gui_test_procedure.md       # GUIテスト手順
└── vst3sdk/               # VST3 SDK（自動クローン）
```

## ライセンス

このプロジェクトのライセンスについては、LICENSEファイルを参照してください。

## 開発者向け情報

### ドキュメント

詳細な技術情報は以下のドキュメントを参照してください：

- **[requirements.md](docs/requirements.md)**: 機能要件、パラメータ仕様、受け入れ基準
- **[design.md](docs/design.md)**: クラス設計、アルゴリズム、実装詳細
- **[tasks.md](docs/tasks.md)**: 実装タスク、テスト計画、進捗管理

### 品質保証

このプロジェクトは **Test-Driven Development (TDD)** アプローチで開発されています：

- **ユニットテスト**: 107個
  - DSP: parameter conversion, delay line, smoother, pan calculation
  - GUI: formatting (18), parameter sync (13), visual stability (8)
- **統合テスト**: 17個（processor state management）
- **VST3 Validator**: 47テスト全てパス
- **総テスト数**: 171テスト、100% pass rate
- **マニュアルテスト**: GUI操作、DAW互換性テスト手順完備

### テスト実行

```bash
cd build
ctest --output-on-failure
```

すべてのテストが通ることを確認してください。

### VST3 Validator

```bash
cd build
bin/Release/validator VST3/Release/SimplePanner.vst3
```

Validator は公式の VST3 仕様準拠チェックツールです。

### アーキテクチャ

- **Processor**: オーディオ処理（DSP、遅延、パンニング）
- **Controller**: パラメータ管理、UI通信
- **Editor**: VSTGUI による GUI 実装（600x400px）
  - スライダー（Pan）、ノブ（Gain/Delay）、トグルボタン
  - リアルタイム値表示、双方向パラメータ同期
- **DelayLine**: サーキュラーバッファによる遅延実装
- **ParameterSmoother**: One-pole IIRフィルタによる平滑化
- **等パワーパンニング**: cos/sin法（-3dB center）

### 技術仕様

- **VST3 SDK**: 3.7.0以降
- **サンプル精度遅延**: 補間なし
- **パラメータスムージング**: 約10ms収束時間
- **対応サンプルレート**: 22.05kHz - 384kHz以上
- **レイテンシー**: 遅延パラメータの最大値を報告

## 参考資料

- [VST3 SDK Documentation](https://steinbergmedia.github.io/vst3_doc/)
- [VST3 SDK GitHub](https://github.com/steinbergmedia/vst3sdk)
- [Equal Power Panning](https://www.cs.cmu.edu/~music/icm-online/readings/panlaws/)

## ライセンス

MITライセンス - 詳細は[LICENSE](LICENSE)ファイルを参照
