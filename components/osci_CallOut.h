#pragma once

namespace osci {

// Opens `content` in a call-out box pointing at `area` of `host`, kept within
// `available` (in host's coordinates; the whole top-level window when empty),
// in a window of its own above host's: never painted over a visualiser, whose
// picture is a native surface (an OpenGL child window on Windows and Linux, a
// surface beneath the window on macOS) that painting over can't keep in step
// with. Styled as host is and closed with it. Message thread only.
juce::CallOutBox& launchCallOut(std::unique_ptr<juce::Component> content, juce::Component& host, juce::Rectangle<int> area, juce::Rectangle<int> available = {});

} // namespace osci
