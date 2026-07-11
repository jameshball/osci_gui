#pragma once

namespace osci {

class AnimatedTextButton : public juce::TextButton {
public:
    explicit AnimatedTextButton(juce::String buttonText = {});

    void setToggleAnimationDurationMs(int durationMs);
    void snapAnimationToToggleState();

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void buttonStateChanged() override;

private:
    ToggleAnimationController toggleAnimation;
    float toggleProgress = 0.0f;
    int toggleAnimationDurationMs = 180;
    bool previousToggleState = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedTextButton)
};

} // namespace osci
