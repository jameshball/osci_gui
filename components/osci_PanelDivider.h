#pragma once

namespace osci {
// A narrow resize handle. The owner defines the limits and persists its layout.
class PanelDivider : public juce::Component {
public:
    explicit PanelDivider(bool vertical) : vertical(vertical) {
        setMouseCursor(vertical ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::UpDownResizeCursor);
        setWantsKeyboardFocus(true);
    }
    std::function<void()> onStart;
    std::function<void(int)> onDrag;
    std::function<void()> onReset;
    void mouseDown(const juce::MouseEvent& event) override {
        origin = event.getScreenPosition();
        if (onStart) {
            onStart();
        }
    }
    void mouseDrag(const juce::MouseEvent& event) override {
        const auto delta = event.getScreenPosition() - origin;
        if (onDrag) {
            onDrag(vertical ? delta.x : delta.y);
        }
    }
    void mouseDoubleClick(const juce::MouseEvent&) override {
        if (onReset) {
            onReset();
        }
    }
    bool keyPressed(const juce::KeyPress& key) override {
        const auto code = key.getKeyCode();
        const auto negative = code == (vertical ? juce::KeyPress::leftKey : juce::KeyPress::upKey);
        const auto positive = code == (vertical ? juce::KeyPress::rightKey : juce::KeyPress::downKey);
        if (!negative && !positive) {
            return false;
        }
        if (onStart) {
            onStart();
        }
        if (onDrag) {
            onDrag((negative ? -1 : 1) * (key.getModifiers().isShiftDown() ? 40 : 10));
        }
        return true;
    }
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    void focusGained(FocusChangeType) override { repaint(); }
    void focusLost(FocusChangeType) override { repaint(); }
    void paint(juce::Graphics& g) override {
        g.setColour(Colours::text().withAlpha(isMouseOverOrDragging() || hasKeyboardFocus(false) ? 0.5f : 0.14f));
        const auto area = getLocalBounds().toFloat();
        const auto line = vertical ? area.withSizeKeepingCentre(1, std::min(60.0f, area.getHeight()))
                                   : area.withSizeKeepingCentre(std::min(60.0f, area.getWidth()), 1);
        g.fillRect(line);
    }
private:
    bool vertical;
    juce::Point<int> origin;
};
}
