#pragma once

namespace osci {

class CardComponent : public juce::Component {
public:
    struct Style {
        juce::Colour backgroundColour { Colours::surfaceRaised().withAlpha(0.42f) };
        juce::Colour outlineColour { Colours::neutralStroke(0.16f) };
        juce::Colour shadowColour { juce::Colours::transparentBlack };
        juce::Point<int> shadowOffset;
        float cornerRadius = 14.0f;
        float outlineWidth = 1.0f;
        int shadowRadius = 0;
    };

    CardComponent() = default;
    explicit CardComponent(Style style);

    void setStyle(Style style);
    const Style& getStyle() const;
    void setPaintsCard(bool shouldPaint);
    bool getPaintsCard() const;

    void paint(juce::Graphics& g) override;
    static void drawCard(juce::Graphics& g, juce::Rectangle<float> bounds, const Style& style);

private:
    Style style;
    bool paintsCard = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CardComponent)
};

} // namespace osci
