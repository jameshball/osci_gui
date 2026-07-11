#include "osci_FileDropZoneComponent.h"

namespace osci {
namespace {
constexpr auto cornerRadius = 18.0f;
constexpr auto borderThickness = 2.0f;
constexpr auto dashLength = 12.0f;
constexpr auto dashGap = 8.0f;

juce::Colour accentColour()
{
    return Colours::categoryFileSources();
}

juce::Colour statusColour (FileDropZoneComponent::Status status)
{
    switch (status) {
        case FileDropZoneComponent::Status::dragAccept:
        case FileDropZoneComponent::Status::selected:
        case FileDropZoneComponent::Status::success:
            return accentColour();
        case FileDropZoneComponent::Status::warning:
        case FileDropZoneComponent::Status::dragReject:
            return Colours::warning();
        case FileDropZoneComponent::Status::error:
            return Colours::danger();
        case FileDropZoneComponent::Status::busy:
            return Colours::categoryScripting();
        case FileDropZoneComponent::Status::disabled:
        case FileDropZoneComponent::Status::empty:
            return accentColour();
    }

    return accentColour();
}

void drawDashedRoundedRectangle (juce::Graphics& g,
                                 juce::Rectangle<float> bounds,
                                 float radius,
                                 float thickness,
                                 juce::Colour colour,
                                 float phase)
{
    juce::ignoreUnused (phase);
    juce::Path outline;
    outline.addRoundedRectangle (bounds, radius);

    const float dashes[] { dashLength, dashGap };
    juce::Path dashedOutline;
    juce::PathStrokeType stroke (thickness,
                                 juce::PathStrokeType::JointStyle::curved,
                                 juce::PathStrokeType::EndCapStyle::rounded);
    stroke.createDashedStroke (dashedOutline, outline, dashes, 2, {}, 1.0f);

    g.setColour (colour);
    g.fillPath (dashedOutline);
}

void drawUploadGlyph (juce::Graphics& g,
                      juce::Rectangle<float> area,
                      juce::Colour colour,
                      float lift)
{
    const auto scale = 1.0f + lift * 0.075f;
    area = area.withSizeKeepingCentre (area.getWidth() * scale, area.getHeight() * scale);

    const auto tile = area.withSizeKeepingCentre (58.0f, 58.0f);
    g.setColour (colour.withAlpha (0.10f + lift * 0.08f));
    g.fillRoundedRectangle (tile, 18.0f);
    g.setColour (colour.withAlpha (0.18f + lift * 0.18f));
    g.drawRoundedRectangle (tile.reduced (0.5f), 18.0f, 1.0f);

    const auto centre = tile.getCentre();
    juce::Path arrow;
    arrow.startNewSubPath (centre.x, tile.getY() + 35.0f - lift * 2.0f);
    arrow.lineTo (centre.x, tile.getY() + 18.0f - lift * 2.0f);
    arrow.startNewSubPath (centre.x - 8.0f, tile.getY() + 26.0f - lift * 2.0f);
    arrow.lineTo (centre.x, tile.getY() + 18.0f - lift * 2.0f);
    arrow.lineTo (centre.x + 8.0f, tile.getY() + 26.0f - lift * 2.0f);

    juce::Path tray;
    tray.startNewSubPath (tile.getX() + 19.0f, tile.getY() + 41.0f);
    tray.lineTo (tile.getX() + 39.0f, tile.getY() + 41.0f);

    juce::PathStrokeType iconStroke (3.0f,
                                     juce::PathStrokeType::JointStyle::curved,
                                     juce::PathStrokeType::EndCapStyle::rounded);
    g.setColour (colour.withAlpha (0.86f));
    g.strokePath (arrow, iconStroke);
    g.setColour (colour.withAlpha (0.56f));
    g.strokePath (tray, iconStroke);
}

void drawFileGlyph (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour colour)
{
    area = area.withSizeKeepingCentre (30.0f, 36.0f);

    juce::Path page;
    page.startNewSubPath (area.getX() + 5.0f, area.getY());
    page.lineTo (area.getRight() - 8.0f, area.getY());
    page.lineTo (area.getRight(), area.getY() + 8.0f);
    page.lineTo (area.getRight(), area.getBottom());
    page.lineTo (area.getX() + 5.0f, area.getBottom());
    page.closeSubPath();

    g.setColour (colour.withAlpha (0.10f));
    g.fillPath (page);
    g.setColour (colour.withAlpha (0.70f));
    g.strokePath (page, juce::PathStrokeType (1.4f));

    juce::Path fold;
    fold.startNewSubPath (area.getRight() - 8.0f, area.getY());
    fold.lineTo (area.getRight() - 8.0f, area.getY() + 8.0f);
    fold.lineTo (area.getRight(), area.getY() + 8.0f);
    g.strokePath (fold, juce::PathStrokeType (1.2f));
}

juce::String parentPathForDisplay (const juce::File& file)
{
    const auto parent = file.getParentDirectory().getFullPathName();
    if (parent.length() <= 78)
        return parent;

    const auto parentFile = file.getParentDirectory();
    const auto grandParentName = parentFile.getParentDirectory().getFileName();
    return ".../" + (grandParentName.isNotEmpty() ? grandParentName + "/" : juce::String()) + parentFile.getFileName();
}
} // namespace

FileDropZoneComponent::FileDropZoneComponent()
    : actionButton ("fileDropZoneAction"),
      hoverAnimation (this),
      dragAnimation (this),
      pulseAnimation (this),
      rejectAnimation (this)
{
    setName ("File Drop Zone");
    setComponentID ("fileDropZone");
    setWantsKeyboardFocus (true);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);

    auto repaintOnAnimation = [this] (float)
    {
        repaint();
        resized();
    };
    hoverAnimation.setValueChangedCallback (repaintOnAnimation);
    dragAnimation.setValueChangedCallback (repaintOnAnimation);
    pulseAnimation.setValueChangedCallback (repaintOnAnimation);
    rejectAnimation.setValueChangedCallback (repaintOnAnimation);

    actionButton.onClick = [this]
    {
        if (canAcceptDrops() && onBrowseRequested != nullptr)
            onBrowseRequested();
    };
    addAndMakeVisible (actionButton);

    lookAndFeelChanged();
}

void FileDropZoneComponent::setTitle (juce::String text)
{
    title = std::move (text);
    repaint();
}

void FileDropZoneComponent::setSubtitle (juce::String text)
{
    subtitle = std::move (text);
    repaint();
}

void FileDropZoneComponent::setActionText (juce::String text)
{
    actionText = std::move (text);
    updateActionButton();
}

void FileDropZoneComponent::setSelectedFile (juce::File file, juce::String detail)
{
    selectedFile = std::move (file);
    selectedDetail = std::move (detail);
    hasSelectedFile = selectedFile.existsAsFile();

    if (hasSelectedFile && status == Status::empty)
        status = Status::selected;

    updateActionButton();
    repaint();
}

void FileDropZoneComponent::clearSelectedFile()
{
    selectedFile = juce::File();
    selectedDetail = {};
    hasSelectedFile = false;

    if (status == Status::selected)
        status = Status::empty;

    updateActionButton();
    repaint();
}

void FileDropZoneComponent::setStatus (Status newStatus, juce::String message)
{
    status = newStatus;
    statusMessage = std::move (message);

    if (status == Status::dragAccept || status == Status::dragReject)
    {
        dragActive = true;
        dragAccepted = status == Status::dragAccept;
    }
    else if (dragActive)
    {
        clearDragState();
    }

    updateActionButton();
    updateAnimationTimer();
    repaint();
}

void FileDropZoneComponent::setAcceptedDescription (juce::String text)
{
    acceptedDescription = std::move (text);
    repaint();
}

void FileDropZoneComponent::setIsFileAccepted (std::function<bool (const juce::File&)> predicate)
{
    isFileAccepted = std::move (predicate);
}

bool FileDropZoneComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    return canAcceptDrops() && files.size() > 0;
}

void FileDropZoneComponent::fileDragEnter (const juce::StringArray& files, int, int)
{
    updateDragState (files);
}

void FileDropZoneComponent::fileDragMove (const juce::StringArray& files, int, int)
{
    updateDragState (files);
}

void FileDropZoneComponent::fileDragExit (const juce::StringArray&)
{
    clearDragState();
}

void FileDropZoneComponent::filesDropped (const juce::StringArray& paths, int, int)
{
    const auto files = makeFiles (paths);
    auto accepted = acceptedFilesFrom (files);
    auto rejected = rejectedFilesFrom (files);
    clearDragState();

    if (! accepted.empty())
    {
        triggerDropPulse();
        if (onFilesDropped != nullptr)
            onFilesDropped (std::move (accepted));
    }

    if (! rejected.empty() && onRejectedFiles != nullptr)
        onRejectedFiles (std::move (rejected));

    if (accepted.empty() && ! files.empty())
        triggerRejectNudge();
}

void FileDropZoneComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    if (bounds.isEmpty())
        return;

    const auto effectiveStatus = getEffectiveStatus();
    const auto accent = statusColour (effectiveStatus);
    const auto hover = hoverAnimation.getProgress();
    const auto drag = dragAnimation.getProgress();
    const auto pulse = pulseAnimation.getProgress();
    const auto reject = rejectAnimation.getProgress();
    const auto disabled = effectiveStatus == Status::disabled;
    const auto nudge = std::sin (reject * juce::MathConstants<float>::twoPi) * 4.0f;

    bounds = bounds.translated (nudge, -drag * 3.0f);

    juce::Path shadowPath;
    shadowPath.addRoundedRectangle (bounds.reduced (1.0f), cornerRadius);
    melatonin::DropShadow shadow ({ { Colours::shadow().withAlpha (0.22f + drag * 0.16f), 18, { 0, 3 }, -2 },
                                    { Colours::shadow().withAlpha (0.16f + drag * 0.10f), 6, { 0, 0 }, 0 } });
    shadow.render (g, shadowPath);

    auto fill = Colours::surfaceRaised().interpolatedWith (accent, 0.035f + hover * 0.025f + drag * 0.055f + pulse * 0.065f);
    if (effectiveStatus == Status::dragReject || effectiveStatus == Status::error)
        fill = Colours::surfaceRaised().interpolatedWith (Colours::danger(), 0.060f + drag * 0.080f + reject * 0.050f);
    else if (effectiveStatus == Status::warning)
        fill = Colours::surfaceRaised().interpolatedWith (Colours::warning(), 0.060f);

    const auto alpha = disabled ? 0.52f : 1.0f;
    g.setColour (fill.withMultipliedAlpha (alpha));
    g.fillRoundedRectangle (bounds, cornerRadius);

    const auto borderAlpha = disabled ? 0.18f : (0.40f + hover * 0.18f + drag * 0.30f + pulse * 0.22f);
    drawDashedRoundedRectangle (g,
                                bounds.reduced (borderThickness * 0.5f),
                                cornerRadius,
                                borderThickness,
                                accent.withAlpha (borderAlpha),
                                dashPhase);

    const auto textColour = Colours::text().withAlpha (disabled ? 0.42f : 0.92f);
    const auto mutedText = Colours::textSubtle().withAlpha (disabled ? 0.32f : 0.68f);
    auto content = bounds.reduced (26.0f, 20.0f);

    if (hasSelectedFile)
    {
        auto heading = content.removeFromTop (76);
        drawUploadGlyph (g, heading.withSizeKeepingCentre (68.0f, 62.0f), accent, drag + pulse * 0.4f);
        content.removeFromTop (2);

        auto card = content.removeFromTop (62).reduced (0.0f, 1.0f);
        g.setColour (Colours::surfaceSunken().withAlpha (0.50f));
        g.fillRoundedRectangle (card, 11.0f);
        g.setColour (accent.withAlpha (0.18f + pulse * 0.22f));
        g.drawRoundedRectangle (card.reduced (0.5f), 11.0f, 1.0f);

        auto cardContent = card.reduced (14.0f, 9.0f);
        drawFileGlyph (g, cardContent.removeFromLeft (38.0f), accent);
        cardContent.removeFromLeft (8.0f);
        auto textArea = cardContent;
        textArea.removeFromRight (actionButton.isVisible() ? 116.0f : 0.0f);

        g.setColour (textColour);
        g.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)));
        LookAndFeelHelpers::drawFittedText (g,
                                            selectedFile.getFileName(),
                                            textArea.removeFromTop (21.0f),
                                            juce::Justification::centredLeft,
                                            1);

        g.setColour (mutedText);
        g.setFont (juce::Font (juce::FontOptions (10.4f)));
        LookAndFeelHelpers::drawFittedText (g,
                                            selectedDetail.isNotEmpty() ? selectedDetail : parentPathForDisplay (selectedFile),
                                            textArea,
                                            juce::Justification::centredLeft,
                                            1,
                                            0.58f);

        if (effectiveStatus == Status::success || effectiveStatus == Status::warning || effectiveStatus == Status::error || effectiveStatus == Status::busy)
        {
            const auto pillX = actionButton.isVisible() ? (float) actionButton.getX() - 116.0f
                                                        : card.getRight() - 126.0f;
            auto pill = juce::Rectangle<float> (pillX, card.getY() + 8.0f, 108.0f, 20.0f);
            g.setColour (accent.withAlpha (0.16f));
            g.fillRoundedRectangle (pill, 10.0f);
            g.setColour (accent.withAlpha (0.32f));
            g.drawRoundedRectangle (pill.reduced (0.5f), 10.0f, 1.0f);
            g.setColour (accent.withAlpha (0.92f));
            g.setFont (juce::Font (juce::FontOptions (9.8f, juce::Font::bold)));
            LookAndFeelHelpers::drawFittedText (g,
                                                statusMessage.isNotEmpty() ? statusMessage : "Ready",
                                                pill.reduced (8.0f, 1.0f),
                                                juce::Justification::centred,
                                                1);
        }

        return;
    }

    auto iconArea = content.removeFromTop (76.0f);
    drawUploadGlyph (g, iconArea, accent, drag + pulse * 0.4f);
    content.removeFromTop (3.0f);

    juce::String headline = title;
    juce::String detail = subtitle;

    if (effectiveStatus == Status::dragAccept)
    {
        headline = "Release to import";
        detail = "Drop to continue.";
    }
    else if (effectiveStatus == Status::dragReject)
    {
        headline = "That file is not supported";
        detail = "Drop a " + acceptedDescription + ".";
    }
    else if ((effectiveStatus == Status::busy || effectiveStatus == Status::success || effectiveStatus == Status::warning || effectiveStatus == Status::error)
             && statusMessage.isNotEmpty())
    {
        headline = statusMessage;
        detail = subtitle;
    }

    g.setColour (textColour);
    g.setFont (juce::Font (juce::FontOptions (15.8f, juce::Font::bold)));
    LookAndFeelHelpers::drawFittedText (g,
                                        headline,
                                        content.removeFromTop (24.0f),
                                        juce::Justification::centred,
                                        1);
    g.setColour (mutedText);
    g.setFont (juce::Font (juce::FontOptions (11.5f)));
    LookAndFeelHelpers::drawFittedText (g,
                                        detail,
                                        content.removeFromTop (22.0f),
                                        juce::Justification::centred,
                                        1);

    if (effectiveStatus == Status::busy)
    {
        auto shimmer = bounds.reduced (44.0f, 0.0f).removeFromBottom (20.0f).withHeight (3.0f);
        g.setColour (Colours::neutralFill (0.08f));
        g.fillRoundedRectangle (shimmer, 1.5f);
        const auto shimmerWidth = shimmer.getWidth() * 0.30f;
        auto active = shimmer.withWidth (shimmerWidth).translated ((shimmer.getWidth() + shimmerWidth) * busyPhase - shimmerWidth, 0.0f);
        g.setColour (accent.withAlpha (0.44f));
        g.fillRoundedRectangle (active, 1.5f);
    }
}

void FileDropZoneComponent::resized()
{
    auto area = getLocalBounds().reduced (26, 20);

    if (hasSelectedFile)
    {
        area.removeFromTop (81);
        auto card = area.removeFromTop (62).reduced (0, 1);
        auto buttonArea = card.removeFromRight (112).withSizeKeepingCentre (96, 28);
        actionButton.setBounds (buttonArea);
    }
    else
    {
        area.removeFromTop (129);
        actionButton.setBounds (area.withSizeKeepingCentre (142, 30));
    }

    updateActionButton();
}

void FileDropZoneComponent::lookAndFeelChanged()
{
    actionButton.setColour (juce::TextButton::buttonColourId, accentColour().withAlpha (0.20f));
    actionButton.setColour (juce::TextButton::buttonOnColourId, accentColour().withAlpha (0.32f));
    actionButton.setColour (juce::TextButton::textColourOffId, Colours::text().withAlpha (0.92f));
}

void FileDropZoneComponent::enablementChanged()
{
    updateActionButton();
    repaint();
}

void FileDropZoneComponent::mouseEnter (const juce::MouseEvent&)
{
    if (canAcceptDrops())
        hoverAnimation.animateTo (true, 130, juce::Easings::createEaseOut());
}

void FileDropZoneComponent::mouseExit (const juce::MouseEvent&)
{
    hoverAnimation.animateTo (false, 150, juce::Easings::createEaseOut());
}

void FileDropZoneComponent::mouseDown (const juce::MouseEvent& event)
{
    mouseWasDownInside = getLocalBounds().contains (event.getPosition());
}

void FileDropZoneComponent::mouseUp (const juce::MouseEvent& event)
{
    const auto shouldBrowse = mouseWasDownInside
        && getLocalBounds().contains (event.getPosition())
        && canAcceptDrops()
        && onBrowseRequested != nullptr;
    mouseWasDownInside = false;

    if (shouldBrowse)
        onBrowseRequested();
}

FileDropZoneComponent::Status FileDropZoneComponent::getEffectiveStatus() const
{
    if (! isEnabled() || status == Status::disabled)
        return Status::disabled;

    if (dragActive)
        return dragAccepted ? Status::dragAccept : Status::dragReject;

    return status;
}

bool FileDropZoneComponent::canAcceptDrops() const
{
    return isEnabled() && status != Status::busy && status != Status::disabled;
}

bool FileDropZoneComponent::acceptsFile (const juce::File& file) const
{
    if (isFileAccepted == nullptr)
        return true;

    return isFileAccepted (file);
}

std::vector<juce::File> FileDropZoneComponent::makeFiles (const juce::StringArray& paths) const
{
    std::vector<juce::File> files;
    files.reserve (static_cast<size_t> (paths.size()));

    for (const auto& path : paths)
        files.push_back (juce::File (path));

    return files;
}

std::vector<juce::File> FileDropZoneComponent::acceptedFilesFrom (const std::vector<juce::File>& files) const
{
    std::vector<juce::File> accepted;
    for (const auto& file : files)
        if (acceptsFile (file))
            accepted.push_back (file);

    return accepted;
}

std::vector<juce::File> FileDropZoneComponent::rejectedFilesFrom (const std::vector<juce::File>& files) const
{
    std::vector<juce::File> rejected;
    for (const auto& file : files)
        if (! acceptsFile (file))
            rejected.push_back (file);

    return rejected;
}

void FileDropZoneComponent::updateDragState (const juce::StringArray& paths)
{
    if (! canAcceptDrops())
        return;

    const auto accepted = acceptedFilesFrom (makeFiles (paths));
    dragActive = true;
    dragAccepted = ! accepted.empty();
    dragAnimation.animateTo (true, 120, juce::Easings::createEaseOut());

    if (! dragAccepted)
        triggerRejectNudge();

    updateAnimationTimer();
    repaint();
}

void FileDropZoneComponent::clearDragState()
{
    dragActive = false;
    dragAccepted = false;
    dragAnimation.animateTo (false, 160, juce::Easings::createEaseOut());
    updateAnimationTimer();
    repaint();
}

void FileDropZoneComponent::triggerDropPulse()
{
    pulseAnimation.snapTo (false);
    pulseAnimation.animateTo (true,
                              120,
                              juce::Easings::createEaseOut(),
                              [safeThis = juce::Component::SafePointer<FileDropZoneComponent> (this)]
                              {
                                  if (safeThis != nullptr)
                                      safeThis->pulseAnimation.animateTo (false, 220, juce::Easings::createEaseOut());
                              });
}

void FileDropZoneComponent::triggerRejectNudge()
{
    if (rejectAnimation.isAnimating())
        return;

    rejectAnimation.snapTo (false);
    rejectAnimation.animateTo (true,
                               120,
                               juce::Easings::createEaseOut(),
                               [safeThis = juce::Component::SafePointer<FileDropZoneComponent> (this)]
                               {
                                   if (safeThis != nullptr)
                                       safeThis->rejectAnimation.animateTo (false, 160, juce::Easings::createEaseOut());
                               });
}

void FileDropZoneComponent::updateActionButton()
{
    actionButton.setButtonText (hasSelectedFile ? "Change..." : actionText);
    actionButton.setEnabled (canAcceptDrops());
    actionButton.setVisible (status != Status::busy && status != Status::disabled && isEnabled());
}

void FileDropZoneComponent::updateAnimationTimer()
{
    const auto shouldRun = dragActive || status == Status::busy || pulseAnimation.isAnimating() || rejectAnimation.isAnimating();

    if (shouldRun && ! isTimerRunning())
        startTimerHz (30);
    else if (! shouldRun && isTimerRunning())
        stopTimer();
}

void FileDropZoneComponent::timerCallback()
{
    dashPhase += dragActive ? 1.35f : 0.42f;
    if (dashPhase > dashLength + dashGap)
        dashPhase -= dashLength + dashGap;

    busyPhase += 0.035f;
    if (busyPhase > 1.0f)
        busyPhase -= 1.0f;

    updateAnimationTimer();
    repaint();
}

} // namespace osci
