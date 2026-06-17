#pragma once

#include <juce_core/juce_core.h>

namespace stagemind
{
enum class AutoAssistMode
{
    Off = 0,
    Suggest,
    Auto
};

inline juce::StringArray makeAutoAssistModeNames()
{
    return { "Off", "Suggest", "Auto" };
}

inline AutoAssistMode autoAssistModeFromIndex(int index) noexcept
{
    switch (index)
    {
        case 1:  return AutoAssistMode::Suggest;
        case 2:  return AutoAssistMode::Auto;
        case 0:
        default: return AutoAssistMode::Off;
    }
}
} // namespace stagemind
