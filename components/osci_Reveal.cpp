#include "osci_Reveal.h"

namespace osci {
namespace {

double now() { return juce::Time::getMillisecondCounterHiRes(); }

double easeOutCubic(double t) {
    t = juce::jlimit(0.0, 1.0, t);
    return 1.0 - std::pow(1.0 - t, 3.0);
}

// The visible part of an area of a component, in `top`'s coordinates: cut
// by the component's bounds and every ancestor's, so a mark never draws over
// a header it scrolled under.
juce::Rectangle<int> visibleArea(juce::Component& target, juce::Rectangle<int> local, juce::Component& top) {
    auto area = top.getLocalArea(&target, local.getIntersection(target.getLocalBounds()));
    for (auto* parent = target.getParentComponent(); parent != nullptr && parent != &top; parent = parent->getParentComponent()) {
        area = area.getIntersection(top.getLocalArea(parent, parent->getLocalBounds()));
    }
    return area;
}

juce::Viewport* enclosingViewport(juce::Component& component) {
    for (auto* parent = component.getParentComponent(); parent != nullptr; parent = parent->getParentComponent()) {
        auto* viewport = dynamic_cast<juce::Viewport*>(parent);
        if (viewport != nullptr && viewport->getViewedComponent() != nullptr) { return viewport; }
    }
    return nullptr;
}

void focusOn(juce::Component& target) {
    auto* label = dynamic_cast<juce::Label*>(&target);
    if (label != nullptr && (label->isEditableOnSingleClick() || label->isEditableOnDoubleClick())) {
        label->showEditor();
        return;
    }
    if (target.getWantsKeyboardFocus()) { target.grabKeyboardFocus(); }
}

// A ring round an area of the target (all of it, or a list's row), drawn
// over its window: it settles on as it fades in, holds, then fades away,
// following the target as it moves. It owns itself and goes once done or
// once the target does.
class Mark final : public juce::Component, private juce::Timer {
public:
    Mark(juce::Component& target, std::function<juce::Rectangle<int>()> area, juce::Component& top, const RevealOptions& options)
        : target(&target), area(std::move(area)), colour(options.colour.value_or(Colours::accentColor())), radius(options.cornerRadius),
          duration(juce::jmax(200.0, options.highlightMilliseconds)), start(now()) {
        setInterceptsMouseClicks(false, false);
        setWantsKeyboardFocus(false);
        setAccessible(false);
        setAlwaysOnTop(true);
        top.addAndMakeVisible(this);
        follow();
        startTimerHz(60);
    }

    void paint(juce::Graphics& g) override {
        const auto progress = (now() - start) / duration;
        // In over the first tenth, held to halfway, then out.
        const auto in = easeOutCubic(progress / 0.1);
        const auto out = progress < 0.5 ? 1.0 : 1.0 - easeOutCubic((progress - 0.5) / 0.5);
        const auto alpha = static_cast<float>(in * out);
        if (alpha <= 0.0f) { return; }
        g.reduceClipRegion(clip);
        // Settles in from a little outside, as a browser's find mark does.
        const auto spread = static_cast<float>((1.0 - in) * 6.0);
        // Clear of the target's own outline (a focused field's), so the two read apart.
        const auto box = ring.toFloat().expanded(3.0f + spread);
        g.setColour(colour.withAlpha(0.12f * alpha));
        g.fillRoundedRectangle(ring.toFloat(), radius);
        for (int halo = 1; halo <= 4; ++halo) {
            g.setColour(colour.withAlpha(0.16f * alpha / static_cast<float>(halo)));
            g.drawRoundedRectangle(box.expanded(static_cast<float>(halo)), radius + 3.0f + static_cast<float>(halo), 1.0f);
        }
        g.setColour(colour.withAlpha(0.95f * alpha));
        g.drawRoundedRectangle(box, radius + 3.0f, 2.0f);
    }

private:
    static constexpr int reach = 12;

    void timerCallback() override {
        auto* top = getParentComponent();
        if (target == nullptr || top == nullptr || !target->isShowing() || now() - start > duration) {
            stopTimer();
            // Not from inside its own callback.
            juce::MessageManager::callAsync([self = juce::Component::SafePointer<Mark>(this)] { delete self.getComponent(); });
            return;
        }
        follow();
        repaint();
    }

    void follow() {
        auto* top = getParentComponent();
        if (target == nullptr || top == nullptr) { return; }
        const auto local = area != nullptr ? area() : target->getLocalBounds();
        const auto bounds = top->getLocalArea(target.getComponent(), local);
        const auto visible = visibleArea(*target, local, *top);
        const auto next = bounds.expanded(reach);
        if (next != getBounds()) { setBounds(next); }
        toFront(false);
        ring = bounds - next.getPosition();
        // The ring may stand out past the target, but not past what holds it.
        clip = visible.isEmpty() ? juce::Rectangle<int>() : visible.expanded(reach).getIntersection(top->getLocalBounds()) - next.getPosition();
    }

    juce::Component::SafePointer<juce::Component> target;
    std::function<juce::Rectangle<int>()> area;
    juce::Colour colour;
    float radius;
    double duration, start;
    juce::Rectangle<int> ring, clip;
};

// Over the window's content, never among a window's own parts (its title
// bar, its resizer).
void mark(juce::Component& target, std::function<juce::Rectangle<int>()> area, const RevealOptions& options) {
    auto* top = &target;
    while (top->getParentComponent() != nullptr && dynamic_cast<juce::ResizableWindow*>(top->getParentComponent()) == nullptr) { top = top->getParentComponent(); }
    if (top == &target || !target.isShowing()) { return; }
    new Mark(target, std::move(area), *top, options);
}

// One reveal's scrolling: each viewport, innermost first, eased from where
// it was to where it shows the target, then the mark. A newer reveal in the
// same viewport takes over; a viewport scrolled by hand meanwhile stops.
class Scroll final : private juce::Timer {
public:
    struct Step {
        juce::Component::SafePointer<juce::Viewport> viewport;
        juce::Point<int> from, to, last;
        bool stopped = false;
    };

    // What is marked once there: a component, and the area of it (all of it
    // when there is none).
    struct Target {
        std::function<juce::Component*()> component;
        std::function<juce::Rectangle<int>()> area;
    };

    static void begin(std::vector<Step> steps, RevealOptions options, Target target) {
        auto& running = active();
        // Any reveal moving one of these viewports gives way.
        for (auto it = running.begin(); it != running.end();) {
            const auto shares = std::any_of((*it)->steps.begin(), (*it)->steps.end(), [&](const Step& step) {
                return std::any_of(steps.begin(), steps.end(), [&](const Step& next) { return next.viewport == step.viewport; });
            });
            it = shares ? running.erase(it) : std::next(it);
        }
        const auto moves = std::any_of(steps.begin(), steps.end(), [](const Step& step) { return step.from != step.to; });
        if (!moves || options.scrollMilliseconds <= 0) {
            for (auto& step : steps) {
                if (step.viewport != nullptr) { step.viewport->setViewPosition(step.to); }
            }
            arrive(options, target);
            return;
        }
        running.push_back(std::unique_ptr<Scroll>(new Scroll(std::move(steps), std::move(options), std::move(target))));
    }

private:
    Scroll(std::vector<Step> steps, RevealOptions options, Target target)
        : steps(std::move(steps)), options(std::move(options)), target(std::move(target)), start(now()) {
        for (auto& step : this->steps) { step.last = step.from; }
        startTimerHz(60);
    }

    static std::vector<std::unique_ptr<Scroll>>& active() {
        static std::vector<std::unique_ptr<Scroll>> running;
        return running;
    }

    static void arrive(const RevealOptions& options, const Target& target) {
        auto* component = target.component != nullptr ? target.component() : nullptr;
        if (component != nullptr) {
            if (options.focus) { focusOn(*component); }
            if (options.highlight) { mark(*component, target.area, options); }
        }
        if (options.onArrived) { options.onArrived(); }
    }

    void timerCallback() override {
        const auto t = easeOutCubic((now() - start) / options.scrollMilliseconds);
        for (auto& step : steps) {
            if (step.viewport == nullptr || step.stopped) { continue; }
            if (step.viewport->getViewPosition() != step.last) {
                step.stopped = true;
                continue;
            }
            const auto position = step.from + ((step.to - step.from).toDouble() * t).roundToInt();
            step.viewport->setViewPosition(position);
            step.last = step.viewport->getViewPosition();
        }
        if (t < 1.0) { return; }
        stopTimer();
        const auto stoppedByHand = std::any_of(steps.begin(), steps.end(), [](const Step& step) { return step.stopped; });
        const auto finishedOptions = options;
        const auto finishedTarget = target;
        // Removing this destroys it: nothing of it is used after.
        auto& running = active();
        for (auto it = running.begin(); it != running.end(); ++it) {
            if (it->get() == this) {
                auto self = std::move(*it);
                running.erase(it);
                if (!stoppedByHand) { arrive(finishedOptions, finishedTarget); }
                return;
            }
        }
    }

    std::vector<Step> steps;
    RevealOptions options;
    Target target;
    double start;
};

// Where a viewport must be to show `area` (in its viewed component's
// coordinates), as the options ask.
juce::Point<int> positionShowing(juce::Viewport& viewport, juce::Rectangle<int> area, const RevealOptions& options) {
    const auto* viewed = viewport.getViewedComponent();
    const auto width = viewport.getMaximumVisibleWidth(), height = viewport.getMaximumVisibleHeight();
    auto position = viewport.getViewPosition();
    const auto axis = [&](int current, int low, int high, int visible) {
        if (options.align == RevealOptions::Align::centre) { return (low + high - visible) / 2; }
        const auto shown = low - options.margin >= current && high + options.margin <= current + visible;
        if (options.align == RevealOptions::Align::start) { return shown ? current : low - options.margin; }
        if (high - low + 2 * options.margin > visible || low - options.margin < current) { return low - options.margin; }
        if (high + options.margin > current + visible) { return high + options.margin - visible; }
        return current;
    };
    position.x = axis(position.x, area.getX(), area.getRight(), width);
    position.y = axis(position.y, area.getY(), area.getBottom(), height);
    position.x = juce::jlimit(0, juce::jmax(0, viewed->getWidth() - width), position.x);
    position.y = juce::jlimit(0, juce::jmax(0, viewed->getHeight() - height), position.y);
    return position;
}

// From the innermost viewport out: each scrolled to show the area, the next
// to show where the area will then be inside the one before.
std::vector<Scroll::Step> stepsShowing(juce::Viewport* viewport, juce::Rectangle<int> area, const RevealOptions& options) {
    std::vector<Scroll::Step> steps;
    while (viewport != nullptr) {
        const auto to = positionShowing(*viewport, area, options);
        steps.push_back({viewport, viewport->getViewPosition(), to, {}});
        // Where it will sit in this viewport, as the next one out sees it.
        const auto inViewport = (area - to).getIntersection(viewport->getLocalBounds());
        auto* outer = enclosingViewport(*viewport);
        if (outer == nullptr) { break; }
        area = outer->getViewedComponent()->getLocalArea(viewport, inViewport.isEmpty() ? viewport->getLocalBounds() : inViewport);
        viewport = outer;
    }
    return steps;
}

} // namespace

void reveal(juce::Component& target, RevealOptions options) {
    auto* viewport = enclosingViewport(target);
    const juce::Component::SafePointer<juce::Component> safe(&target);
    const auto area = viewport != nullptr ? viewport->getViewedComponent()->getLocalArea(&target, target.getLocalBounds()) : juce::Rectangle<int>();
    Scroll::begin(viewport != nullptr ? stepsShowing(viewport, area, options) : std::vector<Scroll::Step>{}, std::move(options), {[safe] { return safe.getComponent(); }, nullptr});
}

void revealRow(juce::ListBox& list, int row, RevealOptions options) {
    if (row < 0 || row >= list.getListBoxModel()->getNumRows()) { return; }
    auto* viewport = list.getViewport();
    const juce::Component::SafePointer<juce::ListBox> safe(&list);
    // The row's place among the list's rows, scrolled or not.
    const juce::Rectangle<int> area(0, row * list.getRowHeight(), viewport->getViewedComponent()->getWidth(), list.getRowHeight());
    // The row is marked as an area of the list: a list may paint its rows
    // rather than make components of them.
    Scroll::begin(stepsShowing(viewport, area, options), std::move(options), {[safe] { return static_cast<juce::Component*>(safe.getComponent()); }, [safe, row] {
        return safe != nullptr ? safe->getRowPosition(row, true).getIntersection(safe->getViewport()->getBounds()) : juce::Rectangle<int>();
    }});
}

void highlight(juce::Component& target, RevealOptions options) {
    mark(target, nullptr, options);
}

} // namespace osci
