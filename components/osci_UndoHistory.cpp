#include "osci_UndoHistory.h"

namespace osci {

void showUndoHistory(juce::UndoManager& undoManager, bool redo, juce::Component& anchor, std::function<void()> changed, int limit) {
    const auto steps = redo ? undoManager.getRedoDescriptions() : undoManager.getUndoDescriptions();
    if (steps.isEmpty()) { return; }
    const auto verb = juce::String(redo ? "Redo " : "Undo ");
    juce::PopupMenu menu;
    const auto shown = std::min(steps.size(), limit);
    for (int index = 0; index < shown; ++index) {
        const auto name = steps[index].isNotEmpty() ? steps[index] : juce::String("change");
        // The first is what Undo alone does; each further one goes back a step more.
        menu.addItem(index + 1, verb + name);
    }
    if (steps.size() > shown) {
        menu.addSeparator();
        menu.addItem(-1, juce::String(steps.size() - shown) + " older " + (steps.size() - shown == 1 ? "step" : "steps"), false);
    }
    // Styled as the button it opens from.
    menu.setLookAndFeel(&anchor.getLookAndFeel());
    juce::Component::SafePointer<juce::Component> safeAnchor(&anchor);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&anchor), [&undoManager, redo, changed, safeAnchor, steps](int result) {
        // Only the history the menu showed: an edit meanwhile changes what each choice means.
        if (result <= 0 || safeAnchor == nullptr || (redo ? undoManager.getRedoDescriptions() : undoManager.getUndoDescriptions()) != steps) { return; }
        for (int step = 0; step < result; ++step) {
            const auto moved = redo ? undoManager.redo() : undoManager.undo();
            if (!moved) { break; }
        }
        if (changed) { changed(); }
    });
}

} // namespace osci
