#pragma once

namespace osci {

class GridItemComponent : public HoverAnimationMixin, public juce::SettableTooltipClient {
public:
    enum class LayoutMode {
        List,
        IconTile,
    };

    GridItemComponent (const juce::String& name,
                       const juce::String& iconSvg,
                       const juce::String& id,
                       juce::Colour iconTint = juce::Colours::white.withAlpha (0.7f));

    GridItemComponent (const juce::String& name,
                       juce::Image icon,
                       const juce::String& id);

    ~GridItemComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseMove (const juce::MouseEvent& event) override;
    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;

    const juce::String& getId() const { return itemId; }
    const juce::String& getName() const { return itemName; }

    std::function<void (const juce::String& id)> onItemSelected;
    std::function<void (const juce::String& id)> onHoverStart;
    std::function<void()> onHoverEnd;
    std::function<void()> onLockedClick;

    void setInteractive (bool shouldBeInteractive);
    bool isInteractive() const { return interactive; }

    void setDescription (const juce::String& text);
    const juce::String& getDescription() const { return description; }

    void setLocked (bool shouldBeLocked);
    bool isLocked() const { return locked; }

    void setSelected (bool shouldBeSelected);
    bool isSelected() const { return selected; }

    void setLayoutMode (LayoutMode newMode);
    LayoutMode getLayoutMode() const { return layoutMode; }

    int getPreferredHeight (int width) const;

private:
    struct TextLayoutMetrics {
        juce::TextLayout headingLayout;
        juce::TextLayout bodyLayout;
        float headingHeight = 0.0f;
        float bodyHeight = 0.0f;
        float totalHeight = 0.0f;
        bool hasDescription = false;
    };

    [[nodiscard]] TextLayoutMetrics computeTextLayouts (int textWidth) const;
    void paintListItem (juce::Graphics& g, juce::Rectangle<float> bounds, float animationProgress);
    void paintIconTile (juce::Graphics& g, juce::Rectangle<float> bounds, float animationProgress);
    juce::Rectangle<int> getListIconBounds() const;

    juce::String itemName;
    juce::String itemId;

    std::unique_ptr<SvgButton> iconButton;
    juce::Image iconImage;

    bool interactive { true };
    bool locked { false };
    bool selected { false };
    LayoutMode layoutMode { LayoutMode::List };
    juce::String description;
    juce::Colour iconColour { juce::Colours::white.withAlpha (0.7f) };

    static constexpr int cornerRadius = 8;
    static constexpr int textHorizontalPadding = 40;
    static constexpr int titleDescriptionSpacing = 5;
    static constexpr float hoverLiftAmount = 2.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridItemComponent)
};

} // namespace osci
