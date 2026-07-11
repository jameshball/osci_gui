#include "osci_ImagePreviewComponent.h"

namespace osci {

ImagePreviewComponent::ImagePreviewComponent()
    : juce::Button("Image preview"), hoverAnimation(this) {
    setName("Image preview");
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTriggeredOnMouseDown(false);
    hoverAnimation.setValueChangedCallback([this](float) { repaint(); });
    onClick = [this] {
        if (image.isValid() && onOpenRequested != nullptr) {
            onOpenRequested();
        }
    };
}

void ImagePreviewComponent::setImage(juce::Image newImage) {
    image = std::move(newImage);
    setEnabled(image.isValid());
    repaint();
}

const juce::Image& ImagePreviewComponent::getImage() const {
    return image;
}

void ImagePreviewComponent::setCaption(juce::String newCaption) {
    caption = std::move(newCaption);
    setName(caption.isNotEmpty() ? caption : "Image preview");
    repaint();
}

void ImagePreviewComponent::setAccentColour(juce::Colour colour) {
    accentColour = colour;
    repaint();
}

void ImagePreviewComponent::setMagnifierSvg(juce::String svg) {
    rebuildMagnifier(svg);
    repaint();
}

void ImagePreviewComponent::setRemoveAction(std::function<void()> action, juce::String closeButtonSvg, juce::String componentID) {
    removeButton.reset();
    if (action == nullptr || closeButtonSvg.isEmpty()) {
        return;
    }

    removeButton = std::make_unique<CloseButton>(std::move(closeButtonSvg), "Remove image", juce::Colours::white, juce::Colours::white);
    removeButton->setComponentID(std::move(componentID));
    removeButton->setPaintsBackground(true);
    removeButton->setIconPadding(8);
    removeButton->onClick = std::move(action);
    addAndMakeVisible(*removeButton);
    resized();
}

void ImagePreviewComponent::paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) {
    constexpr auto radius = 12.0f;
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    if (bounds.isEmpty()) {
        return;
    }

    g.saveState();
    juce::Path clip;
    clip.addRoundedRectangle(bounds, radius);
    g.reduceClipRegion(clip);
    drawImageCheckerboard(g, bounds.getSmallestIntegerContainer());

    if (image.isValid()) {
        const auto source = image.getBounds().toFloat();
        const auto destination = juce::RectanglePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize)
                                     .appliedTo(source, bounds.reduced(3.0f));
        g.drawImage(image, destination);
    } else {
        g.setColour(Colours::textSubtle());
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawFittedText("No preview", getLocalBounds().reduced(10), juce::Justification::centred, 1);
    }

    juce::ignoreUnused(isMouseOver);
    const auto hover = hoverAnimation.getProgress();
    if (hover > 0.001f && image.isValid()) {
        g.setColour(juce::Colours::black.withAlpha(hover * (isButtonDown ? 0.48f : 0.36f)));
        g.fillAll();

        auto iconTile = bounds.withSizeKeepingCentre(48.0f, 48.0f);
        g.setColour(juce::Colours::black.withAlpha(0.56f * hover));
        g.fillEllipse(iconTile);
        g.setColour(accentColour.withAlpha(0.78f * hover));
        g.drawEllipse(iconTile.reduced(0.75f), 1.5f);
        if (magnifier != nullptr) {
            magnifier->drawWithin(g, iconTile.reduced(12.0f), juce::RectanglePlacement::centred, hover);
        }
    }

    if (caption.isNotEmpty()) {
        auto captionArea = bounds.getSmallestIntegerContainer().removeFromBottom(28);
        juce::ColourGradient gradient(juce::Colours::transparentBlack,
                                     static_cast<float>(captionArea.getCentreX()),
                                     static_cast<float>(captionArea.getY()),
                                     juce::Colours::black.withAlpha(0.78f),
                                     static_cast<float>(captionArea.getCentreX()),
                                     static_cast<float>(captionArea.getBottom()),
                                     false);
        g.setGradientFill(gradient);
        g.fillRect(captionArea);
        g.setColour(juce::Colours::white.withAlpha(0.94f));
        g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        g.drawFittedText(caption, captionArea.reduced(8, 2), juce::Justification::centredLeft, 1);
    }

    g.restoreState();
    g.setColour(accentColour.withAlpha(0.28f + hover * 0.52f));
    g.drawRoundedRectangle(bounds, radius, 1.0f + hover);
}

void ImagePreviewComponent::resized() {
    if (removeButton != nullptr) {
        removeButton->setBounds(getLocalBounds().removeFromRight(34).removeFromTop(34).reduced(3));
    }
}

void ImagePreviewComponent::mouseEnter(const juce::MouseEvent& event) {
    juce::Button::mouseEnter(event);
    hoverAnimation.animateTo(true, 155, juce::Easings::createEaseOut());
}

void ImagePreviewComponent::mouseExit(const juce::MouseEvent& event) {
    juce::Button::mouseExit(event);
    hoverAnimation.animateTo(false, 155, juce::Easings::createEaseOut());
}

void ImagePreviewComponent::rebuildMagnifier(juce::StringRef svg) {
    const auto xml = juce::XmlDocument::parse(juce::String(svg));
    magnifier = xml != nullptr ? juce::Drawable::createFromSVG(*xml) : nullptr;
}

} // namespace osci
