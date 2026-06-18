#include "HardwareLookAndFeel.h"
#include "Theme.h"

#include <cmath>

namespace stagemind
{
namespace
{
juce::Rectangle<float> reducedPixelBounds(int width, int height, float amount = 0.5f)
{
    return juce::Rectangle<float> { 0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height) }
        .reduced(amount);
}

void drawSoftPanel(juce::Graphics& g, juce::Rectangle<float> bounds, float corner)
{
    juce::DropShadow(theme::shadow, 9, { 0, 3 }).drawForRectangle(g, bounds.toNearestInt());

    juce::ColourGradient gradient(
        theme::panelRaised,
        bounds.getCentreX(),
        bounds.getY(),
        theme::panelInset,
        bounds.getCentreX(),
        bounds.getBottom(),
        false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, corner);

    g.setColour(juce::Colours::white.withAlpha(0.75f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), corner, 1.0f);
    g.setColour(theme::border.withAlpha(0.85f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), corner, 1.0f);
}

juce::Path roundedTriangle(juce::Rectangle<float> area)
{
    juce::Path path;
    path.startNewSubPath(area.getX(), area.getY());
    path.lineTo(area.getCentreX(), area.getBottom());
    path.lineTo(area.getRight(), area.getY());
    path.closeSubPath();
    return path;
}

bool hasComponentId(const juce::Component& component, juce::StringRef id) noexcept
{
    return component.getComponentID() == id;
}

void drawCyanGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float corner, float alpha)
{
    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(alpha * 0.16f));
    g.fillRoundedRectangle(bounds.expanded(7.0f), corner + 7.0f);
    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(alpha * 0.25f));
    g.drawRoundedRectangle(bounds.expanded(3.0f), corner + 3.0f, 1.4f);
}
} // namespace

HardwareLookAndFeel::HardwareLookAndFeel()
{
    setColour(juce::ComboBox::textColourId, theme::textOnDisplay);
    setColour(juce::ComboBox::backgroundColourId, theme::display);
    setColour(juce::ComboBox::outlineColourId, theme::borderDark.withAlpha(0.55f));
    setColour(juce::ComboBox::arrowColourId, theme::textOnDisplay);
    setColour(juce::PopupMenu::backgroundColourId, theme::displayRaised);
    setColour(juce::PopupMenu::textColourId, theme::textOnDisplay);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, theme::accent.withAlpha(0.55f));
    setColour(juce::PopupMenu::highlightedTextColourId, theme::textOnDisplay);
    setColour(juce::Slider::rotarySliderFillColourId, theme::accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, theme::border);
    setColour(juce::Slider::thumbColourId, theme::text.withAlpha(0.78f));
    setColour(juce::Slider::trackColourId, theme::accent);
    setColour(juce::Slider::backgroundColourId, theme::border.withAlpha(0.65f));
    setColour(juce::Slider::textBoxTextColourId, theme::text);
    setColour(juce::Slider::textBoxBackgroundColourId, theme::panelInset);
    setColour(juce::Slider::textBoxOutlineColourId, theme::border.withAlpha(0.7f));
}

juce::Colour HardwareLookAndFeel::glassTop() noexcept
{
    return juce::Colour { 0xff222930 };
}

juce::Colour HardwareLookAndFeel::glassBottom() noexcept
{
    return juce::Colour { 0xff080b0f };
}

juce::Font HardwareLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::FontOptions { juce::jlimit(11.0f, 14.0f, static_cast<float> (buttonHeight) * 0.42f), juce::Font::plain };
}

void HardwareLookAndFeel::drawButtonBackground(
    juce::Graphics& g,
    juce::Button& button,
    const juce::Colour&,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = reducedPixelBounds(button.getWidth(), button.getHeight(), 1.0f);
    const auto corner = std::min(hasComponentId(button, "pillGlowButton") ? 16.0f : 7.0f, bounds.getHeight() * 0.48f);
    const auto toggled = button.getToggleState();
    const auto glowButton = hasComponentId(button, "glowButton") || hasComponentId(button, "pillGlowButton");

    if (glowButton || toggled)
        drawCyanGlow(g, bounds, corner, toggled ? 0.95f : 0.60f);

    juce::ColourGradient gradient(
        toggled ? theme::accent.brighter(0.28f) : theme::panelRaised.brighter(0.05f),
        bounds.getCentreX(),
        bounds.getY(),
        toggled ? theme::accent.darker(0.05f) : theme::panelInset,
        bounds.getCentreX(),
        bounds.getBottom(),
        false);

    if (shouldDrawButtonAsDown)
        gradient = juce::ColourGradient(
            theme::panelInset,
            bounds.getCentreX(),
            bounds.getY(),
            theme::panelRaised,
            bounds.getCentreX(),
            bounds.getBottom(),
            false);

    g.setColour(theme::shadow.withAlpha(0.18f));
    g.fillRoundedRectangle(bounds.translated(0.0f, 1.0f), corner);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, corner);

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.42f));
        g.fillRoundedRectangle(bounds.reduced(1.0f), corner);
    }

    g.setColour(toggled ? theme::accent.darker(0.55f) : theme::borderDark.withAlpha(0.72f));
    g.drawRoundedRectangle(bounds, corner, 1.0f);
    if (glowButton || toggled)
    {
        g.setColour(juce::Colour { 0xff26eff8 }.withAlpha(0.95f));
        g.drawRoundedRectangle(bounds.reduced(1.8f), corner - 1.0f, 1.4f);
    }
    g.setColour(juce::Colours::white.withAlpha(toggled ? 0.22f : 0.65f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), corner, 1.0f);
}

void HardwareLookAndFeel::drawComboBox(
    juce::Graphics& g,
    int width,
    int height,
    bool isButtonDown,
    int,
    int,
    int,
    int,
    juce::ComboBox& box)
{
    auto bounds = reducedPixelBounds(width, height, 0.5f);
    const auto headerCombo = hasComponentId(box, "headerCombo");
    const auto miniCombo = hasComponentId(box, "miniCombo");
    const auto corner = std::min(8.0f, bounds.getHeight() * 0.24f);

    if (headerCombo)
    {
        if (isButtonDown)
        {
            g.setColour(juce::Colours::white.withAlpha(0.08f));
            g.fillRoundedRectangle(bounds.reduced(2.0f), 6.0f);
        }

        const auto arrowArea = juce::Rectangle<float> {
            bounds.getRight() - 18.0f,
            bounds.getCentreY() - 3.0f,
            9.0f,
            6.0f
        };
        g.setColour(theme::textOnDisplay.withAlpha(box.isEnabled() ? 0.88f : 0.30f));
        g.fillPath(roundedTriangle(arrowArea));
        return;
    }

    juce::ColourGradient glass(glassTop(), bounds.getCentreX(), bounds.getY(), glassBottom(), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(glass);
    g.fillRoundedRectangle(bounds, corner);

    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.fillRoundedRectangle(bounds.reduced(2.0f).withHeight(bounds.getHeight() * 0.34f), corner * 0.65f);

    g.setColour(isButtonDown ? theme::accentWarm.withAlpha(0.65f) : box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, corner, 1.0f);

    const auto arrowArea = juce::Rectangle<float> {
        bounds.getRight() - 22.0f,
        bounds.getCentreY() - 3.0f,
        9.0f,
        6.0f
    };
    g.setColour(theme::textOnDisplay.withAlpha(box.isEnabled() ? (miniCombo ? 0.78f : 0.88f) : 0.30f));
    g.fillPath(roundedTriangle(arrowArea));
}

juce::Font HardwareLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    if (hasComponentId(box, "headerCombo"))
        return juce::FontOptions { juce::jlimit(15.0f, 19.0f, static_cast<float> (box.getHeight()) * 0.62f), juce::Font::bold };

    if (hasComponentId(box, "miniCombo"))
        return juce::FontOptions { 10.5f };

    return juce::FontOptions { juce::jlimit(11.0f, 14.0f, static_cast<float> (box.getHeight()) * 0.46f), juce::Font::plain };
}

void HardwareLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    if (hasComponentId(box, "headerCombo"))
        label.setBounds(0, 0, box.getWidth() - 24, box.getHeight());
    else
        label.setBounds(8, 1, box.getWidth() - 32, box.getHeight() - 2);

    label.setFont(getComboBoxFont(box));
    label.setColour(juce::Label::textColourId, hasComponentId(box, "headerCombo") ? juce::Colour { 0xff68f2fb } : theme::textOnDisplay);
}

void HardwareLookAndFeel::drawPopupMenuItem(
    juce::Graphics& g,
    const juce::Rectangle<int>& area,
    bool isSeparator,
    bool isActive,
    bool isHighlighted,
    bool isTicked,
    bool,
    const juce::String& text,
    const juce::String& shortcutKeyText,
    const juce::Drawable*,
    const juce::Colour* textColour)
{
    auto row = area.toFloat().reduced(4.0f, 1.0f);

    if (isSeparator)
    {
        g.setColour(theme::textOnDisplay.withAlpha(0.18f));
        g.drawHorizontalLine(area.getCentreY(), static_cast<float> (area.getX() + 8), static_cast<float> (area.getRight() - 8));
        return;
    }

    if (isHighlighted)
    {
        drawCyanGlow(g, row, 5.0f, 0.60f);
        g.setColour(theme::accent.withAlpha(0.70f));
        g.fillRoundedRectangle(row, 5.0f);
    }

    if (isTicked)
    {
        g.setColour(juce::Colour { 0xff2ff4ff });
        g.fillEllipse(row.getX() + 6.0f, row.getCentreY() - 3.0f, 6.0f, 6.0f);
    }

    const auto colour = textColour != nullptr
        ? *textColour
        : (isActive ? theme::textOnDisplay : theme::textOnDisplay.withAlpha(0.35f));
    g.setColour(colour);
    g.setFont(juce::FontOptions { 13.0f });
    g.drawText(text, area.reduced(22, 0), juce::Justification::centredLeft);

    if (shortcutKeyText.isNotEmpty())
    {
        g.setColour(colour.withAlpha(0.55f));
        g.drawText(shortcutKeyText, area.reduced(10, 0), juce::Justification::centredRight);
    }
}

void HardwareLookAndFeel::drawLinearSlider(
    juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float,
    float,
    juce::Slider::SliderStyle,
    juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> {
        static_cast<float> (x),
        static_cast<float> (y),
        static_cast<float> (width),
        static_cast<float> (height)
    }.reduced(4.0f);

    const auto horizontal = slider.isHorizontal();
    const auto trackThickness = horizontal ? 5.0f : 4.0f;
    const auto track = horizontal
        ? juce::Rectangle<float> { bounds.getX() + 6.0f, bounds.getCentreY() - trackThickness * 0.5f, bounds.getWidth() - 12.0f, trackThickness }
        : juce::Rectangle<float> { bounds.getCentreX() - trackThickness * 0.5f, bounds.getY() + 6.0f, trackThickness, bounds.getHeight() - 12.0f };

    g.setColour(theme::shadow.withAlpha(0.20f));
    g.fillRoundedRectangle(track.expanded(1.0f).translated(0.0f, 1.0f), 3.0f);
    g.setColour(theme::display.withAlpha(0.85f));
    g.fillRoundedRectangle(track, 3.0f);

    const auto thumbCentre = horizontal
        ? juce::Point<float> { juce::jlimit(track.getX(), track.getRight(), sliderPos), track.getCentreY() }
        : juce::Point<float> { track.getCentreX(), juce::jlimit(track.getY(), track.getBottom(), sliderPos) };

    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(0.35f));
    if (horizontal)
        g.fillRoundedRectangle(track.withWidth(std::max(1.0f, thumbCentre.x - track.getX())), 3.0f);
    else
        g.fillRoundedRectangle(track.withY(thumbCentre.y).withBottom(track.getBottom()), 3.0f);

    auto thumb = horizontal
        ? juce::Rectangle<float> { 12.0f, 30.0f }.withCentre(thumbCentre)
        : juce::Rectangle<float> { 22.0f, 12.0f }.withCentre(thumbCentre);

    drawSoftPanel(g, thumb, 4.0f);
    g.setColour(theme::borderDark.withAlpha(0.45f));

    if (horizontal)
        g.drawVerticalLine(static_cast<int> (thumb.getCentreX()), thumb.getY() + 5.0f, thumb.getBottom() - 5.0f);
    else
        g.drawHorizontalLine(static_cast<int> (thumb.getCentreY()), thumb.getX() + 4.0f, thumb.getRight() - 4.0f);
}

void HardwareLookAndFeel::drawRotarySlider(
    juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider&)
{
    auto bounds = juce::Rectangle<float> {
        static_cast<float> (x),
        static_cast<float> (y),
        static_cast<float> (width),
        static_cast<float> (height)
    }.reduced(8.0f);

    bounds.removeFromBottom(20.0f);
    const auto size = std::min(bounds.getWidth(), bounds.getHeight());
    auto knob = juce::Rectangle<float> { size, size }.withCentre(bounds.getCentre()).reduced(7.0f);
    const auto radius = knob.getWidth() * 0.5f;
    const auto centre = knob.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    for (int tick = 0; tick <= 10; ++tick)
    {
        const auto proportion = static_cast<float> (tick) / 10.0f;
        const auto tickAngle = rotaryStartAngle + proportion * (rotaryEndAngle - rotaryStartAngle);
        const auto tickRadius = radius + 17.0f;
        const auto tickCentre = centre.getPointOnCircumference(tickRadius, tickAngle);
        const auto active = proportion <= sliderPosProportional;
        g.setColour(active ? juce::Colour { 0xff2ff4ff }.withAlpha(0.55f) : theme::borderDark.withAlpha(0.28f));
        g.fillEllipse(juce::Rectangle<float> { active ? 3.2f : 2.2f, active ? 3.2f : 2.2f }.withCentre(tickCentre));
    }

    g.setColour(theme::shadow.withAlpha(0.24f));
    g.fillEllipse(knob.translated(0.0f, 4.0f).expanded(3.0f));

    g.setColour(theme::border.withAlpha(0.82f));
    g.fillEllipse(knob.expanded(5.0f));
    g.setColour(theme::panelInset);
    g.fillEllipse(knob.expanded(2.0f));

    juce::ColourGradient metal(
        juce::Colours::white,
        knob.getX(),
        knob.getY(),
        theme::panelInset.darker(0.16f),
        knob.getRight(),
        knob.getBottom(),
        false);
    metal.addColour(0.45, theme::panelRaised);
    metal.addColour(1.0, theme::border.brighter(0.04f));
    g.setGradientFill(metal);
    g.fillEllipse(knob);

    g.setColour(juce::Colours::white.withAlpha(0.72f));
    g.fillEllipse(knob.reduced(radius * 0.18f).withY(knob.getY() + radius * 0.10f).withHeight(radius * 0.72f));

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, radius + 9.0f, radius + 9.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(theme::borderDark.withAlpha(0.24f));
    g.strokePath(backgroundArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius + 9.0f, radius + 9.0f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(0.18f));
    g.strokePath(valueArc, juce::PathStrokeType(10.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(0.92f));
    g.strokePath(valueArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto pointerLength = radius * 0.62f;
    const auto pointerEnd = centre.getPointOnCircumference(pointerLength, angle);
    g.setColour(theme::text.withAlpha(0.72f));
    g.drawLine(juce::Line<float> { centre, pointerEnd }, 2.0f);
    g.fillEllipse(juce::Rectangle<float> { 5.0f, 5.0f }.withCentre(pointerEnd));

    g.setColour(theme::borderDark.withAlpha(0.42f));
    g.drawEllipse(knob, 1.0f);
}

void HardwareLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor&)
{
    auto bounds = reducedPixelBounds(width, height, 0.5f);
    g.setColour(theme::panelInset);
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 3.0f, 1.0f);
}

void HardwareLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    auto bounds = reducedPixelBounds(width, height, 0.5f);
    g.setColour(textEditor.hasKeyboardFocus(true) ? theme::accent : theme::borderDark.withAlpha(0.55f));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}

juce::Label* HardwareLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox(slider);
    label->setJustificationType(juce::Justification::centred);
    label->setFont(juce::FontOptions { 11.0f });
    label->setColour(juce::Label::textColourId, juce::Colour { 0xff10dbe6 });
    label->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    return label;
}
} // namespace stagemind
