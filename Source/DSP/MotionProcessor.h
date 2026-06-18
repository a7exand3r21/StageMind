#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace stagemind
{
struct MotionConfig
{
    float amount = 0.0f;
    float rateHz = 0.2f;
    int preset = 0;
};

class MotionProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& buffer, const MotionConfig& config) noexcept;

private:
    double currentSampleRate = 44100.0;
    float phase = 0.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> rateHz { 0.2f };
};
} // namespace stagemind
