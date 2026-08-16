#pragma once

// Generic responsive grid for both owned GridItemComponents and externally-owned components.
namespace osci {

class GridComponent : public juce::Component {
public:
    explicit GridComponent(juce::FlexBox::JustifyContent alignment = juce::FlexBox::JustifyContent::center);
    ~GridComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void clearItems();
    void addItem(GridItemComponent* item); // takes ownership
    void addItem(juce::Component& item); // caller retains ownership
    juce::OwnedArray<GridItemComponent>& getItems() { return ownedItems; }
    int getNumItems() const { return layoutItems.size(); }
    int calculateRequiredHeight(int availableWidth) const;

    void setJustifyContent(juce::FlexBox::JustifyContent alignment);
    juce::FlexBox::JustifyContent getJustifyContent() const { return justifyContent; }

    // Legacy convenience for existing callers. Alignment is now handled directly by FlexBox.
    void setUseCenteringPlaceholders(bool shouldCenter) {
        setJustifyContent(shouldCenter ? juce::FlexBox::JustifyContent::center
                                      : juce::FlexBox::JustifyContent::flexStart);
    }

    // Configuration: when true (default), GridComponent uses its own internal Viewport.
    // When false, the grid lays out directly without an internal scroll container (for embedding
    // inside a parent Viewport).
    void setUseViewport(bool shouldUseViewport);

    // When false, items will not respond to clicks or pointer cursor changes.
    void setItemsInteractive(bool shouldBeInteractive);

    // Adjust the fixed height allocated to each grid tile (default 80px).
    void setItemHeight(int newHeight);
    int getItemHeight() const { return itemHeight; }
    void setItemMargin(int newMargin);
    int getItemMargin() const { return itemMargin; }

    void setMinItemWidth(int newWidth);
    int getMinItemWidth() const { return minItemWidth; }
    int getItemWidthFor(int availableWidth) const;

private:
    ScrollFadeViewport viewport; // scroll container with fades
    juce::Component content; // holds the grid items
    juce::OwnedArray<GridItemComponent> ownedItems;
    juce::Array<juce::Component*> layoutItems;
    juce::FlexBox flexBox;

    static constexpr int DEFAULT_ITEM_HEIGHT = 80;
    static constexpr int DEFAULT_MIN_ITEM_WIDTH = 200;

    bool useInternalViewport { true };
    bool itemsInteractive { true };
    int itemHeight { DEFAULT_ITEM_HEIGHT };
    int itemMargin { 0 };
    int minItemWidth { DEFAULT_MIN_ITEM_WIDTH };
    juce::FlexBox::JustifyContent justifyContent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridComponent)
};

} // namespace osci
