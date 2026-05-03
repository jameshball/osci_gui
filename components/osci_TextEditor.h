#pragma once

namespace osci {

class TextEditor : public juce::TextEditor {
public:
    TextEditor() = default;

    explicit TextEditor (const juce::String& componentName)
        : juce::TextEditor (componentName) {
    }

    bool keyPressed (const juce::KeyPress& key) override {
        if (key == juce::KeyPress::escapeKey && ! onEscapeKey) {
            giveAwayKeyboardFocus();
            return true;
        }

        return juce::TextEditor::keyPressed (key);
    }

    void resized() override {
        updateSingleLineVerticalIndent();
        juce::TextEditor::resized();
    }

private:
    void updateSingleLineVerticalIndent() {
        if (updatingIndent) {
            return;
        }

        if (isMultiLine()) {
            if (lastCentredTopIndent >= 0 && getTopIndent() == lastCentredTopIndent) {
                const juce::ScopedValueSetter<bool> scopedSetter (updatingIndent, true);
                setIndents (getLeftIndent(), 4);
                lastCentredTopIndent = -1;
            }

            return;
        }

        const auto usableHeight = getHeight() - getBorder().getTopAndBottom();
        if (usableHeight <= 0) {
            return;
        }

        const auto centredTopIndent = juce::jmax (0, juce::roundToInt ((usableHeight - getFont().getHeight()) * 0.5f));
        if (getTopIndent() == centredTopIndent) {
            return;
        }

        const juce::ScopedValueSetter<bool> scopedSetter (updatingIndent, true);
        setIndents (getLeftIndent(), centredTopIndent);
        lastCentredTopIndent = centredTopIndent;
    }

    bool updatingIndent = false;
    int lastCentredTopIndent = -1;
};

} // namespace osci
