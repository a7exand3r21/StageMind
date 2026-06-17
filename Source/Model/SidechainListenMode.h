#pragma once

#include <JuceHeader.h>

namespace stagemind
{
enum class SidechainListenMode
{
    Off = 0,
    SidechainOnly,
    MainOnly,
    Delta
};

inline juce::StringArray makeSidechainListenModeNames()
{
    return { "Off", "Sidechain Only", "Main Only", "Delta" };
}

inline SidechainListenMode sidechainListenModeFromIndex(int index) noexcept
{
    switch (index)
    {
        case 1: return SidechainListenMode::SidechainOnly;
        case 2: return SidechainListenMode::MainOnly;
        case 3: return SidechainListenMode::Delta;
        case 0:
        default: return SidechainListenMode::Off;
    }
}
} // namespace stagemind
