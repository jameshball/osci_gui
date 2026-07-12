#include "osci_FormFieldLabel.h"

namespace osci {

FormFieldLabel::FormFieldLabel() {
    setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    setColour(juce::Label::textColourId, Colours::text());
    setColour(requiredMarkerColourId, Colours::danger());
    setJustificationType(juce::Justification::centredLeft);
    setInterceptsMouseClicks(false, false);
}

FormFieldLabel::FormFieldLabel(juce::String name, bool isRequired)
    : FormFieldLabel() {
    setField(std::move(name), isRequired);
}

void FormFieldLabel::setField(juce::String name, bool isRequired) {
    fieldName = std::move(name);
    required = isRequired;
    updateAccessibleText();
    repaint();
}

void FormFieldLabel::setRequired(bool shouldBeRequired) {
    if (required == shouldBeRequired) {
        return;
    }
    required = shouldBeRequired;
    updateAccessibleText();
    repaint();
}

const juce::String& FormFieldLabel::getFieldName() const {
    return fieldName;
}

bool FormFieldLabel::isRequired() const {
    return required;
}

void FormFieldLabel::paint(juce::Graphics& g) {
    const auto font = getFont();
    const auto bounds = getLocalBounds();
    g.setFont(font);
    g.setColour(findColour(juce::Label::textColourId));
    g.drawText(fieldName, bounds, juce::Justification::centredLeft, false);

    if (required) {
        const auto markerOffset = juce::roundToInt(juce::GlyphArrangement::getStringWidth(font, fieldName)) + 4;
        g.setColour(findColour(requiredMarkerColourId));
        g.drawText("*", bounds.withTrimmedLeft(markerOffset), juce::Justification::centredLeft, false);
    }
}

void FormFieldLabel::updateAccessibleText() {
    const auto accessibleText = fieldName + (required ? " (required)" : juce::String());
    setText(accessibleText, juce::dontSendNotification);
    setName(accessibleText);
}

} // namespace osci
