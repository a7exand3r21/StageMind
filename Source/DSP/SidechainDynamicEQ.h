#pragma once

#include "SidechainDetector.h"
#include "../Model/SidechainConflictMode.h"
#include <JuceHeader.h>
#include <array>

namespace stagemind
{
inline constexpr size_t maxSidechainBands = 2;

struct SidechainBand
{
    float frequencyStartHz = 0.0f;
    float frequencyEndHz = 0.0f;
    float maxReductionDb = 0.0f;
    bool processMid = true;
    bool processSide = false;
};

struct SidechainConflictProfile
{
    SidechainConflictMode mode = SidechainConflictMode::Off;
    std::array<SidechainBand, maxSidechainBands> bands {};
    uint8_t bandCount = 0;
    bool preserveLowEnd = true;
    bool preserveTransient = false;
};

struct SidechainDynamicEQConfig
{
    SidechainConflictMode mode = SidechainConflictMode::Off;
    bool enabled = false;
    float amount = 0.0f;
    float attackMs = 50.0f;
    float releaseMs = 250.0f;
    float customRangeStartHz = 2000.0f;
    float customRangeEndHz = 5000.0f;
};

class SidechainDynamicEQ
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;

    float process(
        juce::AudioBuffer<float>& buffer,
        const SidechainAnalysis& sidechain,
        const SidechainDynamicEQConfig& config) noexcept;

    static SidechainConflictProfile makeProfile(const SidechainDynamicEQConfig& config) noexcept;

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

    struct DynamicBand
    {
        BandPath midPath;
        BandPath sidePath;
        SidechainBand band;
        float startCoefficient = 0.0f;
        float endCoefficient = 0.0f;
        float currentReductionDb = 0.0f;

        void reset() noexcept;
        void configure(const SidechainBand& newBand, double sampleRate) noexcept;
        void smoothReduction(float targetReductionDb, float attackCoefficient, float releaseCoefficient) noexcept;
        float processMid(float input) noexcept;
        float processSide(float input) noexcept;
    };

    static float coefficientForCutoff(double sampleRate, float cutoffHz) noexcept;
    static float coefficientForMs(double sampleRate, float milliseconds) noexcept;
    static float targetReductionDb(const SidechainAnalysis& sidechain, const SidechainDynamicEQConfig& config, const SidechainBand& band) noexcept;

    double currentSampleRate = 44100.0;
    std::array<DynamicBand, maxSidechainBands> bands {};
    SidechainConflictProfile lastNonEmptyProfile {};
};
} // namespace stagemind
