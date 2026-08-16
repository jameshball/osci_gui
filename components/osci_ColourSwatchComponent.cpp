#include "osci_ColourSwatchComponent.h"

namespace osci {

ColourSwatchComponent::ColourSwatchComponent(std::function<juce::Colour()> colourProvider, juce::String tooltip, int refreshRateHz)
    : colourProvider(std::move(colourProvider)) {
    setTooltip(std::move(tooltip));
    updateColour();
    startTimerHz(juce::jmax(1, refreshRateHz));
}

void ColourSwatchComponent::paint(juce::Graphics& g) {
    const auto diameter = juce::jlimit(0.0f, 14.0f, static_cast<float>(getHeight() - 4));
    if (diameter <= 0.0f) {
        return;
    }

    const auto bounds = juce::Rectangle<float>(diameter, diameter).withCentre(getLocalBounds().toFloat().getCentre());
    g.setColour(Colours::outline());
    g.fillEllipse(bounds.expanded(1.0f));
    g.setColour(currentColour);
    g.fillEllipse(bounds);
}

void ColourSwatchComponent::timerCallback() {
    updateColour();
}

void ColourSwatchComponent::updateColour() {
    const auto colour = colourProvider != nullptr ? colourProvider() : juce::Colours::transparentBlack;
    if (colour != currentColour) {
        currentColour = colour;
        repaint();
    }
}

} // namespace osci
