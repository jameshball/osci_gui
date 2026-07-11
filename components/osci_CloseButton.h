#pragma once

#include "osci_SvgButton.h"

namespace osci {

class CloseButton final : public juce::Component {
public:
    explicit CloseButton (juce::String closeButtonSvg,
                          juce::String nameToUse = "Close",
                          juce::Colour iconColour = Colours::text().withAlpha (0.62f),
                          juce::Colour iconHoverColour = Colours::text().withAlpha (0.90f))
        : iconButton (nameToUse + " icon",
                      std::move (closeButtonSvg),
                      iconColour,
                      iconHoverColour) {
        setName (std::move (nameToUse));
        setInterceptsMouseClicks (false, true);
        setMouseCursor (juce::MouseCursor::PointingHandCursor);

        iconButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);
        iconButton.onClick = [this] {
            if (onClick != nullptr) {
                onClick();
            }
        };
        addAndMakeVisible (iconButton);
    }

    std::function<void()> onClick;

    void setIconPadding (int padding) {
        iconPadding = juce::jmax (0, padding);
        iconButton.setEdgeIndent (iconPadding);
        if (paintsBackground) {
            iconButton.setCircularBackground (true, iconPadding);
        }
    }

    void setPaintsBackground (bool shouldPaintBackground) {
        paintsBackground = shouldPaintBackground;
        iconButton.setCircularBackground (shouldPaintBackground, iconPadding);
    }

    void setIconColours (juce::Colour normalColour, juce::Colour hoverColour) {
        iconButton.setColours (normalColour, hoverColour);
    }

    void setColours (juce::Colour normalColour, juce::Colour hoverColour) {
        setIconColours (normalColour, hoverColour);
    }

    void resized() override {
        iconButton.setBounds (getLocalBounds());
    }

private:
    SvgButton iconButton;
    int iconPadding = 0;
    bool paintsBackground = false;
};

} // namespace osci
