#pragma once

#include "osci_AlphaHitTest.h"
#include "osci_OpenGLFrameMirror.h"

namespace osci {

// Displays frames from an OpenGLFrameMirror without copying them through the CPU.
// The optional alpha mask is a small readback intended for native hit testing.
class OpenGLTextureView : public juce::Component, private juce::OpenGLRenderer, private juce::Timer {
public:
    explicit OpenGLTextureView(OpenGLFrameMirror& source);
    ~OpenGLTextureView() override;

    void setActive(bool active);
    void setTransparent(bool transparent);
    void setNativeTransparencySupported(bool supported);
    void setCheckerColours(juce::Colour first, juce::Colour second);
    void setOpaqueBackground(juce::Colour colour);
    void setFadeAlpha(float alpha);
    void setAlphaMaskCaptureEnabled(bool enabled);
    void requestAlphaMaskRefresh();
    void setContextCreatedCallback(std::function<void(void*)> callback);
    void executeOnGLThread(std::function<void(juce::OpenGLContext&)> callback);

    juce::Point<int> getAlphaMaskSize() const;
    std::uint64_t getAlphaMaskGeneration() const { return alphaMaskGeneration.load(); }
    bool alphaMaskHasAlphaNear(juce::Point<float> point, juce::Point<float> radius, std::uint8_t threshold) const;

private:
    void timerCallback() override;
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
    void updateContext();
    void drawTexture(Texture texture, bool preserveAlpha, bool checkerboard, float fade);
    void captureAlphaMask(Texture texture);
    void createAlphaMaskTexture(int width, int height);

    OpenGLFrameMirror& source;
    juce::OpenGLContext context;
    std::unique_ptr<juce::OpenGLShaderProgram> textureShader;
    GLuint vertexBuffer = 0;
    GLuint frameBuffer = 0;
    GLint positionAttribute = -1;
    Texture alphaMaskTexture;
    void* attachedSourceContext = nullptr;
    std::uint64_t attachedSourceEpoch = 0;
    std::uint64_t lastDisplayedGeneration = 0;
    std::uint64_t lastAlphaMaskGeneration = 0;
    std::atomic<float> fadeAlpha { 0.0f };
    std::atomic<bool> transparent { false };
    std::atomic<bool> nativeTransparencySupported { false };
    std::atomic<bool> alphaMaskCaptureEnabled { false };
    std::atomic<bool> alphaMaskRefreshRequested { false };
    std::atomic<std::uint64_t> alphaMaskGeneration { 0 };
    bool active = false;
    std::atomic<juce::uint32> checkerColour0 { juce::Colours::darkgrey.getARGB() };
    std::atomic<juce::uint32> checkerColour1 { juce::Colours::grey.getARGB() };
    std::atomic<juce::uint32> opaqueBackground { juce::Colours::black.getARGB() };
    std::function<void(void*)> contextCreatedCallback;
    mutable juce::SpinLock alphaMaskLock;
    std::vector<std::uint8_t> alphaMaskPixels;
    std::vector<std::uint8_t> alphaMaskReadback;
    int alphaMaskWidth = 0;
    int alphaMaskHeight = 0;

    static constexpr int alphaMaskResolution = 64;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLTextureView)
};

}
