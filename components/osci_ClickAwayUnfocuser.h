#pragma once

namespace osci {

// Clicking away from a text field being typed in ends the typing, as it does
// in native apps. JUCE leaves the focus where it is when what is clicked
// doesn't take it, so a click on a panel's empty space would otherwise leave
// the field focused. Buttons and scrollbars are left alone: a button beside a
// field (Bold, Apply) can act on it while it keeps the focus.
class ClickAwayUnfocuser final : private juce::MouseListener, private juce::FocusChangeListener {
public:
    // `rootToUse`: the window's content; the focus goes no further up.
    explicit ClickAwayUnfocuser(juce::Component& rootToUse) : root(rootToUse) {
        auto& desktop = juce::Desktop::getInstance();
        desktop.addGlobalMouseListener(this);
        desktop.addFocusChangeListener(this);
    }

    ~ClickAwayUnfocuser() override {
        auto& desktop = juce::Desktop::getInstance();
        desktop.removeFocusChangeListener(this);
        desktop.removeGlobalMouseListener(this);
    }

private:
    // Told asynchronously, so a field a click has just put up (to rename
    // what was clicked) is not yet the one typed in before it.
    void globalFocusChanged(juce::Component* focused) override { typingIn = dynamic_cast<juce::TextEditor*>(focused); }

    void mouseDown(const juce::MouseEvent& event) override {
        auto* field = typingIn.getComponent();
        auto* clicked = event.eventComponent;
        if (field == nullptr || clicked == nullptr || !field->hasKeyboardFocus(true)) {
            return;
        }
        if (clicked == field || field->isParentOf(clicked) || clicked->getTopLevelComponent() != field->getTopLevelComponent()) {
            return;
        }
        if (field->getScreenBounds().contains(event.getScreenPosition())) {
            return;
        }
        for (auto* component = clicked; component != nullptr; component = component->getParentComponent()) {
            if (dynamic_cast<juce::Button*>(component) != nullptr || dynamic_cast<juce::ScrollBar*>(component) != nullptr) {
                return;
            }
        }
        typingIn = nullptr;
        // The focus goes to what was clicked, or the nearest thing round it
        // that takes keys (a popover, the window's content), so shortcuts
        // keep working.
        for (auto* component = clicked; component != nullptr; component = component->getParentComponent()) {
            if (component->getWantsKeyboardFocus() && component->isEnabled() && component->isShowing()) {
                component->grabKeyboardFocus();
                return;
            }
            if (component == &root) {
                break;
            }
        }
        field->giveAwayKeyboardFocus();
    }

    juce::Component& root;
    juce::Component::SafePointer<juce::TextEditor> typingIn;
};

} // namespace osci
