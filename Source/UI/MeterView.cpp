#include "MeterView.h"
#include "Theme.h"

#include <algorithm>

namespace stagemind
{
MeterView::MeterView(juce::String labelText)
    : label(std::move(labelText))
{
}

void MeterView::setLevels(float newRms, float newPeak)
{
    rms = juce::jlimit(0.0f, 1.0f, newRms);
    peak = juce::jlimit(0.0f, 1.0f, newPeak);
    repaint();
}

void MeterView::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    const auto labelArea = getLocalBounds().removeFromTop(20);
    g.setColour(theme::textMuted);
    g.setFont(juce::FontOptions { 10.5f, juce::Font::plain });
    g.drawText(label, labelArea, juce::Justification::centred);

    auto meterArea = getLocalBounds().reduced(8);
    meterArea.removeFromTop(22);
    meterArea.reduce(2, 2);

    const auto peakY = meterArea.getBottom() - static_cast<int> (static_cast<float> (meterArea.getHeight()) * peak);

    const auto slot = meterArea.toFloat();
    g.setColour(theme::shadow.withAlpha(0.20f));
    g.fillRoundedRectangle(slot.expanded(1.0f).translated(0.0f, 1.0f), 3.0f);

    juce::ColourGradient glass(
        theme::displayRaised,
        slot.getCentreX(),
        slot.getY(),
        theme::display,
        slot.getCentreX(),
        slot.getBottom(),
        false);
    g.setGradientFill(glass);
    g.fillRoundedRectangle(slot, 3.0f);

    const auto segmentCount = 22;
    const auto segmentGap = 3.0f;
    const auto segmentHeight = std::max(2.0f, (slot.getHeight() - segmentGap * static_cast<float> (segmentCount - 1) - 8.0f) / static_cast<float> (segmentCount));
    auto segment = juce::Rectangle<float> {
        slot.getX() + 5.0f,
        slot.getBottom() - 5.0f - segmentHeight,
        slot.getWidth() - 10.0f,
        segmentHeight
    };

    for (int i = 0; i < segmentCount; ++i)
    {
        const auto threshold = static_cast<float> (i + 1) / static_cast<float> (segmentCount);
        const auto active = rms >= threshold;
        const auto hot = threshold > 0.86f;
        const auto warm = threshold > 0.68f;
        const auto colour = hot ? theme::meterHot : (warm ? theme::meterWarm : juce::Colour { 0xff2ff4ff });
        g.setColour(active ? colour.withAlpha(0.88f) : theme::textOnDisplay.withAlpha(0.08f));
        g.fillRoundedRectangle(segment, 1.5f);
        segment.translate(0.0f, -(segmentHeight + segmentGap));
    }

    g.setColour(theme::textOnDisplay.withAlpha(0.75f));
    g.drawHorizontalLine(peakY, slot.getX() + 2.0f, slot.getRight() - 2.0f);
}
} // namespace stagemind
