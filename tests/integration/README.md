# Integration Tests

このディレクトリには、複数のコンポーネントを統合したテストが含まれます。

## テストファイル

- `test_processor_state.cpp`: Processorの状態管理テスト
- `test_controller_parameters.cpp`: Controllerのパラメータ管理テスト
- `test_audio_processing_basic.cpp`: 基本的なオーディオ処理テスト
- `test_parameter_smoothing.cpp`: パラメータスムージング統合テスト
- `test_link_gain.cpp`: Link L/R Gain機能テスト
- `test_master_gain.cpp`: Master Gain機能テスト
- `test_sample_rate_change.cpp`: サンプルレート変更テスト

## 実行方法

```bash
cd build
ctest -R test_ --output-on-failure
```

## テスト用オーディオファイル

テスト用のオーディオファイル（ピンクノイズ、サイン波など）は `tests/audio_files/` に配置してください。
