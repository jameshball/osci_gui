#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>

#include "osci_OpenGLTexture.h"

namespace osci {

// Publishes textures from one OpenGL context to a consumer context in the same
// native share group. All frame acquisition/release calls run on a GL thread.
class OpenGLFrameMirror {
public:
    struct WriteFrame {
        Texture texture;
        int slot = -1;
    };

    struct ReadFrame {
        Texture texture;
        std::uint64_t generation = 0;

    private:
        friend class OpenGLFrameMirror;
        int slot = -1;
    };

    using TextureAllocator = std::function<Texture(int width, int height, GLuint existingTexture)>;
    using TargetActivator = std::function<void(Texture)>;

    OpenGLFrameMirror() = default;
    ~OpenGLFrameMirror();

    void sourceContextCreated(void* nativeContext);
    void sourceContextClosing();
    void setConsumerActive(bool active);
    void setSourceRepaintCallback(std::function<void()> callback) { sourceRepaint = std::move(callback); }
    void requestSourceFrame() const;

    void* getSourceNativeContext() const { return sourceNativeContext.load(); }
    std::uint64_t getSourceEpoch() const { return sourceEpoch.load(); }
    std::uint64_t getPublishedGeneration() const { return publishedGeneration.load(); }
    bool isSourceReady() const { return sourceReady.load(); }
    bool hasConsumer() const { return consumerActive.load(); }
    bool hasPublishedFrame() const { return publishedSlot.load() >= 0; }

    WriteFrame beginWrite(Texture fallback, const TextureAllocator& allocateTexture);
    void publish(const WriteFrame& frame);
    void publishExisting(Texture source, const TextureAllocator& allocateTexture, const TargetActivator& activateTarget);

    std::optional<ReadFrame> acquireLatest(void* expectedNativeContext, std::uint64_t expectedEpoch);
    void release(const ReadFrame& frame);

private:
    struct Slot {
        Texture texture;
        GLsync writeFence = nullptr;
        GLsync readFence = nullptr;
        bool beingRead = false;
    };

    static constexpr int slotCount = 2;

    void clearFences();

    juce::CriticalSection lock;
    std::array<Slot, slotCount> slots;
    std::atomic<void*> sourceNativeContext { nullptr };
    std::atomic<std::uint64_t> sourceEpoch { 0 };
    std::atomic<std::uint64_t> publishedGeneration { 0 };
    std::atomic<int> publishedSlot { -1 };
    std::atomic<int> readReservations { 0 };
    std::atomic<bool> sourceReady { false };
    std::atomic<bool> consumerActive { false };
    std::function<void()> sourceRepaint;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLFrameMirror)
};

}
