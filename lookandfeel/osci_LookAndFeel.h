#pragma once

namespace osci {

enum ColourIds {
    groupComponentBackgroundColourId,
    groupComponentHeaderColourId,
    effectComponentBackgroundColourId,
    effectComponentHandleColourId,
    sliderThumbOutlineColourId,
    scrollFadeOverlayBackgroundColourId,
};

// ============================================================================
// Theme system - define colour palettes and switch between them at runtime.
// ============================================================================

enum class ThemeMode
{
    light,
    dark
};

struct Theme {
    ThemeMode mode = ThemeMode::dark;
    juce::Colour dark;
    juce::Colour darker;
    juce::Colour darkerer;
    juce::Colour evenDarker;
    juce::Colour veryDark;
    juce::Colour grey;
    juce::Colour accentColor;
    juce::Colour text;
    juce::Colour textMuted;
    juce::Colour textSubtle;
    juce::Colour textOnAccent;
    juce::Colour canvasBackground;
    juce::Colour gridMinor;
    juce::Colour gridMajor;
    juce::Colour surface;
    juce::Colour surfaceRaised;
    juce::Colour surfaceSunken;
    juce::Colour overlay;
    juce::Colour outline;
    juce::Colour outlineSubtle;
    juce::Colour shadow;
    juce::Colour danger;
    juce::Colour warning;
    juce::Colour midiLearn;
    juce::Colour portInput;
    juce::Colour portOutput;
    juce::Colour categoryGenerators;
    juce::Colour categoryControls;
    juce::Colour categoryMath;
    juce::Colour categoryEffects;
    juce::Colour categoryFileSources;
    juce::Colour categoryScripting;
    juce::Colour categoryInput;
    juce::Colour categoryOutput;
    juce::Colour categoryPlugins;
    juce::Colour codeBackground;
    juce::Colour codeCurrentLine;
    juce::Colour codeSelection;
    juce::Colour codeForeground;
    juce::Colour codeComment;
    juce::Colour codeCyan;
    juce::Colour codeGreen;
    juce::Colour codeOrange;
    juce::Colour codePink;
    juce::Colour codePurple;
    juce::Colour codeRed;
    juce::Colour codeYellow;

    static constexpr int kLabelHeight = 14;

    // --- Built-in themes ---

    static const Theme& darkTheme();
    static const Theme& lightTheme();
    static const Theme& classic();
    static const Theme& blue();
    static const Theme& green();

    // --- Active theme ---

    static const Theme& current();
    static ThemeMode currentMode();
    static bool isDark();

    static void setCurrent(const Theme& theme);
    static void setCurrentMode(ThemeMode mode);

private:
    static const Theme*& activePalette();
};

namespace Colours {
    // These are thin accessors into the active theme.
    inline juce::Colour dark()            { return Theme::current().dark; }
    inline juce::Colour darker()          { return Theme::current().darker; }
    inline juce::Colour darkerer()        { return Theme::current().darkerer; }
    inline juce::Colour evenDarker()      { return Theme::current().evenDarker; }
    inline juce::Colour veryDark()        { return Theme::current().veryDark; }
    inline juce::Colour grey()            { return Theme::current().grey; }
    inline juce::Colour accentColor()     { return Theme::current().accentColor; }
    inline juce::Colour text()            { return Theme::current().text; }
    inline juce::Colour textMuted()       { return Theme::current().textMuted; }
    inline juce::Colour textSubtle()      { return Theme::current().textSubtle; }
    inline juce::Colour textOnAccent()    { return Theme::current().textOnAccent; }
    inline juce::Colour canvasBackground(){ return Theme::current().canvasBackground; }
    inline juce::Colour gridMinor()       { return Theme::current().gridMinor; }
    inline juce::Colour gridMajor()       { return Theme::current().gridMajor; }
    inline juce::Colour surface()         { return Theme::current().surface; }
    inline juce::Colour surfaceRaised()   { return Theme::current().surfaceRaised; }
    inline juce::Colour surfaceSunken()   { return Theme::current().surfaceSunken; }
    inline juce::Colour overlay()         { return Theme::current().overlay; }
    inline juce::Colour outline()         { return Theme::current().outline; }
    inline juce::Colour outlineSubtle()   { return Theme::current().outlineSubtle; }
    inline juce::Colour shadow()          { return Theme::current().shadow; }
    inline juce::Colour danger()          { return Theme::current().danger; }
    inline juce::Colour warning()         { return Theme::current().warning; }
    inline juce::Colour portInput()       { return Theme::current().portInput; }
    inline juce::Colour portOutput()      { return Theme::current().portOutput; }
    inline juce::Colour categoryGenerators() { return Theme::current().categoryGenerators; }
    inline juce::Colour categoryControls()   { return Theme::current().categoryControls; }
    inline juce::Colour categoryMath()       { return Theme::current().categoryMath; }
    inline juce::Colour categoryEffects()    { return Theme::current().categoryEffects; }
    inline juce::Colour categoryFileSources(){ return Theme::current().categoryFileSources; }
    inline juce::Colour categoryScripting()  { return Theme::current().categoryScripting; }
    inline juce::Colour categoryInput()      { return Theme::current().categoryInput; }
    inline juce::Colour categoryOutput()     { return Theme::current().categoryOutput; }
    inline juce::Colour categoryPlugins()    { return Theme::current().categoryPlugins; }
    inline juce::Colour codeBackground()     { return Theme::current().codeBackground; }
    inline juce::Colour codeCurrentLine()    { return Theme::current().codeCurrentLine; }
    inline juce::Colour codeSelection()      { return Theme::current().codeSelection; }
    inline juce::Colour codeForeground()     { return Theme::current().codeForeground; }
    inline juce::Colour codeComment()        { return Theme::current().codeComment; }
    inline juce::Colour codeCyan()           { return Theme::current().codeCyan; }
    inline juce::Colour codeGreen()          { return Theme::current().codeGreen; }
    inline juce::Colour codeOrange()         { return Theme::current().codeOrange; }
    inline juce::Colour codePink()           { return Theme::current().codePink; }
    inline juce::Colour codePurple()         { return Theme::current().codePurple; }
    inline juce::Colour codeRed()            { return Theme::current().codeRed; }
    inline juce::Colour codeYellow()         { return Theme::current().codeYellow; }

    static constexpr int kLabelHeight = Theme::kLabelHeight;
    static constexpr float kPillRadius = kLabelHeight * 0.5f;

    inline juce::Colour transparent()         { return juce::Colour(); }
    inline juce::Colour neutralFill(float alpha) { return text().withAlpha(alpha); }
    inline juce::Colour neutralStroke(float alpha) { return text().withAlpha(alpha); }
    inline juce::Colour invertedNeutral(float alpha) { return shadow().withAlpha(alpha); }
    inline juce::Colour midiLearnBackground() { return Theme::current().midiLearn.withAlpha(0.6f); }
    inline juce::Colour midiLearnText()       { return Theme::current().midiLearn; }
    inline const juce::String& midiLearnLabel() { static const juce::String s("MIDI Learn..."); return s; }
}

namespace LookAndFeelHelpers {
    inline juce::Colour createBaseColour(juce::Colour buttonColour, bool hasKeyboardFocus, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown, bool isEnabled) noexcept {
        const juce::Colour baseColour(buttonColour.withMultipliedAlpha(isEnabled ? 1.0f : 0.5f));

        if (shouldDrawButtonAsDown) {
            return baseColour.contrasting (0.15f);
        }

        if (shouldDrawButtonAsHighlighted) {
            return baseColour.contrasting (0.05f);
        }

        return baseColour;
    }

    inline void drawFittedText (juce::Graphics& g,
                                const juce::String& text,
                                juce::Rectangle<float> area,
                                juce::Justification justification,
                                int maximumNumberOfLines,
                                float minimumHorizontalScale = 0.0f,
                                juce::GlyphArrangementOptions options = {}) {
        if (text.isEmpty() || area.isEmpty() || maximumNumberOfLines <= 0) {
            return;
        }

        const auto localArea = area.withPosition (0.0f, 0.0f).getSmallestIntegerContainer();
        if (localArea.isEmpty()) {
            return;
        }

        g.saveState();
        g.addTransform (juce::AffineTransform::translation (area.getX(), area.getY()));
        g.drawFittedText (text,
                          localArea,
                          justification,
                          maximumNumberOfLines,
                          minimumHorizontalScale,
                          options);
        g.restoreState();
    }

    inline void drawFittedText (juce::Graphics& g,
                                const juce::String& text,
                                juce::Rectangle<int> area,
                                juce::Justification justification,
                                int maximumNumberOfLines,
                                float minimumHorizontalScale = 0.0f,
                                juce::GlyphArrangementOptions options = {}) {
        if (text.isEmpty() || area.isEmpty() || maximumNumberOfLines <= 0) {
            return;
        }

        g.drawFittedText (text, area, justification, maximumNumberOfLines, minimumHorizontalScale, options);
    }

    inline void drawFittedText (juce::Graphics& g,
                                const juce::String& text,
                                int x,
                                int y,
                                int width,
                                int height,
                                juce::Justification justification,
                                int maximumNumberOfLines,
                                float minimumHorizontalScale = 0.0f,
                                juce::GlyphArrangementOptions options = {}) {
        if (text.isEmpty() || width <= 0 || height <= 0 || maximumNumberOfLines <= 0) {
            return;
        }

        g.drawFittedText (text,
                          x,
                          y,
                          width,
                          height,
                          justification,
                          maximumNumberOfLines,
                          minimumHorizontalScale,
                          options);
    }


    inline juce::TextLayout layoutTooltipText(const juce::String& text, juce::Colour colour) noexcept {
        const float tooltipFontSize = 13.0f;
        const int maxToolTipWidth = 400;

        juce::AttributedString s;
        s.setJustification (juce::Justification::centred);
        s.append (text, juce::FontOptions (tooltipFontSize, juce::Font::bold), colour);

        juce::TextLayout tl;
        tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
        return tl;
    }
}

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
    struct TypefaceData {
        const void* regularData = nullptr;
        int regularSize = 0;
        const void* boldData = nullptr;
        int boldSize = 0;
        const void* italicData = nullptr;
        int italicSize = 0;
    };

    LookAndFeel();
    explicit LookAndFeel (TypefaceData typefaceData);

    static LookAndFeel& getSharedInstance();

    static void applyColours (juce::LookAndFeel& lookAndFeel);
    static void drawComboBoxStyle (juce::Graphics& g, int width, int height, juce::ComboBox& box);
    static void positionComboBoxTextStyle (juce::LookAndFeel& lookAndFeel, juce::ComboBox& box, juce::Label& label);

    static const int RECT_RADIUS = 5;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override;
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
    void drawTickBox(juce::Graphics& g, juce::Component& component,
        float x, float y, float w, float h,
        const bool ticked,
        const bool isEnabled,
        const bool shouldDrawButtonAsHighlighted,
        const bool shouldDrawButtonAsDown) override;
    static void drawGroupComponentDropShadow(juce::Graphics& g, juce::GroupComponent& group);
    void drawGroupComponentOutline(juce::Graphics&, int w, int h, const juce::String &text, const juce::Justification&, juce::GroupComponent&) override;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override;
    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
    void drawMenuBarBackground(juce::Graphics& g, int width, int height, bool, juce::MenuBarComponent& menuBar) override;
    juce::TextLayout layoutTooltipText(const juce::String& text, juce::Colour colour);
    juce::Rectangle<int> getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    static juce::CodeEditorComponent::ColourScheme getDefaultColourScheme();
    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override;
    void drawCornerResizer(juce::Graphics&, int w, int h, bool isMouseOver, bool isMouseDragging) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    juce::MouseCursor getMouseCursorFor(juce::Component& component) override;
    void drawCallOutBoxBackground(juce::CallOutBox& box, juce::Graphics& g, const juce::Path& path, juce::Image& cachedImage) override;
    void drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar, int width, int height, double progress, const juce::String& textToShow) override;
    void customDrawLinearProgressBar(juce::Graphics& g, const juce::ProgressBar& progressBar, int width, int height, double progress, const juce::String& textToShow);
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;

    // AlertWindow overrides
    void drawAlertBox(juce::Graphics& g, juce::AlertWindow& alert, const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout) override;
    int getAlertWindowButtonHeight() override;
    juce::Font getAlertWindowTitleFont() override;
    juce::Font getAlertWindowMessageFont() override;
    juce::Font getAlertWindowFont() override;
    void drawStretchableLayoutResizerBar(juce::Graphics& g, int w, int h, bool isVerticalBar, bool isMouseOver, bool isMouseDragging) override;
    void drawBubble(juce::Graphics& g, juce::BubbleComponent& bubble,
                    const juce::Point<float>& tip, const juce::Rectangle<float>& body) override;

    // PopupMenu
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;
    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
                                   int standardMenuItemHeight, int& idealWidth, int& idealHeight) override;
    int getPopupMenuBorderSize() override;

protected:
    LookAndFeel (TypefaceData typefaceData, bool shouldSetAsDefaultLookAndFeel);

    juce::Typeface::Ptr regularTypeface;
    juce::Typeface::Ptr boldTypeface;
    juce::Typeface::Ptr italicTypeface;
};

} // namespace osci
