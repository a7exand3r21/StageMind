#pragma once

#include <JuceHeader.h>

namespace stagemind
{
enum class TriggerMode
{
    Off = 0,
    Self,
    ExternalSidechain,
    StageMindLink
};

inline juce::StringArray makeTriggerModeNames()
{
    return { "Off", "Self", "External Sidechain", "StageMind Link" };
}

inline TriggerMode triggerModeFromIndex(int index) noexcept
{
    switch (index)
    {
        case 1: return TriggerMode::Self;
        case 2: return TriggerMode::ExternalSidechain;
        case 3: return TriggerMode::StageMindLink;
        case 0:
        default: return TriggerMode::Off;
    }
}
} // namespace stagemind
