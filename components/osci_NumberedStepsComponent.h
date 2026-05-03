#pragma once

namespace osci {

class NumberedStepsComponent final : public juce::Component {
public:
    NumberedStepsComponent() = default;

    void setSteps (juce::StringArray stepsToUse) {
        steps = std::move (stepsToUse);
        repaint();
    }

    void setTextColour (juce::Colour colour) {
        textColour = colour;
        repaint();
    }

    void setAccentColour (juce::Colour colour) {
        accentColour = colour;
        repaint();
    }

    void setFont (juce::Font fontToUse) {
        textFont = fontToUse;
        repaint();
    }

    void setMetrics (int rowHeightToUse, int rowGapToUse, int badgeSizeToUse) {
        rowHeight = rowHeightToUse;
        rowGap = rowGapToUse;
        badgeSize = badgeSizeToUse;
        repaint();
    }

    int getPreferredHeight() const {
        if (steps.isEmpty()) {
            return 0;
        }

        return steps.size() * rowHeight + (steps.size() - 1) * rowGap;
    }

    void paint (juce::Graphics& g) override {
        auto area = getLocalBounds();

        for (int index = 0; index < steps.size(); ++index) {
            auto row = area.removeFromTop (rowHeight);
            drawBadge (g, row.removeFromLeft (badgeSize), index + 1);
            row.removeFromLeft (badgeTextGap);

            g.setColour (textColour);
            g.setFont (textFont);
            g.drawFittedText (steps[index], row, juce::Justification::centredLeft, 1, 0.82f);

            area.removeFromTop (rowGap);
        }
    }

private:
    void drawBadge (juce::Graphics& g, juce::Rectangle<int> area, int number) const {
        auto badgeBounds = area.withSizeKeepingCentre (badgeSize, badgeSize).toFloat().reduced (1.0f);
        g.setColour (accentColour.withAlpha (0.14f));
        g.fillEllipse (badgeBounds);
        g.setColour (accentColour.withAlpha (0.82f));
        g.drawEllipse (badgeBounds, 1.0f);
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (juce::String (number), area, juce::Justification::centred);
    }

    juce::StringArray steps;
    juce::Colour textColour { juce::Colours::white.withAlpha (0.78f) };
    juce::Colour accentColour { Colours::accentColor() };
    juce::Font textFont { juce::FontOptions (13.0f) };
    int rowHeight = 26;
    int rowGap = 12;
    int badgeSize = 20;
    int badgeTextGap = 12;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumberedStepsComponent)
};

} // namespace osci
