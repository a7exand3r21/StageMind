#include "ResonanceListView.h"
#include "Theme.h"

namespace stagemind
{
void ResonanceListView::setSnapshot(const ResonanceSnapshot& newSnapshot)
{
    snapshot = newSnapshot;
    repaint();
}

void ResonanceListView::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour(theme::panelRaised);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(theme::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 5.0f, 1.0f);

    auto area = getLocalBounds().reduced(8);
    g.setColour(theme::textMuted);
    g.setFont(juce::FontOptions { 12.0f });
    g.drawText("Resonances", area.removeFromTop(18), juce::Justification::centred);

    g.setFont(juce::FontOptions { 11.0f });

    if (snapshot.peakCount == 0)
    {
        g.setColour(theme::textMuted.withAlpha(0.7f));
        g.drawText("No narrow peaks", area, juce::Justification::centred);
        return;
    }

    for (int i = 0; i < static_cast<int> (snapshot.peakCount); ++i)
    {
        auto row = area.removeFromTop(22);
        const auto& peak = snapshot.peaks[static_cast<size_t> (i)];
        const auto severity = juce::jlimit(0.0f, 1.0f, peak.severity);

        auto bar = row.removeFromRight(54).reduced(0, 8);
        g.setColour(theme::border);
        g.fillRoundedRectangle(bar.toFloat(), 3.0f);

        g.setColour(severity > 0.7f ? theme::warning : theme::accentWarm);
        g.fillRoundedRectangle(bar.withWidth(static_cast<int> (static_cast<float> (bar.getWidth()) * severity)).toFloat(), 3.0f);

        const auto freqText = peak.frequencyHz >= 1000.0f
            ? juce::String(peak.frequencyHz / 1000.0f, 2) + " kHz"
            : juce::String(peak.frequencyHz, 0) + " Hz";

        g.setColour(theme::text);
        g.drawText(freqText, row.removeFromLeft(70), juce::Justification::centredLeft);
        g.setColour(theme::textMuted);
        g.drawText("-" + juce::String(peak.suggestedReductionDb, 1) + " dB", row, juce::Justification::centredRight);
    }
}
} // namespace stagemind
