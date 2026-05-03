#pragma once

namespace osci {

// Base class for full-window overlay panels. Subclasses populate the content
// area; the overlay owns the backdrop, panel chrome, close button, and dismiss
// callback.
class OverlayComponent : public juce::Component {
public:
    explicit OverlayComponent (juce::String closeButtonSvg)
        : closeOverlayButton ("closeOverlay", std::move (closeButtonSvg), juce::Colours::white, juce::Colours::white) {
        setOpaque (false);
        setAlwaysOnTop (true);
        setInterceptsMouseClicks (true, true);
        setWantsKeyboardFocus (true);

        configureLabel (overlayTitleLabel,
                        juce::Font (juce::FontOptions (20.0f, juce::Font::bold)),
                        juce::Justification::centredLeft);
        overlayTitleLabel.setVisible (false);
        addAndMakeVisible (overlayTitleLabel);

        closeOverlayButton.setTooltip ("Close");
        closeOverlayButton.onClick = [this] {
            dismiss();
        };
        addAndMakeVisible (closeOverlayButton);
    }

    ~OverlayComponent() override = default;

    std::function<void()> onDismissRequested;
    bool lightweight = false;

    void setOverlayTitle (const juce::String& title) {
        overlayTitleLabel.setText (title, juce::dontSendNotification);
        overlayTitleLabel.setVisible (title.isNotEmpty());
    }

    void setDismissible (bool shouldBeDismissible) {
        dismissible = shouldBeDismissible;
        closeOverlayButton.setVisible (dismissible);
    }

    void paint (juce::Graphics& g) override {
        if (lightweight) {
            return;
        }

        g.fillAll (juce::Colours::black.withAlpha (0.58f));

        const auto cornerRadius = 12.0f;
        auto panelFloat = panelBounds.toFloat();

        juce::Path panelPath;
        panelPath.addRoundedRectangle (panelFloat, cornerRadius);
        juce::DropShadow (juce::Colours::black.withAlpha (0.38f), 24, { 0, 8 }).drawForPath (g, panelPath);

        g.setColour (Colours::veryDark().brighter (0.015f));
        g.fillRoundedRectangle (panelFloat, cornerRadius);

        g.setColour (juce::Colours::white.withAlpha (0.14f));
        g.drawRoundedRectangle (panelFloat.reduced (0.5f), cornerRadius, 1.0f);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        auto preferred = getPreferredPanelSize();
        if (preferred.x > 0 && preferred.y > 0) {
            auto available = bounds.reduced (40, 24);
            panelBounds.setSize (juce::jmin (preferred.x, available.getWidth()),
                                 juce::jmin (preferred.y, available.getHeight()));
            panelBounds.setCentre (bounds.getCentre());
        } else {
            const auto horizontalMargin = juce::jmax (40, bounds.getWidth() / 6);
            const auto verticalMargin = juce::jmax (24, bounds.getHeight() / 10);
            panelBounds = bounds.reduced (horizontalMargin, verticalMargin);
        }

        auto contentArea = panelBounds.reduced (24);
        if (overlayTitleLabel.isVisible() || closeOverlayButton.isVisible()) {
            auto titleArea = contentArea.removeFromTop (28);
            closeOverlayButton.setBounds (titleArea.removeFromRight (28).withSizeKeepingCentre (22, 22));
            overlayTitleLabel.setBounds (titleArea);
            contentArea.removeFromTop (10);
        } else {
            closeOverlayButton.setBounds ({});
            overlayTitleLabel.setBounds ({});
        }

        resizeContent (contentArea);
    }

    void mouseDown (const juce::MouseEvent& event) override {
        if (! dismissible || panelBounds.contains (event.getPosition())) {
            return;
        }

        if (auto* focused = juce::Component::getCurrentlyFocusedComponent()) {
            if (dynamic_cast<juce::TextEditor*> (focused) != nullptr
                && (focused == this || isParentOf (focused))) {
                return;
            }
        }

        dismiss();
    }

    bool keyPressed (const juce::KeyPress& key) override {
        if (dismissible && key == juce::KeyPress::escapeKey) {
            dismiss();
            return true;
        }

        return juce::Component::keyPressed (key);
    }

protected:
    virtual void resizeContent (juce::Rectangle<int> contentArea) = 0;
    virtual juce::Point<int> getPreferredPanelSize() const { return {}; }

    void dismiss() {
        if (onDismissRequested) {
            onDismissRequested();
        }
    }

    juce::Rectangle<int> panelBounds;

    static void configureLabel (juce::Label& label,
                                const juce::Font& font,
                                juce::Justification justification) {
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        label.setFont (font);
        label.setJustificationType (justification);
        label.setEditable (false, false, false);
        label.setInterceptsMouseClicks (false, false);
    }

private:
    juce::Label overlayTitleLabel;
    SvgButton closeOverlayButton;
    bool dismissible = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayComponent)
};

} // namespace osci
