#pragma once

namespace osci {
// Sections stacked top to bottom, each as tall as it asks (0 hides it).
class SectionStack final : public juce::Component {
public:
    void add(juce::Component& section, std::function<int()> height) {
        sections.push_back({&section, std::move(height)});
        addChildComponent(section);
    }
    int preferredHeight() const {
        int total = 0;
        for (const auto& section : sections) { total += section.height(); }
        return total;
    }
    void resized() override {
        int y = 0;
        for (const auto& section : sections) {
            const auto height = section.height();
            section.component->setVisible(height > 0);
            section.component->setBounds(0, y, getWidth(), height);
            y += height;
        }
    }
private:
    struct Section {
        juce::Component* component;
        std::function<int()> height;
    };
    std::vector<Section> sections;
};
}
