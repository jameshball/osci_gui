#pragma once

namespace osci {

// Scroll fades over a ListBox, which scrolls itself: an overlay on the list,
// kept in step with its scrollbar and its rows.
class ListBoxScrollFades final : private juce::ScrollBar::Listener, private juce::ComponentListener {
public:
    explicit ListBoxScrollFades(juce::ListBox& listToFade, int fadeHeight = 32) : list(listToFade) {
        overlay.setFadeHeight(fadeHeight);
        list.addChildComponent(overlay);
        overlay.setAlwaysOnTop(true);
        list.getVerticalScrollBar().addListener(this);
        list.addComponentListener(this);
        // The rows' holder grows and shrinks with the list.
        if (content() != nullptr) {
            content()->addComponentListener(this);
        }
        update();
    }

    ~ListBoxScrollFades() override {
        list.getVerticalScrollBar().removeListener(this);
        list.removeComponentListener(this);
        if (content() != nullptr) {
            content()->removeComponentListener(this);
        }
    }

    // The colour the fades blend into: what is behind the list.
    void setBackground(juce::Colour colour) {
        overlay.setColour(scrollFadeOverlayBackgroundColourId, colour);
        overlay.repaint();
    }

    void update() {
        overlay.layoutOver(list.getLocalBounds());
        overlay.updateVisibilityFromViewport(list.getViewport(), true);
    }

private:
    juce::Component* content() const {
        return list.getViewport() != nullptr ? list.getViewport()->getViewedComponent() : nullptr;
    }
    void scrollBarMoved(juce::ScrollBar*, double) override { update(); }
    void componentMovedOrResized(juce::Component&, bool, bool) override { update(); }

    juce::ListBox& list;
    ScrollFadeOverlay overlay;
};

} // namespace osci
