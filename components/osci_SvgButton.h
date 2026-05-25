#pragma once

namespace osci {

namespace Svg {
    inline void applyFill(juce::XmlElement* xml, juce::Colour colour) {
        if (xml == nullptr) {
            return;
        }

        for (auto* xmlNode : xml->getChildIterator()) {
            xmlNode->setAttribute("fill", '#' + colour.toDisplayString(false));
        }
    }

    inline std::unique_ptr<juce::Drawable> createDrawable(juce::String svg, juce::Colour colour) {
        auto doc = juce::XmlDocument::parse(svg);
        if (doc == nullptr) {
            return nullptr;
        }

        applyFill(doc.get(), colour);
        return juce::Drawable::createFromSVG(*doc);
    }
}

class SvgButton : public juce::DrawableButton, public juce::AudioProcessorParameter::Listener, public juce::AsyncUpdater {
 public:
    SvgButton(juce::String name, juce::String svg, juce::Colour colour, juce::Colour colourOn, juce::AudioProcessorParameter* toggle = nullptr, juce::String toggledSvg = "") : juce::DrawableButton(name, juce::DrawableButton::ButtonStyle::ImageFitted), toggle(toggle), updater(this), svgSource(std::move(svg)), toggledSvgSource(std::move(toggledSvg)) {
        rebuildImages (colour, colourOn);

        if (normalImage == nullptr) {
            return;
        }

        basePath = normalImage->getOutlineAsPath();

        setColour(juce::DrawableButton::backgroundOnColourId, Colours::transparent());

        if (colour != colourOn) {
            setClickingTogglesState(true);
        }

        if (toggle != nullptr) {
            toggle->addListener(this);
            setToggleState(toggle->getValue() >= 0.5f, juce::NotificationType::dontSendNotification);
        }

        updater.addAnimator(pulse);
    }

    SvgButton(juce::String name, juce::String svg, juce::Colour colour) : SvgButton(name, svg, colour, colour) {}

    ~SvgButton() override {
        if (toggle != nullptr) {
            toggle->removeListener(this);
        }
    }

    void parameterValueChanged(int parameterIndex, float newValue) override {
        triggerAsyncUpdate();
    }

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    void handleAsyncUpdate() override {
        if (toggle != nullptr) {
            setToggleState(toggle->getValue() >= 0.5f, juce::NotificationType::dontSendNotification);
        }
    }

    void mouseEnter(const juce::MouseEvent& e) override {
        juce::DrawableButton::mouseEnter(e);
    }

    void mouseExit(const juce::MouseEvent& e) override {
        juce::DrawableButton::mouseExit(e);
    }

    bool hitTest(int x, int y) override {
        return isEnabled() && juce::DrawableButton::hitTest(x, y);
    }

    void setPulseAnimation(bool pulseUsed) {
        this->pulseUsed = pulseUsed;
    }

    void resized() override {
        juce::DrawableButton::resized();
        if (pulseUsed) {
            resizedPath = basePath;
            resizedPath.applyTransform(getImageTransform());
            repaint();
        }
    }

    void paintOverChildren(juce::Graphics& g) override {
        if (pulseUsed && getToggleState()) {
            g.setColour(Colours::shadow().withAlpha(juce::jlimit(0.0f, 1.0f, colourFade / 1.5f)));
            g.fillPath(resizedPath);
        }
    }

    void buttonStateChanged() override {
        juce::DrawableButton::buttonStateChanged();
        if (pulseUsed && getToggleState() != prevToggleState) {
            if (getToggleState()) {
                pulse.start();
            } else {
                pulse.complete();
                colourFade = 1.0;
            }
            prevToggleState = getToggleState();
        }
    }

private:
    std::unique_ptr<juce::Drawable> normalImage;
    std::unique_ptr<juce::Drawable> overImage;
    std::unique_ptr<juce::Drawable> downImage;
    std::unique_ptr<juce::Drawable> disabledImage;

    std::unique_ptr<juce::Drawable> normalImageOn;
    std::unique_ptr<juce::Drawable> overImageOn;
    std::unique_ptr<juce::Drawable> downImageOn;
    std::unique_ptr<juce::Drawable> disabledImageOn;

    juce::AudioProcessorParameter* toggle;

    juce::VBlankAnimatorUpdater updater;
    float colourFade = 0.0;
    bool pulseUsed = false;
    bool prevToggleState = false;
    juce::Path basePath;
    juce::Path resizedPath;
    juce::AffineTransform imageTransform; // Transform applied to all state images
    juce::Animator pulse = juce::ValueAnimatorBuilder {}
        .withEasing([] (float t) { return std::sin(3.14159f * t) / 2.0f + 0.5f; })
        .withDurationMs(500)
        .runningInfinitely()
        .withValueChangedCallback([this] (auto value) {
            colourFade = value;
            repaint();
        })
        .build();

    juce::String svgSource;
    juce::String toggledSvgSource;
    juce::Colour lastOffColour;
    juce::Colour lastOnColour;

public:
    // Allows callers to adjust the placement/scale/rotation of the SVG within the button.
    void setImageTransform(const juce::AffineTransform& t) {
        if (normalImage == nullptr) {
            return;
        }

        imageTransform = juce::RectanglePlacement(juce::RectanglePlacement::centred).getTransformToFit(normalImage->getDrawableBounds(), getImageBounds()).followedBy(t);
        if (getLocalBounds().isEmpty()) {
            return;
        }
        setButtonStyle(juce::DrawableButton::ButtonStyle::ImageRaw);
        applyImageTransform();
        // Keep the pulse overlay in sync
        resized();
    }

    juce::AffineTransform getImageTransform() const { return imageTransform; }

    // Rebuild the "on" state images with a new colour.
    void setOnColour(juce::Colour colourOn) {
        rebuildImages (lastOffColour, colourOn);
    }

    void setColours(juce::Colour colour, juce::Colour colourOn) {
        rebuildImages (colour, colourOn);
    }

private:
    static juce::Colour hoverColour (juce::Colour colour) { return colour.interpolatedWith (Colours::accentColor(), 0.10f); }
    static juce::Colour downColour (juce::Colour colour) { return colour.interpolatedWith (Colours::shadow(), Theme::isDark() ? 0.25f : 0.18f); }
    static juce::Colour disabledColour (juce::Colour colour) { return colour.withMultipliedAlpha (0.36f); }

    std::unique_ptr<juce::Drawable> createImage (const juce::String& source, juce::Colour colour) {
        auto doc = juce::XmlDocument::parse (source);
        if (doc == nullptr) {
            return nullptr;
        }

        Svg::applyFill (doc.get(), colour);
        return juce::Drawable::createFromSVG (*doc);
    }

    void rebuildImages (juce::Colour colour, juce::Colour colourOn) {
        lastOffColour = colour;
        lastOnColour = colourOn;

        const auto& onSource = toggledSvgSource.isNotEmpty() ? toggledSvgSource : svgSource;

        normalImage = createImage (svgSource, colour);
        overImage = createImage (svgSource, hoverColour (colour));
        downImage = createImage (svgSource, downColour (colour));
        disabledImage = createImage (svgSource, disabledColour (colour));

        normalImageOn = createImage (onSource, colourOn);
        overImageOn = createImage (onSource, hoverColour (colourOn));
        downImageOn = createImage (onSource, downColour (colourOn));
        disabledImageOn = createImage (onSource, disabledColour (colourOn));

        if (normalImage != nullptr) {
            basePath = normalImage->getOutlineAsPath();
        }

        applyImageTransform();
    }

    void applyImageTransform() {
        auto apply = [this](std::unique_ptr<juce::Drawable>& d) {
            if (d != nullptr) {
                d->setTransform(imageTransform);
            }
        };
        apply(normalImage);
        apply(overImage);
        apply(downImage);
        apply(disabledImage);
        apply(normalImageOn);
        apply(overImageOn);
        apply(downImageOn);
        apply(disabledImageOn);

        setImages(normalImage.get(), overImage.get(), downImage.get(), disabledImage.get(), normalImageOn.get(), overImageOn.get(), downImageOn.get(), disabledImageOn.get());
    }
};

} // namespace osci
