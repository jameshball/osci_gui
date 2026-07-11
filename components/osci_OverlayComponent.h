#pragma once

namespace osci {

class OverlayComponent;

class OverlayHost {
public:
    virtual ~OverlayHost() = default;
    virtual void showOverlay(std::unique_ptr<OverlayComponent> overlay) = 0;
};

// Base class for full-window overlay panels. Subclasses populate the content
// area; the overlay owns the backdrop, panel background, close button, and dismiss
// callback.
class OverlayComponent : public juce::Component {
public:
    explicit OverlayComponent (juce::String closeButtonSvg)
        : backdropLayer ("overlayBackdrop", juce::Graphics::mediumResamplingQuality),
          blurredBackdropLayer ("overlayBlurredBackdrop", juce::Graphics::mediumResamplingQuality),
          panelLayer (*this),
          panelAnimationLayer ("overlayPanelAnimation", juce::Graphics::mediumResamplingQuality),
          transitionController (this),
          closeOverlayButton (std::move (closeButtonSvg), "closeOverlay", Colours::textMuted(), Colours::text()) {
        setOpaque (false);
        setAlwaysOnTop (true);
        setInterceptsMouseClicks (true, true);
        setWantsKeyboardFocus (true);

        transitionController.setValueChangedCallback ([this] (auto value) {
            transitionProgress = static_cast<float>(value);
            updateBackdropAlpha();
            updatePanelTransform();
        });
        transitionController.snapTo (false);

        addAndMakeVisible (backdropLayer);
        addAndMakeVisible (blurredBackdropLayer);

        panelLayer.setInterceptsMouseClicks (false, true);
        addAndMakeVisible (panelLayer);
        panelAnimationLayer.setVisible (false);
        addAndMakeVisible (panelAnimationLayer);

        contentViewport.setViewedComponent (&contentLayer, false);
        contentViewport.setScrollBarsShown (true, false);
        contentViewport.setColour (juce::ScrollBar::thumbColourId, Colours::grey());
        contentViewport.setColour (juce::ScrollBar::trackColourId, Colours::transparent());
        panelLayer.addAndMakeVisible (contentViewport);

        configureLabel (overlayTitleLabel,
                        juce::Font (juce::FontOptions (20.0f, juce::Font::bold)),
                        juce::Justification::centred);
        overlayTitleLabel.setVisible (false);
        panelLayer.addAndMakeVisible (overlayTitleLabel);

        closeOverlayButton.onClick = [this] {
            dismiss();
        };
        panelLayer.addAndMakeVisible (closeOverlayButton);
        refreshThemeColours();
    }

    ~OverlayComponent() override {
        clearOverlayLookAndFeelRecursively (panelLayer);
    }

    std::function<void()> onDismissRequested;
    bool lightweight = false;

    static OverlayComponent* show(juce::Component& parent, std::unique_ptr<OverlayComponent> overlay) {
        jassert(overlay != nullptr);

        auto* rawOverlay = overlay.get();
        auto* host = findOverlayHost(parent);
        if (host != nullptr) {
            host->showOverlay(std::move(overlay));
            return rawOverlay;
        }

        showUnmanaged(parent, std::move(overlay));
        return rawOverlay;
    }

    void requestDismiss() {
        dismiss();
    }

    void captureBackdropFrom (juce::Component& source) {
        backdropPrepared = true;
        if (lightweight) {
            return;
        }

        const auto sourceBounds = source.getLocalBounds();
        if (sourceBounds.isEmpty()) {
            backdropSnapshot = {};
            backdropLayer.setImage ({});
            blurredBackdropLayer.setImage ({});
            return;
        }

        prepareBackdropSnapshot (source.createComponentSnapshot (sourceBounds, true, backdropSnapshotScale));
    }

    void captureBackdropFrom (const juce::Image& sourceSnapshot, float sourceScale = 1.0f) {
        backdropPrepared = true;
        if (lightweight || !sourceSnapshot.isValid()) {
            prepareBackdropSnapshot ({});
            return;
        }

        const auto scale = backdropSnapshotScale / juce::jmax (0.01f, sourceScale);
        if (juce::approximatelyEqual (scale, 1.0f)) {
            prepareBackdropSnapshot (sourceSnapshot);
            return;
        }

        const auto width = juce::jmax (1, juce::roundToInt (sourceSnapshot.getWidth() * scale));
        const auto height = juce::jmax (1, juce::roundToInt (sourceSnapshot.getHeight() * scale));
        prepareBackdropSnapshot (sourceSnapshot.rescaled (width, height, juce::Graphics::mediumResamplingQuality));
    }

    bool hasCapturedBackdrop() const {
        return backdropPrepared;
    }

private:
    void prepareBackdropSnapshot (juce::Image snapshot) {
        backdropSnapshot = std::move (snapshot);
        backdropLayer.setImage (backdropSnapshot);

        if (!backdropSnapshot.isValid()) {
            blurredBackdropLayer.setImage ({});
            return;
        }

        backdropBlur.setRadius (backdropBlurRadius);
        backdropBlur.update (backdropSnapshot);
        blurredBackdropLayer.setImage (backdropBlur.render());
        updateBackdropAlpha();
    }

public:
    void setOverlayTitle (const juce::String& title) {
        overlayTitleLabel.setText (title, juce::dontSendNotification);
        overlayTitleLabel.setVisible (title.isNotEmpty());
    }

    void setDismissible (bool shouldBeDismissible) {
        dismissible = shouldBeDismissible;
        closeOverlayButton.setVisible (dismissible);
    }

    void paint (juce::Graphics& g) override {
        juce::ignoreUnused (g);
    }

    void parentHierarchyChanged() override {
        startPresentationAnimationIfReady();
    }

    void resized() override {
        auto bounds = getLocalBounds();
        backdropLayer.setBounds (bounds);
        blurredBackdropLayer.setBounds (bounds);
        auto preferred = getPreferredPanelSize();
        if (preferred.x > 0 && preferred.y > 0) {
            auto available = bounds.reduced (40, 24);
            panelBounds.setSize (juce::jmin (preferred.x, available.getWidth()),
                                 juce::jmin (preferred.y, available.getHeight()));
            panelBounds.setCentre (bounds.getCentre());
        } else {
            const auto horizontalMargin = juce::jmax (40, bounds.getWidth() / 6);
            const auto verticalMargin = juce::jmax (24, bounds.getHeight() / 10);
            panelBounds = bounds.reduced (horizontalMargin, verticalMargin);
        }

        updatePanelLayerBounds();
        panelLayer.setBounds (panelLayerBounds);
        panelAnimationLayer.setBounds (panelLayerBounds);

        auto contentArea = getPanelBoundsInPanelLayer().reduced (panelPadding);
        if (overlayTitleLabel.isVisible() || closeOverlayButton.isVisible()) {
            const auto headerArea = contentArea.withHeight (panelHeaderHeight);
            if (closeOverlayButton.isVisible()) {
                auto closeButtonArea = headerArea;
                closeOverlayButton.setBounds (closeButtonArea.removeFromRight (28).withSizeKeepingCentre (22, 22));
            } else {
                closeOverlayButton.setBounds ({});
            }

            auto titleArea = headerArea;
            if (closeOverlayButton.isVisible()) {
                titleArea = titleArea.withTrimmedLeft (28).withTrimmedRight (28);
            }

            overlayTitleLabel.setBounds (titleArea);

            if (reserveHeaderSpace) {
                contentArea.removeFromTop (panelHeaderHeight);
                contentArea.removeFromTop (panelHeaderGap);
            }
        } else {
            closeOverlayButton.setBounds ({});
            overlayTitleLabel.setBounds ({});
        }

        updateContentViewport (contentArea, preferred);
        resizeContent (contentLayer.getLocalBounds());
        applyOverlayLookAndFeelRecursively (contentLayer);
        bringPanelControlsToFront();
        panelLayer.repaint();
        updatePanelTransform();
        startPresentationAnimationIfReady();
    }

    void mouseDown (const juce::MouseEvent& event) override {
        if (!dismissible || panelBounds.contains (event.getPosition())) {
            return;
        }

        auto* focused = juce::Component::getCurrentlyFocusedComponent();
        if (focused != nullptr) {
            const auto focusedTextEditor = dynamic_cast<juce::TextEditor*> (focused);
            if (focusedTextEditor != nullptr && (focused == this || isParentOf (focused))) {
                return;
            }
        }

        dismiss();
    }

    bool keyPressed (const juce::KeyPress& key) override {
        if (dismissible && key == juce::KeyPress::escapeKey) {
            dismiss();
            return true;
        }

        return juce::Component::keyPressed (key);
    }

protected:
    virtual void resizeContent (juce::Rectangle<int> contentArea) = 0;
    virtual juce::Point<int> getPreferredPanelSize() const { return {}; }

    void dismiss() {
        if (dismissInProgress) {
            return;
        }

        dismissInProgress = true;
        backdropLayer.setVisible (!lightweight && backdropSnapshot.isValid());
        beginPanelSnapshotAnimation();
        transitionController.animateTo (false,
                                        transitionDurationMs,
                                        juce::Easings::createEaseIn(),
                                        [this] {
                                            const juce::Component::SafePointer<OverlayComponent> safeThis { this };
                                            juce::MessageManager::callAsync ([safeThis] {
                                                if (safeThis != nullptr && safeThis->onDismissRequested) {
                                                    safeThis->onDismissRequested();
                                                }
                                            });
                                        });
    }

    void addPanelContentAndMakeVisible (juce::Component& component) {
        contentLayer.addAndMakeVisible (component);
        applyOverlayLookAndFeelRecursively (component);
        bringPanelControlsToFront();
    }

    void addPanelControlAndMakeVisible (juce::Component& component) {
        panelLayer.addAndMakeVisible (component);
        applyOverlayLookAndFeelRecursively (component);
        bringPanelControlsToFront();
    }

    void setReserveHeaderSpace (bool shouldReserveHeaderSpace) {
        reserveHeaderSpace = shouldReserveHeaderSpace;
    }

    virtual void panelChildBoundsChanged (juce::Component*) {}

    void requestOverlayLayout() {
        if (overlayLayoutUpdatePending) {
            return;
        }

        overlayLayoutUpdatePending = true;
        const juce::Component::SafePointer<OverlayComponent> safeThis { this };
        juce::MessageManager::callAsync ([safeThis] {
            if (safeThis == nullptr) {
                return;
            }

            safeThis->overlayLayoutUpdatePending = false;
            safeThis->resized();
            safeThis->repaint();
        });
    }

    juce::Rectangle<int> panelBounds;

    juce::Rectangle<int> getPanelBoundsInPanelLayer() const {
        return panelBounds.translated (-panelLayerBounds.getX(), -panelLayerBounds.getY());
    }

    static void configureLabel (juce::Label& label,
                                const juce::Font& font,
                                juce::Justification justification) {
        label.setColour (juce::Label::textColourId, Colours::text());
        label.setFont (font);
        label.setJustificationType (justification);
        label.setEditable (false, false, false);
        label.setInterceptsMouseClicks (false, false);
    }

    static juce::Point<int> getPanelSizeForContentSize (juce::Point<int> contentSize, bool hasHeader = true) {
        return { contentSize.x + panelPadding * 2,
                 contentSize.y + panelPadding * 2 + (hasHeader ? panelHeaderHeight + panelHeaderGap : 0) };
    }

private:
    static OverlayHost* findOverlayHost(juce::Component& source) {
        auto* component = &source;
        while (component != nullptr) {
            auto* host = dynamic_cast<OverlayHost*>(component);
            if (host != nullptr) {
                return host;
            }

            component = component->getParentComponent();
        }

        return nullptr;
    }

    static void showUnmanaged(juce::Component& parent, std::unique_ptr<OverlayComponent> overlay) {
        auto* rawOverlay = overlay.release();
        const juce::Component::SafePointer<OverlayComponent> safeOverlay(rawOverlay);
        auto previousDismissCallback = std::move(rawOverlay->onDismissRequested);
        rawOverlay->onDismissRequested = [safeOverlay, previousDismissCallback = std::move(previousDismissCallback)]() mutable {
            if (safeOverlay == nullptr) {
                return;
            }

            if (previousDismissCallback != nullptr) {
                previousDismissCallback();
            }

            if (safeOverlay != nullptr) {
                delete safeOverlay.getComponent();
            }
        };

        if (!rawOverlay->hasCapturedBackdrop()) {
            rawOverlay->captureBackdropFrom(parent);
        }
        parent.addAndMakeVisible(*rawOverlay);
        rawOverlay->setBounds(parent.getLocalBounds());
        rawOverlay->toFront(false);
        rawOverlay->grabKeyboardFocus();
    }

    class ImageLayer final : public juce::Component {
    public:
        explicit ImageLayer (juce::String componentName,
                             juce::Graphics::ResamplingQuality resamplingQualityToUse)
            : resamplingQuality (resamplingQualityToUse) {
            setName (std::move (componentName));
            setOpaque (false);
            setInterceptsMouseClicks (false, false);
        }

        void setImage (juce::Image imageToUse) {
            image = std::move (imageToUse);
            setVisible (image.isValid());
            repaint();
        }

        void setLayerOpacity (float opacityToUse) {
            const auto newOpacity = juce::jlimit (0.0f, 1.0f, opacityToUse);
            if (juce::approximatelyEqual (opacity, newOpacity)) {
                return;
            }

            opacity = newOpacity;
            repaint();
        }

        void paint (juce::Graphics& g) override {
            if (!image.isValid() || opacity <= 0.0f) {
                return;
            }

            const juce::Graphics::ScopedSaveState saveState (g);
            g.setOpacity (opacity);

            const auto bounds = getLocalBounds();
            if (image.getWidth() == bounds.getWidth() && image.getHeight() == bounds.getHeight()) {
                g.drawImageAt (image, 0, 0);
                return;
            }

            g.setImageResamplingQuality (resamplingQuality);
            g.drawImage (image, bounds.toFloat());
        }

    private:
        juce::Image image;
        juce::Graphics::ResamplingQuality resamplingQuality;
        float opacity = 1.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageLayer)
    };

    class PanelLayer final : public juce::Component {
    public:
        explicit PanelLayer (OverlayComponent& ownerToUse)
            : owner (ownerToUse) {
            setOpaque (false);
        }

        void paint (juce::Graphics& g) override {
            owner.paintPanel (g);
        }

        void childBoundsChanged (juce::Component* child) override {
            owner.panelChildBoundsChanged (child);
        }

    private:
        OverlayComponent& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelLayer)
    };

    class ContentLayer final : public juce::Component {
    public:
        explicit ContentLayer (OverlayComponent& ownerToUse)
            : owner (ownerToUse) {
            setOpaque (false);
        }

        void childBoundsChanged (juce::Component* child) override {
            owner.panelChildBoundsChanged (child);
        }

    private:
        OverlayComponent& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentLayer)
    };

    class ContentViewport final : public ScrollFadeViewport {
    public:
        ContentViewport() {
            setName ("overlayContentViewport");
            setFadeHeight (32);
        }

        void setVerticalScrollBarTopInset (int inset) {
            const auto newInset = juce::jmax (0, inset);
            if (verticalScrollBarTopInset == newInset) {
                adjustVerticalScrollBarBounds();
                return;
            }

            verticalScrollBarTopInset = newInset;
            lastAdjustedVerticalScrollBarBounds = {};
            adjustVerticalScrollBarBounds();
        }

        void resized() override {
            ScrollFadeViewport::resized();
            adjustVerticalScrollBarBounds();
        }

        void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea) override {
            ScrollFadeViewport::visibleAreaChanged (newVisibleArea);
            adjustVerticalScrollBarBounds();
        }

        void componentMovedOrResized (juce::Component& component, bool wasMoved, bool wasResized) override {
            ScrollFadeViewport::componentMovedOrResized (component, wasMoved, wasResized);
            adjustVerticalScrollBarBounds();
        }

    private:
        void adjustVerticalScrollBarBounds() {
            auto& scrollBar = getVerticalScrollBar();
            if (!scrollBar.isVisible() || verticalScrollBarTopInset <= 0) {
                lastAdjustedVerticalScrollBarBounds = {};
                return;
            }

            const auto bounds = scrollBar.getBounds();
            if (bounds == lastAdjustedVerticalScrollBarBounds) {
                return;
            }

            const auto inset = juce::jmin (verticalScrollBarTopInset, juce::jmax (0, bounds.getHeight() - 1));
            lastAdjustedVerticalScrollBarBounds = bounds.withTrimmedTop (inset);
            scrollBar.setBounds (lastAdjustedVerticalScrollBarBounds);
        }

        int verticalScrollBarTopInset = 0;
        juce::Rectangle<int> lastAdjustedVerticalScrollBarBounds;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentViewport)
    };

    juce::Point<int> getPreferredContentSize (juce::Point<int> preferredPanelSize) const {
        if (preferredPanelSize.x <= 0 || preferredPanelSize.y <= 0) {
            return {};
        }

        juce::Point<int> preferredContentSize {
            preferredPanelSize.x - panelPadding * 2,
            preferredPanelSize.y - panelPadding * 2
        };
        if (reserveHeaderSpace && (overlayTitleLabel.isVisible() || closeOverlayButton.isVisible())) {
            preferredContentSize.y -= panelHeaderHeight + panelHeaderGap;
        }

        return { juce::jmax (1, preferredContentSize.x), juce::jmax (1, preferredContentSize.y) };
    }

    void updateContentViewport (juce::Rectangle<int> contentArea, juce::Point<int> preferredPanelSize) {
        contentViewport.setBounds (contentArea);
        contentViewport.setVerticalScrollBarTopInset (getContentViewportScrollBarTopInset (contentArea));

        auto contentSize = getPreferredContentSize (preferredPanelSize);
        const auto contentHeight = juce::jmax (contentArea.getHeight(), contentSize.y);
        auto contentWidth = contentArea.getWidth();
        if (contentHeight > contentArea.getHeight()) {
            contentWidth -= contentViewport.getScrollBarThickness();
        }

        contentLayer.setSize (juce::jmax (1, contentWidth), juce::jmax (1, contentHeight));
    }

    int getContentViewportScrollBarTopInset (juce::Rectangle<int> contentArea) const {
        if (reserveHeaderSpace || !closeOverlayButton.isVisible()) {
            return 0;
        }

        return juce::jmax (0, closeOverlayButton.getBottom() + panelHeaderGap - contentArea.getY());
    }

    void paintPanel (juce::Graphics& g) {
        if (lightweight) {
            return;
        }

        paintPanelShadow (g);

        paintPanelBody (g, getPanelBoundsInPanelLayer().toFloat());
    }

    void paintPanelShadow (juce::Graphics& g) {
        renderPanelShadowCacheIfNeeded (g);

        if (!panelShadowImage.isValid()) {
            return;
        }

        const auto imageBounds = panelShadowImage.getBounds();
        const auto targetBounds = panelLayer.getLocalBounds();
        g.setImageResamplingQuality (juce::Graphics::mediumResamplingQuality);
        g.drawImageTransformed (panelShadowImage,
                                juce::AffineTransform::scale (static_cast<float>(targetBounds.getWidth())
                                                                  / static_cast<float>(imageBounds.getWidth()),
                                                              static_cast<float>(targetBounds.getHeight())
                                                                  / static_cast<float>(imageBounds.getHeight())),
                                false);
    }

    void paintPanelBody (juce::Graphics& g, juce::Rectangle<float> panelFloat) {
        const auto cornerRadius = 12.0f;

        g.setColour (Colours::veryDark().brighter (0.015f));
        g.fillRoundedRectangle (panelFloat, cornerRadius);

        g.setColour (Colours::neutralStroke (0.24f));
        g.drawRoundedRectangle (panelFloat.reduced (0.5f), cornerRadius, 1.0f);
    }

    void renderPanelShadowCacheIfNeeded (juce::Graphics& g) {
        const auto targetBounds = panelLayer.getLocalBounds();
        const auto panelLocalBounds = getPanelBoundsInPanelLayer();
        if (targetBounds.isEmpty()) {
            resetPanelShadowCache();
            return;
        }

        const auto renderScale = getPanelShadowRenderScale (g);
        const auto imageBounds = targetBounds * renderScale;
        if (panelShadowImage.isValid()
            && panelShadowImage.getBounds() == imageBounds
            && panelShadowImageBounds == targetBounds
            && panelShadowPanelBounds == panelLocalBounds
            && juce::approximatelyEqual (panelShadowImageScale, renderScale)) {
            return;
        }

        const auto imageType = g.getInternalContext().getPreferredImageTypeForTemporaryImages();
        panelShadowImage = juce::Image (juce::Image::ARGB,
                                        juce::jmax (1, imageBounds.getWidth()),
                                        juce::jmax (1, imageBounds.getHeight()),
                                        true,
                                        *imageType);
        panelShadowImage.setBackupEnabled (false);
        panelShadowImageBounds = targetBounds;
        panelShadowPanelBounds = panelLocalBounds;
        panelShadowImageScale = renderScale;

        juce::Graphics imageGraphics (panelShadowImage);
        imageGraphics.getInternalContext().addTransform (juce::AffineTransform::scale (renderScale));

        constexpr auto cornerRadius = 12.0f;
        juce::Path panelPath;
        panelPath.addRoundedRectangle (panelLocalBounds.toFloat(), cornerRadius);
        melatonin::DropShadow panelShadow {
            { Colours::shadow().withAlpha (0.72f), 22, { 0, 1 }, -1 },
            { Colours::shadow().withAlpha (0.62f), 10, { 0, 0 }, 0 },
            { Colours::shadow().withAlpha (0.48f), 4, { 0, 0 }, 0 }
        };
        panelShadow.render (imageGraphics, panelPath);
    }

    static float getPanelShadowRenderScale (juce::Graphics& g) {
        const auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();
        return juce::jmax (1.0f, scale);
    }

    void applyOverlayLookAndFeelRecursively (juce::Component& component) {
        if (usesOverlayLookAndFeel (component)) {
            component.setLookAndFeel (&overlayLookAndFeel);
        }

        for (auto i = 0; i < component.getNumChildComponents(); ++i) {
            auto* child = component.getChildComponent (i);
            if (child != nullptr) {
                applyOverlayLookAndFeelRecursively (*child);
            }
        }
    }

    void clearOverlayLookAndFeelRecursively (juce::Component& component) {
        if (&component.getLookAndFeel() == &overlayLookAndFeel) {
            component.setLookAndFeel (nullptr);
        }

        for (auto i = 0; i < component.getNumChildComponents(); ++i) {
            auto* child = component.getChildComponent (i);
            if (child != nullptr) {
                clearOverlayLookAndFeelRecursively (*child);
            }
        }
    }

    static bool usesOverlayLookAndFeel (juce::Component& component) {
        return dynamic_cast<juce::TextButton*> (&component) != nullptr
            || dynamic_cast<juce::ComboBox*> (&component) != nullptr
            || dynamic_cast<juce::TextEditor*> (&component) != nullptr
            || dynamic_cast<juce::ListBox*> (&component) != nullptr;
    }

    void bringPanelControlsToFront() {
        overlayTitleLabel.toFront (false);
        closeOverlayButton.toFront (false);
    }

    void updatePanelLayerBounds() {
        if (lightweight || panelBounds.isEmpty()) {
            panelLayerBounds = {};
            resetPanelShadowCache();
            return;
        }

        const auto newPanelLayerBounds = panelBounds.expanded (panelShadowPadding).getIntersection (getLocalBounds());
        if (panelLayerBounds != newPanelLayerBounds) {
            resetPanelShadowCache();
        }

        panelLayerBounds = newPanelLayerBounds;
    }

    void resetPanelShadowCache() {
        panelShadowImage = {};
        panelShadowImageBounds = {};
        panelShadowPanelBounds = {};
        panelShadowImageScale = 0.0f;
    }

    void refreshThemeColours() {
        contentViewport.setColour (juce::ScrollBar::thumbColourId, Colours::grey());
        contentViewport.setColour (juce::ScrollBar::trackColourId, Colours::transparent());
        contentViewport.setColour (scrollFadeOverlayBackgroundColourId, Colours::veryDark().brighter (0.015f));
        overlayTitleLabel.setColour (juce::Label::textColourId, Colours::text());
        closeOverlayButton.setColours (Colours::textMuted(), Colours::text());
        resetPanelShadowCache();
        repaint();
    }

    void lookAndFeelChanged() override {
        refreshThemeColours();
    }

    void updateBackdropAlpha() {
        blurredBackdropLayer.setLayerOpacity (lightweight ? 0.0f : transitionProgress);
        backdropLayer.setVisible (!lightweight
                                  && backdropSnapshot.isValid()
                                  && (dismissInProgress || transitionProgress < 0.9999f));
    }

    void updatePanelTransform() {
        updateAnimatedPanelOpacity();

        if (panelBounds.isEmpty()) {
            getAnimatedPanelLayer().setTransform (juce::AffineTransform());
            return;
        }

        const auto scale = juce::jmap (transitionProgress, 0.0f, 1.0f, 0.965f, 1.0f);
        if (scale >= 0.9999f) {
            getAnimatedPanelLayer().setTransform (juce::AffineTransform());
            return;
        }

        const auto centre = panelBounds.toFloat().getCentre();
        getAnimatedPanelLayer().setTransform (juce::AffineTransform::translation (-centre.x, -centre.y)
            .followedBy (juce::AffineTransform::scale (scale))
            .followedBy (juce::AffineTransform::translation (centre.x, centre.y)));
    }

    void updateAnimatedPanelOpacity() {
        if (usingPanelAnimationSnapshot) {
            panelAnimationLayer.setLayerOpacity (transitionProgress);
            panelLayer.setAlpha (1.0f);
            return;
        }

        panelLayer.setAlpha (transitionProgress);
    }

    juce::Component& getAnimatedPanelLayer() {
        if (usingPanelAnimationSnapshot) {
            return panelAnimationLayer;
        }

        return panelLayer;
    }

    void startPresentationAnimationIfReady() {
        if (!isAttachedForPresentation()
            || presentationStarted
            || dismissInProgress
            || panelLayerBounds.isEmpty()) {
            return;
        }

        transitionController.snapTo (false);
        beginPanelSnapshotAnimation();
        updateBackdropAlpha();
        updatePanelTransform();
        repaint();

        const juce::Component::SafePointer<OverlayComponent> safeThis { this };
        juce::MessageManager::callAsync ([safeThis] {
            if (safeThis != nullptr) {
                safeThis->runPresentationAnimation();
            }
        });
    }

    void runPresentationAnimation() {
        if (!isAttachedForPresentation() || presentationStarted || dismissInProgress || panelLayerBounds.isEmpty()) {
            return;
        }

        presentationStarted = true;
        if (!usingPanelAnimationSnapshot) {
            beginPanelSnapshotAnimation();
        }

        transitionController.animateTo (true,
                                        transitionDurationMs,
                                        createPopInEasing(),
                                        [this] {
                                            finishPanelSnapshotAnimation();
                                        });
    }

    void beginPanelSnapshotAnimation() {
        if (lightweight || panelLayerBounds.isEmpty()) {
            usingPanelAnimationSnapshot = false;
            return;
        }

        auto snapshot = createPanelAnimationSnapshot();
        if (!snapshot.isValid()) {
            usingPanelAnimationSnapshot = false;
            return;
        }

        panelAnimationLayer.setBounds (panelLayerBounds);
        panelAnimationLayer.setImage (std::move (snapshot));
        panelAnimationLayer.setLayerOpacity (transitionProgress);
        panelAnimationLayer.setVisible (true);
        panelAnimationLayer.toFront (false);

        panelLayer.setVisible (false);
        panelLayer.setAlpha (1.0f);
        panelLayer.setTransform (juce::AffineTransform());
        usingPanelAnimationSnapshot = true;
        updatePanelTransform();
    }

    void finishPanelSnapshotAnimation() {
        if (!usingPanelAnimationSnapshot) {
            return;
        }

        usingPanelAnimationSnapshot = false;
        panelAnimationLayer.setImage ({});
        panelAnimationLayer.setLayerOpacity (1.0f);
        panelAnimationLayer.setTransform (juce::AffineTransform());

        panelLayer.setVisible (true);
        panelLayer.toFront (false);
        bringPanelControlsToFront();
        panelLayer.setAlpha (1.0f);
        panelLayer.setTransform (juce::AffineTransform());
    }

    juce::Image createPanelAnimationSnapshot() {
        const auto previousAlpha = panelLayer.getAlpha();
        const auto previousTransform = panelLayer.getTransform();
        const auto wasVisible = panelLayer.isVisible();

        panelLayer.setVisible (true);
        panelLayer.setAlpha (1.0f);
        panelLayer.setTransform (juce::AffineTransform());

        auto snapshot = panelLayer.createComponentSnapshot (panelLayer.getLocalBounds(), false, getPanelSnapshotScale());

        panelLayer.setTransform (previousTransform);
        panelLayer.setAlpha (previousAlpha);
        panelLayer.setVisible (wasVisible);

        return snapshot;
    }

    float getPanelSnapshotScale() const {
        const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect (panelLayer.getScreenBounds());
        const auto displayScale = display != nullptr ? static_cast<float>(display->scale) : 1.0f;
        const auto componentScale = juce::Component::getApproximateScaleFactorForComponent (&panelLayer);
        return juce::jlimit (1.0f, 2.0f, displayScale * componentScale);
    }

    static std::function<float (float)> createPopInEasing() {
        return juce::Easings::createCubicBezier (0.18f, 0.72f, 0.26f, 1.0f);
    }

    bool isAttachedForPresentation() const {
        return getParentComponent() != nullptr || isOnDesktop();
    }

    static constexpr int transitionDurationMs = 300;
    static constexpr size_t backdropBlurRadius = 18;
    static constexpr float backdropSnapshotScale = 0.45f;
    static constexpr int panelPadding = 24;
    static constexpr int panelHeaderHeight = 28;
    static constexpr int panelHeaderGap = 10;
    static constexpr int panelShadowPadding = 36;

    OverlayLookAndFeel overlayLookAndFeel;
    ImageLayer backdropLayer;
    ImageLayer blurredBackdropLayer;
    PanelLayer panelLayer;
    ContentViewport contentViewport;
    ContentLayer contentLayer { *this };
    ImageLayer panelAnimationLayer;
    ToggleAnimationController transitionController;
    juce::Image backdropSnapshot;
    melatonin::CachedBlur backdropBlur { backdropBlurRadius };
    juce::Rectangle<int> panelLayerBounds;
    juce::Image panelShadowImage;
    juce::Rectangle<int> panelShadowImageBounds;
    juce::Rectangle<int> panelShadowPanelBounds;
    float panelShadowImageScale = 0.0f;
    juce::Label overlayTitleLabel;
    CloseButton closeOverlayButton;
    bool dismissible = true;
    bool reserveHeaderSpace = true;
    bool presentationStarted = false;
    bool dismissInProgress = false;
    bool overlayLayoutUpdatePending = false;
    bool usingPanelAnimationSnapshot = false;
    bool backdropPrepared = false;
    float transitionProgress = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayComponent)
};

class ComponentOverlay : public OverlayComponent {
public:
    ComponentOverlay (std::unique_ptr<juce::Component> contentToUse,
                      juce::String closeButtonSvg,
                      juce::String title,
                      juce::Point<int> preferredContentSizeToUse,
                      bool shouldReserveHeaderSpace)
        : OverlayComponent (std::move (closeButtonSvg)),
          ownedContent (std::move (contentToUse)),
          content (ownedContent.get()),
          reserveHeaderSpaceForPreferredSize (shouldReserveHeaderSpace) {
        initialiseContent (std::move (title), preferredContentSizeToUse, shouldReserveHeaderSpace);
    }

    ComponentOverlay (juce::Component& contentToUse,
                      juce::String closeButtonSvg,
                      juce::String title,
                      juce::Point<int> preferredContentSizeToUse,
                      bool shouldReserveHeaderSpace)
        : OverlayComponent (std::move (closeButtonSvg)),
          content (&contentToUse),
          reserveHeaderSpaceForPreferredSize (shouldReserveHeaderSpace) {
        initialiseContent (std::move (title), preferredContentSizeToUse, shouldReserveHeaderSpace);
    }

private:
    std::unique_ptr<juce::Component> ownedContent;
    juce::Component* content = nullptr;
    juce::Point<int> preferredContentSize;
    bool reserveHeaderSpaceForPreferredSize = true;

    void initialiseContent (juce::String title,
                            juce::Point<int> preferredContentSizeToUse,
                            bool shouldReserveHeaderSpace) {
        setOverlayTitle (title);
        setReserveHeaderSpace (shouldReserveHeaderSpace);
        if (content != nullptr) {
            preferredContentSize = preferredContentSizeToUse;
            if (preferredContentSize.x <= 0 || preferredContentSize.y <= 0) {
                preferredContentSize = { content->getWidth(), content->getHeight() };
            }

            addPanelContentAndMakeVisible (*content);
        }
    }

    juce::Point<int> getPreferredPanelSize() const override {
        return getPanelSizeForContentSize (preferredContentSize, reserveHeaderSpaceForPreferredSize);
    }

    void resizeContent (juce::Rectangle<int> area) override {
        if (content == nullptr) {
            return;
        }

        content->setBounds (area);
        if (updatePreferredContentSizeFromContent()) {
            requestOverlayLayout();
        }
    }

    void panelChildBoundsChanged (juce::Component* child) override {
        if (child != content) {
            return;
        }

        if (updatePreferredContentSizeFromContent()) {
            requestOverlayLayout();
        }
    }

    bool updatePreferredContentSizeFromContent() {
        if (content == nullptr) {
            return false;
        }

        const juce::Point<int> contentSize { content->getWidth(), content->getHeight() };
        if (contentSize.x <= 0 || contentSize.y <= 0) {
            return false;
        }

        const juce::Point<int> newPreferredContentSize {
            juce::jmax (preferredContentSize.x, contentSize.x),
            juce::jmax (preferredContentSize.y, contentSize.y)
        };

        if (newPreferredContentSize == preferredContentSize) {
            return false;
        }

        preferredContentSize = newPreferredContentSize;
        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentOverlay)
};

} // namespace osci
