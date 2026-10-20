#include "osci_UndoRedoComponent.h"

namespace osci {

UndoRedoComponent::UndoRedoComponent(juce::UndoManager& undoManagerToUse,
                                     juce::String undoSvg,
                                     juce::String redoSvg)
    : undoManager(undoManagerToUse),
      undoButton("Undo", std::move(undoSvg), Colours::text()),
      redoButton("Redo", std::move(redoSvg), Colours::text()) {
    addAndMakeVisible(actionLabel);
    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);

    actionLabel.setJustificationType(juce::Justification::centredRight);
    actionLabel.setFont(juce::Font(juce::FontOptions(12.0f)));

    undoButton.onClick = [this] { undo(); };
    redoButton.onClick = [this] { redo(); };
    undoButton.onPopupMenu = [this] { showUndoHistory(undoManager, false, undoButton, [this] { refresh(); }); };
    redoButton.onPopupMenu = [this] { showUndoHistory(undoManager, true, redoButton, [this] { refresh(); }); };

    lookAndFeelChanged();
    refresh();
    startTimer(100);
}

UndoRedoComponent::~UndoRedoComponent() {
    stopTimer();
}

void UndoRedoComponent::undo() {
    undoManager.undo();
    refresh();
}

void UndoRedoComponent::redo() {
    undoManager.redo();
    refresh();
}

void UndoRedoComponent::refresh() {
    undoButton.setEnabled(undoManager.canUndo());
    redoButton.setEnabled(undoManager.canRedo());

    auto undoDesc = undoManager.getUndoDescription();
    actionLabel.setText(undoDesc.isNotEmpty() ? "Undo " + undoDesc : "", juce::dontSendNotification);

    // Right-click lists the steps, so several go at once.
    undoButton.setTooltip((undoDesc.isNotEmpty() ? "Undo " + undoDesc : juce::String("Undo")) + (undoManager.canUndo() ? " (right-click for history)" : ""));
    auto redoDesc = undoManager.getRedoDescription();
    redoButton.setTooltip((redoDesc.isNotEmpty() ? "Redo " + redoDesc : juce::String("Redo")) + (undoManager.canRedo() ? " (right-click for history)" : ""));
}

int UndoRedoComponent::getPreferredWidth() const {
    return 200;
}

void UndoRedoComponent::resized() {
    auto area = getLocalBounds();

    redoButton.setBounds(area.removeFromRight(juce::jmin(25, area.getWidth())).reduced(2, 2));
    undoButton.setBounds(area.removeFromRight(juce::jmin(25, area.getWidth())).reduced(2, 2));
    actionLabel.setBounds(area.reduced(2, 2));
}

void UndoRedoComponent::lookAndFeelChanged() {
    const auto textColour = Colours::text();
    actionLabel.setColour(juce::Label::textColourId, textColour.withAlpha(0.7f));
    undoButton.setColours(textColour, textColour);
    redoButton.setColours(textColour, textColour);
}

void UndoRedoComponent::timerCallback() {
    refresh();
}

} // namespace osci
