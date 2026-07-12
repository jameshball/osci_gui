#include "osci_NotificationComponent.h"

namespace osci {

NotificationComponent::NotificationComponent(juce::String closeButtonSvg)
    : closeButton(std::move(closeButtonSvg), "Dismiss notification", Colours::text(), Colours::text()) {
    setVisible(false);

    titleLabel.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, Colours::text());
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setInterceptsMouseClicks(false, false);

    messageLabel.setFont(juce::Font(juce::FontOptions(13.0f)));
    messageLabel.setColour(juce::Label::textColourId, Colours::textMuted());
    messageLabel.setJustificationType(juce::Justification::centredLeft);
    messageLabel.setMinimumHorizontalScale(0.8f);
    messageLabel.setInterceptsMouseClicks(false, false);

    closeButton.setPaintsBackground(true);
    closeButton.setIconPadding(8);
    closeButton.onClick = [this] {
        if (onDismiss != nullptr) {
            onDismiss();
        }
    };

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(messageLabel);
    addAndMakeVisible(closeButton);
}

void NotificationComponent::setNotification(juce::String title, juce::String message) {
    titleLabel.setText(std::move(title), juce::dontSendNotification);
    messageLabel.setText(std::move(message), juce::dontSendNotification);
    setVisible(true);
    repaint();
}

void NotificationComponent::paint(juce::Graphics& g) {
    const auto bounds = getCardBounds().toFloat();
    juce::Path shadowPath;
    shadowPath.addRoundedRectangle(bounds, cornerRadius);
    shadow.render(g, shadowPath);

    g.setColour(Colours::veryDark().withAlpha(0.985f));
    g.fillRoundedRectangle(bounds, cornerRadius);
    g.setColour(Colours::accentColor().withAlpha(0.72f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void NotificationComponent::resized() {
    auto content = getCardBounds().reduced(18, 10);
    closeButton.setBounds(content.removeFromRight(28).withSizeKeepingCentre(28, 28));
    content.removeFromRight(10);
    titleLabel.setBounds(content.removeFromTop(20));
    messageLabel.setBounds(content.removeFromTop(20));
}

juce::Rectangle<int> NotificationComponent::getCardBounds() const {
    return getLocalBounds().reduced(shadowPadding);
}

} // namespace osci
