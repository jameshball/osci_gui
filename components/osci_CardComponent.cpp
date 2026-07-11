#include "osci_CardComponent.h"

namespace osci {

CardComponent::CardComponent(Style styleToUse)
    : style(std::move(styleToUse)) {}

void CardComponent::setStyle(Style styleToUse) {
    style = std::move(styleToUse);
    repaint();
}

const CardComponent::Style& CardComponent::getStyle() const {
    return style;
}

void CardComponent::setPaintsCard(bool shouldPaint) {
    if (paintsCard == shouldPaint) {
        return;
    }

    paintsCard = shouldPaint;
    repaint();
}

bool CardComponent::getPaintsCard() const {
    return paintsCard;
}

void CardComponent::paint(juce::Graphics& g) {
    if (paintsCard) {
        drawCard(g, getLocalBounds().toFloat(), style);
    }
}

void CardComponent::drawCard(juce::Graphics& g, juce::Rectangle<float> bounds, const Style& style) {
    if (style.shadowRadius > 0 && !style.shadowColour.isTransparent()) {
        juce::Path shadowPath;
        shadowPath.addRoundedRectangle(bounds, style.cornerRadius);
        juce::DropShadow(style.shadowColour, style.shadowRadius, style.shadowOffset).drawForPath(g, shadowPath);
    }

    if (!style.backgroundColour.isTransparent()) {
        g.setColour(style.backgroundColour);
        g.fillRoundedRectangle(bounds, style.cornerRadius);
    }

    if (style.outlineWidth > 0.0f && !style.outlineColour.isTransparent()) {
        g.setColour(style.outlineColour);
        g.drawRoundedRectangle(bounds.reduced(style.outlineWidth * 0.5f), style.cornerRadius, style.outlineWidth);
    }
}

} // namespace osci
