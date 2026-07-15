#pragma once

#include "osci_ParameterContextMenu.h"
#include <osci_render_core/effect/osci_EffectParameter.h>

// Reusable helper for adding MIDI CC support to any component wrapping a
// BooleanParameter. Embed as a member, call init() in constructor,
// wireMidiCC() to enable CC. Handles change-listener lifecycle,
// learning-state query, and right-click context menu.
struct BooleanParamCCHelper : private juce::ChangeListener {
    BooleanParamCCHelper() = default;

    ~BooleanParamCCHelper() {
        if (midiManager != nullptr)
            midiManager->removeChangeListener(this);
    }

    void init(osci::BooleanParameter* p, juce::Component* ownerComp) {
        parameter = p;
        owner = ownerComp;
        if (p != nullptr && p->midiManager != nullptr)
            wireMidiCC(*p->midiManager);
    }

    void wireMidiCC(osci::MidiManager& mgr) {
        ParameterContextMenu::wireMidiCCListener(midiManager, mgr, this);
    }

    bool isLearning() const {
        return midiManager != nullptr && parameter != nullptr
               && midiManager->isLearning(parameter);
    }

    void showContextMenu(juce::Point<int> screenPos) {
        if (midiManager == nullptr || parameter == nullptr) return;

        ParameterContextMenu::Context ctx;
        ctx.param = parameter;
        ctx.midiManager = midiManager;
        ctx.canResetToDefault = true;
        ctx.canSetValue = false;

        ParameterContextMenu::showAsync(ctx, screenPos, owner,
            [p = parameter]() { p->setBoolValueNotifyingHost(p->getDefaultValue() >= 0.5f); },
            nullptr, nullptr);
    }

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override {
        if (owner) owner->repaint();
    }

    osci::BooleanParameter* parameter = nullptr;
    osci::MidiManager* midiManager = nullptr;
    juce::Component* owner = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BooleanParamCCHelper)
};
