#pragma once

#include "osci_SvgButton.h"

namespace osci {

class CloseButton final : public juce::Component {
public:
    explicit CloseButton (juce::String nameToUse = "Close",
                          juce::Colour iconColour = Colours::text().withAlpha (0.62f),
                          juce::Colour iconHoverColour = Colours::text().withAlpha (0.90f))
        : iconButton (nameToUse + " icon",
                      closeButtonSvg,
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
    static constexpr auto closeButtonSvg = R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M19,6.41L17.59,5L12,10.59L6.41,5L5,6.41L10.59,12L5,17.59L6.41,19L12,13.41L17.59,19L19,17.59L13.41,12L19,6.41Z" /></svg>)svg";

    SvgButton iconButton;
    int iconPadding = 0;
    bool paintsBackground = false;
};

} // namespace osci
