#pragma once

namespace osci {

class ColourSwatchComponent : public juce::Component,
                              public juce::SettableTooltipClient,
                              private juce::Timer {
public:
    explicit ColourSwatchComponent(std::function<juce::Colour()> colourProvider, juce::String tooltip = {}, int refreshRateHz = 30);

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;
    void updateColour();

    std::function<juce::Colour()> colourProvider;
    juce::Colour currentColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourSwatchComponent)
};

} // namespace osci
