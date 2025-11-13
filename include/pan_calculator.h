// pan_calculator.h
// Equal power panning calculations

#pragma once

#include <cmath>
#include <algorithm>

namespace Steinberg {
namespace SimplePanner {

/**
 * @brief Pan gain structure
 *
 * Contains left and right gain values for panning.
 */
struct PanGains {
    float left;   ///< Left channel gain (0.0 to 1.0)
    float right;  ///< Right channel gain (0.0 to 1.0)
};

/**
 * @brief Calculate equal power pan gains
 * @param pan Pan position (-100.0 = full left, 0.0 = center, +100.0 = full right)
 * @return PanGains structure with left and right gains
 *
 * Uses equal power panning law (constant power):
 * - Pan is mapped to angle: angle = (pan / 100) * (π/4)
 * - Left gain = cos(angle)
 * - Right gain = sin(angle)
 *
 * This ensures that left² + right² = 1 (constant power).
 * At center (pan=0), both channels are at -3dB (0.707).
 */
inline PanGains calculatePanGains(float pan) {
    // Clamp pan to valid range
    pan = std::max(-100.0f, std::min(100.0f, pan));

    // Convert pan to normalized position (0.0 = left, 1.0 = right)
    float normalizedPan = (pan + 100.0f) / 200.0f;

    // Calculate angle for equal power panning
    // Range: 0 to π/2 (0 to 90 degrees)
    const float kPi = 3.14159265358979323846f;
    float angle = normalizedPan * (kPi / 2.0f);

    // Calculate gains using cos/sin
    PanGains gains;
    gains.left = std::cos(angle);
    gains.right = std::sin(angle);

    return gains;
}

} // namespace SimplePanner
} // namespace Steinberg
