#pragma once

#include <functional>
#include <optional>

namespace osci {

// How osci::reveal brings something into view and marks it.
struct RevealOptions {
    // Scroll just far enough (nearest), put it near the top as a browser does
    // for an anchor (start; nothing moves if it is already in full view), or
    // in the middle of the view (centre).
    enum class Align { nearest, start, centre };
    Align align = Align::nearest;
    // Space kept between it and the viewport's edge once scrolled to.
    int margin = 12;
    double scrollMilliseconds = 280;
    // A ring that fades in round it, then out, once it is in view.
    bool highlight = true;
    double highlightMilliseconds = 1500;
    float cornerRadius = 4.0f;
    // The ring's colour; the accent unless given.
    std::optional<juce::Colour> colour;
    // Once there: keyboard focus, opening a label's editor or taking a text
    // editor's caret.
    bool focus = false;
    // Called once it is in view (after any scroll), for anything more.
    std::function<void()> onArrived;
};

// Brings a component into view the way a browser goes to an anchor: every
// viewport holding it scrolls there smoothly, then it is marked by a ring
// that fades away. A later reveal in the same viewport takes over, and
// scrolling by hand stops the animation. Safe if the component goes away.
// Message thread only.
void reveal(juce::Component& target, RevealOptions options = {});

// The same for a list's row, which a list makes only while it shows: the
// list scrolls to it, then the row is marked once it exists.
void revealRow(juce::ListBox& list, int row, RevealOptions options = {});

// Only the mark, for something already in view.
void highlight(juce::Component& target, RevealOptions options = {});

} // namespace osci
