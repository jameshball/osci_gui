#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace osci {

// A parameter-attachable wheel. Pitch returns to zero; modulation retains its value.
// The spring only animates the drawing, never the parameter or its audio output.
class PerformanceWheel : public juce::Slider, private juce::Timer {
public:
    enum class Mode { pitch, modulation };
    explicit PerformanceWheel(Mode);
    ~PerformanceWheel() override;

    // Optional Alt/Option-drag action, separate from changing the wheel value.
    std::function<void(const juce::MouseEvent&)> onAssignmentDrag;
    void setAccentColour(juce::Colour);
    void setModulatedValue(double value, bool active);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void visibilityChanged() override;

private:
    void valueChanged() override;
    void timerCallback() override;
    void releaseWheel();

    const Mode mode;
    double displayedValue = 0.0, velocity = 0.0, lastTick = 0.0;
    double modulatedValue = 0.0;
    bool dragging = false, showModulation = false, assignmentDrag = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceWheel)
};

} // namespace osci
