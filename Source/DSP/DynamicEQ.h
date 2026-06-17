#pragma once

#include "ResonanceTypes.h"
#include <array>
#include <juce_audio_basics/juce_audio_basics.h>

namespace stagemind
{
struct ResonanceSuppressionConfig
{
    float resonanceAmount = 0.3f;
    float maxReductionDb = 4.0f;
    float attackMs = 30.0f;
    float releaseMs = 250.0f;
};

class DynamicEQ
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;

    float processResonances(
        juce::AudioBuffer<float>& buffer,
        const ResonanceSnapshot& snapshot,
        const ResonanceSuppressionConfig& config) noexcept;

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

    struct Band
    {
        std::array<BandPath, 2> paths {};
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> frequencyHz { 1000.0f };
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> q { 4.0f };
        float currentReductionDb = 0.0f;
        float targetReductionDb = 0.0f;

        void prepare(double sampleRate) noexcept;
        void reset() noexcept;
        void setTarget(const ResonancePeak& peak, float resonanceAmount, float maxReductionDb) noexcept;
        void release() noexcept;
    };

    static float coefficientForCutoff(double sampleRate, float cutoffHz) noexcept;
    static float coefficientForMs(double sampleRate, float milliseconds) noexcept;

    double currentSampleRate = 44100.0;
    std::array<Band, maxResonancePeaks> bands {};
};
} // namespace stagemind
