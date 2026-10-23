#pragma once

#include <functional>

namespace osci {

// A button that offers a menu on a right-click (or Ctrl-click) instead of
// clicking, when onPopupMenu is set: Undo's history, a diamond's other
// actions. Wraps any juce::Button with its constructor.
template <typename ButtonType>
class WithPopupMenu : public ButtonType {
public:
    using ButtonType::ButtonType;
    std::function<void()> onPopupMenu;

    void mouseDown(const juce::MouseEvent& event) override {
        if (event.mods.isPopupMenu() && onPopupMenu) {
            onPopupMenu();
            return;
        }
        ButtonType::mouseDown(event);
    }
    void mouseUp(const juce::MouseEvent& event) override {
        if (event.mods.isPopupMenu() && onPopupMenu) { return; }
        ButtonType::mouseUp(event);
    }
};

} // namespace osci
