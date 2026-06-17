#pragma once

#include <JuceHeader.h>

namespace stagemind
{
class MeterView final : public juce::Component
{
public:
    explicit MeterView(juce::String labelText);

    void setLevels(float newRms, float newPeak);
    void paint(juce::Graphics& g) override;

private:
    juce::String label;
    float rms = 0.0f;
    float peak = 0.0f;
};
} // namespace stagemind
