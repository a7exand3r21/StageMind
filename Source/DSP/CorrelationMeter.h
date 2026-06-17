#pragma once

#include <JuceHeader.h>

namespace stagemind
{
class CorrelationMeter
{
public:
    void reset() noexcept;
    float processBlock(const juce::AudioBuffer<float>& buffer) noexcept;

private:
    float lastValue = 1.0f;
};
} // namespace stagemind
