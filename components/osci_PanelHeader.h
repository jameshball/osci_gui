#pragma once

namespace osci {

/** Shared horizontal header surface for workspace panels and toolbars. */
class PanelHeader : public juce::Component {
public:
    static constexpr int height = 30;
    static constexpr int panelGap = 3;

    explicit PanelHeader(juce::String text = {}) : text(std::move(text)) {
        setName(this->text);
        setInterceptsMouseClicks(false, false);
    }

    static void paintBackground(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour = Colours::veryDark()) {
        g.setColour(colour);
        g.fillRoundedRectangle(bounds, (float)osci::LookAndFeel::RECT_RADIUS);
    }

    juce::Rectangle<int> contentBounds() const { return getLocalBounds().reduced(8, 3); }

    void paint(juce::Graphics& g) override {
        paintBackground(g, getLocalBounds().toFloat());
        g.setColour(Colours::text());
        g.setFont(juce::Font(juce::FontOptions(15.0f)));
        g.drawText(text, contentBounds(), juce::Justification::centredLeft);
    }

private:
    juce::String text;
};

} // namespace osci
