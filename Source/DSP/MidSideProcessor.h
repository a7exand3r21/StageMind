#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

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
    static float coefficientForCutoff(double sampleRate, float cutoffHz) noexcept;

    double currentSampleRate = 44100.0;
    float monoSpreadLowState = 0.0f;
    float monoSpreadCoefficient = 0.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthAmount { 1.0f };
};
} // namespace stagemind
