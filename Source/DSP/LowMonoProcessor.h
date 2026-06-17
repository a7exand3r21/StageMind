#pragma once

#include <JuceHeader.h>

namespace stagemind
{
class LowMonoProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void setCutoffHz(float cutoffHz) noexcept;
    void process(juce::AudioBuffer<float>& buffer) noexcept;

private:
    void updateCoefficient(float cutoffHz) noexcept;

    double currentSampleRate = 44100.0;
    float coefficient = 0.0f;
    float sideLowState = 0.0f;
    float lastCutoffHz = 160.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> cutoff { 160.0f };
};
} // namespace stagemind
