#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <osci_render_core/effect/osci_EffectParameter.h>

#include "../osci_BooleanParamCCHelper.h"
#include "../../lookandfeel/osci_LookAndFeel.h"

// A toggleable rounded-label pill that wraps a BooleanParameter.
// When enabled, the label text is bright and the pill has an accent highlight.
// When disabled, the text and pill are dimmed.
// Clicking toggles the parameter and notifies the DAW host.
class ToggleLabelComponent : public osci::HoverAnimationMixin,
                             public juce::SettableTooltipClient,
                             public juce::AudioProcessorParameter::Listener,
                             public juce::AsyncUpdater {
    class ToggleAccessibilityHandler final : public juce::AccessibilityHandler {
    public:
        explicit ToggleAccessibilityHandler(ToggleLabelComponent& owner)
            : juce::AccessibilityHandler(owner, juce::AccessibilityRole::toggleButton, makeActions(owner)), owner(owner) {}

        juce::AccessibleState getCurrentState() const override {
            auto state = juce::AccessibilityHandler::getCurrentState().withCheckable();
            return owner.parameter->getBoolValue() ? state.withChecked() : state;
        }

    private:
        static juce::AccessibilityActions makeActions(ToggleLabelComponent& owner) {
            juce::AccessibilityActions actions;
            actions.addAction(juce::AccessibilityActionType::press, [&owner] { owner.toggleParameter(); });
            return actions;
        }

        ToggleLabelComponent& owner;
    };

public:
    explicit ToggleLabelComponent(osci::BooleanParameter* param, juce::String label = {}, bool useUppercase = true)
        : parameter(param),
          displayText(label.isNotEmpty() ? std::move(label) : param->name),
          toggleAnimation(this) {
        jassert(param != nullptr);
        if (useUppercase) {
            displayText = displayText.toUpperCase();
        }
        setTooltip(param->getDescription());
        setTitle(displayText);
        setDescription(param->getDescription());
        parameter->addListener(this);
        ccHelper.init(param, this);
        setWantsKeyboardFocus(true);
        toggleAnimation.setValueChangedCallback([this](float progress) {
            toggleProgress = progress;
            repaint();
        });
        toggleAnimation.snapTo(parameter->getBoolValue());
    }

    ~ToggleLabelComponent() override {
        parameter->removeListener(this);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f, 0.0f);
        const auto hoverProgress = isEnabled() ? getAnimationProgress() : 0.0f;
        const auto disabledAlpha = isEnabled() ? 1.0f : 0.45f;

        juce::Colour bgColour;
        if (ccHelper.isLearning()) {
            bgColour = osci::Colours::midiLearnBackground();
        } else {
            const auto offColour = osci::Colours::evenDarker();
            const auto onColour = osci::Colours::accentColor().withMultipliedBrightness(0.5f);
            const auto hoverOffColour = osci::Colours::dark();
            const auto hoverOnColour = osci::Colours::accentColor().withMultipliedBrightness(0.62f);
            bgColour = offColour.interpolatedWith(onColour, toggleProgress);
            const auto hoverColour = hoverOffColour.interpolatedWith(hoverOnColour, toggleProgress);
            bgColour = bgColour.interpolatedWith(hoverColour, hoverProgress);
        }
        g.setColour(bgColour.withMultipliedAlpha(disabledAlpha));
        g.fillRoundedRectangle(bounds, osci::Colours::kPillRadius);

        const auto selectedTextColour = juce::Colours::white;
        auto textColour = osci::Colours::text().interpolatedWith(selectedTextColour, toggleProgress);
        const auto hoverTextColour = osci::Colours::text().brighter(0.12f).interpolatedWith(selectedTextColour, toggleProgress);
        textColour = ccHelper.isLearning() ? osci::Colours::text()
                                          : textColour.interpolatedWith(hoverTextColour, hoverProgress * 0.45f);
        g.setColour(textColour.withMultipliedAlpha(disabledAlpha));
        g.setFont(juce::Font{juce::FontOptions{juce::jlimit(9.0f, 12.0f, getHeight() * 0.4f)}});
        g.drawText(displayText, bounds, juce::Justification::centred, false);

        if (toggleProgress > 0.001f) {
            const auto diameter = juce::jlimit(3.0f, 5.0f, bounds.getHeight() * 0.18f);
            const auto indicator = juce::Rectangle<float>(diameter, diameter).withCentre({bounds.getX() + 8.0f, bounds.getY() + 7.0f});
            g.setColour(textColour.withMultipliedAlpha(toggleProgress * disabledAlpha));
            g.fillEllipse(indicator);
        }

    }

    void mouseEnter(const juce::MouseEvent& event) override {
        osci::HoverAnimationMixin::mouseEnter(event);
        if (isEnabled()) {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }
    }

    void mouseExit(const juce::MouseEvent& event) override {
        osci::HoverAnimationMixin::mouseExit(event);
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    void mouseDown(const juce::MouseEvent& event) override {
        osci::HoverAnimationMixin::mouseDown(event);
    }

    void mouseUp(const juce::MouseEvent& e) override {
        osci::HoverAnimationMixin::mouseUp(e);
        if (!isEnabled()) {
            return;
        }
        if (e.mouseWasClicked() && e.mods.isPopupMenu()) {
            ccHelper.showContextMenu(e.getScreenPosition());
            return;
        }
        if (e.mouseWasClicked() && getLocalBounds().toFloat().contains(e.position)) {
            grabKeyboardFocus();
            toggleParameter();
        }
    }

    bool keyPressed(const juce::KeyPress& key) override {
        if (isEnabled() && (key == juce::KeyPress::returnKey || key == juce::KeyPress::spaceKey)) {
            toggleParameter();
            return true;
        }
        return false;
    }

    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override {
        return std::make_unique<ToggleAccessibilityHandler>(*this);
    }

    void parameterValueChanged(int, float) override { triggerAsyncUpdate(); }
    void parameterGestureChanged(int, bool) override {}
    void handleAsyncUpdate() override {
        const auto shouldBeOn = parameter->getBoolValue();
        if (toggleAnimation.getTargetState() != shouldBeOn) {
            toggleAnimation.animateTo(shouldBeOn, 85, juce::Easings::createEaseInOut());
            if (auto* handler = getAccessibilityHandler()) {
                handler->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
            }
        }
        repaint();
    }

protected:
    int getHoverAnimationDurationMs() const override { return 65; }

private:
    void toggleParameter() {
        if (!isEnabled()) {
            return;
        }
        parameter->beginChangeGesture();
        parameter->setBoolValueNotifyingHost(!parameter->getBoolValue());
        parameter->endChangeGesture();
    }

    osci::BooleanParameter* parameter;
    juce::String displayText;
    BooleanParamCCHelper ccHelper;
    osci::ToggleAnimationController toggleAnimation;
    float toggleProgress = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleLabelComponent)
};
