#pragma once

namespace osci {

class VolumeComponent : public juce::Component,
                        public juce::AsyncUpdater,
                        public AudioBackgroundThread {
public:
    VolumeComponent(AudioBackgroundThreadManager& threadManager,
                    EffectParameter& volumeParameter,
                    EffectParameter& thresholdParameter,
                    BooleanParameter& muteParameter,
                    juce::String volumeSvg,
                    juce::String muteSvg,
                    float outputClipBypassThreshold,
                    float outputClipPeakEpsilon);
    ~VolumeComponent() override;

    void paint(juce::Graphics&) override;
    void handleAsyncUpdate() override;
    int prepareTask(double sampleRate, int bufferSize) override;
    void runTask(const juce::AudioBuffer<float>& points) override;
    void stopTask() override;
    void resized() override;

private:
    class VolumeThumbLookAndFeel : public LookAndFeel {
    public:
        explicit VolumeThumbLookAndFeel(int thumbRadius) : thumbRadius(thumbRadius) {}

        int getSliderThumbRadius(juce::Slider& slider) override {
            return juce::jmin(thumbRadius, slider.isHorizontal() ? slider.getHeight() : slider.getWidth());
        }

    private:
        int thumbRadius = 12;
    };

    class ThresholdThumbLookAndFeel : public VolumeThumbLookAndFeel {
    public:
        explicit ThresholdThumbLookAndFeel(int thumbRadius) : VolumeThumbLookAndFeel(thumbRadius) {}

        void drawLinearSliderThumb(juce::Graphics& g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const juce::Slider::SliderStyle style,
                                   juce::Slider& slider) override;

        void drawLinearSlider(juce::Graphics& g,
                              int x,
                              int y,
                              int width,
                              int height,
                              float sliderPos,
                              float minSliderPos,
                              float maxSliderPos,
                              const juce::Slider::SliderStyle style,
                              juce::Slider& slider) override;
    };

    static constexpr double bufferDurationSecs = 1.0 / 60.0;

    bool isOutputClipActive(float threshold) const noexcept;

    EffectParameter& volumeParameter;
    EffectParameter& thresholdParameter;
    BooleanParameter& muteParameter;
    float outputClipBypassThreshold = 1.0f;
    float outputClipPeakEpsilon = 0.0f;

    std::atomic<float> leftVolume = 0.0f;
    std::atomic<float> rightVolume = 0.0f;
    std::atomic<float> leftPeak = 0.0f;
    std::atomic<float> rightPeak = 0.0f;

    VolumeThumbLookAndFeel volumeLookAndFeel{12};
    juce::Slider volumeSlider;
    ThresholdThumbLookAndFeel thresholdLookAndFeel{7};
    juce::Slider thresholdSlider;

    SvgButton volumeButton;

    juce::SmoothedValue<float> leftVolumeSmoothed;
    juce::SmoothedValue<float> rightVolumeSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VolumeComponent)
};

} // namespace osci
