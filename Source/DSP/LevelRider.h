#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace stagemind
{
enum class LevelRiderMode
{
    Off = 0,
    LegacyAuto,
    Static,
    Ride
};

struct LevelRiderConfig
{
    LevelRiderMode mode = LevelRiderMode::Off;
    bool enabled = false;
    bool analyzeRequested = false;
    float activeThresholdRms = 0.004f;
    float fixedTargetRms = 0.125f;
    float targetDisplayDb = -18.0f;
    float ceilingDb = -1.0f;
    float response = 0.65f;
    float targetFloorRms = 0.018f;
    float targetCeilingRms = 0.45f;
    float maxBoostDb = 4.0f;
    float maxCutDb = 5.0f;
    float learnSeconds = 18.0f;
};

struct LevelRiderStatus
{
    bool active = false;
    bool held = false;
    bool analyzing = false;
    int modeIndex = 0;
    float targetRms = 0.0f;
    float targetDb = -120.0f;
    float gainDb = 0.0f;
    float outputPeakDb = -120.0f;
};

class LevelRider
{
public:
    static int getLookAheadSamplesForSampleRate(double sampleRate) noexcept;

    void prepare(double sampleRate, int maximumBlockSize);
    void reset() noexcept;
    void setTargetRms(float newTargetRms) noexcept;
    float getTargetRms() const noexcept { return targetRms; }
    void setHeldGainDb(float newHeldGainDb) noexcept;
    float getHeldGainDb() const noexcept { return heldGainDb; }
    bool hasHeldGain() const noexcept { return staticGainReady; }
    void clearHeldGain() noexcept;
    LevelRiderStatus process(
        juce::AudioBuffer<float>& buffer,
        float currentRms,
        float currentPeak,
        const LevelRiderConfig& config) noexcept;

private:
    static float responseToSeconds(float response) noexcept;
    void setResponseSeconds(float seconds) noexcept;
    float calculateTargetGainDb(float sourceRms, float sourcePeak, float desiredTargetRms, const LevelRiderConfig& config) const noexcept;
    void applyLookAheadLimiter(juce::AudioBuffer<float>& buffer, float ceilingDb, bool limiterEnabled) noexcept;
    void applyGain(juce::AudioBuffer<float>& buffer) noexcept;

    double currentSampleRate = 44100.0;
    int lookAheadSamples = 0;
    int lookAheadWriteIndex = 0;
    float targetRms = 0.0f;
    float heldGainDb = 0.0f;
    float currentResponseSeconds = 0.35f;
    float limiterGain = 1.0f;
    float limiterReleaseCoefficient = 0.02f;
    int limiterHoldSamples = 0;
    bool staticGainReady = false;
    bool staticAnalyzeArmed = false;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gain { 1.0f };
    juce::AudioBuffer<float> lookAheadBuffer;
};
} // namespace stagemind
