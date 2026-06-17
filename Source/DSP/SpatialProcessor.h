#pragma once

#include "LowMonoProcessor.h"
#include "MidSideProcessor.h"
#include "SideHighPassProcessor.h"
#include "../Model/RoleProfile.h"

namespace stagemind
{
class SpatialProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void setParams(const SpatialParams& params) noexcept;
    void process(juce::AudioBuffer<float>& buffer) noexcept;

private:
    void applyPan(juce::AudioBuffer<float>& buffer) noexcept;

    LowMonoProcessor lowMono;
    SideHighPassProcessor sideHighPass;
    MidSideProcessor midSide;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pan { 0.0f };
};
} // namespace stagemind
