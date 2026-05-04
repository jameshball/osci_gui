#pragma once

namespace osci {

class ToggleAnimationController {
public:
    using EasingFunction = std::function<float (float)>;
    using ValueChangedCallback = std::function<void (float)>;
    using CompletionCallback = std::function<void()>;

    explicit ToggleAnimationController (juce::Component* componentToUpdate);
    ~ToggleAnimationController();

    void setValueChangedCallback (ValueChangedCallback callback);
    void animateTo (bool shouldBeOn,
                    int durationMs,
                    EasingFunction easing,
                    CompletionCallback completion = {});
    void snapTo (bool shouldBeOn);

    float getProgress() const { return progress; }
    bool getTargetState() const { return targetState; }
    bool isAnimating() const { return animating; }

private:
    void cancelActiveAnimator();
    void setProgress (float newProgress);

    juce::VBlankAnimatorUpdater animatorUpdater;
    std::optional<juce::Animator> activeAnimator;
    ValueChangedCallback onValueChanged;
    float progress = 0.0f;
    bool targetState = false;
    bool animating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleAnimationController)
};

} // namespace osci
