#pragma once

namespace osci {

class UndoRedoComponent final : public juce::Component,
                                private juce::Timer {
public:
    UndoRedoComponent(juce::UndoManager& undoManager,
                      juce::String undoSvg,
                      juce::String redoSvg);
    ~UndoRedoComponent() override;

    void undo();
    void redo();
    void refresh();
    int getPreferredWidth() const;

    void resized() override;
    void lookAndFeelChanged() override;

private:
    void timerCallback() override;

    juce::UndoManager& undoManager;
    // Right-clicked, each lists the steps it would take, to go back (or on) several at once.
    WithPopupMenu<SvgButton> undoButton;
    WithPopupMenu<SvgButton> redoButton;
    juce::Label actionLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UndoRedoComponent)
};

} // namespace osci
