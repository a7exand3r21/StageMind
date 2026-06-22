#pragma once

#include "../Model/StageGainMeterMode.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

namespace stagemind
{
class StageGainLoudnessMeter
{
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;

    float process(const juce::AudioBuffer<float>& buffer, StageGainMeterMode mode) noexcept;
    float currentDb(StageGainMeterMode mode) const noexcept;

    static float targetLevelFromDb(StageGainMeterMode mode, float targetDb) noexcept;
    static float levelToDb(StageGainMeterMode mode, float level) noexcept;

private:
    float rawMeanSquare(const juce::AudioBuffer<float>& buffer) const noexcept;
    float kWeightedMeanSquare(const juce::AudioBuffer<float>& buffer) noexcept;
    float smoothMeanSquare(float previous, float incoming, float seconds, int numSamples) const noexcept;
    void updateIntegrated(float blockMeanSquare, int numSamples) noexcept;

    double currentSampleRate = 44100.0;
    float vuMeanSquare = 0.0f;
    float momentaryMeanSquare = 0.0f;
    float shortTermMeanSquare = 0.0f;
    float integratedMeanSquare = 0.0f;
    double integratedSamples = 0.0;
    float lastRawMeanSquare = 0.0f;
    float lastWeightedMeanSquare = 0.0f;
    std::array<float, 2> hpPreviousInput {};
    std::array<float, 2> hpPreviousOutput {};
};
} // namespace stagemind
