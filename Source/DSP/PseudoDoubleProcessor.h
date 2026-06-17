#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>

namespace stagemind
{
struct PseudoDoubleConfig
{
    float amount = 0.0f;
};

class PseudoDoubleProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& buffer, const PseudoDoubleConfig& config) noexcept;

private:
    static constexpr int maxChannels = 2;

    float readDelay(int channel, int delaySamples) const noexcept;
    void writeDelay(int channel, float value) noexcept;

    double currentSampleRate = 44100.0;
    int delayBufferSize = 1;
    int writePosition = 0;
    juce::AudioBuffer<float> delayBuffer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount { 0.0f };
};
} // namespace stagemind
