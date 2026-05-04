#pragma once

namespace osci {

class OverlayLookAndFeel : public LookAndFeel {
public:
    OverlayLookAndFeel();

    static void applyOverlayColours (juce::LookAndFeel& lookAndFeel);

    static constexpr float containerCornerRadius = 8.0f;

    static juce::Colour containerBackground();
    static juce::Colour containerOutline();
    static juce::Colour buttonBackground();
    static juce::Colour buttonDownBackground();
    static juce::Colour tickBoxBackground();

    static void configureListBox (juce::ListBox& listBox);
    static void paintContainerBackground (juce::Graphics& g, juce::Rectangle<int> bounds);
    static void drawContainerOutline (juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour outlineColour);
    static void paintRowHighlight (juce::Component& owner, juce::Graphics& g, int width, int height);

    void drawComboBox (juce::Graphics& g,
                       int width,
                       int height,
                       bool,
                       int,
                       int,
                       int,
                       int,
                       juce::ComboBox& box) override;
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;
    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
    void drawTickBox (juce::Graphics& g,
                      juce::Component& component,
                      float x,
                      float y,
                      float w,
                      float h,
                      bool ticked,
                      bool isEnabled,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;
    void drawLevelMeter (juce::Graphics& g, int width, int height, float level) override;
    juce::Typeface::Ptr getTypefaceForFont (const juce::Font& font) override;
};

} // namespace osci
