#pragma once

#include <juce_core/juce_core.h>

namespace stagemind
{
enum class StageGainMode
{
    Off = 0,
    Static,
    Ride
};

inline juce::StringArray makeStageGainModeNames()
{
    return { "Off", "Static", "Ride" };
}

inline StageGainMode stageGainModeFromIndex(int index) noexcept
{
    switch (index)
    {
        case 1:  return StageGainMode::Static;
        case 2:  return StageGainMode::Ride;
        case 0:
        default: return StageGainMode::Off;
    }
}
} // namespace stagemind
