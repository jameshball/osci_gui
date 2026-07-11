#include "osci_ImagePreviewComponent.h"

namespace osci {
namespace {
constexpr auto defaultMagnifierSvg = R"svg(
<svg width="24" height="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
  <circle cx="10.5" cy="10.5" r="6.5" fill="none" stroke="#ffffff" stroke-width="2.2"/>
  <path d="M15.5 15.5 L21 21" fill="none" stroke="#ffffff" stroke-width="2.2" stroke-linecap="round"/>
</svg>)svg";

void drawCheckerboard(juce::Graphics& g, juce::Rectangle<int> area) {
    constexpr int tileSize = 10;
    const auto first = Colours::surfaceSunken();
    const auto second = Colours::surfaceRaised().interpolatedWith(first, 0.55f);
    for (int y = area.getY(); y < area.getBottom(); y += tileSize) {
        for (int x = area.getX(); x < area.getRight(); x += tileSize) {
            const auto alternate = ((x - area.getX()) / tileSize + (y - area.getY()) / tileSize) % 2 != 0;
            g.setColour(alternate ? second : first);
            g.fillRect(juce::Rectangle<int>(x, y, tileSize, tileSize).getIntersection(area));
        }
    }
}
} // namespace

ImagePreviewComponent::ImagePreviewComponent()
    : juce::Button("Image preview"), hoverAnimation(this) {
    setName("Image preview");
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTriggeredOnMouseDown(false);
    hoverAnimation.setValueChangedCallback([this](float) { repaint(); });
    rebuildMagnifier(defaultMagnifierSvg);
    removeButton.setVisible(false);
    addAndMakeVisible(removeButton);
    onClick = [this] {
        if (image.isValid() && onOpenRequested != nullptr) {
            onOpenRequested();
        }
    };
}

ImagePreviewComponent::RemoveButton::RemoveButton()
    : juce::Button("Remove image") {
    setName("Remove image");
    setTooltip("Remove image");
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void ImagePreviewComponent::RemoveButton::paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) {
    auto bounds = getLocalBounds().toFloat().reduced(0.75f);
    const auto fillAlpha = isButtonDown ? 0.92f : (isMouseOver ? 0.82f : 0.68f);
    g.setColour(juce::Colours::black.withAlpha(fillAlpha));
    g.fillEllipse(bounds);
    g.setColour(juce::Colours::white.withAlpha(isMouseOver ? 0.98f : 0.86f));
    g.drawEllipse(bounds, isMouseOver ? 1.6f : 1.25f);

    const auto cross = bounds.reduced(8.0f);
    juce::Path path;
    path.startNewSubPath(cross.getTopLeft());
    path.lineTo(cross.getBottomRight());
    path.startNewSubPath(cross.getTopRight());
    path.lineTo(cross.getBottomLeft());
    g.strokePath(path,
                 juce::PathStrokeType(2.0f,
                                      juce::PathStrokeType::JointStyle::curved,
                                      juce::PathStrokeType::EndCapStyle::rounded));
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

void ImagePreviewComponent::setRemoveAction(std::function<void()> action, juce::String componentID) {
    removeButton.onClick = std::move(action);
    removeButton.setComponentID(std::move(componentID));
    removeButton.setVisible(removeButton.onClick != nullptr);
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
    drawCheckerboard(g, bounds.getSmallestIntegerContainer());

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
    removeButton.setBounds(getLocalBounds().removeFromRight(34).removeFromTop(34).reduced(3));
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
