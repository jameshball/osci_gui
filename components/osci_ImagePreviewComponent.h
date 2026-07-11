#pragma once

namespace osci {

class ImagePreviewComponent final : public juce::Button {
public:
    ImagePreviewComponent();

    void setImage(juce::Image newImage);
    const juce::Image& getImage() const;
    void setCaption(juce::String newCaption);
    void setAccentColour(juce::Colour colour);
    void setMagnifierSvg(juce::String svg);

    std::function<void()> onOpenRequested;

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

private:
    void rebuildMagnifier(juce::StringRef svg);

    juce::Image image;
    juce::String caption;
    juce::Colour accentColour { Colours::accentColor() };
    std::unique_ptr<juce::Drawable> magnifier;
    ToggleAnimationController hoverAnimation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagePreviewComponent)
};

} // namespace osci
