#pragma once

#include "../lookandfeel/osci_LookAndFeel.h"

namespace osci {

inline void showContextMenuAsync(juce::PopupMenu menu,
                                 juce::Point<int> screenPosition,
                                 juce::Component* owner,
                                 std::function<void(int)> onResult) {
    auto options = juce::PopupMenu::Options().withTargetScreenArea(
        juce::Rectangle<int>(screenPosition.x, screenPosition.y, 1, 1));
    auto safeOwner = juce::Component::SafePointer<juce::Component>(owner);
    menu.showMenuAsync(options, [safeOwner, onResult = std::move(onResult)](int result) {
        if (safeOwner != nullptr && onResult != nullptr) {
            onResult(result);
        }
    });
}

class ContextMenuLabel : public juce::Label {
public:
    ContextMenuLabel() {
        setFont(juce::FontOptions(14.0f));
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        setInterceptsMouseClicks(true, false);
        updateTextColour();
    }

    void setTextColours(juce::Colour normal, juce::Colour hover) {
        normalColour = normal;
        hoverColour = hover;
        updateTextColour();
    }

    std::function<void(juce::Point<int>)> onContextMenu;

private:
    bool hitTest(int x, int y) override {
        if (getText().isEmpty()) {
            return false;
        }

        const auto contentBounds = getBorderSize().subtractedFrom(getLocalBounds());
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(getFont(), getText(), 0.0f, 0.0f);
        const auto glyphBounds = glyphs.getBoundingBox(0, glyphs.getNumGlyphs(), true);
        const int textWidth = juce::jmin(contentBounds.getWidth(), juce::roundToInt(glyphBounds.getWidth()) + 6);
        const int textHeight = juce::jmin(contentBounds.getHeight(), juce::roundToInt(getFont().getHeight()) + 4);
        const auto textBounds = getJustificationType().appliedToRectangle(
            juce::Rectangle<int>(textWidth, textHeight), contentBounds);
        return textBounds.contains(x, y);
    }

    void mouseDown(const juce::MouseEvent& event) override {
        if ((event.mods.isLeftButtonDown() || event.mods.isPopupMenu()) && onContextMenu != nullptr) {
            onContextMenu(event.getScreenPosition());
            return;
        }
        juce::Label::mouseDown(event);
    }

    void mouseEnter(const juce::MouseEvent& event) override {
        hovered = true;
        updateTextColour();
        juce::Label::mouseEnter(event);
    }

    void mouseExit(const juce::MouseEvent& event) override {
        hovered = false;
        updateTextColour();
        juce::Label::mouseExit(event);
    }

    void updateTextColour() {
        setColour(juce::Label::textColourId, hovered ? hoverColour : normalColour);
    }

    juce::Colour normalColour = juce::Colours::white;
    juce::Colour hoverColour = osci::Colours::accentColor();
    bool hovered = false;
};

} // namespace osci
