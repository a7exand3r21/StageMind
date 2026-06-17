#include "LinkActivityEnvelope.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
namespace
{
constexpr auto attackSeconds = 0.035;
constexpr auto releaseSeconds = 0.550;
}

void LinkActivityEnvelope::prepare(double sampleRateToUse, int maxBlockSizeToUse) noexcept
{
    sampleRate = sampleRateToUse > 0.0 ? sampleRateToUse : 44100.0;
    maxBlockSize = std::max(1, maxBlockSizeToUse);
    reset();
}

void LinkActivityEnvelope::reset() noexcept
{
    currentValue = 0.0f;
}

float LinkActivityEnvelope::process(float rawActivity, int numSamples) noexcept
{
    const auto target = std::clamp(rawActivity, 0.0f, 1.0f);
    const auto safeNumSamples = std::clamp(numSamples, 1, maxBlockSize);
    const auto timeSeconds = target > currentValue ? attackSeconds : releaseSeconds;
    const auto coefficient = coefficientFor(sampleRate, safeNumSamples, timeSeconds);

    currentValue = target + coefficient * (currentValue - target);
    currentValue = std::clamp(currentValue, 0.0f, 1.0f);
    return currentValue;
}

float LinkActivityEnvelope::coefficientFor(double sampleRateToUse, int numSamples, double timeSeconds) noexcept
{
    if (sampleRateToUse <= 0.0 || numSamples <= 0 || timeSeconds <= 0.0)
        return 0.0f;

    const auto blockSeconds = static_cast<double> (numSamples) / sampleRateToUse;
    return static_cast<float> (std::exp(-blockSeconds / timeSeconds));
}
} // namespace stagemind

