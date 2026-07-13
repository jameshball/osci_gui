#include "osci_LookAndFeel.h"

namespace osci {

namespace
{
Theme makeDarkTheme()
{
    Theme theme;
    theme.mode = ThemeMode::dark;
    theme.dark = juce::Colour (0xff575a60);
    theme.darker = juce::Colour (0xff44474d);
    theme.darkerer = juce::Colour (0xff32353a);
    theme.evenDarker = juce::Colour (0xff222428);
    theme.veryDark = juce::Colour (0xff111215);
    theme.grey = juce::Colour (0xff6b6e75);
    theme.accentColor = juce::Colour (0xff00cc00);
    theme.text = juce::Colour (0xfff8f8f2);
    theme.textMuted = theme.text.withAlpha (0.78f);
    theme.textSubtle = theme.text.withAlpha (0.56f);
    theme.textOnAccent = juce::Colour (0xff071008);
    theme.canvasBackground = juce::Colour (0xff0f1012);
    theme.gridMinor = theme.text.withAlpha (0.014f);
    theme.gridMajor = theme.text.withAlpha (0.038f);
    theme.surface = juce::Colour (0xff191a1d);
    theme.surfaceRaised = juce::Colour (0xff252629);
    theme.surfaceSunken = juce::Colour (0xff08090b);
    theme.overlay = theme.darkerer.withAlpha (0.92f);
    theme.outline = theme.text.withAlpha (0.22f);
    theme.outlineSubtle = theme.text.withAlpha (0.10f);
    theme.shadow = juce::Colour (0xff000000);
    theme.danger = juce::Colour (0xffff5555);
    theme.warning = juce::Colour (0xffffb86c);
    theme.midiLearn = juce::Colour (0xffff5555);
    theme.portInput = juce::Colour (0xff55d987);
    theme.portOutput = juce::Colour (0xff57a5ff);
    theme.categoryGenerators = juce::Colour (0xff72e4ff);
    theme.categoryControls = juce::Colour (0xfff6d365);
    theme.categoryMath = juce::Colour (0xffd98cff);
    theme.categoryEffects = theme.accentColor;
    theme.categoryFileSources = juce::Colour (0xffffa66b);
    theme.categoryScripting = juce::Colour (0xff8cc7ff);
    theme.categoryInput = juce::Colour (0xff7bed9f);
    theme.categoryOutput = juce::Colour (0xffff7675);
    theme.categoryPlugins = juce::Colour (0xffb2bec3);
    theme.codeBackground = juce::Colour (0xff282a36);
    theme.codeCurrentLine = juce::Colour (0xff44475a);
    theme.codeSelection = juce::Colour (0xff44475a);
    theme.codeForeground = juce::Colour (0xfff8f8f2);
    theme.codeComment = juce::Colour (0xff6272a4);
    theme.codeCyan = juce::Colour (0xff8be9fd);
    theme.codeGreen = juce::Colour (0xff50fa7b);
    theme.codeOrange = juce::Colour (0xffffb86c);
    theme.codePink = juce::Colour (0xffff79c6);
    theme.codePurple = juce::Colour (0xffbd93f9);
    theme.codeRed = theme.danger;
    theme.codeYellow = juce::Colour (0xfff1fa8c);
    return theme;
}

Theme makeLightTheme()
{
    Theme theme;
    theme.mode = ThemeMode::light;
    theme.dark = juce::Colour (0xffc8d0dc);
    theme.darker = juce::Colour (0xffdce2ea);
    theme.darkerer = juce::Colour (0xffebeff5);
    theme.evenDarker = juce::Colour (0xfff3f6fa);
    theme.veryDark = juce::Colour (0xfffafbfe);
    theme.grey = juce::Colour (0xff697383);
    theme.accentColor = juce::Colour (0xff168f3c);
    theme.text = juce::Colour (0xff17202b);
    theme.textMuted = theme.text.withAlpha (0.74f);
    theme.textSubtle = theme.text.withAlpha (0.54f);
    theme.textOnAccent = juce::Colour (0xffffffff);
    theme.canvasBackground = juce::Colour (0xfff4f7fb);
    theme.gridMinor = theme.text.withAlpha (0.034f);
    theme.gridMajor = theme.text.withAlpha (0.076f);
    theme.surface = juce::Colour (0xffffffff);
    theme.surfaceRaised = juce::Colour (0xffeef2f7);
    theme.surfaceSunken = juce::Colour (0xffe6ebf2);
    theme.overlay = theme.surface.withAlpha (0.94f);
    theme.outline = theme.text.withAlpha (0.20f);
    theme.outlineSubtle = theme.text.withAlpha (0.10f);
    theme.shadow = juce::Colour (0xff2a3442);
    theme.danger = juce::Colour (0xffc93d4b);
    theme.warning = juce::Colour (0xffbd6b1f);
    theme.midiLearn = theme.danger;
    theme.portInput = juce::Colour (0xff18884a);
    theme.portOutput = juce::Colour (0xff286ec9);
    theme.categoryGenerators = juce::Colour (0xff0f82ad);
    theme.categoryControls = juce::Colour (0xffa87116);
    theme.categoryMath = juce::Colour (0xff9754c8);
    theme.categoryEffects = theme.accentColor;
    theme.categoryFileSources = juce::Colour (0xffbc642c);
    theme.categoryScripting = juce::Colour (0xff326fa9);
    theme.categoryInput = juce::Colour (0xff1a8a56);
    theme.categoryOutput = juce::Colour (0xffc24758);
    theme.categoryPlugins = juce::Colour (0xff697383);
    theme.codeBackground = juce::Colour (0xfff8fafc);
    theme.codeCurrentLine = juce::Colour (0xffe9eef6);
    theme.codeSelection = juce::Colour (0xffdbe7f7);
    theme.codeForeground = juce::Colour (0xff17202b);
    theme.codeComment = juce::Colour (0xff697383);
    theme.codeCyan = juce::Colour (0xff0f82ad);
    theme.codeGreen = juce::Colour (0xff18884a);
    theme.codeOrange = juce::Colour (0xffbd6b1f);
    theme.codePink = juce::Colour (0xffb3447a);
    theme.codePurple = juce::Colour (0xff7a5bb7);
    theme.codeRed = theme.danger;
    theme.codeYellow = juce::Colour (0xff8a7400);
    return theme;
}
} // namespace

const Theme& Theme::darkTheme()
{
    static const Theme theme = makeDarkTheme();
    return theme;
}

const Theme& Theme::lightTheme()
{
    static const Theme theme = makeLightTheme();
    return theme;
}

const Theme& Theme::classic()
{
    return darkTheme();
}

const Theme& Theme::blue()
{
    return darkTheme();
}

const Theme& Theme::green()
{
    return darkTheme();
}

const Theme& Theme::current()
{
    return *activePalette();
}

ThemeMode Theme::currentMode()
{
    return current().mode;
}

bool Theme::isDark()
{
    return currentMode() == ThemeMode::dark;
}

void Theme::setCurrent(const Theme& theme)
{
    activePalette() = &theme;
}

void Theme::setCurrentMode(ThemeMode mode)
{
    setCurrent (mode == ThemeMode::light ? lightTheme() : darkTheme());
}

const Theme*& Theme::activePalette()
{
    static const Theme* palette = &darkTheme();
    return palette;
}

LookAndFeel::LookAndFeel()
    : LookAndFeel (TypefaceData {}, true) {
}

LookAndFeel::LookAndFeel (TypefaceData typefaceData)
    : LookAndFeel (typefaceData, true) {
}

LookAndFeel::LookAndFeel (TypefaceData typefaceData, bool shouldSetAsDefaultLookAndFeel) {
    if (typefaceData.regularData != nullptr && typefaceData.regularSize > 0) {
        regularTypeface = juce::Typeface::createSystemTypefaceFor (typefaceData.regularData, typefaceData.regularSize);
    }

    if (typefaceData.boldData != nullptr && typefaceData.boldSize > 0) {
        boldTypeface = juce::Typeface::createSystemTypefaceFor (typefaceData.boldData, typefaceData.boldSize);
    }

    if (typefaceData.italicData != nullptr && typefaceData.italicSize > 0) {
        italicTypeface = juce::Typeface::createSystemTypefaceFor (typefaceData.italicData, typefaceData.italicSize);
    }

    applyColours(*this);

    // UI colours
    getCurrentColourScheme().setUIColour(ColourScheme::widgetBackground, Colours::veryDark());

    // I have to do this, otherwise components are initialised before the look and feel is set
    if (shouldSetAsDefaultLookAndFeel) {
        juce::LookAndFeel::setDefaultLookAndFeel(this);
    }
}

LookAndFeel& LookAndFeel::getSharedInstance() {
    static LookAndFeel instance;
    return instance;
}

void LookAndFeel::applyColours(juce::LookAndFeel& lookAndFeel) {
    // slider
    lookAndFeel.setColour(juce::Slider::thumbColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::Slider::textBoxBackgroundColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::Slider::textBoxHighlightColourId, Colours::accentColor().withMultipliedAlpha(0.5f));
    lookAndFeel.setColour(juce::Slider::trackColourId, Colours::grey().brighter(0.3f));
    lookAndFeel.setColour(juce::Slider::backgroundColourId, Colours::dark());
    lookAndFeel.setColour(sliderThumbOutlineColourId, Colours::text());

    // buttons
    lookAndFeel.setColour(juce::TextButton::buttonColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::TextButton::textColourOffId, Colours::text());
    lookAndFeel.setColour(juce::TextButton::textColourOnId, Colours::textOnAccent());

    // windows & menus
    lookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, Colours::veryDark().brighter(0.1f));
    lookAndFeel.setColour(groupComponentBackgroundColourId, Colours::darker());
    lookAndFeel.setColour(scrollFadeOverlayBackgroundColourId, Colours::darker());
    lookAndFeel.setColour(groupComponentHeaderColourId, Colours::veryDark());
    // Alpha 0xFE (not 0xFF) forces JUCE to treat the window as non-opaque,
    // enabling compositor transparency so rounded corners don't show the
    // native window background colour behind them.
    lookAndFeel.setColour(juce::PopupMenu::backgroundColourId, Colours::veryDark().withAlpha((juce::uint8)0xFE));
    lookAndFeel.setColour(juce::PopupMenu::highlightedBackgroundColourId, Colours::accentColor());
    lookAndFeel.setColour(juce::TooltipWindow::backgroundColourId, Colours::darker().darker(0.5f));
    lookAndFeel.setColour(juce::TooltipWindow::outlineColourId, Colours::darker());
    lookAndFeel.setColour(juce::TextButton::buttonOnColourId, Colours::darker());
    lookAndFeel.setColour(juce::AlertWindow::outlineColourId, Colours::grey().withAlpha(0.5f));
    lookAndFeel.setColour(juce::AlertWindow::backgroundColourId, Colours::darkerer());
    lookAndFeel.setColour(juce::AlertWindow::textColourId, Colours::text());
    lookAndFeel.setColour(juce::ColourSelector::backgroundColourId, Colours::darker());

    // combo box
    lookAndFeel.setColour(juce::ComboBox::backgroundColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::ComboBox::outlineColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::ComboBox::textColourId, Colours::text());
    lookAndFeel.setColour(juce::ComboBox::arrowColourId, Colours::text());

    // text box
    lookAndFeel.setColour(juce::TextEditor::backgroundColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::TextEditor::textColourId, Colours::text());
    lookAndFeel.setColour(juce::TextEditor::outlineColourId, Colours::grey().withAlpha(0.32f));
    lookAndFeel.setColour(juce::TextEditor::focusedOutlineColourId, Colours::accentColor());
    lookAndFeel.setColour(juce::CaretComponent::caretColourId, Colours::text());
    lookAndFeel.setColour(juce::TextEditor::highlightColourId, Colours::accentColor());
    lookAndFeel.setColour(juce::TextEditor::highlightedTextColourId, Colours::textOnAccent());

    // list box
    lookAndFeel.setColour(juce::ListBox::backgroundColourId, Colours::darker());
    lookAndFeel.setColour(juce::ListBox::textColourId, Colours::text());

    // scroll bar
    lookAndFeel.setColour(juce::ScrollBar::thumbColourId, Colours::textMuted());
    lookAndFeel.setColour(juce::ScrollBar::trackColourId, Colours::veryDark());
    lookAndFeel.setColour(juce::ScrollBar::backgroundColourId, Colours::veryDark());

    // custom components
    lookAndFeel.setColour(effectComponentBackgroundColourId, Colours::transparent());
    lookAndFeel.setColour(effectComponentHandleColourId, Colours::veryDark());

    // code editor
    lookAndFeel.setColour(juce::CodeEditorComponent::backgroundColourId, Colours::codeBackground());
    lookAndFeel.setColour(juce::CodeEditorComponent::defaultTextColourId, Colours::codeForeground());
    lookAndFeel.setColour(juce::CodeEditorComponent::lineNumberBackgroundId, Colours::veryDark());
    lookAndFeel.setColour(juce::CodeEditorComponent::lineNumberTextId, Colours::codeForeground());
    lookAndFeel.setColour(juce::CodeEditorComponent::highlightColourId, Colours::codeSelection());

    // progress bar
    lookAndFeel.setColour(juce::ProgressBar::backgroundColourId, Colours::transparent());
    lookAndFeel.setColour(juce::ProgressBar::foregroundColourId, Colours::accentColor());
}

void LookAndFeel::drawComboBoxStyle(juce::Graphics& g, int width, int height, juce::ComboBox& box) {
    juce::Rectangle<int> boxBounds{0, 0, width, height};

    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(boxBounds.toFloat(), Colours::kPillRadius);

    juce::Rectangle<int> arrowZone{width - 15, 0, 10, height};
    juce::Path path;
    path.startNewSubPath((float)arrowZone.getX(), (float)arrowZone.getCentreY() - 3.0f);
    path.lineTo((float)arrowZone.getCentreX(), (float)arrowZone.getCentreY() + 4.0f);
    path.lineTo((float)arrowZone.getRight(), (float)arrowZone.getCentreY() - 3.0f);
    path.closeSubPath();

    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(box.isEnabled() ? 1.0f : 0.5f));
    g.fillPath(path);
}

void LookAndFeel::positionComboBoxTextStyle(juce::LookAndFeel& lookAndFeel, juce::ComboBox& box, juce::Label& label) {
    label.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    label.setBounds(1, 1, box.getWidth() - 15, box.getHeight() - 2);
    label.setFont(lookAndFeel.getComboBoxFont(box));
}

void LookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    label.setRepaintsOnMouseActivity(true);
    auto baseColour = label.findColour(juce::Label::backgroundColourId);
    if (label.isEditable()) {
        baseColour = LookAndFeelHelpers::createBaseColour(baseColour, false, label.isMouseOver(true), false, label.isEnabled());
    }
    g.setColour(baseColour);
    g.fillRoundedRectangle(label.getLocalBounds().toFloat(), Colours::kPillRadius);

    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font (getLabelFont (label));

        g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        osci::LookAndFeelHelpers::drawFittedText (g, label.getText(), textArea, label.getJustificationType(),
            juce::jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
            label.getMinimumHorizontalScale());

        g.setColour (label.findColour (juce::Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        auto outlineColour = label.findColour(juce::Label::outlineColourId);
        if (label.isEditable()) {
            outlineColour = LookAndFeelHelpers::createBaseColour(outlineColour, false, label.isMouseOver(true), false, label.isEnabled());
        }
        g.setColour(outlineColour);
        g.drawRoundedRectangle(label.getLocalBounds().toFloat(), Colours::kPillRadius, 1);
    }
}

void LookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) {
    textEditor.setRepaintsOnMouseActivity(true);
    if (dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) != nullptr)
    {
        g.setColour (textEditor.findColour (juce::TextEditor::backgroundColourId));
        g.fillRect (0, 0, width, height);

        g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
        g.drawHorizontalLine (height - 1, 0.0f, static_cast<float> (width));
    }
    else
    {
        auto backgroundColour = textEditor.findColour (juce::TextEditor::backgroundColourId);
        auto baseColour = LookAndFeelHelpers::createBaseColour(backgroundColour, false, textEditor.isMouseOver(true), false, textEditor.isEnabled());
        g.setColour(baseColour);
        g.fillRoundedRectangle(textEditor.getLocalBounds().toFloat(), Colours::kPillRadius);
    }
}

void LookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) {
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus (true) && !textEditor.isReadOnly())
        {
            const float border = 1.0f;
            const float half = border * 0.5f;

            g.setColour (textEditor.findColour (juce::TextEditor::focusedOutlineColourId));
            g.drawRoundedRectangle(half, half, width - border, height - border, Colours::kPillRadius, border);
        }
        else
        {
            const float border = 1.0f;
            const float half = border * 0.5f;

            auto outlineColour = textEditor.findColour(juce::TextEditor::outlineColourId);
            outlineColour = LookAndFeelHelpers::createBaseColour(outlineColour, false, textEditor.isMouseOver(true), false, textEditor.isEnabled());
            g.setColour(outlineColour);
            g.drawRoundedRectangle(half, half, width - border, height - border, Colours::kPillRadius, border);
        }
    }
}

void LookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) {
    drawComboBoxStyle(g, width, height, box);
}

void LookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label) {
    positionComboBoxTextStyle(*this, box, label);
}

void LookAndFeel::drawTickBox(juce::Graphics& g, juce::Component& component,
    float x, float y, float w, float h,
    const bool ticked,
    const bool isEnabled,
    const bool shouldDrawButtonAsHighlighted,
    const bool shouldDrawButtonAsDown) {
    juce::Rectangle<float> tickBounds(x, y, w, h);

    g.setColour(component.findColour(juce::TextButton::buttonColourId));
    g.fillRoundedRectangle(tickBounds, Colours::kPillRadius);

    if (ticked) {
        g.setColour(component.findColour(juce::ToggleButton::tickColourId));
        auto tick = getTickShape(0.75f);
        g.fillPath(tick, tick.getTransformToScaleToFit(tickBounds.reduced(4, 5).toFloat(), false));
    }
}

void LookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height, const juce::String& text, const juce::Justification& position, juce::GroupComponent& group) {
    auto bounds = group.getLocalBounds();

    const float textH = 15.0f;
    const float indent = 3.0f;
    const float textEdgeGap = 4.0f;
    auto cs = 5.0f;

    juce::Font f(textH);

    juce::Path p;
    auto x = indent;
    auto y = f.getAscent() - 3.0f;
    auto w = juce::jmax(0.0f, (float)width - x * 2.0f);
    auto h = juce::jmax(0.0f, (float)height - y - indent);
    cs = juce::jmin(cs, w * 0.5f, h * 0.5f);
    auto cs2 = 2.0f * cs;

    auto textW = text.isEmpty() ? 0
        : juce::jlimit(0.0f,
            juce::jmax(0.0f, w - cs2 - textEdgeGap * 2),
            juce::GlyphArrangement::getStringWidth(f, text) + textEdgeGap * 2.0f);
    auto textX = cs + textEdgeGap;

    if (position.testFlags(juce::Justification::horizontallyCentred))
        textX = cs + (w - cs2 - textW) * 0.5f;
    else if (position.testFlags(juce::Justification::right))
        textX = w - cs - textW - textEdgeGap;

    auto alpha = group.isEnabled() ? 1.0f : 0.5f;

    juce::Path background;
	background.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), RECT_RADIUS, RECT_RADIUS);
    g.setColour(group.findColour(groupComponentBackgroundColourId).withMultipliedAlpha(alpha));
    g.fillPath(background);

    auto header = bounds.removeFromTop(2 * textH);

    juce::Path headerPath;
    headerPath.addRoundedRectangle(header.getX(), header.getY(), header.getWidth(), header.getHeight(), RECT_RADIUS, RECT_RADIUS);

    g.setColour(group.findColour(groupComponentHeaderColourId).withMultipliedAlpha(alpha));
    g.fillPath(headerPath);

    g.setColour(group.findColour(juce::GroupComponent::textColourId).withMultipliedAlpha(alpha));
    g.setFont(f);
    g.drawText (text,
                juce::Rectangle<float> (header.getX() + x + textX, header.getY() + 7.0f, textW, textH),
                juce::Justification::centred,
                true);
}

void LookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) {
    juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    
    // --- Thumb outline ring ---
    auto kx = slider.isHorizontal() ? sliderPos : ((float)x + (float)width * 0.5f);
    auto ky = slider.isHorizontal() ? ((float)y + (float)height * 0.5f) : sliderPos;

    juce::Point<float> point = { kx, ky };

    auto thumbWidth = getSliderThumbRadius(slider);

	juce::Path thumb;
	thumb.addEllipse(juce::Rectangle<float>(static_cast<float>(thumbWidth), static_cast<float>(thumbWidth)).withCentre(point));
   
    g.setColour(slider.findColour(sliderThumbOutlineColourId).withAlpha(slider.isEnabled() ? 1.0f : 0.5f));
	g.strokePath(thumb, juce::PathStrokeType(1.0f));
}

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider& slider) {
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    float diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
    auto centre = bounds.getCentre();

    float disabledAlpha = slider.isEnabled() ? 1.0f : 0.3f;
    bool hovered = slider.isEnabled() && slider.isMouseOverOrDragging();

    float baseTrackWidth = diameter * 0.09f;
    float trackWidth = hovered ? baseTrackWidth * 1.2f : baseTrackWidth;

    float baseMarkerWidth = diameter * 0.08f;
    float markerWidth = hovered ? baseMarkerWidth * 1.2f : baseMarkerWidth;

    float radius = (diameter - trackWidth) * 0.5f - 4.0f;
    float valueAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Dark circle background
    {
        float bgRadius = radius + trackWidth * 0.5f + 3.0f;
        g.setColour(Colours::veryDark().withAlpha(disabledAlpha));
        g.fillEllipse(centre.x - bgRadius, centre.y - bgRadius,
                      bgRadius * 2.0f, bgRadius * 2.0f);
    }

    // Background track
    {
        juce::Path bgTrack;
        bgTrack.addCentredArc(centre.x, centre.y, radius, radius,
                              0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(Colours::darker().withAlpha(disabledAlpha));
        g.strokePath(bgTrack, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }

    // Filled arc (accent colour)
    if (sliderPos > 0.001f) {
        juce::Path filledArc;
        filledArc.addCentredArc(centre.x, centre.y, radius, radius,
                                0.0f, rotaryStartAngle, valueAngle, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId).withAlpha(disabledAlpha));
        g.strokePath(filledArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }

    // Marker (rounded rect pointing toward centre)
    {
        float markerLength = diameter * 0.30f;
        float markerRadius = markerWidth * 0.5f;
        float outerR = radius + trackWidth * 0.5f;

        juce::Rectangle<float> markerRect(-markerWidth * 0.5f, -outerR,
                                           markerWidth, markerLength);
        juce::Path marker;
        marker.addRoundedRectangle(markerRect, markerRadius);

        auto transform = juce::AffineTransform::rotation(valueAngle)
                            .translated(centre.x, centre.y);
        marker.applyTransform(transform);

        g.setColour(Colours::text().withAlpha(disabledAlpha));
        g.fillPath(marker);
    }
}

void LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

    auto baseColour = LookAndFeelHelpers::createBaseColour(backgroundColour, button.hasKeyboardFocus(true), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown, button.isEnabled());

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, Colours::kPillRadius);
}

void LookAndFeel::drawMenuBarBackground(juce::Graphics& g, int width, int height, bool, juce::MenuBarComponent& menuBar) {
    juce::Rectangle<int> r(width, height);

    g.setColour(menuBar.findColour(juce::TextButton::buttonColourId));
    g.fillRect(r);
}

juce::TextLayout LookAndFeel::layoutTooltipText(const juce::String& text, juce::Colour colour) {
    const float tooltipFontSize = 17.0f;
    const int maxToolTipWidth = 600;

    juce::AttributedString s;
    s.setJustification (juce::Justification::centred);
    s.append (text, juce::Font (tooltipFontSize, juce::Font::bold), colour);

    juce::TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
    return tl;
}

juce::Rectangle<int> LookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) {
    const juce::TextLayout tl (layoutTooltipText(tipText, Colours::text()));

    auto w = (int) (tl.getWidth() + 14.0f);
    auto h = (int) (tl.getHeight() + 6.0f);

    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
        screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
        w, h)
        .constrainedWithin (parentArea);
}

void LookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) {
    juce::Rectangle<int> bounds (width, height);

    g.setColour(findColour(juce::TooltipWindow::backgroundColourId));
    g.fillRect(bounds);

    layoutTooltipText(text, findColour(juce::TooltipWindow::textColourId))
        .draw(g, {static_cast<float> (width), static_cast<float> (height)});
}

void LookAndFeel::drawCornerResizer(juce::Graphics&, int w, int h, bool isMouseOver, bool isMouseDragging) {
    
}

void LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    LookAndFeel_V4::drawToggleButton(g, button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

juce::CodeEditorComponent::ColourScheme LookAndFeel::getDefaultColourScheme() {
    juce::CodeEditorComponent::ColourScheme cs;

    const juce::CodeEditorComponent::ColourScheme::TokenType types[] = {
        {"Error", Colours::codeRed()},
        {"Comment", Colours::codeComment()},
        {"Keyword", Colours::codePink()},
        {"Operator", Colours::codeForeground()},
        {"Identifier", Colours::codeForeground()},
        {"Integer", Colours::codePurple()},
        {"Float", Colours::codePurple()},
        {"String", Colours::codeYellow()},
        {"Bracket", Colours::codeForeground()},
        {"Punctuation", Colours::codePink()},
        {"Preprocessor Text", Colours::codeComment()}
    };

    for (auto& t : types) {
        cs.set(t.name, juce::Colour(t.colour));
    }

    return cs;
}

juce::MouseCursor LookAndFeel::getMouseCursorFor(juce::Component& component) {
    juce::Label* label = dynamic_cast<juce::Label*>(&component);
    if (label != nullptr && label->isEditable()) {
        return juce::MouseCursor::IBeamCursor;
    }
    juce::TextEditor* textEditor = dynamic_cast<juce::TextEditor*>(&component);
    if (textEditor != nullptr) {
        return juce::MouseCursor::IBeamCursor;
    }
    juce::Button* button = dynamic_cast<juce::Button*>(&component);
    if (button != nullptr) {
        return button->isEnabled() ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor;
    }
    juce::ComboBox* comboBox = dynamic_cast<juce::ComboBox*>(&component);
    if (comboBox != nullptr) {
        return juce::MouseCursor::PointingHandCursor;
    }
    return juce::LookAndFeel_V4::getMouseCursorFor(component);
}

void LookAndFeel::drawCallOutBoxBackground(juce::CallOutBox& box, juce::Graphics& g, const juce::Path& path, juce::Image& cachedImage) {
    if (cachedImage.isNull()) {
        cachedImage = juce::Image(juce::Image::ARGB, box.getWidth(), box.getHeight(), true);
        juce::Graphics g2(cachedImage);

        juce::DropShadow(Colours::shadow().withAlpha(0.7f), 8, juce::Point<int>(0, 2)).drawForPath(g2, path);
    }

    g.setColour(Colours::shadow());
    g.drawImageAt(cachedImage, 0, 0);

    g.setColour(Colours::darker());
    g.fillPath(path);

    g.setColour(Colours::outline());
    g.strokePath(path, juce::PathStrokeType(1.0f));
}

void LookAndFeel::drawProgressBar(juce::Graphics& g, juce::ProgressBar& progressBar, int width, int height, double progress, const juce::String& textToShow) {
    switch (progressBar.getResolvedStyle()) {
        case juce::ProgressBar::Style::linear:
            customDrawLinearProgressBar(g, progressBar, width, height, progress, textToShow);
            break;
        case juce::ProgressBar::Style::circular:
            juce::LookAndFeel_V4::drawProgressBar(g, progressBar, width, height, progress, textToShow);
            break;
    }
}

void LookAndFeel::customDrawLinearProgressBar(juce::Graphics& g, const juce::ProgressBar& progressBar, int width, int height, double progress, const juce::String& textToShow) {
    auto background = progressBar.findColour(juce::ProgressBar::backgroundColourId);
    auto foreground = progressBar.findColour(juce::ProgressBar::foregroundColourId).withAlpha(0.5f);
    int rectRadius = 2;

    auto barBounds = progressBar.getLocalBounds().toFloat();

    g.setColour(background);
    g.fillRoundedRectangle(barBounds, rectRadius);
    
    juce::String text = textToShow.isEmpty() ? "waiting..." : textToShow;

    if (progress >= 0.0f && progress <= 1.0f) {
        juce::Path p;
        p.addRoundedRectangle(barBounds, rectRadius);
        g.reduceClipRegion(p);

        barBounds.setWidth(barBounds.getWidth() * (float) progress);
        g.setColour(foreground);
        g.fillRoundedRectangle(barBounds, rectRadius);
    } else {
        if (progress == -2) {
            background = Colours::danger();
            text = "Error";
        }
        
        // spinning bar..
        g.setColour(background);

        auto stripeWidth = height * 2;
        auto position = static_cast<int>(juce::Time::getMillisecondCounter() / 15) % stripeWidth;

        juce::Path p;

        for (auto x = static_cast<float> (-position); x < (float) (width + stripeWidth); x += (float) stripeWidth) {
            p.addQuadrilateral (x, 0.0f,
                                x + (float) stripeWidth * 0.5f, 0.0f,
                                x, static_cast<float> (height),
                                x - (float) stripeWidth * 0.5f, static_cast<float> (height));
        }

        juce::Image im(juce::Image::ARGB, width, height, true);

        {
            juce::Graphics g2(im);
            g2.setColour(foreground);
            g2.fillRoundedRectangle(barBounds, rectRadius);
        }

        g.setTiledImageFill(im, 0, 0, 0.85f);
        g.fillPath(p);
    }
    
    g.setColour(Colours::text());
    juce::Font font = juce::Font(juce::FontOptions((float) height * 0.9f, juce::Font::bold));
    g.setFont(font);

    g.drawText (text,
                juce::Rectangle<float> (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height)),
                juce::Justification::centred,
                false);
}

juce::Typeface::Ptr LookAndFeel::getTypefaceForFont(const juce::Font& font) {
    if (font.getTypefaceName() == juce::Font::getDefaultSansSerifFontName()) {
        if (font.getTypefaceStyle() == "Regular" && regularTypeface != nullptr) {
            return regularTypeface;
        } else if (font.getTypefaceStyle() == "Bold" && boldTypeface != nullptr) {
            return boldTypeface;
        } else if (font.getTypefaceStyle() == "Italic" && italicTypeface != nullptr) {
            return italicTypeface;
        }
    }

    return juce::Font::getDefaultTypefaceForFont(font);
}

void LookAndFeel::drawAlertBox(juce::Graphics& g, juce::AlertWindow& alert,
                                          const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout) {
    auto cornerSize = static_cast<float>(RECT_RADIUS);
    auto bounds = alert.getLocalBounds().toFloat();

    // Fill background
    g.setColour(alert.findColour(juce::AlertWindow::backgroundColourId));
    g.fillRoundedRectangle(bounds, cornerSize);

    // Subtle border
    g.setColour(alert.findColour(juce::AlertWindow::outlineColourId));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);

    auto iconSpaceUsed = 0;
    auto iconWidth = 80;
    auto iconSize = juce::jmin(iconWidth + 50, alert.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = juce::jmin(iconSize, textArea.getHeight() + 50);

    juce::Rectangle<int> iconRect(iconSize / -10, iconSize / -10, iconSize, iconSize);

    if (alert.getAlertType() != juce::AlertWindow::NoIcon) {
        juce::Path icon;
        char character;
        juce::Colour iconColour;

        if (alert.getAlertType() == juce::AlertWindow::WarningIcon) {
            character = '!';

            icon.addTriangle(
                (float)iconRect.getX() + (float)iconRect.getWidth() * 0.5f, (float)iconRect.getY(),
                static_cast<float>(iconRect.getRight()), static_cast<float>(iconRect.getBottom()),
                static_cast<float>(iconRect.getX()), static_cast<float>(iconRect.getBottom()));

            icon = icon.createPathWithRoundedCorners(5.0f);
            iconColour = Colours::danger().withAlpha(0.5f);
        } else {
            iconColour = Colours::accentColor().withAlpha(0.4f);
            character = alert.getAlertType() == juce::AlertWindow::InfoIcon ? 'i' : '?';
            icon.addEllipse(iconRect.toFloat());
        }

        juce::GlyphArrangement ga;
        ga.addFittedText(juce::Font((float)iconRect.getHeight() * 0.9f, juce::Font::bold),
                         juce::String::charToString((juce::juce_wchar)(juce::uint8)character),
                         static_cast<float>(iconRect.getX()), static_cast<float>(iconRect.getY()),
                         static_cast<float>(iconRect.getWidth()), static_cast<float>(iconRect.getHeight()),
                         juce::Justification::centred, false);
        ga.createPath(icon);

        icon.setUsingNonZeroWinding(false);
        g.setColour(iconColour);
        g.fillPath(icon);

        iconSpaceUsed = iconWidth;
    }

    g.setColour(alert.findColour(juce::AlertWindow::textColourId));

    juce::Rectangle<int> alertBounds(alert.getLocalBounds().getX() + iconSpaceUsed, 30,
                                      alert.getLocalBounds().getWidth(), alert.getLocalBounds().getHeight() - getAlertWindowButtonHeight() - 20);

    textLayout.draw(g, alertBounds.toFloat());
}

int LookAndFeel::getAlertWindowButtonHeight() { return 40; }

juce::Font LookAndFeel::getAlertWindowTitleFont() {
    if (boldTypeface == nullptr) {
        return juce::Font (juce::FontOptions (18.0f, juce::Font::bold));
    }

    return juce::Font(boldTypeface).withHeight(18.0f);
}

juce::Font LookAndFeel::getAlertWindowMessageFont() {
    if (regularTypeface == nullptr) {
        return juce::Font (juce::FontOptions (15.0f));
    }

    return juce::Font(regularTypeface).withHeight(15.0f);
}

juce::Font LookAndFeel::getAlertWindowFont() {
    if (regularTypeface == nullptr) {
        return juce::Font (juce::FontOptions (14.0f));
    }

    return juce::Font(regularTypeface).withHeight(14.0f);
}

void LookAndFeel::drawStretchableLayoutResizerBar(juce::Graphics& g, int w, int h, bool isVerticalBar, bool isMouseOver, bool isMouseDragging) {
    if (isMouseOver || isMouseDragging) {
        g.setColour(Colours::accentColor().withAlpha(0.5f));
        g.fillRoundedRectangle(0, 0, w, h, 4.0f);
    }
}

void LookAndFeel::drawBubble(juce::Graphics& g, juce::BubbleComponent&,
                                        const juce::Point<float>&, const juce::Rectangle<float>& body) {
    g.setColour(Colours::veryDark());
    g.fillRoundedRectangle(body, 4.0f);
}

// ============================================================================
// PopupMenu — modern rounded design
// ============================================================================

void LookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    constexpr float radius = 8.0f;

    // Clear to transparent so rounded corners don't show the window background
    g.fillAll(Colours::transparent());

    // Background fill
    g.setColour(Colours::veryDark());
    g.fillRoundedRectangle(bounds, radius);

    // Subtle border
    g.setColour(Colours::outlineSubtle());
    g.drawRoundedRectangle(bounds.reduced(0.5f), radius, 1.0f);
}

void LookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                               bool isSeparator, bool isActive, bool isHighlighted,
                                               bool isTicked, bool hasSubMenu,
                                               const juce::String& text, const juce::String& shortcutKeyText,
                                               const juce::Drawable* icon, const juce::Colour* textColour) {
    if (isSeparator) {
        auto sepArea = area.reduced(8, 0);
        g.setColour(Colours::neutralStroke(0.08f));
        g.fillRect(sepArea.getX(), area.getCentreY(), sepArea.getWidth(), 1);
        return;
    }

    auto r = area.reduced(5, 1);
    constexpr float itemRadius = 4.0f;

    if (isHighlighted && isActive) {
        // Vital-style: accent colour highlight
        g.setColour(Colours::accentColor().withAlpha(0.6f));
        g.fillRoundedRectangle(r.toFloat(), itemRadius);
    }

    auto textColourToUse = isHighlighted && isActive
                         ? Colours::textOnAccent()
                         : (isActive ? Colours::text().withAlpha(0.88f)
                                     : Colours::text().withAlpha(0.35f));
    if (textColour != nullptr)
        textColourToUse = *textColour;

    auto font = juce::Font(13.0f);
    g.setFont(font);
    g.setColour(textColourToUse);

    auto textArea = r.reduced(2, 0);

    // Tick column — always reserve space so text is aligned across all items
    constexpr int tickW = 14;
    auto tickArea = textArea.removeFromLeft(tickW).toFloat();
    textArea.removeFromLeft(4);
    if (isTicked) {
        // Draw a simple checkmark
        auto cx = tickArea.getCentreX();
        auto cy = tickArea.getCentreY();
        juce::Path tick;
        tick.startNewSubPath(cx - 4.0f, cy);
        tick.lineTo(cx - 1.0f, cy + 3.5f);
        tick.lineTo(cx + 5.0f, cy - 4.0f);
        g.setColour(textColourToUse);
        g.strokePath(tick, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    if (hasSubMenu) {
        auto arrowArea = textArea.removeFromRight(14).toFloat();
        juce::Path arrow;
        auto ay = arrowArea.getCentreY();
        auto ax = arrowArea.getCentreX();
        arrow.startNewSubPath(ax - 2.0f, ay - 4.0f);
        arrow.lineTo(ax + 2.0f, ay);
        arrow.lineTo(ax - 2.0f, ay + 4.0f);
        g.strokePath(arrow, juce::PathStrokeType(1.5f));
    }

    if (text.isNotEmpty())
        osci::LookAndFeelHelpers::drawFittedText (g, text, textArea, juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty()) {
        g.setColour(textColourToUse.withAlpha(0.5f));
        g.setFont(juce::Font(11.0f));
        g.drawText(shortcutKeyText, textArea.toFloat(), juce::Justification::centredRight, true);
    }
}

void LookAndFeel::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
                                                       int standardMenuItemHeight, int& idealWidth, int& idealHeight) {
    if (isSeparator) {
        idealWidth = 50;
        idealHeight = 8;
        return;
    }

    auto font = juce::Font(13.0f);
    idealWidth = juce::GlyphArrangement::getStringWidthInt(font, text) + 40;
    idealHeight = 28;
}

int LookAndFeel::getPopupMenuBorderSize() {
    return 4;
}

} // namespace osci
