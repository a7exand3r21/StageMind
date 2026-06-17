#pragma once

#include "ResonanceTypes.h"
#include <array>

namespace stagemind
{
class ResonanceLearner
{
public:
    enum class Status
    {
        Idle = 0,
        Learning,
        Learned
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void resetRuntime() noexcept;
    void beginLearn() noexcept;
    void setLearnedSnapshot(const ResonanceSnapshot& snapshot) noexcept;

    ResonanceSnapshot process(const ResonanceSnapshot& liveSnapshot, int numSamples) noexcept;

    Status getStatus() const noexcept { return status; }
    float getProgress() const noexcept;
    bool hasLearnedSnapshot() const noexcept { return learnedReady; }
    ResonanceSnapshot getLearnedSnapshot() const noexcept { return learnedSnapshot; }

private:
    static constexpr size_t learnCandidateCount = 8;

    struct HoldSlot
    {
        ResonancePeak peak;
        int samplesRemaining = 0;
    };

    struct LearnCandidate
    {
        ResonancePeak peak;
        float score = 0.0f;
        int hits = 0;
    };

    static bool frequenciesMatch(float firstHz, float secondHz) noexcept;
    static float clamp01(float value) noexcept;

    void updateHeldSnapshot(const ResonanceSnapshot& liveSnapshot, int numSamples) noexcept;
    void mergeHeldPeak(const ResonancePeak& peak) noexcept;
    ResonanceSnapshot buildHeldSnapshot() const noexcept;
    ResonanceSnapshot buildRidingSnapshot(const ResonanceSnapshot& heldSnapshot) const noexcept;

    void addLearnPeak(const ResonancePeak& peak) noexcept;
    void finalizeLearn() noexcept;
    void insertLearnedPeak(ResonancePeak peak, float rank) noexcept;

    int holdSamples = 1;
    int learnTotalSamples = 1;
    int learnSamplesRemaining = 0;

    Status status = Status::Idle;
    bool learnedReady = false;

    ResonanceSnapshot learnedSnapshot;
    ResonanceSnapshot buildSnapshot;
    std::array<float, maxResonancePeaks> buildRanks {};
    std::array<HoldSlot, maxResonancePeaks> heldPeaks {};
    std::array<LearnCandidate, learnCandidateCount> learnCandidates {};
};
} // namespace stagemind
