#pragma once

namespace osci {

class FormFieldLabel final : public juce::Label {
public:
    enum ColourIds {
        requiredMarkerColourId = 0x20260100,
    };

    FormFieldLabel();
    FormFieldLabel(juce::String fieldName, bool required);

    void setField(juce::String fieldName, bool required);
    void setRequired(bool required);
    const juce::String& getFieldName() const;
    bool isRequired() const;

    void paint(juce::Graphics& g) override;

private:
    void updateAccessibleText();

    juce::String fieldName;
    bool required = false;
};

} // namespace osci
