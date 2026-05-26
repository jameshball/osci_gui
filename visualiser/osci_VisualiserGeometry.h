#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include <cmath>
#include <cstdint>
#include <vector>

struct VisualiserRenderSize {
    int width = 1024;
    int height = 1024;

    bool operator==(const VisualiserRenderSize& other) const {
        return width == other.width && height == other.height;
    }

    bool operator!=(const VisualiserRenderSize& other) const {
        return !(*this == other);
    }
};

struct VisualiserGraticuleLayout {
    int xDivisions = 10;
    int yDivisions = 10;
    float cellSizePixels = 90.0f;
    float xOriginPixels = 62.0f;
    float yOriginPixels = 62.0f;
    float lineWidthPixels = 2.0f;
};

struct VisualiserRenderTargetSizes {
    VisualiserRenderSize output;
    VisualiserRenderSize tightBlur;
    VisualiserRenderSize wideBlur;
};

enum class VisualiserCanvasPreset : int {
    Square = 1,
    HDLandscape = 2,
    HDPortrait = 3,
    Custom = 4,
};

namespace VisualiserGeometry {
    constexpr int minCanvasDimension = 128;
    constexpr int maxCanvasDimension = 4096;

    inline int sanitiseCanvasDimension(int value) {
        int clamped = juce::jlimit(minCanvasDimension, maxCanvasDimension, value);
        return clamped - (clamped % 2);
    }

    inline VisualiserRenderSize sanitiseRenderSize(int width, int height) {
        return {sanitiseCanvasDimension(width), sanitiseCanvasDimension(height)};
    }

    inline VisualiserCanvasPreset sanitiseCanvasPreset(int value, VisualiserCanvasPreset fallback = VisualiserCanvasPreset::Square) {
        const auto preset = static_cast<VisualiserCanvasPreset>(value);
        switch (preset) {
            case VisualiserCanvasPreset::Square:
            case VisualiserCanvasPreset::HDLandscape:
            case VisualiserCanvasPreset::HDPortrait:
            case VisualiserCanvasPreset::Custom:
                return preset;
            default:
                return fallback;
        }
    }

    inline VisualiserRenderSize getRenderSizeForPreset(VisualiserCanvasPreset preset) {
        switch (preset) {
            case VisualiserCanvasPreset::Square:
                return {1024, 1024};
            case VisualiserCanvasPreset::HDLandscape:
                return {1920, 1080};
            case VisualiserCanvasPreset::HDPortrait:
                return {1080, 1920};
            case VisualiserCanvasPreset::Custom:
            default:
                return {1024, 1024};
        }
    }

    inline VisualiserCanvasPreset getPresetForRenderSize(VisualiserRenderSize size) {
        size = sanitiseRenderSize(size.width, size.height);
        if (size == getRenderSizeForPreset(VisualiserCanvasPreset::Square)) {
            return VisualiserCanvasPreset::Square;
        }
        if (size == getRenderSizeForPreset(VisualiserCanvasPreset::HDLandscape)) {
            return VisualiserCanvasPreset::HDLandscape;
        }
        if (size == getRenderSizeForPreset(VisualiserCanvasPreset::HDPortrait)) {
            return VisualiserCanvasPreset::HDPortrait;
        }
        return VisualiserCanvasPreset::Custom;
    }

    inline juce::String getPresetName(VisualiserCanvasPreset preset) {
        switch (preset) {
            case VisualiserCanvasPreset::Square:
                return "1024 x 1024";
            case VisualiserCanvasPreset::HDLandscape:
                return "1920 x 1080";
            case VisualiserCanvasPreset::HDPortrait:
                return "1080 x 1920";
            case VisualiserCanvasPreset::Custom:
            default:
                return "Custom";
        }
    }

    inline double getAspectRatio(VisualiserRenderSize size) {
        size = sanitiseRenderSize(size.width, size.height);
        return static_cast<double>(size.width) / static_cast<double>(size.height);
    }

    inline bool isSquare(VisualiserRenderSize size) {
        size = sanitiseRenderSize(size.width, size.height);
        return size.width == size.height;
    }

    inline juce::Point<float> getWorldToClipScale(VisualiserRenderSize size) {
        const float aspect = static_cast<float>(getAspectRatio(size));
        if (aspect >= 1.0f) {
            return {1.0f / aspect, 1.0f};
        }
        return {1.0f, aspect};
    }

    inline int roundUpToEven(int value) {
        return value + (value % 2);
    }

    inline int fitEvenDivisionsToCanvas(int divisions, float cellSize, int dimension, int minimumDivisions) {
        divisions = roundUpToEven(juce::jmax(minimumDivisions, divisions));
        while (divisions > minimumDivisions && static_cast<float>(divisions) * cellSize > static_cast<float>(dimension)) {
            divisions -= 2;
        }
        return divisions;
    }

    inline VisualiserGraticuleLayout getGraticuleLayout(VisualiserRenderSize size) {
        size = sanitiseRenderSize(size.width, size.height);

        constexpr int baseDivisions = 10;
        constexpr float virtualTextureSize = 512.0f;
        constexpr float virtualCellSize = 45.0f;
        constexpr float virtualLineWidth = 4.0f;
        constexpr float originalTextureSize = 2048.0f;

        const float shortSide = static_cast<float>(juce::jmin(size.width, size.height));
        const float virtualScale = shortSide / virtualTextureSize;
        const float aspect = static_cast<float>(size.width) / static_cast<float>(size.height);
        const float cellSize = virtualCellSize * virtualScale;

        int xDivisions = baseDivisions;
        int yDivisions = baseDivisions;
        if (aspect >= 1.0f) {
            xDivisions = fitEvenDivisionsToCanvas(static_cast<int>(std::ceil(baseDivisions * aspect)), cellSize, size.width, baseDivisions);
        } else {
            yDivisions = fitEvenDivisionsToCanvas(static_cast<int>(std::ceil(baseDivisions / aspect)), cellSize, size.height, baseDivisions);
        }

        return {
            xDivisions,
            yDivisions,
            cellSize,
            (static_cast<float>(size.width) - static_cast<float>(xDivisions) * cellSize) * 0.5f,
            (static_cast<float>(size.height) - static_cast<float>(yDivisions) * cellSize) * 0.5f,
            juce::jmax(1.0f, virtualLineWidth * shortSide / originalTextureSize)
        };
    }

    inline float graticulePixelToClip(float pixel, int dimension) {
        return pixel / static_cast<float>(dimension) * 2.0f - 1.0f;
    }

    inline VisualiserRenderSize getAspectScaledRenderSize(VisualiserRenderSize size, int maxLongSide) {
        size = sanitiseRenderSize(size.width, size.height);
        const int longSide = juce::jmax(size.width, size.height);
        const double scale = static_cast<double>(maxLongSide) / static_cast<double>(longSide);
        auto sanitiseTextureDimension = [](int value) {
            const int clamped = juce::jmax(16, value);
            return clamped - (clamped % 2);
        };
        return {sanitiseTextureDimension(juce::roundToInt(static_cast<double>(size.width) * scale)),
                sanitiseTextureDimension(juce::roundToInt(static_cast<double>(size.height) * scale))};
    }

    inline juce::Rectangle<int> getAspectFitBoundsForAspect(juce::Rectangle<int> area, double targetAspect) {
        if (area.isEmpty()) {
            return area;
        }

        const double areaAspect = static_cast<double>(area.getWidth()) / static_cast<double>(area.getHeight());

        int fittedWidth = area.getWidth();
        int fittedHeight = area.getHeight();
        if (areaAspect > targetAspect) {
            fittedWidth = juce::roundToInt(static_cast<double>(fittedHeight) * targetAspect);
        } else {
            fittedHeight = juce::roundToInt(static_cast<double>(fittedWidth) / targetAspect);
        }
        fittedWidth = juce::jlimit(1, area.getWidth(), fittedWidth);
        fittedHeight = juce::jlimit(1, area.getHeight(), fittedHeight);

        return juce::Rectangle<int>(area.getX() + (area.getWidth() - fittedWidth) / 2,
                                    area.getY() + (area.getHeight() - fittedHeight) / 2,
                                    fittedWidth,
                                    fittedHeight);
    }

    inline juce::Rectangle<int> getAspectFitBounds(juce::Rectangle<int> area, VisualiserRenderSize size) {
        return getAspectFitBoundsForAspect(area, getAspectRatio(size));
    }

    inline juce::Rectangle<int> getCropToFillSourceBounds(juce::Rectangle<int> sourceBounds, double targetAspect) {
        if (sourceBounds.isEmpty()) {
            return sourceBounds;
        }

        const double sourceAspect = static_cast<double>(sourceBounds.getWidth()) / static_cast<double>(sourceBounds.getHeight());
        int sourceWidth = sourceBounds.getWidth();
        int sourceHeight = sourceBounds.getHeight();

        if (targetAspect > sourceAspect) {
            sourceHeight = juce::roundToInt(static_cast<double>(sourceWidth) / targetAspect);
        } else {
            sourceWidth = juce::roundToInt(static_cast<double>(sourceHeight) * targetAspect);
        }

        sourceWidth = juce::jlimit(1, sourceBounds.getWidth(), sourceWidth);
        sourceHeight = juce::jlimit(1, sourceBounds.getHeight(), sourceHeight);

        return juce::Rectangle<int>(sourceBounds.getX() + (sourceBounds.getWidth() - sourceWidth) / 2,
                                    sourceBounds.getY() + (sourceBounds.getHeight() - sourceHeight) / 2,
                                    sourceWidth,
                                    sourceHeight);
    }

    inline std::vector<float> getGraticuleLineVertices(VisualiserRenderSize size) {
        size = sanitiseRenderSize(size.width, size.height);
        const auto graticule = getGraticuleLayout(size);
        constexpr int minorDivisions = 5;
        const float tickScale = graticule.cellSizePixels / 45.0f;
        const float shortTickNegative = 2.0f * tickScale;
        const float shortTickPositive = 1.0f * tickScale;
        const float centreTickNegative = 5.0f * tickScale;
        const float centreTickPositive = 4.0f * tickScale;
        const float quarterTickLength = 2.0f * tickScale;
        const float xMax = graticule.xOriginPixels + graticule.cellSizePixels * static_cast<float>(graticule.xDivisions);
        const float yMax = graticule.yOriginPixels + graticule.cellSizePixels * static_cast<float>(graticule.yDivisions);

        const int majorLineCount = graticule.yDivisions + graticule.xDivisions + 2;
        const int horizontalTickLineCount = (graticule.yDivisions - 1) * (graticule.xDivisions * minorDivisions + 1);
        const int verticalTickLineCount = (graticule.xDivisions - 1) * (graticule.yDivisions * minorDivisions + 1);
        const int quarterTickLineCount = 2 * graticule.xDivisions * (minorDivisions - 1);
        const int lineCount = majorLineCount + horizontalTickLineCount + verticalTickLineCount + quarterTickLineCount;
        std::vector<float> data;
        data.reserve(static_cast<size_t>(lineCount) * 4u);

        auto addLine = [&](float x1, float y1, float x2, float y2) {
            data.push_back(graticulePixelToClip(x1, size.width));
            data.push_back(graticulePixelToClip(y1, size.height));
            data.push_back(graticulePixelToClip(x2, size.width));
            data.push_back(graticulePixelToClip(y2, size.height));
        };

        for (int i = 0; i <= graticule.yDivisions; i++) {
            const float y = graticule.yOriginPixels + graticule.cellSizePixels * static_cast<float>(i);
            addLine(graticule.xOriginPixels, y, xMax, y);
        }

        for (int i = 0; i <= graticule.xDivisions; i++) {
            const float x = graticule.xOriginPixels + graticule.cellSizePixels * static_cast<float>(i);
            addLine(x, graticule.yOriginPixels, x, yMax);
        }

        for (int i = 1; i < graticule.yDivisions; i++) {
            const float y = graticule.yOriginPixels + graticule.cellSizePixels * static_cast<float>(i);
            const bool centreLine = i == graticule.yDivisions / 2;
            const float tickNegative = centreLine ? centreTickNegative : shortTickNegative;
            const float tickPositive = centreLine ? centreTickPositive : shortTickPositive;

            for (int j = 0; j <= graticule.xDivisions * minorDivisions; j++) {
                const float tx = graticule.xOriginPixels + graticule.cellSizePixels * static_cast<float>(j) / static_cast<float>(minorDivisions);
                addLine(tx, y - tickNegative, tx, y + tickPositive);
            }
        }

        for (int i = 1; i < graticule.xDivisions; i++) {
            const float x = graticule.xOriginPixels + graticule.cellSizePixels * static_cast<float>(i);
            const bool centreLine = i == graticule.xDivisions / 2;
            const float tickNegative = centreLine ? centreTickNegative : shortTickNegative;
            const float tickPositive = centreLine ? centreTickPositive : shortTickPositive;

            for (int j = 0; j <= graticule.yDivisions * minorDivisions; j++) {
                const float ty = graticule.yOriginPixels + graticule.cellSizePixels * static_cast<float>(j) / static_cast<float>(minorDivisions);
                addLine(x - tickNegative, ty, x + tickPositive, ty);
            }
        }

        const float quarterY1 = graticule.yOriginPixels + graticule.cellSizePixels * static_cast<float>(graticule.yDivisions) * 0.25f;
        const float quarterY2 = graticule.yOriginPixels + graticule.cellSizePixels * static_cast<float>(graticule.yDivisions) * 0.75f;
        for (int j = 0; j <= graticule.xDivisions * minorDivisions; j++) {
            if (j % minorDivisions != 0) {
                const float x = graticule.xOriginPixels + graticule.cellSizePixels * static_cast<float>(j) / static_cast<float>(minorDivisions);
                addLine(x - quarterTickLength, quarterY1, x + quarterTickLength, quarterY1);
                addLine(x - quarterTickLength, quarterY2, x + quarterTickLength, quarterY2);
            }
        }

        return data;
    }

    inline VisualiserRenderTargetSizes getRenderTargetSizes(VisualiserRenderSize size) {
        size = sanitiseRenderSize(size.width, size.height);
        return {size, getAspectScaledRenderSize(size, 512), getAspectScaledRenderSize(size, 128)};
    }

    inline std::uint64_t packRenderSize(VisualiserRenderSize size) {
        size = sanitiseRenderSize(size.width, size.height);
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(size.width)) << 32)
            | static_cast<std::uint32_t>(size.height);
    }

    inline VisualiserRenderSize unpackRenderSize(std::uint64_t packed) {
        if (packed == 0) {
            return {1024, 1024};
        }

        const int width = static_cast<int>((packed >> 32) & 0xffffffffu);
        const int height = static_cast<int>(packed & 0xffffffffu);
        return sanitiseRenderSize(width, height);
    }

}
