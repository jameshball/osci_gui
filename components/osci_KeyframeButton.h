#pragma once

namespace osci {

// Compact animation affordance that remains legible at inspector-row sizes.
class KeyframeButton : public juce::Button {
public:
    enum class State { unanimated, animated, keyed };
    KeyframeButton() : juce::Button("Keyframe") {}

    void setState(State value) {
        if (state != value) {
            state = value;
            setToggleState(state == State::keyed, juce::dontSendNotification);
            repaint();
        }
    }

    void paintButton(juce::Graphics& g, bool highlighted, bool down) override {
        const auto centre = getLocalBounds().toFloat().getCentre();
        const auto radius = std::min(5.0f, std::min(getWidth(), getHeight()) * 0.3f);
        juce::Path diamond;
        diamond.startNewSubPath(centre.x, centre.y - radius);
        diamond.lineTo(centre.x + radius, centre.y);
        diamond.lineTo(centre.x, centre.y + radius);
        diamond.lineTo(centre.x - radius, centre.y);
        diamond.closeSubPath();
        const auto alpha = isEnabled() ? (highlighted || down ? 1.0f : 0.8f) : 0.25f;
        g.setColour((state == State::unanimated ? Colours::text() : juce::Colour(0xff72de98)).withAlpha(alpha));
        if (state == State::keyed || down) {
            g.fillPath(diamond);
        } else {
            g.strokePath(diamond, juce::PathStrokeType(1.2f));
        }
        if (hasKeyboardFocus(true)) {
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 3, 1);
        }
    }

private:
    State state = State::unanimated;
};
}
