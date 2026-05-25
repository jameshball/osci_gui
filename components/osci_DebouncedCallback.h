#pragma once

namespace osci {

class DebouncedCallback final : private juce::Timer {
public:
    explicit DebouncedCallback (int defaultDelayMs = 250)
        : delayMs (juce::jmax (0, defaultDelayMs)) {}

    ~DebouncedCallback() override {
        cancel();
    }

    void setDefaultDelayMs (int newDelayMs) {
        delayMs = juce::jmax (0, newDelayMs);
    }

    int getDefaultDelayMs() const {
        return delayMs;
    }

    bool isPending() const {
        return pending != nullptr;
    }

    void trigger (std::function<void()> callback) {
        trigger (delayMs, std::move (callback));
    }

    void trigger (int delayMsToUse, std::function<void()> callback) {
        pending = std::move (callback);

        if (pending == nullptr) {
            stopTimer();
            return;
        }

        const auto delay = juce::jmax (0, delayMsToUse);
        if (delay == 0) {
            flush();
            return;
        }

        startTimer (delay);
    }

    void cancel() {
        stopTimer();
        pending = nullptr;
    }

    void flush() {
        stopTimer();

        auto callback = std::move (pending);
        pending = nullptr;

        if (callback != nullptr) {
            callback();
        }
    }

private:
    void timerCallback() override {
        flush();
    }

    int delayMs = 250;
    std::function<void()> pending;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DebouncedCallback)
};

} // namespace osci
