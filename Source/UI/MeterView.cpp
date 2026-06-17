#include "MeterView.h"
#include "Theme.h"

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
    g.setColour(theme::panelRaised);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(theme::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 5.0f, 1.0f);

    const auto labelArea = getLocalBounds().removeFromTop(20);
    g.setColour(theme::textMuted);
    g.setFont(12.0f);
    g.drawText(label, labelArea, juce::Justification::centred);

    auto meterArea = getLocalBounds().reduced(8);
    meterArea.removeFromTop(22);

    const auto rmsHeight = static_cast<int> (static_cast<float> (meterArea.getHeight()) * rms);
    const auto peakY = meterArea.getBottom() - static_cast<int> (static_cast<float> (meterArea.getHeight()) * peak);

    g.setColour(theme::meterCold.withAlpha(0.25f));
    g.fillRect(meterArea);

    g.setColour(peak > 0.85f ? theme::meterHot : (peak > 0.65f ? theme::meterWarm : theme::accent));
    g.fillRect(meterArea.removeFromBottom(rmsHeight));

    g.setColour(theme::text);
    g.drawHorizontalLine(peakY, static_cast<float> (meterArea.getX()), static_cast<float> (meterArea.getRight()));
}
} // namespace stagemind
