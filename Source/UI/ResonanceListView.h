#pragma once

#include "../DSP/ResonanceTypes.h"
#include <JuceHeader.h>

namespace stagemind
{
class ResonanceListView final : public juce::Component
{
public:
    void setSnapshot(const ResonanceSnapshot& newSnapshot);
    void paint(juce::Graphics& g) override;

private:
    ResonanceSnapshot snapshot;
};
} // namespace stagemind
