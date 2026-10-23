#pragma once

namespace osci {

// A key's code for matching shortcut letters: a-z read as A-Z, since Linux
// reports Ctrl with a letter as the lowercase letter. Nothing else changes, so
// a key code like Windows' F4 (0x10073) is never cut to a character and read
// as a letter.
inline int shortcutKey(const juce::KeyPress& key) {
    const auto code = key.getKeyCode();
    return code >= 'a' && code <= 'z' ? code - 'a' + 'A' : code;
}

} // namespace osci
