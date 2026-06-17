#pragma once

#include <JuceHeader.h>

namespace stagemind
{
class MidSideProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void setWidthAmount(float targetWidthAmount) noexcept;
    void process(juce::AudioBuffer<float>& buffer) noexcept;

private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthAmount { 1.0f };
};
} // namespace stagemind
