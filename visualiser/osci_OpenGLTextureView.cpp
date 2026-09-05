#include "osci_OpenGLTextureView.h"

namespace {

constexpr auto textureViewVertexShader = R"(
attribute vec2 aPosition;
varying vec2 vTextureCoordinate;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTextureCoordinate = aPosition * 0.5 + 0.5;
}
)";

constexpr auto textureViewFragmentShader = R"(
uniform sampler2D uTexture;
uniform float uPreserveAlpha;
uniform float uCheckerboard;
uniform float uFadeAlpha;
uniform vec3 uCheckerColour0;
uniform vec3 uCheckerColour1;
varying vec2 vTextureCoordinate;

void main() {
    vec4 colour = texture2D(uTexture, vTextureCoordinate);
    if (uCheckerboard > 0.5) {
        float cell = mod(floor(gl_FragCoord.x / 12.0) + floor(gl_FragCoord.y / 12.0), 2.0);
        vec3 background = mix(uCheckerColour0, uCheckerColour1, cell);
        colour.rgb += background * (1.0 - colour.a);
        colour.a = 1.0;
    } else if (uPreserveAlpha < 0.5) {
        colour.a = 1.0;
    }
    colour.rgb *= 1.0 - uFadeAlpha;
    colour.a = mix(colour.a, 1.0, uFadeAlpha);
    gl_FragColor = colour;
}
)";

constexpr std::array<float, 12> fullScreenQuad {
    -1.0f, 1.0f,
     1.0f, 1.0f,
     1.0f, -1.0f,
    -1.0f, 1.0f,
     1.0f, -1.0f,
    -1.0f, -1.0f,
};

juce::Rectangle<int> aspectFit(juce::Rectangle<int> target, int sourceWidth, int sourceHeight) {
    if (target.isEmpty() || sourceWidth <= 0 || sourceHeight <= 0) {
        return {};
    }

    const double scale = juce::jmin(static_cast<double>(target.getWidth()) / static_cast<double>(sourceWidth),
                                    static_cast<double>(target.getHeight()) / static_cast<double>(sourceHeight));
    const int width = juce::roundToInt(static_cast<double>(sourceWidth) * scale);
    const int height = juce::roundToInt(static_cast<double>(sourceHeight) * scale);
    return juce::Rectangle<int>(width, height).withCentre(target.getCentre());
}

}

namespace osci {

OpenGLTextureView::OpenGLTextureView(OpenGLFrameMirror& source) : source(source) {
    constexpr auto alphaMaskBufferSize = static_cast<std::size_t>(alphaMaskResolution * alphaMaskResolution * 4);
    alphaMaskPixels.resize(alphaMaskBufferSize);
    alphaMaskReadback.resize(alphaMaskBufferSize);
    setOpaque(false);
    context.setRenderer(this);
    context.setComponentPaintingEnabled(true);
    context.setContinuousRepainting(false);
}

OpenGLTextureView::~OpenGLTextureView() {
    stopTimer();
    source.setConsumerActive(false);
    context.detach();
    context.setNativeSharedContext(nullptr);
}

void OpenGLTextureView::setActive(bool shouldBeActive) {
    active = shouldBeActive;
    if (shouldBeActive) {
        startTimerHz(60);
        updateContext();
    } else {
        stopTimer();
        source.setConsumerActive(false);
    }
}

void OpenGLTextureView::setTransparent(bool shouldBeTransparent) {
    transparent.store(shouldBeTransparent);
    context.triggerRepaint();
}

void OpenGLTextureView::setNativeTransparencySupported(bool supported) {
    nativeTransparencySupported.store(supported);
    context.triggerRepaint();
}

void OpenGLTextureView::setCheckerColours(juce::Colour first, juce::Colour second) {
    checkerColour0.store(first.getARGB());
    checkerColour1.store(second.getARGB());
    context.triggerRepaint();
}

void OpenGLTextureView::setOpaqueBackground(juce::Colour colour) {
    opaqueBackground.store(colour.getARGB());
    context.triggerRepaint();
}

void OpenGLTextureView::setFadeAlpha(float alpha) {
    fadeAlpha.store(juce::jlimit(0.0f, 1.0f, alpha));
    context.triggerRepaint();
}

void OpenGLTextureView::setAlphaMaskCaptureEnabled(bool enabled) {
    alphaMaskCaptureEnabled.store(enabled);
}

void OpenGLTextureView::requestAlphaMaskRefresh() {
    alphaMaskRefreshRequested.store(true);
    context.triggerRepaint();
}

void OpenGLTextureView::setContextCreatedCallback(std::function<void(void*)> callback) {
    contextCreatedCallback = std::move(callback);
}

void OpenGLTextureView::executeOnGLThread(std::function<void(juce::OpenGLContext&)> callback) {
    if (context.isAttached()) {
        context.executeOnGLThread(std::move(callback), false);
    }
}

juce::Point<int> OpenGLTextureView::getAlphaMaskSize() const {
    juce::SpinLock::ScopedLockType scopedLock(alphaMaskLock);
    if (alphaMaskWidth <= 0 || alphaMaskHeight <= 0) {
        return {};
    }
    return { alphaMaskWidth, alphaMaskHeight };
}

bool OpenGLTextureView::alphaMaskHasAlphaNear(juce::Point<float> point, juce::Point<float> radius,
                                              std::uint8_t threshold) const {
    juce::SpinLock::ScopedLockType scopedLock(alphaMaskLock);
    const auto requiredSize = static_cast<std::size_t>(alphaMaskWidth) * static_cast<std::size_t>(alphaMaskHeight) * 4u;
    if (alphaMaskPixels.size() < requiredSize) {
        return false;
    }
    return alphaHitTest(alphaMaskPixels.data(), alphaMaskWidth, alphaMaskHeight,
                        point.x, point.y, radius.x, radius.y, threshold);
}

void OpenGLTextureView::timerCallback() {
    updateContext();
}

void OpenGLTextureView::updateContext() {
    if (!active) {
        return;
    }

    auto* sourceContext = source.getSourceNativeContext();
    const auto sourceEpoch = source.getSourceEpoch();
    if (sourceContext == nullptr || !source.isSourceReady()) {
        source.setConsumerActive(false);
        if (context.isAttached() || attachedSourceContext != nullptr) {
            context.detach();
            context.setNativeSharedContext(nullptr);
            attachedSourceContext = nullptr;
            attachedSourceEpoch = 0;
            lastDisplayedGeneration = 0;
            lastAlphaMaskGeneration = 0;
        }
        return;
    }

    if (sourceContext != attachedSourceContext || sourceEpoch != attachedSourceEpoch) {
        source.setConsumerActive(false);
        context.detach();
        context.setNativeSharedContext(sourceContext);
        attachedSourceContext = sourceContext;
        attachedSourceEpoch = sourceEpoch;
        lastDisplayedGeneration = 0;
        lastAlphaMaskGeneration = 0;
    }

    if (isShowing() && getPeer() != nullptr && !context.isAttached()) {
        context.attachTo(*this);
    }

    const bool ready = context.isAttached() && source.isSourceReady();
    source.setConsumerActive(ready);
    if (!ready) {
        return;
    }

    const auto generation = source.getPublishedGeneration();
    if (generation != lastDisplayedGeneration) {
        context.triggerRepaint();
    }
    if (!source.hasPublishedFrame()) {
        source.requestSourceFrame();
    }
}

void OpenGLTextureView::newOpenGLContextCreated() {
    using namespace juce::gl;

    textureShader = std::make_unique<juce::OpenGLShaderProgram>(context);
    textureShader->addVertexShader(juce::OpenGLHelpers::translateVertexShaderToV3(textureViewVertexShader));
    textureShader->addFragmentShader(juce::OpenGLHelpers::translateFragmentShaderToV3(textureViewFragmentShader));
    textureShader->link();
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * fullScreenQuad.size(), fullScreenQuad.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    positionAttribute = glGetAttribLocation(textureShader->getProgramID(), "aPosition");
    glGenFramebuffers(1, &frameBuffer);
    if (contextCreatedCallback != nullptr) {
        contextCreatedCallback(context.getRawContext());
    }
}

void OpenGLTextureView::renderOpenGL() {
    if (!context.isActive() || textureShader == nullptr) {
        return;
    }

    const bool shouldPreserveAlpha = transparent.load() && nativeTransparencySupported.load();
    const bool shouldShowCheckerboard = transparent.load() && !nativeTransparencySupported.load();
    const auto background = juce::Colour(opaqueBackground.load());
    juce::OpenGLHelpers::clear(shouldPreserveAlpha ? juce::Colours::transparentBlack : background);

    auto frame = source.acquireLatest(attachedSourceContext, attachedSourceEpoch);
    if (!frame.has_value()) {
        return;
    }

    const auto scale = static_cast<float>(context.getRenderingScale());
    const juce::Rectangle<int> displayBounds(0, 0,
                                             juce::roundToInt(static_cast<float>(getWidth()) * scale),
                                             juce::roundToInt(static_cast<float>(getHeight()) * scale));
    const auto fitted = aspectFit(displayBounds, frame->texture.width, frame->texture.height);
    juce::gl::glViewport(fitted.getX(), fitted.getY(), fitted.getWidth(), fitted.getHeight());
    drawTexture(frame->texture, shouldPreserveAlpha, shouldShowCheckerboard, fadeAlpha.load());

    const bool captureEnabled = alphaMaskCaptureEnabled.load();
    const bool refreshRequested = captureEnabled && alphaMaskRefreshRequested.exchange(false);
    if (captureEnabled && (refreshRequested || frame->generation != lastAlphaMaskGeneration)) {
        captureAlphaMask(frame->texture);
        lastAlphaMaskGeneration = frame->generation;
        juce::gl::glViewport(fitted.getX(), fitted.getY(), fitted.getWidth(), fitted.getHeight());
    }

    lastDisplayedGeneration = frame->generation;
    source.release(*frame);
}

void OpenGLTextureView::openGLContextClosing() {
    using namespace juce::gl;

    source.setConsumerActive(false);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(1, &alphaMaskTexture.id);
    vertexBuffer = 0;
    frameBuffer = 0;
    alphaMaskTexture = {};
    positionAttribute = -1;
    textureShader.reset();
}

void OpenGLTextureView::drawTexture(Texture texture, bool preserveAlpha, bool checkerboard, float fade) {
    using namespace juce::gl;

    if (positionAttribute < 0) {
        return;
    }

    glDisable(GL_BLEND);
    textureShader->use();
    textureShader->setUniform("uTexture", 0);
    textureShader->setUniform("uPreserveAlpha", preserveAlpha ? 1.0f : 0.0f);
    textureShader->setUniform("uCheckerboard", checkerboard ? 1.0f : 0.0f);
    textureShader->setUniform("uFadeAlpha", fade);
    const auto firstCheckerColour = juce::Colour(checkerColour0.load());
    const auto secondCheckerColour = juce::Colour(checkerColour1.load());
    textureShader->setUniform("uCheckerColour0", firstCheckerColour.getFloatRed(),
                              firstCheckerColour.getFloatGreen(), firstCheckerColour.getFloatBlue());
    textureShader->setUniform("uCheckerColour1", secondCheckerColour.getFloatRed(),
                              secondCheckerColour.getFloatGreen(), secondCheckerColour.getFloatBlue());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glEnableVertexAttribArray(positionAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(positionAttribute);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
}

void OpenGLTextureView::captureAlphaMask(Texture texture) {
    using namespace juce::gl;

    const auto scale = static_cast<float>(alphaMaskResolution)
                     / static_cast<float>(juce::jmax(texture.width, texture.height));
    const int width = juce::jmax(1, juce::roundToInt(static_cast<float>(texture.width) * scale));
    const int height = juce::jmax(1, juce::roundToInt(static_cast<float>(texture.height) * scale));
    if (alphaMaskTexture.width != width || alphaMaskTexture.height != height) {
        createAlphaMaskTexture(width, height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, alphaMaskTexture.id, 0);
    glViewport(0, 0, width, height);
    juce::OpenGLHelpers::clear(juce::Colours::transparentBlack);
    drawTexture(texture, true, false, 0.0f);

    const auto byteCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u;
    jassert(byteCount <= alphaMaskReadback.size());
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, alphaMaskReadback.data());
    {
        juce::SpinLock::ScopedLockType scopedLock(alphaMaskLock);
        std::swap(alphaMaskPixels, alphaMaskReadback);
        alphaMaskWidth = width;
        alphaMaskHeight = height;
        alphaMaskGeneration.fetch_add(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLTextureView::createAlphaMaskTexture(int width, int height) {
    using namespace juce::gl;

    if (alphaMaskTexture.id == 0) {
        glGenTextures(1, &alphaMaskTexture.id);
    }
    glBindTexture(GL_TEXTURE_2D, alphaMaskTexture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    alphaMaskTexture.width = width;
    alphaMaskTexture.height = height;
}

}
