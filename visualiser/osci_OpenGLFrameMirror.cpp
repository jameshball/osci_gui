#include "osci_OpenGLFrameMirror.h"

namespace osci {

OpenGLFrameMirror::~OpenGLFrameMirror() {
    jassert(!sourceReady.load());
}

void OpenGLFrameMirror::sourceContextCreated(void* nativeContext) {
    juce::CriticalSection::ScopedLockType scopedLock(lock);
    sourceNativeContext.store(nativeContext);
    sourceReady.store(true);
    sourceEpoch.fetch_add(1);
}

void OpenGLFrameMirror::sourceContextClosing() {
    {
        juce::CriticalSection::ScopedLockType scopedLock(lock);
        sourceReady.store(false);
        consumerActive.store(false);
        sourceNativeContext.store(nullptr);
        sourceEpoch.fetch_add(1);
    }

    while (readReservations.load() > 0) {
        juce::Thread::yield();
    }

    juce::CriticalSection::ScopedLockType scopedLock(lock);
    clearFences();
    for (auto& slot : slots) {
        juce::gl::glDeleteTextures(1, &slot.texture.id);
        slot.texture = {};
    }
}

void OpenGLFrameMirror::setConsumerActive(bool active) {
    consumerActive.store(active && sourceReady.load());
}

void OpenGLFrameMirror::requestSourceFrame() const {
    if (sourceRepaint != nullptr) {
        sourceRepaint();
    }
}

OpenGLFrameMirror::WriteFrame OpenGLFrameMirror::beginWrite(Texture fallback, const TextureAllocator& allocateTexture) {
    using namespace juce::gl;

    if (!consumerActive.load()) {
        return { fallback, -1 };
    }

    juce::CriticalSection::ScopedLockType scopedLock(lock);
    const int currentSlot = publishedSlot.load();
    for (int candidate = 0; candidate < slotCount; ++candidate) {
        if (candidate == currentSlot) {
            continue;
        }

        auto& slot = slots[static_cast<std::size_t>(candidate)];
        if (slot.beingRead) {
            continue;
        }
        if (slot.readFence != nullptr) {
            const auto status = glClientWaitSync(slot.readFence, 0, 0);
            if (status != GL_ALREADY_SIGNALED && status != GL_CONDITION_SATISFIED) {
                continue;
            }
            glDeleteSync(slot.readFence);
            slot.readFence = nullptr;
        }
        if (slot.texture.width != fallback.width || slot.texture.height != fallback.height) {
            slot.texture = allocateTexture(fallback.width, fallback.height, slot.texture.id);
        }
        return { slot.texture, candidate };
    }

    return { fallback, -1 };
}

void OpenGLFrameMirror::publish(const WriteFrame& frame) {
    using namespace juce::gl;

    if (frame.slot < 0 || frame.slot >= slotCount) {
        return;
    }

    juce::CriticalSection::ScopedLockType scopedLock(lock);
    auto& slot = slots[static_cast<std::size_t>(frame.slot)];
    if (slot.writeFence != nullptr) {
        glDeleteSync(slot.writeFence);
    }
    slot.writeFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    publishedSlot.store(frame.slot);
    publishedGeneration.fetch_add(1);
}

void OpenGLFrameMirror::publishExisting(Texture source, const TextureAllocator& allocateTexture,
                                        const TargetActivator& activateTarget) {
    using namespace juce::gl;

    if (!consumerActive.load() || hasPublishedFrame() || source.id == 0) {
        return;
    }

    juce::CriticalSection::ScopedLockType scopedLock(lock);
    if (publishedSlot.load() >= 0) {
        return;
    }

    constexpr int targetSlot = 0;
    auto& slot = slots[targetSlot];
    if (slot.texture.width != source.width || slot.texture.height != source.height) {
        slot.texture = allocateTexture(source.width, source.height, slot.texture.id);
    }
    activateTarget(source);
    glBindTexture(GL_TEXTURE_2D, slot.texture.id);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, source.width, source.height);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (slot.writeFence != nullptr) {
        glDeleteSync(slot.writeFence);
    }
    slot.writeFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    publishedSlot.store(targetSlot);
    publishedGeneration.fetch_add(1);
}

std::optional<OpenGLFrameMirror::ReadFrame> OpenGLFrameMirror::acquireLatest(
    void* expectedNativeContext, std::uint64_t expectedEpoch) {
    using namespace juce::gl;

    juce::CriticalSection::ScopedLockType scopedLock(lock);
    if (!sourceReady.load() || sourceNativeContext.load() != expectedNativeContext || sourceEpoch.load() != expectedEpoch) {
        return std::nullopt;
    }

    const int slotIndex = publishedSlot.load();
    if (slotIndex < 0 || slotIndex >= slotCount) {
        return std::nullopt;
    }

    auto& slot = slots[static_cast<std::size_t>(slotIndex)];
    if (slot.writeFence != nullptr) {
        glWaitSync(slot.writeFence, 0, GL_TIMEOUT_IGNORED);
        glDeleteSync(slot.writeFence);
        slot.writeFence = nullptr;
    }
    slot.beingRead = true;
    readReservations.fetch_add(1);

    ReadFrame frame;
    frame.texture = slot.texture;
    frame.generation = publishedGeneration.load();
    frame.slot = slotIndex;
    return frame;
}

void OpenGLFrameMirror::release(const ReadFrame& frame) {
    using namespace juce::gl;

    if (frame.slot < 0 || frame.slot >= slotCount) {
        return;
    }

    juce::CriticalSection::ScopedLockType scopedLock(lock);
    auto& slot = slots[static_cast<std::size_t>(frame.slot)];
    if (slot.readFence != nullptr) {
        glDeleteSync(slot.readFence);
    }
    slot.readFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    slot.beingRead = false;
    readReservations.fetch_sub(1);
}

void OpenGLFrameMirror::clearFences() {
    using namespace juce::gl;

    for (auto& slot : slots) {
        if (slot.writeFence != nullptr) {
            glDeleteSync(slot.writeFence);
            slot.writeFence = nullptr;
        }
        if (slot.readFence != nullptr) {
            glDeleteSync(slot.readFence);
            slot.readFence = nullptr;
        }
        slot.beingRead = false;
    }
    publishedSlot.store(-1);
    publishedGeneration.store(0);
}

}
