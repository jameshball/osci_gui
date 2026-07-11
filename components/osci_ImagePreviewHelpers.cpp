#include "osci_ImagePreviewHelpers.h"

namespace osci {

void drawImageCheckerboard(juce::Graphics& g, juce::Rectangle<int> area, int tileSize) {
    tileSize = juce::jmax(1, tileSize);
    const auto first = Colours::surfaceSunken();
    const auto second = Colours::surfaceRaised().interpolatedWith(first, 0.55f);
    for (int y = area.getY(); y < area.getBottom(); y += tileSize) {
        for (int x = area.getX(); x < area.getRight(); x += tileSize) {
            const auto alternate = ((x - area.getX()) / tileSize + (y - area.getY()) / tileSize) % 2 != 0;
            g.setColour(alternate ? second : first);
            g.fillRect(juce::Rectangle<int>(x, y, tileSize, tileSize).getIntersection(area));
        }
    }
}

} // namespace osci
