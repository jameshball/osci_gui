#pragma once

namespace osci {

class OverlayListBox : public juce::ListBox {
public:
    explicit OverlayListBox (juce::String noItemsMessageToUse = {})
        : juce::ListBox ({}, nullptr),
          noItemsMessage (std::move (noItemsMessageToUse)) {
        OverlayLookAndFeel::configureListBox (*this);
    }

    void setNoItemsMessage (juce::String message) {
        noItemsMessage = std::move (message);
        repaint();
    }

    void paint (juce::Graphics& g) override {
        OverlayLookAndFeel::paintContainerBackground (g, getLocalBounds());
        juce::ListBox::paint (g);

        if (!hasListItems() && noItemsMessage.isNotEmpty()) {
            g.setColour (Colours::textSubtle());
            g.setFont (0.5f * static_cast<float>(getRowHeight()));
            g.drawText (noItemsMessage,
                        juce::Rectangle<float> (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()) * 0.5f),
                        juce::Justification::centred,
                        true);
        }
    }

    void paintOverChildren (juce::Graphics& g) override {
        OverlayLookAndFeel::drawContainerOutline (g, getLocalBounds(), findColour (juce::ListBox::outlineColourId));
    }

protected:
    virtual bool hasListItems() const { return true; }

private:
    juce::String noItemsMessage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayListBox)
};

} // namespace osci
