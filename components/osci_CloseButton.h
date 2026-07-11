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
        resized();
    }

    void setPaintsBackground (bool shouldPaintBackground) {
        paintsBackground = shouldPaintBackground;
        repaint();
    }

    void setIconColours (juce::Colour normalColour, juce::Colour hoverColour) {
        iconButton.setColours (normalColour, hoverColour);
    }

    void setColours (juce::Colour normalColour, juce::Colour hoverColour) {
        setIconColours (normalColour, hoverColour);
    }

    void paint (juce::Graphics& g) override {
        if (! paintsBackground) {
            return;
        }

        auto bounds = getLocalBounds().toFloat();
        g.setColour (Colours::neutralFill (0.035f));
        g.fillEllipse (bounds);
        g.setColour (Colours::neutralStroke (0.18f));
        g.drawEllipse (bounds.reduced (0.5f), 1.0f);
    }

    void resized() override {
        iconButton.setBounds (getLocalBounds().reduced (iconPadding));
    }

private:
    SvgButton iconButton;
    int iconPadding = 0;
    bool paintsBackground = false;
};

} // namespace osci
