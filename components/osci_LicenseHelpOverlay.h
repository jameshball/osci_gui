#pragma once

namespace osci {

class LicenseHelpOverlay final : public OverlayComponent {
public:
    explicit LicenseHelpOverlay (juce::String closeButtonSvg)
        : OverlayComponent (std::move (closeButtonSvg)) {
        setOverlayTitle ("Find your license key");

        configureLabel (messageLabel, juce::Font (juce::FontOptions (14.2f)), juce::Justification::centredLeft);
        messageLabel.setText ("Your key is on the download page linked from your purchase email.",
                              juce::dontSendNotification);
        messageLabel.setMinimumHorizontalScale (0.82f);
        addPanelContentAndMakeVisible (messageLabel);

        juce::StringArray licenseSteps;
        licenseSteps.add ("Open the download link from your Gumroad or Payhip email.");
        licenseSteps.add ("Copy the license key shown near the top of the page.");
        licenseSteps.add ("Paste it into the installer and activate.");
        stepsComponent.setSteps (licenseSteps);
        stepsComponent.setFont (juce::Font (juce::FontOptions (14.2f)));
        stepsComponent.setMetrics (28, 13, 21);
        addPanelContentAndMakeVisible (stepsComponent);

        configureLabel (supportLabel, juce::Font (juce::FontOptions (13.0f)), juce::Justification::centred);
        supportLabel.setText ("Still stuck?", juce::dontSendNotification);
        addPanelContentAndMakeVisible (supportLabel);

        setupButton (discordButton,
                     "Join Discord",
                     juce::URL ("https://discord.gg/ekjpQvT68C"),
                     Colours::categoryScripting());
        setupButton (emailButton,
                     "Email james@ball.sh",
                     juce::URL ("mailto:james@ball.sh"),
                     Colours::accentColor());
        refreshThemeColours();
    }

private:
    juce::Label messageLabel;
    NumberedStepsComponent stepsComponent;
    juce::Label supportLabel;
    juce::TextButton discordButton;
    juce::TextButton emailButton;

    juce::Point<int> getPreferredPanelSize() const override {
        return { 520, 318 };
    }

    void resizeContent (juce::Rectangle<int> area) override {
        messageLabel.setBounds (area.removeFromTop (24));
        area.removeFromTop (16);

        stepsComponent.setBounds (area.removeFromTop (stepsComponent.getPreferredHeight()));

        area.removeFromTop (24);
        supportLabel.setBounds (area.removeFromTop (20));
        area.removeFromTop (8);

        auto buttonRow = area.removeFromTop (32).withSizeKeepingCentre (310, 32);
        discordButton.setBounds (buttonRow.removeFromLeft (148));
        buttonRow.removeFromLeft (14);
        emailButton.setBounds (buttonRow.removeFromLeft (148));
    }

    void lookAndFeelChanged() override {
        refreshThemeColours();
    }

    void setupButton (juce::TextButton& button,
                      const juce::String& text,
                      juce::URL url,
                      juce::Colour colour) {
        button.setButtonText (text);
        styleButton (button, colour);
        button.onClick = [url] {
            url.launchInDefaultBrowser();
        };
        addPanelContentAndMakeVisible (button);
    }

    void refreshThemeColours() {
        messageLabel.setColour (juce::Label::textColourId, Colours::textMuted());
        stepsComponent.setTextColour (Colours::textMuted());
        stepsComponent.setAccentColour (Colours::accentColor());
        supportLabel.setColour (juce::Label::textColourId, Colours::textSubtle());
        styleButton (discordButton, Colours::categoryScripting());
        styleButton (emailButton, Colours::accentColor());
        repaint();
    }

    static void styleButton (juce::TextButton& button, juce::Colour colour) {
        button.setColour (juce::TextButton::buttonColourId, colour.withAlpha (0.11f));
        button.setColour (juce::TextButton::buttonOnColourId, colour.withAlpha (0.18f));
        button.setColour (juce::TextButton::textColourOffId, colour);
        button.setColour (juce::TextButton::textColourOnId, colour);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseHelpOverlay)
};

} // namespace osci
