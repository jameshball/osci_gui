#include "osci_GridItemComponent.h"

namespace osci {

GridItemComponent::TextLayoutMetrics GridItemComponent::computeTextLayouts (int textWidth) const {
    TextLayoutMetrics metrics;
    metrics.hasDescription = description.isNotEmpty();

    if (metrics.hasDescription) {
        juce::AttributedString headingString;
        headingString.setJustification (juce::Justification::topLeft);
        headingString.append (itemName, juce::FontOptions (17.0f, juce::Font::bold), juce::Colours::white);

        metrics.headingLayout.createLayout (headingString, static_cast<float> (textWidth));
        metrics.headingHeight = metrics.headingLayout.getHeight();

        juce::AttributedString bodyString;
        bodyString.setJustification (juce::Justification::topLeft);
        bodyString.append (description, juce::FontOptions (14.0f), juce::Colours::white.withAlpha (0.85f));

        metrics.bodyLayout.createLayout (bodyString, static_cast<float> (textWidth));
        metrics.bodyHeight = metrics.bodyLayout.getHeight();

        metrics.totalHeight = metrics.headingHeight + titleDescriptionSpacing + metrics.bodyHeight;
    } else {
        juce::Font headingFont { juce::FontOptions (16.0f, juce::Font::plain) };
        metrics.headingHeight = headingFont.getHeight();
        metrics.totalHeight = metrics.headingHeight;
    }

    return metrics;
}

GridItemComponent::GridItemComponent (const juce::String& name,
                                      const juce::String& iconSvg,
                                      const juce::String& id,
                                      juce::Colour iconTint)
    : itemName (name),
      itemId (id),
      iconColour (iconTint) {
    iconButton = std::make_unique<SvgButton> ("gridItemIcon", iconSvg, iconColour);
    iconButton->setInterceptsMouseClicks (false, false);
    addAndMakeVisible (*iconButton);
}

GridItemComponent::GridItemComponent (const juce::String& name,
                                      juce::Image icon,
                                      const juce::String& id)
    : itemName (name),
      itemId (id),
      iconImage (std::move (icon)),
      layoutMode (LayoutMode::IconTile) {
}

GridItemComponent::~GridItemComponent() = default;

void GridItemComponent::paint (juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced (10);
    const auto canAnimate = interactive && ! locked && isEnabled();
    const auto animationProgress = canAnimate ? getAnimationProgress() : 0.0f;

    bounds = bounds.translated (0.0f, -animationProgress * hoverLiftAmount);

    if (layoutMode == LayoutMode::IconTile) {
        paintIconTile (g, bounds, animationProgress);
    } else {
        paintListItem (g, bounds, animationProgress);
    }

    if (! isEnabled()) {
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (bounds, cornerRadius);
    }

    if (locked) {
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillRoundedRectangle (bounds, cornerRadius);
    }
}

void GridItemComponent::paintListItem (juce::Graphics& g, juce::Rectangle<float> bounds, float animationProgress) {
    if (animationProgress > 0.01f) {
        const auto shadowRadius = juce::roundToInt(15.0f * animationProgress);

        if (shadowRadius > 0) {
            juce::Path shadowPath;
            shadowPath.addRoundedRectangle (bounds, cornerRadius);
            juce::DropShadow (Colours::accentColor().withAlpha (animationProgress * 0.2f),
                              shadowRadius,
                              { 0, 4 }).drawForPath (g, shadowPath);
        }
    }

    const auto normalBgColour = description.isNotEmpty() ? Colours::veryDark().brighter (0.1f)
                                                         : Colours::veryDark();
    const auto bgColour = normalBgColour.interpolatedWith (normalBgColour.brighter (0.05f), animationProgress);
    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, cornerRadius);

    const auto outlineColour = description.isNotEmpty()
        ? Colours::accentColor()
        : juce::Colour::fromRGB (160, 160, 160);
    g.setColour (outlineColour.withAlpha (description.isNotEmpty() ? 0.8f : 0.9f));
    g.drawRoundedRectangle (bounds, cornerRadius, 1.0f);

    auto textArea = bounds.reduced (10, 8);
    textArea.removeFromLeft (description.isNotEmpty() ? 52.0f : 28.0f);

    const auto textLayouts = computeTextLayouts (juce::jmax (1, juce::roundToInt (textArea.getWidth())));

    if (textLayouts.hasDescription) {
        auto textBounds = textArea;
        textLayouts.headingLayout.draw (g, textBounds);

        textBounds.removeFromTop (static_cast<float> (std::ceil (textLayouts.headingHeight)));
        textBounds.removeFromTop (titleDescriptionSpacing);

        if (textBounds.getHeight() > 0.0f) {
            textLayouts.bodyLayout.draw (g, textBounds);
        }
    } else {
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (16.0f));
        g.drawText (itemName, textArea, juce::Justification::centred, true);
    }
}

void GridItemComponent::paintIconTile (juce::Graphics& g, juce::Rectangle<float> bounds, float animationProgress) {
    juce::Path tilePath;
    tilePath.addRoundedRectangle (bounds, cornerRadius);

    const auto shadowAlpha = selected ? 0.28f : 0.13f + animationProgress * 0.12f;
    juce::DropShadow (juce::Colours::black.withAlpha (shadowAlpha),
                      selected ? 18 : juce::roundToInt (10.0f + 8.0f * animationProgress),
                      { 0, selected ? 8 : 5 }).drawForPath (g, tilePath);

    auto normalBg = Colours::veryDark().brighter (0.18f);
    auto bg = normalBg.interpolatedWith (normalBg.brighter (0.1f), animationProgress);
    if (selected) {
        bg = bg.interpolatedWith (Colours::accentColor(), 0.08f);
    }

    g.setColour (bg);
    g.fillPath (tilePath);

    const auto outlineAlpha = selected ? 0.88f : 0.24f + animationProgress * 0.3f;
    g.setColour ((selected ? Colours::accentColor() : juce::Colours::white).withAlpha (outlineAlpha));
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerRadius, selected ? 1.5f : 1.0f);

    auto content = bounds.reduced (20.0f, 18.0f);
    auto labelArea = content.removeFromBottom (34.0f);
    content.removeFromBottom (8.0f);

    const auto iconSide = juce::jmin (juce::jmin (content.getWidth(), content.getHeight()), 126.0f);
    auto iconArea = juce::Rectangle<float> (iconSide, iconSide).withCentre (content.getCentre());

    if (iconImage.isValid()) {
        g.setOpacity (isEnabled() ? 1.0f : 0.55f);
        g.drawImage (iconImage, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity (1.0f);
    }

    g.setColour (juce::Colours::white.withAlpha (selected ? 1.0f : 0.88f));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawFittedText (itemName, labelArea.toNearestInt(), juce::Justification::centred, 1);
}

void GridItemComponent::resized() {
    if (iconButton == nullptr) {
        return;
    }

    iconButton->setBounds (getListIconBounds());

    const bool canAnimate = interactive && ! locked && isEnabled();
    const auto animationProgress = canAnimate ? getAnimationProgress() : 0.0f;
    iconButton->setTransform (canAnimate ? juce::AffineTransform::translation (0.0f, -animationProgress * hoverLiftAmount)
                                         : juce::AffineTransform::identity);
}

juce::Rectangle<int> GridItemComponent::getListIconBounds() const {
    auto bounds = getLocalBounds().reduced (10);
    auto iconArea = bounds.removeFromLeft (description.isNotEmpty() ? 56 : 60);
    return iconArea.withSizeKeepingCentre (40, 40);
}

void GridItemComponent::mouseDown (const juce::MouseEvent& event) {
    if (! interactive || ! isEnabled()) {
        return;
    }

    HoverAnimationMixin::mouseDown (event);

    if (locked) {
        if (onLockedClick) {
            onLockedClick();
        }
        return;
    }

    if (onHoverEnd) {
        onHoverEnd();
    }

    if (onItemSelected) {
        onItemSelected (itemId);
    }
}

void GridItemComponent::mouseMove (const juce::MouseEvent& event) {
    juce::ignoreUnused (event);
    setMouseCursor (interactive && isEnabled() ? juce::MouseCursor::PointingHandCursor
                                               : juce::MouseCursor::NormalCursor);
    juce::Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
}

void GridItemComponent::mouseEnter (const juce::MouseEvent& event) {
    if (! interactive || ! isEnabled()) {
        setMouseCursor (juce::MouseCursor::NormalCursor);
        return;
    }

    HoverAnimationMixin::mouseEnter (event);

    if (locked) {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        return;
    }

    if (onHoverStart) {
        onHoverStart (itemId);
    }
}

void GridItemComponent::mouseExit (const juce::MouseEvent& event) {
    if (! interactive || ! isEnabled()) {
        setMouseCursor (juce::MouseCursor::NormalCursor);
        return;
    }

    HoverAnimationMixin::mouseExit (event);

    if (! locked && onHoverEnd) {
        onHoverEnd();
    }
}

void GridItemComponent::setInteractive (bool shouldBeInteractive) {
    if (interactive == shouldBeInteractive) {
        return;
    }

    interactive = shouldBeInteractive;
    if (! interactive) {
        animateHover (false);
    }

    if (iconButton != nullptr && ! interactive) {
        iconButton->setTransform (juce::AffineTransform());
    }

    setMouseCursor (interactive && isEnabled() ? juce::MouseCursor::PointingHandCursor
                                               : juce::MouseCursor::NormalCursor);
}

void GridItemComponent::setDescription (const juce::String& text) {
    if (description == text) {
        return;
    }

    description = text;
    resized();
    repaint();
}

void GridItemComponent::setLocked (bool shouldBeLocked) {
    if (locked == shouldBeLocked) {
        return;
    }

    locked = shouldBeLocked;
    if (locked) {
        animateHover (false);
    } else {
        setTooltip ({});
    }

    if (iconButton != nullptr) {
        iconButton->setEnabled (isEnabled() && ! locked);
        if (locked || ! isEnabled()) {
            iconButton->setTransform (juce::AffineTransform());
        }
    }

    repaint();
}

void GridItemComponent::setSelected (bool shouldBeSelected) {
    if (selected == shouldBeSelected) {
        return;
    }

    selected = shouldBeSelected;
    repaint();
}

void GridItemComponent::setLayoutMode (LayoutMode newMode) {
    if (layoutMode == newMode) {
        return;
    }

    layoutMode = newMode;
    if (iconButton != nullptr) {
        iconButton->setVisible (layoutMode == LayoutMode::List);
    }
    resized();
    repaint();
}

int GridItemComponent::getPreferredHeight (int width) const {
    if (layoutMode == LayoutMode::IconTile) {
        return 220;
    }

    const int minimumHeight = 80;
    const int outerPadding = 20;
    const int innerVerticalPadding = 16;
    const int iconHeight = 40;
    const int iconPadding = description.isNotEmpty() ? 52 : 28;
    const int textWidth = juce::jmax (1, width - textHorizontalPadding - iconPadding);
    const auto textLayouts = computeTextLayouts (textWidth);
    const auto contentHeight = juce::jmax (textLayouts.totalHeight, static_cast<float> (iconHeight));

    return juce::jmax (minimumHeight, juce::roundToInt (contentHeight + outerPadding + innerVerticalPadding + 4.0f));
}

} // namespace osci
