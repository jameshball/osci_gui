#include "osci_CallOut.h"

namespace osci {

namespace {
// A call-out in a window of its own goes when what it points at does (and
// with it the editor whose look it borrowed), or when its window hides.
class CallOutOwner final : private juce::ComponentListener {
public:
    CallOutOwner(juce::CallOutBox& callOut, juce::Component& anchor) : box(&callOut), host(&anchor), top(anchor.getTopLevelComponent()) {
        for (auto* watched : std::initializer_list<juce::Component*> {box.getComponent(), host.getComponent(), top.getComponent()}) { watched->addComponentListener(this); }
    }

private:
    void componentBeingDeleted(juce::Component& component) override {
        if (&component != box.getComponent()) { close(); }
        for (auto* watched : std::initializer_list<juce::Component*> {box.getComponent(), host.getComponent(), top.getComponent()}) {
            if (watched != nullptr) { watched->removeComponentListener(this); }
        }
        delete this;
    }
    void componentVisibilityChanged(juce::Component& component) override {
        // A window hidden (minimised, closed to the tray) takes its call-out with it.
        if (&component == top.getComponent() && !component.isShowing()) { close(); }
    }
    void close() {
        if (box == nullptr) { return; }
        box->setLookAndFeel(nullptr);
        box->dismiss();
    }

    juce::Component::SafePointer<juce::CallOutBox> box;
    juce::Component::SafePointer<juce::Component> host, top;
};
}

juce::CallOutBox& launchCallOut(std::unique_ptr<juce::Component> content, juce::Component& host, juce::Rectangle<int> area, juce::Rectangle<int> available) {
    auto* top = host.getTopLevelComponent();
    if (popupsNeedOwnWindow()) {
        const auto screenArea = host.localAreaToGlobal(area);
        auto& box = juce::CallOutBox::launchAsynchronously(std::move(content), screenArea, nullptr);
        box.setLookAndFeel(&host.getLookAndFeel());
        box.updatePosition(screenArea, available.isEmpty() ? top->getScreenBounds() : host.localAreaToGlobal(available));
        new CallOutOwner(box, host);
        return box;
    }
    const auto localArea = top->getLocalArea(&host, area);
    auto& box = juce::CallOutBox::launchAsynchronously(std::move(content), localArea, top);
    if (!available.isEmpty()) { box.updatePosition(localArea, top->getLocalArea(&host, available)); }
    return box;
}

} // namespace osci
