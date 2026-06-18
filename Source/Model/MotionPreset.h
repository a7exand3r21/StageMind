#pragma once

#include <juce_core/juce_core.h>

namespace stagemind
{
enum class MotionPreset
{
    SlowDrift = 0,
    Orbit,
    Pulse,
    WideSweep
};

inline juce::StringArray makeMotionPresetNames()
{
    return {
        "Slow Drift",
        "Orbit",
        "Pulse",
        "Wide Sweep"
    };
}

inline MotionPreset motionPresetFromIndex(int index) noexcept
{
    switch (index)
    {
        case 1:  return MotionPreset::Orbit;
        case 2:  return MotionPreset::Pulse;
        case 3:  return MotionPreset::WideSweep;
        case 0:
        default: return MotionPreset::SlowDrift;
    }
}

inline float motionPresetEffectiveRateHz(MotionPreset preset, float rateTrimHz) noexcept
{
    const auto trim = juce::jlimit(0.01f, 8.0f, rateTrimHz);

    switch (preset)
    {
        case MotionPreset::Orbit:     return juce::jlimit(0.01f, 8.0f, 0.16f + trim * 0.65f);
        case MotionPreset::Pulse:     return juce::jlimit(0.01f, 8.0f, 0.75f + trim * 1.20f);
        case MotionPreset::WideSweep: return juce::jlimit(0.01f, 8.0f, 0.10f + trim * 0.50f);
        case MotionPreset::SlowDrift:
        default:                      return juce::jlimit(0.01f, 8.0f, 0.06f + trim * 0.25f);
    }
}
} // namespace stagemind
