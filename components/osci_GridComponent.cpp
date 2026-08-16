#include "osci_GridComponent.h"

namespace osci {

GridComponent::GridComponent(juce::FlexBox::JustifyContent alignment) : justifyContent(alignment) {
    // Default: use internal viewport
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
    viewport.setScrollBarsShown(true, false); // vertical only
}

GridComponent::~GridComponent() = default;

void GridComponent::clearItems() {
    content.removeAllChildren();
    layoutItems.clear();
    ownedItems.clear();
}

void GridComponent::addItem(GridItemComponent* item) {
    item->setInteractive(itemsInteractive);
    ownedItems.add(item);
    layoutItems.add(item);
    content.addAndMakeVisible(item);
}

void GridComponent::addItem(juce::Component& item) {
    layoutItems.add(&item);
    content.addAndMakeVisible(item);
}

void GridComponent::paint(juce::Graphics& g) {
    // transparent background
}

void GridComponent::resized() {
    auto bounds = getLocalBounds();
    juce::Rectangle<int> contentArea;
    if (useInternalViewport) {
        viewport.setBounds(bounds);
        viewport.setFadeVisible(true);
        contentArea = viewport.getLocalBounds();
        // Lock content width to viewport width to avoid horizontal scrolling
        content.setSize(contentArea.getWidth(), content.getHeight());
    } else {
        // No internal viewport: lay out content directly within our bounds
        viewport.setBounds(0, 0, 0, 0);
        viewport.setFadeVisible(false);
        contentArea = bounds;
        content.setBounds(contentArea);
        content.setSize(contentArea.getWidth(), contentArea.getHeight());
    }

    // Create FlexBox for responsive grid layout within content
    flexBox = juce::FlexBox();
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.justifyContent = justifyContent;
    flexBox.alignContent = juce::FlexBox::AlignContent::flexStart;
    flexBox.flexDirection = juce::FlexBox::Direction::row;

    // Determine fixed per-item width for this viewport width
    const int viewW = contentArea.getWidth();
    const int itemsPerRow = juce::jmax(1, viewW / minItemWidth);
    const int slotWidth = itemsPerRow > 0 ? viewW / itemsPerRow : viewW;
    const int fixedItemWidth = juce::jmax(1, slotWidth - 2 * itemMargin);
    const int fixedItemHeight = juce::jmax(1, itemHeight - 2 * itemMargin);

    auto addItemFlex = [&](juce::Component* component) {
        flexBox.items.add(juce::FlexItem(*component)
                              .withMinWidth((float) fixedItemWidth)
                              .withMaxWidth((float) fixedItemWidth)
                              .withHeight((float) fixedItemHeight)
                              .withFlex(1.0f)
                              .withMargin(juce::FlexItem::Margin((float)itemMargin)));
    };

    for (auto* item : layoutItems) {
        addItemFlex(item);
    }

    // Compute required content height
    const int requiredHeight = calculateRequiredHeight(viewW);

    // If content is shorter than container, fill height; otherwise, set to required height
    int yOffset = 0;
    if (useInternalViewport) {
        const int viewH = contentArea.getHeight();
        if (requiredHeight < viewH) {
            content.setSize(viewW, viewH);
            yOffset = (viewH - requiredHeight) / 2;
        } else {
            content.setSize(viewW, requiredHeight);
        }
        // Layout items within content at the computed offset
        flexBox.performLayout(juce::Rectangle<float>(0.0f, (float) yOffset, (float) viewW, (float) requiredHeight));
    } else {
        content.setSize(viewW, requiredHeight);
        flexBox.performLayout(juce::Rectangle<float>(0.0f, 0.0f, (float) viewW, (float) requiredHeight));
    }
}

int GridComponent::calculateRequiredHeight(int availableWidth) const {
    if (layoutItems.isEmpty()) {
        return itemHeight;
    }

    // Calculate how many items can fit per row
    int itemsPerRow = juce::jmax(1, availableWidth / minItemWidth);

    // Calculate number of rows needed
    int numRows = (layoutItems.size() + itemsPerRow - 1) / itemsPerRow; // Ceiling division

    return numRows * itemHeight;
}

void GridComponent::setItemsInteractive(bool shouldBeInteractive) {
    if (itemsInteractive == shouldBeInteractive) {
        return;
    }

    itemsInteractive = shouldBeInteractive;

    for (auto* item : ownedItems) {
        if (item != nullptr) {
            item->setInteractive(shouldBeInteractive);
        }
    }
}

void GridComponent::setItemHeight(int newHeight) {
    newHeight = juce::jmax(1, newHeight);
    if (itemHeight == newHeight) {
        return;
    }

    itemHeight = newHeight;
    resized();
}

void GridComponent::setItemMargin(int newMargin) {
    newMargin = juce::jmax(0, newMargin);
    if (itemMargin == newMargin) {
        return;
    }

    itemMargin = newMargin;
    resized();
}

void GridComponent::setJustifyContent(juce::FlexBox::JustifyContent alignment) {
    if (justifyContent == alignment) {
        return;
    }

    justifyContent = alignment;
    resized();
}

void GridComponent::setMinItemWidth(int newWidth) {
    newWidth = juce::jmax(1, newWidth);
    if (minItemWidth == newWidth) {
        return;
    }

    minItemWidth = newWidth;
    resized();
}

int GridComponent::getItemWidthFor(int availableWidth) const {
    if (availableWidth <= 0) {
        return 0;
    }

    const int itemsPerRow = juce::jmax(1, availableWidth / minItemWidth);
    return juce::jmax(1, availableWidth / itemsPerRow);
}


void GridComponent::setUseViewport(bool shouldUseViewport) {
    if (useInternalViewport == shouldUseViewport) {
        return;
    }

    useInternalViewport = shouldUseViewport;

    if (useInternalViewport) {
        // Reattach content to viewport and attach fade listeners
        if (viewport.getViewedComponent() != &content) {
            viewport.setViewedComponent(&content, false);
        }
    } else {
        // Hide viewport and lay out items directly
        viewport.setViewedComponent(nullptr, false);
        if (content.getParentComponent() != this) {
            addAndMakeVisible(content);
        }
    }
    resized();
}

} // namespace osci
