#include "osci_HoverAnimationMixin.h"

namespace osci {

HoverAnimationMixin::HoverAnimationMixin()
    : animationController (this) {
    animationController.setValueChangedCallback ([this] (auto value) {
        animationProgress = static_cast<float>(value);
        repaint();
        resized();
    });
}

void HoverAnimationMixin::animateHover (bool isHovering) {
    isHovered = isHovering;
    animationController.animateTo (isHovering, getHoverAnimationDurationMs(), getEasingFunction());
}

void HoverAnimationMixin::mouseEnter (const juce::MouseEvent&) {
    animateHover (true);
}

void HoverAnimationMixin::mouseExit (const juce::MouseEvent&) {
    // Fixed logic to prevent getting stuck in hovered state
    animateHover (false);
}

void HoverAnimationMixin::mouseDown (const juce::MouseEvent&) {
    // Keep hover animation active during drag
}

void HoverAnimationMixin::mouseUp (const juce::MouseEvent& event) {
    // Un-hover only if the mouse left during the drag
    if (!getLocalBounds().contains (event.getEventRelativeTo (this).getPosition())) {
        animateHover (false);
    }
}

} // namespace osci
