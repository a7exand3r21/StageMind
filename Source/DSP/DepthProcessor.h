#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>

namespace stagemind
{
struct DepthConfig
{
    float amount = 0.0f;
    float presenceReduction = 0.0f;
    float earlyReflectionAmount = 0.0f;
};

class DepthProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& buffer, const DepthConfig& config) noexcept;

private:
    static constexpr int maxChannels = 2;

    float readDelay(int channel, int delaySamples) const noexcept;
    void writeDelay(int channel, float value) noexcept;
    static float coefficientForCutoff(double sampleRate, float cutoffHz) noexcept;

    double currentSampleRate = 44100.0;
    int delayBufferSize = 1;
    int writePosition = 0;
    int reflectionDelaySamples = 1;
    float dampingCoefficient = 0.0f;

    juce::AudioBuffer<float> delayBuffer;
    std::array<float, maxChannels> presenceLowState {};
    std::array<float, maxChannels> wetLowState {};
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> presenceReduction { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> earlyReflectionAmount { 0.0f };
};
} // namespace stagemind
