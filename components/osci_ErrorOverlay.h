#pragma once

namespace osci {

class ErrorOverlay final : public OverlayComponent {
public:
    enum class Icon {
        Error,
        Warning,
        None,
    };

    struct ButtonOptions {
        juce::String text;
        std::function<void()> onClick;
        bool primary = false;
        bool dismissOnClick = true;
    };

    struct Options {
        juce::String closeButtonSvg;
        juce::String title = "Error";
        juce::String message;
        Icon icon = Icon::Error;
        juce::Point<int> preferredPanelSize { 420, 420 };
        juce::Justification messageJustification = juce::Justification::centred;
        std::vector<ButtonOptions> buttons;
        std::function<void()> onDismissed;
        bool dismissible = true;
    };

    explicit ErrorOverlay(Options optionsToUse)
        : OverlayComponent(std::move(optionsToUse.closeButtonSvg)),
          options(std::move(optionsToUse)) {
        setOverlayTitle(options.title);
        setDismissible(options.dismissible);

        iconComponent.setIcon(options.icon);
        addPanelContentAndMakeVisible(iconComponent);

        messageLabel.setText(options.message);
        messageLabel.setJustification(options.messageJustification);
        addPanelContentAndMakeVisible(messageLabel);

        if (options.buttons.empty()) {
            options.buttons.push_back({ "OK", {}, true });
        }

        for (auto buttonIndex = 0; buttonIndex < static_cast<int>(options.buttons.size()); ++buttonIndex) {
            auto button = std::make_unique<juce::TextButton>();
            button->setButtonText(options.buttons[static_cast<size_t>(buttonIndex)].text);
            styleButton(*button, options.buttons[static_cast<size_t>(buttonIndex)].primary);
            button->onClick = [this, buttonIndex] {
                triggerButton(buttonIndex);
            };

            addPanelContentAndMakeVisible(*button);
            buttons.push_back(std::move(button));
        }
    }

    static ErrorOverlay* show(juce::Component& parent, Options optionsToUse) {
        auto overlay = std::make_unique<ErrorOverlay>(std::move(optionsToUse));
        auto* rawOverlay = overlay.get();
        const juce::Component::SafePointer<ErrorOverlay> safeOverlay(rawOverlay);
        rawOverlay->onDismissRequested = [safeOverlay] {
            if (safeOverlay == nullptr) {
                return;
            }

            auto* overlayToDismiss = safeOverlay.getComponent();
            std::function<void()> callback;
            if (overlayToDismiss->pendingButtonCallback != nullptr) {
                callback = std::move(overlayToDismiss->pendingButtonCallback);
            } else if (!overlayToDismiss->dismissedByButton && overlayToDismiss->options.onDismissed != nullptr) {
                callback = std::move(overlayToDismiss->options.onDismissed);
            }

            if (callback != nullptr) {
                callback();
            }
        };

        OverlayComponent::show(parent, std::move(overlay));
        return rawOverlay;
    }

private:
    class IconComponent final : public juce::Component {
    public:
        IconComponent() = default;

        void setIcon(Icon iconToUse) {
            icon = iconToUse;
            setVisible(icon != Icon::None);
            repaint();
        }

        void paint(juce::Graphics& g) override {
            const auto bounds = getLocalBounds().toFloat().reduced(4.0f);
            if (bounds.isEmpty()) {
                return;
            }

            if (icon == Icon::Warning) {
                paintWarning(g, bounds);
                return;
            }

            if (icon == Icon::Error) {
                paintError(g, bounds);
            }
        }

    private:
        Icon icon = Icon::Error;

        static void paintWarning(juce::Graphics& g, juce::Rectangle<float> bounds) {
            juce::Path triangle;
            triangle.startNewSubPath(bounds.getCentreX(), bounds.getY());
            triangle.lineTo(bounds.getRight(), bounds.getBottom());
            triangle.lineTo(bounds.getX(), bounds.getBottom());
            triangle.closeSubPath();

            g.setColour(juce::Colour::fromRGB(0xd8, 0x6b, 0x4f).withAlpha(0.95f));
            g.fillPath(triangle);
            g.setColour(juce::Colours::white.withAlpha(0.92f));
            g.strokePath(triangle, juce::PathStrokeType(2.0f));

            const auto bar = bounds.withSizeKeepingCentre(bounds.getWidth() * 0.12f, bounds.getHeight() * 0.34f)
                                   .withY(bounds.getY() + bounds.getHeight() * 0.28f);
            g.fillRoundedRectangle(bar, 2.0f);

            const auto dotSize = bounds.getWidth() * 0.13f;
            g.fillEllipse(bounds.getCentreX() - dotSize * 0.5f,
                          bounds.getBottom() - bounds.getHeight() * 0.25f,
                          dotSize,
                          dotSize);
        }

        static void paintError(juce::Graphics& g, juce::Rectangle<float> bounds) {
            g.setColour(juce::Colour::fromRGB(0xd8, 0x5a, 0x5a).withAlpha(0.95f));
            g.fillEllipse(bounds);
            g.setColour(juce::Colours::white.withAlpha(0.92f));
            g.drawEllipse(bounds.reduced(1.0f), 2.0f);

            const auto stroke = juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            juce::Path cross;
            const auto inset = bounds.getWidth() * 0.32f;
            cross.startNewSubPath(bounds.getX() + inset, bounds.getY() + inset);
            cross.lineTo(bounds.getRight() - inset, bounds.getBottom() - inset);
            cross.startNewSubPath(bounds.getRight() - inset, bounds.getY() + inset);
            cross.lineTo(bounds.getX() + inset, bounds.getBottom() - inset);
            g.strokePath(cross, stroke);
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IconComponent)
    };

    class MessageLabel final : public juce::Component {
    public:
        MessageLabel() = default;

        void setText(juce::String textToUse) {
            text = std::move(textToUse);
            repaint();
        }

        void setJustification(juce::Justification justificationToUse) {
            justification = justificationToUse;
            repaint();
        }

        void paint(juce::Graphics& g) override {
            g.setColour(juce::Colours::white.withAlpha(0.86f));
            g.setFont(juce::Font(juce::FontOptions(17.0f, juce::Font::plain)));
            g.drawFittedText(text, getLocalBounds(), justification, 12);
        }

    private:
        juce::String text;
        juce::Justification justification = juce::Justification::centred;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MessageLabel)
    };

    Options options;
    IconComponent iconComponent;
    MessageLabel messageLabel;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
    std::function<void()> pendingButtonCallback;
    bool dismissedByButton = false;

    juce::Point<int> getPreferredPanelSize() const override {
        return options.preferredPanelSize;
    }

    void resizeContent(juce::Rectangle<int> area) override {
        if (iconComponent.isVisible()) {
            const auto iconSize = options.icon == Icon::Warning ? 58 : 62;
            iconComponent.setBounds(area.removeFromTop(iconSize + 8).withSizeKeepingCentre(iconSize, iconSize));
            area.removeFromTop(8);
        } else {
            iconComponent.setBounds({});
        }

        auto buttonArea = area.removeFromBottom(40);
        area.removeFromBottom(10);
        messageLabel.setBounds(area);

        const auto buttonWidth = 132;
        const auto buttonGap = 14;
        const auto totalButtonWidth = buttonWidth * static_cast<int>(buttons.size())
            + buttonGap * juce::jmax(0, static_cast<int>(buttons.size()) - 1);
        auto row = buttonArea.withSizeKeepingCentre(totalButtonWidth, 38);

        for (auto& button : buttons) {
            button->setBounds(row.removeFromLeft(buttonWidth));
            row.removeFromLeft(buttonGap);
        }
    }

    void triggerButton(int buttonIndex) {
        if (!juce::isPositiveAndBelow(buttonIndex, static_cast<int>(options.buttons.size()))) {
            return;
        }

        auto callback = options.buttons[static_cast<size_t>(buttonIndex)].onClick;
        const auto dismissOnClick = options.buttons[static_cast<size_t>(buttonIndex)].dismissOnClick;
        dismissedByButton = true;

        if (dismissOnClick) {
            for (auto& button : buttons) {
                button->setEnabled(false);
            }

            pendingButtonCallback = std::move(callback);
            requestDismiss();
            return;
        }

        if (callback != nullptr) {
            callback();
        }
    }

    static void styleButton(juce::TextButton& button, bool primary) {
        const auto accent = primary ? Colours::accentColor() : juce::Colours::white;
        button.setColour(juce::TextButton::buttonColourId,
                         primary ? accent.withAlpha(0.92f) : juce::Colours::white.withAlpha(0.08f));
        button.setColour(juce::TextButton::buttonOnColourId,
                         primary ? accent.brighter(0.12f) : juce::Colours::white.withAlpha(0.14f));
        button.setColour(juce::TextButton::textColourOffId,
                         primary ? juce::Colours::black.withAlpha(0.88f) : juce::Colours::white);
        button.setColour(juce::TextButton::textColourOnId,
                         primary ? juce::Colours::black.withAlpha(0.88f) : juce::Colours::white);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ErrorOverlay)
};

} // namespace osci
