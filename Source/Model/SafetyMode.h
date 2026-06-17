#pragma once

#include <JuceHeader.h>

namespace stagemind
{
enum class SafetyMode
{
    MonoSafe = 0,
    Natural,
    ModernWide,
    HeadphonesWide
};

inline juce::StringArray makeSafetyModeNames()
{
    return { "Mono Safe", "Natural", "Modern Wide", "Headphones Wide" };
}

inline SafetyMode safetyModeFromIndex(int index) noexcept
{
    switch (index)
    {
        case 0: return SafetyMode::MonoSafe;
        case 2: return SafetyMode::ModernWide;
        case 3: return SafetyMode::HeadphonesWide;
        case 1:
        default: return SafetyMode::Natural;
    }
}
} // namespace stagemind
