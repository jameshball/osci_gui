#include "osci_PerformanceWheel.h"
#include "../../lookandfeel/osci_LookAndFeel.h"
#include <cmath>

namespace osci {

PerformanceWheel::PerformanceWheel(Mode wheelMode) : mode(wheelMode) {
    setSliderStyle(juce::Slider::LinearVertical);
    setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    setRange(mode == Mode::pitch ? -1.0 : 0.0, 1.0);
    setValue(0.0, juce::dontSendNotification);
    setDoubleClickReturnValue(true, 0.0);
    setScrollWheelEnabled(false);
    setSliderSnapsToMousePosition(false);
    setMouseDragSensitivity(180);
    setName(mode == Mode::pitch ? "Pitch wheel" : "Modulation wheel");
    setTooltip(mode == Mode::pitch ? "Pitch bend - drag up or down; release to centre" : "Modulation - drag up or down; double-click to reset");
    setColour(juce::Slider::thumbColourId, Colours::accentColor());
    displayedValue = getValue();
}

PerformanceWheel::~PerformanceWheel() { stopTimer(); }

void PerformanceWheel::setAccentColour(juce::Colour colour) {
    setColour(juce::Slider::thumbColourId, colour);
    repaint();
}

void PerformanceWheel::setModulatedValue(double value, bool active) {
    value = juce::jlimit(getMinimum(), getMaximum(), value);
    if (modulatedValue != value || showModulation != active) {
        modulatedValue = value;
        showModulation = active;
        repaint();
    }
}

void PerformanceWheel::valueChanged() {
    if (dragging || !isShowing()) {
        displayedValue = getValue();
        velocity = 0.0;
        stopTimer();
    } else {
        lastTick = juce::Time::getMillisecondCounterHiRes();
        startTimerHz(60);
    }
    repaint();
}

void PerformanceWheel::mouseDown(const juce::MouseEvent& event) {
    assignmentDrag = event.mods.isAltDown() && event.mods.isLeftButtonDown() && bool(onAssignmentDrag);
    if (assignmentDrag) { return; }
    dragging = !event.mods.isPopupMenu();
    stopTimer();
    velocity = 0.0;
    juce::Slider::mouseDown(event);
}

void PerformanceWheel::mouseDrag(const juce::MouseEvent& event) {
    if (assignmentDrag) {
        if (event.getDistanceFromDragStart() > 4 && onAssignmentDrag) { onAssignmentDrag(event); }
    } else {
        juce::Slider::mouseDrag(event);
    }
}

void PerformanceWheel::releaseWheel() {
    if (!dragging) { return; }
    dragging = false;
    if (mode == Mode::pitch) { setValue(0.0, juce::sendNotificationSync); }
}

void PerformanceWheel::mouseUp(const juce::MouseEvent& event) {
    if (assignmentDrag) { assignmentDrag = false; return; }
    // Send the neutral value before Slider closes the host's drag gesture.
    releaseWheel();
    juce::Slider::mouseUp(event);
    repaint();
}

void PerformanceWheel::mouseEnter(const juce::MouseEvent& event) { juce::Slider::mouseEnter(event); repaint(); }
void PerformanceWheel::mouseExit(const juce::MouseEvent& event) { juce::Slider::mouseExit(event); repaint(); }

void PerformanceWheel::visibilityChanged() {
    if (!isShowing()) {
        releaseWheel();
        stopTimer();
        displayedValue = getValue();
        velocity = 0.0;
    }
}

void PerformanceWheel::timerCallback() {
    const auto now = juce::Time::getMillisecondCounterHiRes();
    const auto elapsed = juce::jlimit(0.001, 0.05, (now - lastTick) / 1000.0);
    lastTick = now;
    // Small substeps keep the damped spring stable across delayed UI frames.
    const int steps = int(std::ceil(elapsed / 0.008));
    const double dt = elapsed / steps;
    for (int i = 0; i < steps; ++i) {
        velocity += ((getValue() - displayedValue) * 420.0 - velocity * 30.0) * dt;
        displayedValue += velocity * dt;
    }
    if (std::abs(getValue() - displayedValue) < 0.0005 && std::abs(velocity) < 0.005) {
        displayedValue = getValue();
        velocity = 0.0;
        stopTimer();
    }
    repaint();
}

void PerformanceWheel::paint(juce::Graphics& g) {
    const auto accent = findColour(juce::Slider::thumbColourId).withMultipliedAlpha(isEnabled() ? 1.0f : 0.35f);
    auto bounds = getLocalBounds().toFloat();
    if (bounds.getWidth() < 6.0f || bounds.getHeight() < 8.0f) { return; }
    const float radius = juce::jmin(6.0f, bounds.getWidth() * 0.25f);
    g.setColour(Colours::darkerer()); // Same surface as the Voices label section.
    g.fillRoundedRectangle(bounds, radius);

    const auto drum = bounds.reduced(2.0f, 1.0f);
    juce::Path clip;
    clip.addRoundedRectangle(drum, radius);
    g.saveState();
    g.reduceClipRegion(clip);

    const auto normalised = juce::jlimit(0.0, 1.0, (displayedValue - getMinimum()) / (getMaximum() - getMinimum()));
    const double markerAngle = std::asin((0.5 - normalised) * 1.9);
    for (int i = -7; i <= 7; ++i) {
        const double angle = markerAngle + i * 0.52;
        if (std::abs(angle) >= juce::MathConstants<double>::halfPi) { continue; }
        const float depth = float(std::cos(angle));
        const float y = drum.getCentreY() + float(std::sin(angle)) * drum.getHeight() * 0.5f;
        const float halfHeight = juce::jmax(0.65f, depth * drum.getHeight() * 0.080f);
        const float shoulder = std::abs(float(std::sin(angle))) * drum.getWidth() * 0.16f;
        const float left = drum.getX(), right = drum.getRight();
        const float top = y - halfHeight, bottom = y + halfHeight;
        const float inset = juce::jmin(shoulder, halfHeight);

        // Flat faces with directional shoulders, rounding only the corners.
        juce::Path rib;
        if (angle < 0.0) {
            rib.startNewSubPath(left + shoulder, top);
            rib.lineTo(right - shoulder, top);
            rib.lineTo(right, top + inset);
            rib.lineTo(right, bottom);
            rib.lineTo(left, bottom);
            rib.lineTo(left, top + inset);
        } else {
            rib.startNewSubPath(left, top);
            rib.lineTo(right, top);
            rib.lineTo(right, bottom - inset);
            rib.lineTo(right - shoulder, bottom);
            rib.lineTo(left + shoulder, bottom);
            rib.lineTo(left, bottom - inset);
        }
        rib.closeSubPath();
        g.setColour(i == 0 ? accent : Colours::text().withAlpha(0.23f + 0.24f * depth));
        const float corner = juce::jmin(1.2f, halfHeight * 0.4f);
        if (shoulder < 0.001f) {
            // Avoid duplicate corner vertices at neutral: all four corners must match.
            g.fillRoundedRectangle(left, top, drum.getWidth(), halfHeight * 2.0f, corner);
        } else {
            g.fillPath(rib.createPathWithRoundedCorners(corner));
        }
    }
    g.restoreState();

    if (showModulation) {
        const float position = float((modulatedValue - getMinimum()) / (getMaximum() - getMinimum()));
        const float y = juce::jmap(position, drum.getBottom() - 2.0f, drum.getY());
        g.setColour(accent);
        g.fillEllipse(bounds.getRight() - 3.5f, y, 2.5f, 2.5f);
    }
    if (hasKeyboardFocus(true)) {
        g.setColour(accent.withAlpha(0.7f));
        g.drawRoundedRectangle(bounds, radius, 1.0f);
    }
}

} // namespace osci
