#include "osci_CallOut.h"

namespace osci {

namespace {
// A call-out goes when what it points at does (and with it the editor whose
// look it borrowed), or when its window hides.
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
        // A window hidden (closed to the tray) takes its call-out with it.
        if (&component == top.getComponent() && !component.isShowing()) { close(); }
    }
    // At once, not later: what it shows may call back into the editor going away.
    void close() {
        auto* callOut = box.getComponent();
        if (callOut == nullptr) { return; }
        callOut->removeComponentListener(this);
        box = nullptr;
        callOut->setLookAndFeel(nullptr);
        if (callOut->isCurrentlyModal(false)) { callOut->exitModalState(0); }
        delete callOut;
    }

    juce::Component::SafePointer<juce::CallOutBox> box;
    juce::Component::SafePointer<juce::Component> host, top;
};
}

juce::CallOutBox& launchCallOut(std::unique_ptr<juce::Component> content, juce::Component& host, juce::Rectangle<int> area, juce::Rectangle<int> available, juce::Component* parent) {
    auto* top = parent != nullptr ? parent : host.getTopLevelComponent();
    // Either way it sits outside host, so it borrows host's look, and goes
    // before host does.
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
    box.setLookAndFeel(&host.getLookAndFeel());
    if (!available.isEmpty()) { box.updatePosition(localArea, top->getLocalArea(&host, available)); }
    new CallOutOwner(box, host);
    return box;
}

} // namespace osci
