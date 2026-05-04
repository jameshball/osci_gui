#include "osci_ToggleAnimationController.h"

namespace osci {

ToggleAnimationController::ToggleAnimationController (juce::Component* componentToUpdate)
    : animatorUpdater (componentToUpdate) {
    jassert (componentToUpdate != nullptr);
}

ToggleAnimationController::~ToggleAnimationController() {
    cancelActiveAnimator();
}

void ToggleAnimationController::setValueChangedCallback (ValueChangedCallback callback) {
    onValueChanged = std::move (callback);
}

void ToggleAnimationController::animateTo (bool shouldBeOn,
                                           int durationMs,
                                           EasingFunction easing,
                                           CompletionCallback completion) {
    targetState = shouldBeOn;
    const auto targetProgress = shouldBeOn ? 1.0f : 0.0f;

    cancelActiveAnimator();

    if (durationMs <= 0 || std::abs (progress - targetProgress) <= 0.0001f) {
        setProgress (targetProgress);
        if (completion) {
            completion();
        }
        return;
    }

    const auto startProgress = progress;
    const auto easingToUse = easing != nullptr ? std::move (easing) : juce::Easings::createEaseInOut();

    activeAnimator = juce::ValueAnimatorBuilder {}
        .withDurationMs (durationMs)
        .withEasing (easingToUse)
        .withValueChangedCallback ([this, startProgress, targetProgress] (auto value) {
            const auto eased = juce::jlimit (0.0f, 1.0f, static_cast<float>(value));
            setProgress (startProgress + (targetProgress - startProgress) * eased);
        })
        .build();

    animating = true;
    animatorUpdater.addAnimator (*activeAnimator, [this, targetProgress, completion = std::move (completion)] {
        setProgress (targetProgress);
        animating = false;
        activeAnimator.reset();

        if (completion) {
            completion();
        }
    });
    activeAnimator->start();
}

void ToggleAnimationController::snapTo (bool shouldBeOn) {
    targetState = shouldBeOn;
    cancelActiveAnimator();
    setProgress (shouldBeOn ? 1.0f : 0.0f);
}

void ToggleAnimationController::cancelActiveAnimator() {
    if (activeAnimator.has_value()) {
        animatorUpdater.removeAnimator (*activeAnimator);
        activeAnimator.reset();
    }

    animating = false;
}

void ToggleAnimationController::setProgress (float newProgress) {
    progress = juce::jlimit (0.0f, 1.0f, newProgress);

    if (onValueChanged) {
        onValueChanged (progress);
    }
}

} // namespace osci
