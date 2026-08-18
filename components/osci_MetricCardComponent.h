#pragma once

namespace osci {

class MetricCardComponent final : public CardComponent {
public:
    MetricCardComponent(juce::String title, juce::String componentId = {});

    void setValue(juce::String value, juce::String detail, juce::Colour accent = Colours::accentColor());
    void paint(juce::Graphics& graphics) override;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

private:
    juce::String title;
    juce::String value;
    juce::String detail;
    juce::Colour accent {Colours::accentColor()};
};

} // namespace osci
