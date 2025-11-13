#pragma once

namespace Steinberg {
namespace SimplePanner {

// プラグインのユニークID (変更してください)
static const FUID ProcessorUID(0x12345678, 0x12345678, 0x12345678, 0x12345678);
static const FUID ControllerUID(0x87654321, 0x87654321, 0x87654321, 0x87654321);

// パラメータID
enum {
    kParamPanId = 0,    // パンニングパラメータ
    kParamGainId = 1    // ゲインパラメータ
};

} // namespace SimplePanner
} // namespace Steinberg
