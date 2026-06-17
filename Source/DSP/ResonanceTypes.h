#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace stagemind
{
inline constexpr size_t maxResonancePeaks = 4;

struct ResonancePeak
{
    float frequencyHz = 0.0f;
    float severity = 0.0f;
    float suggestedQ = 1.0f;
    float suggestedReductionDb = 0.0f;
};

struct ResonanceSnapshot
{
    std::array<ResonancePeak, maxResonancePeaks> peaks {};
    uint8_t peakCount = 0;
};

struct AtomicResonanceSnapshot
{
    std::array<std::atomic<float>, maxResonancePeaks> frequencyHz {};
    std::array<std::atomic<float>, maxResonancePeaks> severity {};
    std::array<std::atomic<float>, maxResonancePeaks> reductionDb {};
    std::array<std::atomic<float>, maxResonancePeaks> q {};
    std::atomic<int> peakCount { 0 };
};
} // namespace stagemind
