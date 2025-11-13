# SimplePanner

シンプルなステレオパンニングVST3プラグイン

## 概要

SimplePannerは、オーディオ信号のパンニング（左右の定位）とゲインを制御できるシンプルなVST3プラグインです。

## 機能

- ステレオパンニング: 左右のバランスを調整
- ゲインコントロール: 音量を調整
- 等パワーパンニング: 自然な音響特性を維持

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

1. DAW（Digital Audio Workstation）を起動
2. SimplePannerプラグインを読み込む
3. 以下のパラメータを調整:
   - **Pan**: 左右のパンニング位置（左 ← → 右）
   - **Gain**: 音量レベル

## プロジェクト構成

```
SimplePanner/
├── CMakeLists.txt          # CMakeビルド設定
├── include/                # ヘッダーファイル
│   ├── plugids.h          # プラグインIDの定義
│   ├── pluginprocessor.h  # オーディオ処理クラス
│   └── plugincontroller.h # UIコントロールクラス
├── source/                 # ソースファイル
│   ├── pluginprocessor.cpp
│   ├── plugincontroller.cpp
│   └── pluginfactory.cpp  # プラグインファクトリー
└── vst3sdk/               # VST3 SDK（自動クローン）
```

## ライセンス

このプロジェクトのライセンスについては、LICENSEファイルを参照してください。

## 開発者向け情報

### プラグインIDのカスタマイズ

`include/plugids.h`内のProcessorUIDとControllerUIDを変更して、独自のプラグインIDを設定してください。
これらのIDはプラグインを一意に識別するために使用されます。

### パラメータの追加

新しいパラメータを追加する場合:
1. `include/plugids.h`にパラメータIDを追加
2. `include/pluginprocessor.h`にメンバー変数を追加
3. `source/plugincontroller.cpp`でパラメータを登録
4. `source/pluginprocessor.cpp`で処理ロジックを実装

## 参考資料

- [VST3 SDK Documentation](https://steinbergmedia.github.io/vst3_doc/)
- [VST3 SDK GitHub](https://github.com/steinbergmedia/vst3sdk)
