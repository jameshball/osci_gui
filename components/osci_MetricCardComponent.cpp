#include "osci_MetricCardComponent.h"

namespace osci {

MetricCardComponent::MetricCardComponent(juce::String titleToUse, juce::String componentId)
    : title(std::move(titleToUse)) {
    setComponentID(std::move(componentId));
    setTitle(title);
    setInterceptsMouseClicks(false, false);
    auto style = getStyle();
    style.cornerRadius = 9.0f;
    style.backgroundColour = Colours::surfaceSunken().withAlpha(0.72f);
    setStyle(style);
}

void MetricCardComponent::setValue(juce::String valueToUse, juce::String detailToUse, juce::Colour accentToUse) {
    if (value == valueToUse && detail == detailToUse && accent == accentToUse) {
        return;
    }
    value = std::move(valueToUse);
    detail = std::move(detailToUse);
    accent = accentToUse;
    setTitle(title + ": " + value);
    setDescription(detail);
    repaint();
}

std::unique_ptr<juce::AccessibilityHandler> MetricCardComponent::createAccessibilityHandler() {
    return std::make_unique<juce::AccessibilityHandler>(*this, juce::AccessibilityRole::staticText);
}

void MetricCardComponent::paint(juce::Graphics& graphics) {
    CardComponent::paint(graphics);
    auto bounds = getLocalBounds().toFloat().reduced(12.0f, 9.0f);
    graphics.setColour(accent);
    graphics.fillRoundedRectangle(bounds.removeFromLeft(3.0f), 1.5f);
    bounds.removeFromLeft(9.0f);
    auto heading = bounds.removeFromTop(15.0f);
    graphics.setColour(Colours::textSubtle());
    graphics.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    graphics.drawText(title, heading, juce::Justification::centredLeft);
    graphics.setColour(Colours::text());
    graphics.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    graphics.drawFittedText(value, bounds.removeFromTop(25.0f).toNearestInt(), juce::Justification::centredLeft, 1);
    graphics.setColour(Colours::textMuted());
    graphics.setFont(juce::FontOptions(10.0f));
    graphics.drawFittedText(detail, bounds.toNearestInt(), juce::Justification::centredLeft, 1);
}

} // namespace osci
