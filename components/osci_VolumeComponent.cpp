#include "osci_VolumeComponent.h"

namespace osci {

void VolumeComponent::ThresholdThumbLookAndFeel::drawLinearSliderThumb(juce::Graphics& g,
                                                                       int x,
                                                                       int y,
                                                                       int width,
                                                                       int height,
                                                                       float sliderPos,
                                                                       float minSliderPos,
                                                                       float maxSliderPos,
                                                                       const juce::Slider::SliderStyle style,
                                                                       juce::Slider& slider) {
    juce::ignoreUnused(y, width, height, minSliderPos, maxSliderPos, style);

    auto sliderRadius = static_cast<float>(getSliderThumbRadius(slider));
    auto diameter = sliderRadius * 2.0f;
    auto isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
    auto thumbY = sliderPos - sliderRadius;

    juce::Path p;
    p.startNewSubPath(static_cast<float>(x), thumbY + diameter * 0.5f);
    p.lineTo(static_cast<float>(x) + diameter, thumbY);
    p.lineTo(static_cast<float>(x) + diameter, thumbY + diameter);
    p.closeSubPath();

    g.setColour(juce::Colours::white.withAlpha(isDownOrDragging ? 1.0f : 0.9f));
    g.fillPath(p);
}

void VolumeComponent::ThresholdThumbLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                                                  int x,
                                                                  int y,
                                                                  int width,
                                                                  int height,
                                                                  float sliderPos,
                                                                  float minSliderPos,
                                                                  float maxSliderPos,
                                                                  const juce::Slider::SliderStyle style,
                                                                  juce::Slider& slider) {
    drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}

VolumeComponent::VolumeComponent(AudioBackgroundThreadManager& threadManager,
                                 EffectParameter& volumeParameterToUse,
                                 EffectParameter& thresholdParameterToUse,
                                 BooleanParameter& muteParameterToUse,
                                 juce::String volumeSvg,
                                 juce::String muteSvg,
                                 float outputClipBypassThresholdToUse,
                                 float outputClipPeakEpsilonToUse)
    : AudioBackgroundThread("VolumeComponent", threadManager),
      volumeParameter(volumeParameterToUse),
      thresholdParameter(thresholdParameterToUse),
      muteParameter(muteParameterToUse),
      outputClipBypassThreshold(outputClipBypassThresholdToUse),
      outputClipPeakEpsilon(outputClipPeakEpsilonToUse),
      volumeButton("VolumeButton", std::move(volumeSvg), juce::Colours::white, juce::Colours::red, &muteParameter, std::move(muteSvg)) {
    setOpaque(false);
    setShouldBeRunning(true);

    leftVolumeSmoothed.reset(10);
    rightVolumeSmoothed.reset(10);
    leftVolumeSmoothed.setCurrentAndTargetValue(0.0f);
    rightVolumeSmoothed.setCurrentAndTargetValue(0.0f);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::transparentWhite);
    volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    volumeSlider.setOpaque(false);
    volumeSlider.setRange(volumeParameter.min.load(), volumeParameter.max.load(), volumeParameter.step.load());
    volumeSlider.setValue(volumeParameter.getValueUnnormalised());
    volumeSlider.setDoubleClickReturnValue(true, 1.0);
    volumeSlider.setLookAndFeel(&volumeLookAndFeel);
    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::white);

    addAndMakeVisible(thresholdSlider);
    thresholdSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    thresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    thresholdSlider.setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::transparentWhite);
    thresholdSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    thresholdSlider.setOpaque(false);
    thresholdSlider.setRange(thresholdParameter.min.load(), thresholdParameter.max.load(), thresholdParameter.step.load());
    thresholdSlider.setValue(thresholdParameter.getValueUnnormalised());
    thresholdSlider.setDoubleClickReturnValue(true, 1.0);
    thresholdSlider.setLookAndFeel(&thresholdLookAndFeel);
    thresholdSlider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::black);

    volumeSlider.onValueChange = [this] {
        volumeParameter.setUnnormalisedValueNotifyingHost(static_cast<float>(volumeSlider.getValue()));
    };

    thresholdSlider.onValueChange = [this] {
        thresholdParameter.setUnnormalisedValueNotifyingHost(static_cast<float>(thresholdSlider.getValue()));
    };

    addAndMakeVisible(volumeButton);
    volumeButton.onClick = [this] {
        muteParameter.setBoolValueNotifyingHost(!muteParameter.getBoolValue());
    };
}

VolumeComponent::~VolumeComponent() {
    setShouldBeRunning(false);
}

bool VolumeComponent::isOutputClipActive(float threshold) const noexcept {
    return threshold < outputClipBypassThreshold;
}

void VolumeComponent::paint(juce::Graphics& g) {
    g.setColour(juce::Colours::transparentBlack);
    g.fillAll();

    auto r = getLocalBounds().toFloat();
    r.removeFromTop(20.0f);
    r.removeFromRight(r.getWidth() / 2.0f);
    r.removeFromTop(static_cast<float>(volumeSlider.getLookAndFeel().getSliderThumbRadius(volumeSlider)));
    r.removeFromBottom(static_cast<float>(volumeSlider.getLookAndFeel().getSliderThumbRadius(volumeSlider)));

    auto channelHeight = r.getHeight();
    auto leftVolumeValue = juce::jlimit(0.0f, 1.0f, leftVolume.load());
    auto rightVolumeValue = juce::jlimit(0.0f, 1.0f, rightVolume.load());
    auto leftPeakValue = leftPeak.load();
    auto rightPeakValue = rightPeak.load();
    auto thresholdValue = static_cast<float>(thresholdSlider.getValue());
    auto clipActive = isOutputClipActive(thresholdValue);

    auto overallRect = r;
    auto leftRect = r.removeFromLeft(r.getWidth() / 2.0f);
    auto rightRect = r;
    auto leftRegion = leftRect;
    auto rightRegion = rightRect;

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRoundedRectangle(overallRect, 4.0f, 1.0f);

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    auto drawVolumeMeter = [&](juce::Rectangle<float> rect, float volume) {
        auto displayVolume = juce::jlimit(0.0f, 1.0f, volume);
        auto meterRect = rect.removeFromBottom(displayVolume * channelHeight);
        auto gradient = juce::ColourGradient(juce::Colour(0xff00ff00),
                                             0.0f,
                                             overallRect.getBottom(),
                                             juce::Colour(0xffff0000),
                                             0.0f,
                                             overallRect.getY(),
                                             false);
        gradient.addColour(0.3, juce::Colour(0xffffe600));
        g.setGradientFill(gradient);
        g.saveState();
        g.reduceClipRegion(meterRect.toNearestInt());
        g.fillRoundedRectangle(meterRect.reduced(1.0f), 2.0f);
        g.restoreState();
    };

    drawVolumeMeter(leftRect, leftVolumeValue);
    drawVolumeMeter(rightRect, rightVolumeValue);

    auto drawSmoothIndicator = [&](const juce::Rectangle<float>& region, float value) {
        auto displayValue = juce::jlimit(0.0f, 1.0f, value);

        if (displayValue <= 0.0f) {
            return;
        }

        auto y = region.getBottom() - displayValue * channelHeight;
        auto indicatorHeight = 3.0f;
        auto indicatorRect = juce::Rectangle<float>(region.getX(), y - indicatorHeight / 2.0f, region.getWidth(), indicatorHeight);

        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillRoundedRectangle(indicatorRect, 1.5f);
    };

    drawSmoothIndicator(leftRegion, leftVolumeSmoothed.getNextValue());
    drawSmoothIndicator(rightRegion, rightVolumeSmoothed.getNextValue());

    auto thresholdReference = clipActive ? juce::jlimit(0.0f, 1.0f, thresholdValue) : 1.0f;
    auto thresholdLineHeight = clipActive ? 3.0f : 2.0f;
    auto thresholdY = rightRegion.getBottom() - thresholdReference * channelHeight;
    thresholdY = juce::jlimit(rightRegion.getY() + thresholdLineHeight * 0.5f, rightRegion.getBottom() - thresholdLineHeight * 0.5f, thresholdY);
    auto thresholdRect = juce::Rectangle<float>(leftRegion.getX(),
                                               thresholdY - thresholdLineHeight * 0.5f,
                                               leftRegion.getWidth() + rightRegion.getWidth(),
                                               thresholdLineHeight);

    g.setColour(juce::Colours::white.withAlpha(clipActive ? 0.7f : 0.22f));
    g.fillRoundedRectangle(thresholdRect, thresholdLineHeight * 0.5f);

    auto drawPeakCap = [&](const juce::Rectangle<float>& region, float peak) {
        auto crossedClipThreshold = clipActive && peak > 0.0f && peak >= thresholdValue - outputClipPeakEpsilon;

        if (!crossedClipThreshold) {
            return;
        }

        auto capRect = juce::Rectangle<float>(region.getX() + 1.0f, region.getY() + 1.0f, region.getWidth() - 2.0f, 3.0f);

        g.setColour(juce::Colours::red.withAlpha(0.78f));
        g.fillRoundedRectangle(capRect, 1.5f);
    };

    drawPeakCap(leftRegion, leftPeakValue);
    drawPeakCap(rightRegion, rightPeakValue);
}

void VolumeComponent::handleAsyncUpdate() {
    leftVolumeSmoothed.setTargetValue(leftVolume.load());
    rightVolumeSmoothed.setTargetValue(rightVolume.load());
    repaint();
    volumeSlider.setValue(volumeParameter.getValueUnnormalised(), juce::NotificationType::dontSendNotification);
    thresholdSlider.setValue(thresholdParameter.getValueUnnormalised(), juce::NotificationType::dontSendNotification);
}

void VolumeComponent::runTask(const juce::AudioBuffer<float>& buffer) {
    const int numSamples = buffer.getNumSamples();

    if (numSamples <= 0 || buffer.getNumChannels() < 2) {
        leftVolume = 0.0f;
        rightVolume = 0.0f;
        leftPeak = 0.0f;
        rightPeak = 0.0f;
        triggerAsyncUpdate();
        return;
    }

    float leftRms = 0.0f;
    float rightRms = 0.0f;
    float leftPeakValue = 0.0f;
    float rightPeakValue = 0.0f;
    const float* leftChannel = buffer.getReadPointer(0);
    const float* rightChannel = buffer.getReadPointer(1);

    for (int i = 0; i < numSamples; i++) {
        auto leftSample = leftChannel[i];
        auto rightSample = rightChannel[i];
        leftRms += leftSample * leftSample;
        rightRms += rightSample * rightSample;
        leftPeakValue = juce::jmax(leftPeakValue, std::abs(leftSample));
        rightPeakValue = juce::jmax(rightPeakValue, std::abs(rightSample));
    }

    leftRms = std::sqrt(leftRms / static_cast<float>(numSamples));
    rightRms = std::sqrt(rightRms / static_cast<float>(numSamples));

    if (std::isnan(leftRms) || std::isnan(rightRms)) {
        leftRms = 0.0f;
        rightRms = 0.0f;
    }

    leftVolume = leftRms;
    rightVolume = rightRms;
    leftPeak = leftPeakValue;
    rightPeak = rightPeakValue;

    triggerAsyncUpdate();
}

int VolumeComponent::prepareTask(double sampleRate, int bufferSize) {
    juce::ignoreUnused(bufferSize);
    return static_cast<int>(bufferDurationSecs * sampleRate);
}

void VolumeComponent::stopTask() {}

void VolumeComponent::resized() {
    auto r = getLocalBounds();

    auto iconRow = r.removeFromTop(20);
    volumeButton.setBounds(iconRow);

    volumeSlider.setBounds(r.removeFromLeft(r.getWidth() / 2));
    thresholdSlider.setBounds(r.reduced(0, 5));
}

} // namespace osci
