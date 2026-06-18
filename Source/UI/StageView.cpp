#include "StageView.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
namespace
{
float clamp01(float value) noexcept
{
    return juce::jlimit(0.0f, 1.0f, value);
}
} // namespace

void StageView::setState(State newState)
{
    state = newState;
    repaint();
}

void StageView::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    auto display = bounds.reduced(2.0f, 4.0f);
    g.setColour(theme::shadow.withAlpha(0.22f));
    g.fillRoundedRectangle(display.translated(0.0f, 2.0f), 8.0f);

    juce::ColourGradient glass(
        theme::displayRaised,
        display.getCentreX(),
        display.getY(),
        theme::display,
        display.getCentreX(),
        display.getBottom(),
        false);
    g.setGradientFill(glass);
    g.fillRoundedRectangle(display, 8.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.fillRoundedRectangle(display.reduced(3.0f).withHeight(display.getHeight() * 0.30f), 5.0f);
    g.setColour(theme::borderDark.withAlpha(0.55f));
    g.drawRoundedRectangle(display.reduced(0.5f), 8.0f, 1.0f);

    auto stage = display.reduced(18.0f, 24.0f);

    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(0.10f));
    for (auto x = stage.getX() + 8.0f; x < stage.getRight(); x += 18.0f)
    {
        for (auto yDot = stage.getY() + 8.0f; yDot < stage.getBottom(); yDot += 18.0f)
            g.fillEllipse(x - 0.75f, yDot - 0.75f, 1.5f, 1.5f);
    }

    g.setColour(theme::textOnDisplay.withAlpha(0.20f));
    g.drawLine(stage.getCentreX(), stage.getY(), stage.getCentreX(), stage.getBottom(), 1.0f);
    g.drawLine(stage.getX(), stage.getCentreY(), stage.getRight(), stage.getCentreY(), 1.0f);
    g.drawLine(stage.getX(), stage.getY(), stage.getRight(), stage.getY(), 1.0f);
    g.drawLine(stage.getX(), stage.getBottom(), stage.getRight(), stage.getBottom(), 1.0f);

    g.setColour(theme::textOnDisplay.withAlpha(0.72f));
    g.setFont(juce::FontOptions { 10.0f });
    g.drawText("Back", stage.withHeight(18.0f).toNearestInt(), juce::Justification::centredRight);
    g.drawText("Front", stage.withY(stage.getBottom() - 18.0f).withHeight(18.0f).toNearestInt(), juce::Justification::centredRight);

    const auto dotColour = state.correlation < 0.1f ? theme::warning : theme::textOnDisplay;
    const auto pan = juce::jlimit(-1.0f, 1.0f, state.pan);
    const auto depth = clamp01(state.depth);
    const auto width = clamp01(state.width);
    const auto motion = clamp01(state.motion);
    const auto baseX = juce::jmap(pan, -1.0f, 1.0f, stage.getX() + 10.0f, stage.getRight() - 10.0f);
    const auto y = juce::jmap(depth, 0.0f, 1.0f, stage.getBottom() - 10.0f, stage.getY() + 10.0f);
    const auto maxSpread = std::max(10.0f, stage.getWidth() * 0.36f);
    const auto spread = juce::jmap(width, 0.0f, 1.0f, 8.0f, maxSpread);
    const auto motionOffset = std::sin(state.motionPhase) * motion * spread;
    const auto x = juce::jlimit(stage.getX() + 10.0f, stage.getRight() - 10.0f, baseX + motionOffset);
    const auto size = juce::jmap(depth, 0.0f, 1.0f, 26.0f, 15.0f);

    const auto leftEdge = juce::jlimit(stage.getX() + 5.0f, stage.getRight() - 5.0f, baseX - spread);
    const auto rightEdge = juce::jlimit(stage.getX() + 5.0f, stage.getRight() - 5.0f, baseX + spread);
    g.setColour(dotColour.withAlpha(0.20f));
    g.drawLine(leftEdge, y, rightEdge, y, 4.0f);
    g.fillEllipse(juce::Rectangle<float> { 8.0f, 8.0f }.withCentre({ leftEdge, y }));
    g.fillEllipse(juce::Rectangle<float> { 8.0f, 8.0f }.withCentre({ rightEdge, y }));

    if (motion > 0.01f)
    {
        const auto pathWidth = juce::jlimit(18.0f, maxSpread * 2.0f, spread * motion * 2.0f);
        g.setColour(theme::accentWarm.withAlpha(0.30f));
        g.drawEllipse(juce::Rectangle<float> { pathWidth, 12.0f }.withCentre({ baseX, y }), 1.5f);
    }

    const auto dot = juce::Rectangle<float> { size, size }.withCentre({ x, y });
    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(0.06f));
    g.fillEllipse(dot.expanded(34.0f));
    g.setColour(juce::Colour { 0xff2ff4ff }.withAlpha(0.12f));
    g.fillEllipse(dot.expanded(22.0f));
    g.setColour(dotColour.withAlpha(0.18f));
    g.fillEllipse(dot.expanded(10.0f));
    g.setColour(state.correlation < 0.1f ? theme::warning : theme::accent.brighter(0.55f));
    g.fillEllipse(dot);
    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.fillEllipse(dot.reduced(size * 0.28f).translated(-size * 0.10f, -size * 0.12f));

    juce::String status;
    auto statusColour = theme::textMuted;
    if (! state.motionAllowed && state.requestedMotion > 0.01f)
    {
        status = "Motion locked by role";
        statusColour = theme::warning;
    }
    else if (state.correlation < 0.1f)
    {
        status = "Width safety active";
        statusColour = theme::warning;
    }
    else if (motion > 0.01f)
    {
        status = "Motion active";
        statusColour = theme::accent;
    }

    if (status.isNotEmpty())
    {
        g.setColour(statusColour);
        g.setFont(juce::FontOptions { 10.5f });
        g.drawText(status, getLocalBounds().removeFromBottom(24), juce::Justification::centred);
    }
}
} // namespace stagemind
