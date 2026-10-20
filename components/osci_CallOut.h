#pragma once

namespace osci {

// Whether a popup must be a window of its own to show over an OpenGL view.
// On Windows and Linux an OpenGL view is a native child window that paints
// over any component placed on top of it, so a popup inside the editor would
// be hidden where it overlaps one (the Scope, the visualiser).
constexpr bool popupsNeedOwnWindow() {
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
    return true;
#else
    return false;
#endif
}

// Opens `content` in a call-out box pointing at `area` of `host`, kept within
// `available` (in host's coordinates; the whole top-level window when empty).
// It sits in host's top-level component, or, where popupsNeedOwnWindow(), in
// a window of its own above it, styled as host is and closed with it.
// Message thread only.
juce::CallOutBox& launchCallOut(std::unique_ptr<juce::Component> content, juce::Component& host, juce::Rectangle<int> area, juce::Rectangle<int> available = {});

} // namespace osci
