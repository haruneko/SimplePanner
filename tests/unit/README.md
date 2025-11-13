# Unit Tests

このディレクトリには、個別のクラスや関数のユニットテストが含まれます。

## テストファイル

- `test_parameter_conversion.cpp`: パラメータ変換ユーティリティのテスト
- `test_delay_line.cpp`: DelayLineクラスのテスト
- `test_parameter_smoother.cpp`: ParameterSmootherクラスのテスト
- `test_pan_calculation.cpp`: パンニング計算のテスト

## 実行方法

```bash
cd build
ctest -R test_ --output-on-failure
```

または個別に:

```bash
cd build
./tests/unit/test_parameter_conversion
```
