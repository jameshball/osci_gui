#include "osci_OverlayLookAndFeel.h"

namespace osci {

OverlayLookAndFeel::OverlayLookAndFeel()
    : LookAndFeel (TypefaceData {}, false) {
    applyOverlayColours (*this);
}

void OverlayLookAndFeel::applyOverlayColours (juce::LookAndFeel& lookAndFeel) {
    lookAndFeel.setColour (juce::TextButton::buttonColourId, buttonBackground());
    lookAndFeel.setColour (juce::TextButton::buttonOnColourId, buttonDownBackground());
    lookAndFeel.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    lookAndFeel.setColour (juce::TextButton::textColourOnId, juce::Colours::white);

    lookAndFeel.setColour (juce::ComboBox::backgroundColourId, buttonBackground());
    lookAndFeel.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    lookAndFeel.setColour (juce::ComboBox::arrowColourId, juce::Colours::white);

    lookAndFeel.setColour (juce::TextEditor::backgroundColourId, buttonBackground());
    lookAndFeel.setColour (juce::TextEditor::outlineColourId, containerOutline().withMultipliedAlpha (0.72f));
    lookAndFeel.setColour (juce::TextEditor::focusedOutlineColourId, Colours::accentColor().withAlpha (0.9f));
    lookAndFeel.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    lookAndFeel.setColour (juce::TextEditor::highlightColourId, Colours::accentColor());
    lookAndFeel.setColour (juce::CaretComponent::caretColourId, juce::Colours::white);

    lookAndFeel.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::ListBox::outlineColourId, containerOutline());
    lookAndFeel.setColour (juce::ListBox::textColourId, juce::Colours::white);

    lookAndFeel.setColour (juce::ToggleButton::tickColourId, juce::Colours::white);
    lookAndFeel.setColour (juce::ScrollBar::thumbColourId, juce::Colours::white.withAlpha (0.76f));
    lookAndFeel.setColour (juce::ScrollBar::trackColourId, juce::Colours::transparentBlack);
}

juce::Colour OverlayLookAndFeel::containerBackground() {
    return Colours::darkerer().darker (0.08f);
}

juce::Colour OverlayLookAndFeel::containerOutline() {
    return Colours::grey().withAlpha (0.56f);
}

juce::Colour OverlayLookAndFeel::buttonBackground() {
    return Colours::darkerer();
}

juce::Colour OverlayLookAndFeel::buttonDownBackground() {
    return Colours::darker();
}

juce::Colour OverlayLookAndFeel::tickBoxBackground() {
    return Colours::evenDarker();
}

void OverlayLookAndFeel::configureListBox (juce::ListBox& listBox) {
    listBox.setOutlineThickness (1);
    listBox.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    listBox.setColour (juce::ListBox::outlineColourId, containerOutline());
    listBox.setColour (juce::ListBox::textColourId, juce::Colours::white);
}

void OverlayLookAndFeel::paintContainerBackground (juce::Graphics& g, juce::Rectangle<int> bounds) {
    g.setColour (containerBackground());
    g.fillRoundedRectangle (bounds.toFloat().reduced (0.5f), containerCornerRadius);
}

void OverlayLookAndFeel::drawContainerOutline (juce::Graphics& g,
                                               juce::Rectangle<int> bounds,
                                               juce::Colour outlineColour) {
    g.setColour (outlineColour);
    g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), containerCornerRadius, 1.0f);
}

void OverlayLookAndFeel::paintRowHighlight (juce::Component& owner, juce::Graphics& g, int width, int height) {
    const auto rowBounds = juce::Rectangle<float> (0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
                               .reduced (2.0f, 1.0f);

    g.setColour (owner.findColour (juce::TextEditor::highlightColourId, true).withMultipliedAlpha (0.18f));
    g.fillRoundedRectangle (rowBounds, juce::jmin (containerCornerRadius, rowBounds.getHeight() * 0.5f));
}

void OverlayLookAndFeel::drawComboBox (juce::Graphics& g,
                                       int width,
                                       int height,
                                       bool,
                                       int,
                                       int,
                                       int,
                                       int,
                                       juce::ComboBox& box) {
    const juce::Rectangle<float> bounds (0.5f, 0.5f, static_cast<float>(width) - 1.0f, static_cast<float>(height) - 1.0f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, containerCornerRadius);

    const juce::Rectangle<float> arrowZone (static_cast<float>(width - 19), 0.0f, 12.0f, static_cast<float>(height));
    juce::Path arrow;
    arrow.startNewSubPath (arrowZone.getX(), arrowZone.getCentreY() - 3.0f);
    arrow.lineTo (arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);
    arrow.lineTo (arrowZone.getRight(), arrowZone.getCentreY() - 3.0f);
    arrow.closeSubPath();

    g.setColour (box.findColour (juce::ComboBox::arrowColourId).withAlpha (box.isEnabled() ? 1.0f : 0.5f));
    g.fillPath (arrow);
}

void OverlayLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label) {
    label.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    label.setBounds (10, 1, juce::jmax (1, box.getWidth() - 34), box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void OverlayLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown) {
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const auto baseColour = LookAndFeelHelpers::createBaseColour (backgroundColour,
                                                                  button.hasKeyboardFocus (true),
                                                                  shouldDrawButtonAsHighlighted,
                                                                  shouldDrawButtonAsDown,
                                                                  button.isEnabled());
    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, containerCornerRadius);
}

void OverlayLookAndFeel::drawTickBox (juce::Graphics& g,
                                      juce::Component& component,
                                      float x,
                                      float y,
                                      float w,
                                      float h,
                                      bool ticked,
                                      bool isEnabled,
                                      bool,
                                      bool) {
    const juce::Rectangle<float> tickBounds (x, y, w, h);

    g.setColour (tickBoxBackground().withMultipliedAlpha (isEnabled ? 1.0f : 0.55f));
    g.fillRoundedRectangle (tickBounds, Colours::kPillRadius);

    if (ticked) {
        g.setColour (component.findColour (juce::ToggleButton::tickColourId).withMultipliedAlpha (isEnabled ? 1.0f : 0.55f));
        auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4.0f, 5.0f), false));
    }
}

void OverlayLookAndFeel::drawLevelMeter (juce::Graphics& g, int width, int height, float level) {
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced (0.5f);

    g.setColour (buttonBackground());
    g.fillRoundedRectangle (bounds, containerCornerRadius);

    const auto meterArea = bounds.reduced (4.0f, 4.0f);
    constexpr int numBlocks = 7;
    const auto blockGap = 2.0f;
    const auto blockWidth = (meterArea.getWidth() - blockGap * static_cast<float>(numBlocks - 1))
                          / static_cast<float>(numBlocks);
    const auto activeBlocks = juce::roundToInt (juce::jlimit (0.0f, 1.0f, level) * static_cast<float>(numBlocks));

    for (int i = 0; i < numBlocks; ++i) {
        const auto block = meterArea.withX (meterArea.getX() + static_cast<float>(i) * (blockWidth + blockGap))
                                    .withWidth (blockWidth);
        const auto blockColour = i < activeBlocks
                               ? Colours::accentColor().withAlpha (0.76f)
                               : juce::Colours::white.withAlpha (0.05f);

        g.setColour (blockColour);
        g.fillRoundedRectangle (block, 2.0f);
    }

    drawContainerOutline (g, juce::Rectangle<int> (0, 0, width, height), containerOutline().withMultipliedAlpha (0.72f));
}

juce::Typeface::Ptr OverlayLookAndFeel::getTypefaceForFont (const juce::Font& font) {
    auto& defaultLookAndFeel = juce::LookAndFeel::getDefaultLookAndFeel();
    if (&defaultLookAndFeel != this) {
        return defaultLookAndFeel.getTypefaceForFont (font);
    }

    return LookAndFeel::getTypefaceForFont (font);
}

} // namespace osci
