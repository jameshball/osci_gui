#include "osci_AnimatedTextButton.h"

namespace osci {

AnimatedTextButton::AnimatedTextButton(juce::String buttonText)
    : juce::TextButton(std::move(buttonText)),
      toggleAnimation(this) {
    toggleAnimation.setValueChangedCallback([this](float progress) {
        toggleProgress = progress;
        repaint();
    });
}

void AnimatedTextButton::setToggleAnimationDurationMs(int durationMs) {
    toggleAnimationDurationMs = juce::jmax(0, durationMs);
}

void AnimatedTextButton::snapAnimationToToggleState() {
    previousToggleState = getToggleState();
    toggleAnimation.snapTo(previousToggleState);
}

void AnimatedTextButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto& lookAndFeel = getLookAndFeel();
    const auto background = findColour(juce::TextButton::buttonColourId)
                                .interpolatedWith(findColour(juce::TextButton::buttonOnColourId), toggleProgress);
    lookAndFeel.drawButtonBackground(g, *this, background, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    const auto font = lookAndFeel.getTextButtonFont(*this, getHeight());
    g.setFont(font);
    g.setColour(findColour(juce::TextButton::textColourOffId)
                    .interpolatedWith(findColour(juce::TextButton::textColourOnId), toggleProgress)
                    .withMultipliedAlpha(isEnabled() ? 1.0f : 0.5f));

    const auto yIndent = juce::jmin(4, proportionOfHeight(0.3f));
    const auto cornerSize = juce::jmin(getHeight(), getWidth()) / 2;
    const auto fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
    const auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (isConnectedOnLeft() ? 4 : 2));
    const auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (isConnectedOnRight() ? 4 : 2));
    const auto textWidth = getWidth() - leftIndent - rightIndent;
    if (textWidth > 0) {
        g.drawFittedText(getButtonText(), leftIndent, yIndent, textWidth, getHeight() - yIndent * 2, juce::Justification::centred, 2);
    }
}

void AnimatedTextButton::buttonStateChanged() {
    juce::TextButton::buttonStateChanged();
    const auto toggleState = getToggleState();
    if (toggleState != previousToggleState) {
        previousToggleState = toggleState;
        toggleAnimation.animateTo(toggleState, toggleAnimationDurationMs, juce::Easings::createEaseInOut());
    }
}

} // namespace osci
