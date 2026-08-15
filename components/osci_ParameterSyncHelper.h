#pragma once


// Utility that listens to a set of AudioProcessorParameters and coalesces
// value-change notifications into a single async callback.  Owns the
// listener registrations and cleans them up on destruction.
//
// Usage:
//   ParameterSyncHelper syncHelper { [this] { syncFromProcessorState(); } };
//   syncHelper.track(someParam);
//
class ParameterSyncHelper : public juce::AudioProcessorParameter::Listener,
                            public juce::AsyncUpdater {
public:
    using SyncCallback = std::function<void()>;

    class ScopedUpdateSuppression {
    public:
        explicit ScopedUpdateSuppression(ParameterSyncHelper& owner) : owner(owner) {
            jassert(juce::MessageManager::existsAndIsCurrentThread());
            owner.suppressionDepth.fetch_add(1, std::memory_order_relaxed);
        }

        ~ScopedUpdateSuppression() {
            owner.suppressionDepth.fetch_sub(1, std::memory_order_relaxed);
        }

        ScopedUpdateSuppression(const ScopedUpdateSuppression&) = delete;
        ScopedUpdateSuppression& operator=(const ScopedUpdateSuppression&) = delete;

    private:
        ParameterSyncHelper& owner;
    };

    explicit ParameterSyncHelper(SyncCallback callback)
        : syncCallback(std::move(callback)) {}

    ~ParameterSyncHelper() override {
        cancelPendingUpdate();
        for (auto* p : trackedParams)
            p->removeListener(this);
    }

    void track(juce::AudioProcessorParameter* p) {
        if (p != nullptr) {
            trackedParams.push_back(p);
            p->addListener(this);
        }
    }

    // juce::AudioProcessorParameter::Listener
    void parameterValueChanged(int, float) override {
        const bool suppressedOnMessageThread = suppressionDepth.load(std::memory_order_relaxed) > 0 && juce::MessageManager::existsAndIsCurrentThread();
        if (!suppressedOnMessageThread) {
            triggerAsyncUpdate();
        }
    }
    void parameterGestureChanged(int, bool) override {}

    // juce::AsyncUpdater
    void handleAsyncUpdate() override {
        if (syncCallback)
            syncCallback();
    }

private:
    SyncCallback syncCallback;
    std::vector<juce::AudioProcessorParameter*> trackedParams;
    std::atomic<int> suppressionDepth{0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterSyncHelper)
};
