#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace osci {

inline bool popoutAlphaHitTest(const unsigned char* rgba, int width, int height, float normalisedX, float normalisedY,
                               float normalisedRadiusX, float normalisedRadiusY, std::uint8_t threshold) {
    if (rgba == nullptr || width <= 0 || height <= 0 || normalisedX < 0.0f || normalisedX > 1.0f
        || normalisedY < 0.0f || normalisedY > 1.0f) {
        return false;
    }

    const int centreX = std::clamp(static_cast<int>(std::floor(normalisedX * static_cast<float>(width))), 0, width - 1);
    const int centreY = std::clamp(static_cast<int>(std::floor((1.0f - normalisedY) * static_cast<float>(height))), 0, height - 1);
    const int radiusX = std::max(0, static_cast<int>(std::ceil(normalisedRadiusX * static_cast<float>(width))));
    const int radiusY = std::max(0, static_cast<int>(std::ceil(normalisedRadiusY * static_cast<float>(height))));
    const int left = std::max(0, centreX - radiusX);
    const int right = std::min(width - 1, centreX + radiusX);
    const int bottom = std::max(0, centreY - radiusY);
    const int top = std::min(height - 1, centreY + radiusY);

    for (int y = bottom; y <= top; ++y) {
        for (int x = left; x <= right; ++x) {
            const auto alphaIndex = (static_cast<std::size_t>(y) * static_cast<std::size_t>(width)
                                     + static_cast<std::size_t>(x)) * 4u + 3u;
            if (rgba[alphaIndex] >= threshold) {
                return true;
            }
        }
    }
    return false;
}

} // namespace osci
