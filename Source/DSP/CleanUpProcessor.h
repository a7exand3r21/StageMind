#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>

namespace stagemind
{
struct CleanUpConfig
{
    float amount = 0.0f;
    float lowMidReduction = 0.12f;
    float harshReduction = 0.12f;
    float airLift = 0.02f;
    float lowMidStartHz = 140.0f;
    float lowMidEndHz = 720.0f;
    float harshStartHz = 2400.0f;
    float harshEndHz = 7600.0f;
    float airStartHz = 7200.0f;
};

class CleanUpProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& buffer, const CleanUpConfig& config) noexcept;

private:
    struct OnePolePath
    {
        float lowState = 0.0f;

        void reset() noexcept { lowState = 0.0f; }
        float processLowPass(float input, float coefficient) noexcept;
        float processHighPass(float input, float coefficient) noexcept;
    };

    struct BandPath
    {
        OnePolePath startHighPass;
        OnePolePath endLowPass;

        void reset() noexcept;
        float processBand(float input, float startCoefficient, float endCoefficient) noexcept;
    };

    struct ChannelPath
    {
        BandPath lowMid;
        BandPath harsh;
        OnePolePath airLowPass;

        void reset() noexcept;
    };

    static float coefficientForCutoff(double sampleRate, float cutoffHz) noexcept;

    static constexpr int maxChannels = 2;

    double currentSampleRate = 44100.0;
    std::array<ChannelPath, maxChannels> paths {};
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount { 0.0f };
};
} // namespace stagemind
