#pragma once

namespace osci {

class NotificationComponent final : public juce::Component {
public:
    explicit NotificationComponent(juce::String closeButtonSvg);

    std::function<void()> onDismiss;

    void setNotification(juce::String title, juce::String message);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Rectangle<int> getCardBounds() const;

    juce::Label titleLabel;
    juce::Label messageLabel;
    CloseButton closeButton;
    melatonin::DropShadow shadow {
        { juce::Colours::black.withAlpha(0.52f), 14, { 0, 5 }, 0 },
        { juce::Colours::black.withAlpha(0.32f), 5, { 0, 1 }, 0 }
    };

    static constexpr int shadowPadding = 8;
    static constexpr float cornerRadius = 8.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotificationComponent)
};

} // namespace osci
