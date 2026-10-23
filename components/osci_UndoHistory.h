#pragma once

#include <functional>

namespace osci {

// The steps Undo (or Redo) would take, newest first, as a menu by `anchor`:
// choosing one takes back (or redoes) every step down to it. `changed` runs
// after a choice. Shows at most `limit` steps. The undo manager must outlive
// the menu (as a processor's does its editor's).
void showUndoHistory(juce::UndoManager& undoManager, bool redo, juce::Component& anchor, std::function<void()> changed = {}, int limit = 100);

} // namespace osci
