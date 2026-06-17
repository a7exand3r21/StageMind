#pragma once

#include <algorithm>

namespace stagemind
{
struct LinkSpectralBands
{
    float low = 0.0f;
    float lowMid = 0.0f;
    float presence = 0.0f;
    float air = 0.0f;
};

inline LinkSpectralBands clampedBands(LinkSpectralBands bands) noexcept
{
    bands.low = std::clamp(bands.low, 0.0f, 1.0f);
    bands.lowMid = std::clamp(bands.lowMid, 0.0f, 1.0f);
    bands.presence = std::clamp(bands.presence, 0.0f, 1.0f);
    bands.air = std::clamp(bands.air, 0.0f, 1.0f);
    return bands;
}
} // namespace stagemind
