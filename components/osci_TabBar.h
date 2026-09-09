#pragma once

namespace osci {

/** A horizontal panel tab strip with a shared surface and selection underline.
    Owns the tab controls, not their page content. JUCE supplies tab accessibility
    and overflow handling; clients switch their content in onSelectionChanged.
*/
class TabBar final : public juce::TabbedButtonBar {
public:
    enum ColourIds {
        backgroundColourId = 0x2f31000,
        dividerColourId,
        textColourId,
        selectedTextColourId,
        indicatorColourId,
        hoverColourId
    };

    TabBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::TabsAtTop) {
        setLookAndFeel(&tabLookAndFeel);
        setMinimumTabScaleFactor(1.0);
        setColour(backgroundColourId, Colours::veryDark());
        setColour(dividerColourId, Colours::text().withAlpha(0.13f));
        setColour(textColourId, Colours::textMuted());
        setColour(selectedTextColourId, Colours::text());
        setColour(indicatorColourId, Colours::accentColor());
        setColour(hoverColourId, Colours::text().withAlpha(0.05f));
    }

    ~TabBar() override { setLookAndFeel(nullptr); }

    void addTab(const juce::String& label) {
        juce::TabbedButtonBar::addTab(label, juce::Colours::transparentBlack, -1);
    }

    void setTabEnabled(int index, bool enabled) {
        auto* tab = getTabButton(index);
        if (tab != nullptr) { tab->setEnabled(enabled); }
    }

    void setSelectedIndex(int index, juce::NotificationType notification = juce::sendNotification) {
        setCurrentTabIndex(index, notification != juce::dontSendNotification);
    }

    std::function<void(int)> onSelectionChanged;

    void paint(juce::Graphics& g) override {
        PanelHeader::paintBackground(g, getLocalBounds().toFloat(), findColour(backgroundColourId));

    }

private:
    struct TabLookAndFeel final : osci::LookAndFeel {
        explicit TabLookAndFeel(TabBar& bar) : bar(bar) {}
        juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override {
            auto* parent = bar.getParentComponent();
            return (parent != nullptr ? parent->getLookAndFeel() : juce::LookAndFeel::getDefaultLookAndFeel()).getTypefaceForFont(font);
        }
        void drawTabAreaBehindFrontButton(juce::TabbedButtonBar&, juce::Graphics&, int, int) override {}
        TabBar& bar;
        int getTabButtonOverlap(int) override { return 0; }
        int getTabButtonSpaceAroundImage() override { return 0; }
    } tabLookAndFeel{*this};

    class Tab final : public juce::TabBarButton {
    public:
        Tab(const juce::String& label, TabBar& bar) : juce::TabBarButton(label, bar), bar(bar) {
            setWantsKeyboardFocus(true);
        }

        int getBestTabLength(int) override {
            juce::GlyphArrangement glyphs;
            glyphs.addLineOfText(font(), getButtonText(), 0.0f, 0.0f);
            return juce::jmax(84, juce::roundToInt(glyphs.getBoundingBox(0, -1, true).getWidth()) + 36);
        }

        bool hitTest(int x, int y) override { return getLocalBounds().contains(x, y); }

        void paintButton(juce::Graphics& g, bool over, bool down) override {
            auto bounds = getLocalBounds().toFloat();
            if (isEnabled() && (over || down)) {
                g.setColour(bar.findColour(hoverColourId));
                g.fillRoundedRectangle(bounds.reduced(4.0f, 4.0f), 4.0f);
            }
            const bool selected = isFrontTab();
            g.setColour(bar.findColour(selected ? selectedTextColourId : textColourId).withMultipliedAlpha(isEnabled() ? 1.0f : 0.4f));
            g.setFont(font());
            g.drawFittedText(getButtonText(), getLocalBounds().reduced(18, 0).withTrimmedBottom(2), juce::Justification::centred, 1);
            if (selected) {
                g.setColour(bar.findColour(indicatorColourId));
                g.fillRoundedRectangle(bounds.withTrimmedLeft(18).withTrimmedRight(18).withTop(bounds.getBottom() - 3.0f), 1.5f);
            }
            if (hasKeyboardFocus(false) && keyboardFocus) {
                g.setColour(bar.findColour(indicatorColourId).withAlpha(0.7f));
                g.drawRoundedRectangle(bounds.reduced(4.0f, 5.0f), 3.0f, 1.0f);
            }
        }

        void focusGained(FocusChangeType cause) override {
            if (cause == focusChangedByTabKey) { keyboardFocus = true; }
            if (cause == focusChangedByMouseClick) { keyboardFocus = false; }
            repaint();
        }

        void focusLost(FocusChangeType) override { repaint(); }

        void mouseDown(const juce::MouseEvent& event) override {
            keyboardFocus = false;
            juce::TabBarButton::mouseDown(event);
        }

        bool keyPressed(const juce::KeyPress& key) override {
            const bool previous = key == juce::KeyPress::leftKey;
            const bool next = key == juce::KeyPress::rightKey;
            if (previous || next || key == juce::KeyPress::homeKey || key == juce::KeyPress::endKey) {
                const int count = bar.getNumTabs();
                int index = key == juce::KeyPress::homeKey ? count - 1 : (key == juce::KeyPress::endKey ? 0 : getIndex());
                const int direction = previous || key == juce::KeyPress::endKey ? -1 : 1;
                for (int attempt = 0; attempt < count; ++attempt) {
                    index = (index + direction + count) % count;
                    auto* tab = bar.getTabButton(index);
                    if (tab != nullptr && tab->isEnabled()) {
                        bar.setSelectedIndex(index);
                        static_cast<Tab*>(tab)->keyboardFocus = false;
                        tab->grabKeyboardFocus();
                        break;
                    }
                }
                return true;
            }
            return juce::TabBarButton::keyPressed(key);
        }

    private:
        juce::Font font() const {
            const juce::Font requested(juce::FontOptions(14.0f, juce::Font::bold));
            return juce::Font(juce::FontOptions(bar.getLookAndFeel().getTypefaceForFont(requested))).withHeight(14.0f);
        }
        TabBar& bar;
        bool keyboardFocus = false;
    };

    juce::TabBarButton* createTabButton(const juce::String& label, int) override { return new Tab(label, *this); }

    void currentTabChanged(int index, const juce::String&) override {
        if (onSelectionChanged) { onSelectionChanged(index); }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabBar)
};

} // namespace osci
