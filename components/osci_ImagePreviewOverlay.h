#pragma once

namespace osci {

class ImagePreviewOverlay final : public OverlayComponent {
public:
    ImagePreviewOverlay(juce::String closeButtonSvg, juce::Image image, juce::String title = {});

protected:
    void resizeContent(juce::Rectangle<int> contentArea) override;
    juce::Point<int> getPreferredPanelSize() const override;

private:
    class ImageCanvas final : public juce::Component {
    public:
        void setImage(juce::Image newImage);
        void paint(juce::Graphics& g) override;

    private:
        juce::Image image;
    };

    juce::Image image;
    ImageCanvas canvas;
    juce::Label metadataLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagePreviewOverlay)
};

} // namespace osci
