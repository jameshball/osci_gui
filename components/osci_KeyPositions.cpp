// Included last in osci_gui.cpp: the platform headers here would otherwise
// leak their macros and global names into the rest of the module.
#if JUCE_MAC
 #include <ApplicationServices/ApplicationServices.h>
#elif JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>
#endif

namespace osci {

namespace {
juce::juce_wchar usLetter(KeyPosition position) {
    switch (position) {
        case KeyPosition::w: return 'W';
        case KeyPosition::a: return 'A';
        case KeyPosition::s: return 'S';
        case KeyPosition::d: return 'D';
        case KeyPosition::q: return 'Q';
        case KeyPosition::e: return 'E';
    }
    return 0;
}
}

juce::juce_wchar keyAtPosition(KeyPosition position) {
    juce::juce_wchar typed = 0;
#if JUCE_MAC
    // The virtual key codes are positions (kVK_ANSI_W is where W sits on a US board).
    const auto code = [position]() -> CGKeyCode {
        switch (position) {
            case KeyPosition::w: return 13;
            case KeyPosition::a: return 0;
            case KeyPosition::s: return 1;
            case KeyPosition::d: return 2;
            case KeyPosition::q: return 12;
            case KeyPosition::e: return 14;
        }
        return 13;
    }();
    // An event made but never posted asks the current layout what the key types.
    if (auto event = CGEventCreateKeyboardEvent(nullptr, code, true)) {
        UniChar text[4] {};
        UniCharCount length = 0;
        CGEventKeyboardGetUnicodeString(event, 4, &length, text);
        CFRelease(event);
        if (length == 1) { typed = static_cast<juce::juce_wchar>(text[0]); }
    }
#elif JUCE_WINDOWS
    // Scan codes are positions; the layout maps them to keys and characters.
    const auto scanCode = [position]() -> UINT {
        switch (position) {
            case KeyPosition::w: return 0x11;
            case KeyPosition::a: return 0x1e;
            case KeyPosition::s: return 0x1f;
            case KeyPosition::d: return 0x20;
            case KeyPosition::q: return 0x10;
            case KeyPosition::e: return 0x12;
        }
        return 0x11;
    }();
    const auto virtualKey = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK);
    typed = static_cast<juce::juce_wchar>(MapVirtualKeyW(virtualKey, MAPVK_VK_TO_CHAR) & 0x7fff);
#endif
    // Only a letter is usable as a held key; anything else (or another
    // platform) falls back to the US letter.
    return juce::CharacterFunctions::isLetter(typed) ? juce::CharacterFunctions::toUpperCase(typed) : usLetter(position);
}

bool isKeyPositionDown(KeyPosition position) {
    return juce::KeyPress::isKeyCurrentlyDown(static_cast<int>(keyAtPosition(position)));
}

} // namespace osci
