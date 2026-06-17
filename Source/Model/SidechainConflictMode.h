#pragma once

#include <JuceHeader.h>

namespace stagemind
{
enum class SidechainConflictMode
{
    Off = 0,
    MakeSpace,
    VocalDucksInstrument,
    KickDucksBass,
    SnareDucksInstrument,
    LeadDucksPad,
    Custom
};

inline juce::StringArray makeSidechainConflictModeNames()
{
    return {
        "Off",
        "Make Space",
        "Vocal Ducks Instrument",
        "Kick Ducks Bass",
        "Snare Ducks Instrument",
        "Lead Ducks Pad",
        "Custom"
    };
}

inline SidechainConflictMode sidechainConflictModeFromIndex(int index) noexcept
{
    switch (index)
    {
        case 1: return SidechainConflictMode::MakeSpace;
        case 2: return SidechainConflictMode::VocalDucksInstrument;
        case 3: return SidechainConflictMode::KickDucksBass;
        case 4: return SidechainConflictMode::SnareDucksInstrument;
        case 5: return SidechainConflictMode::LeadDucksPad;
        case 6: return SidechainConflictMode::Custom;
        case 0:
        default: return SidechainConflictMode::Off;
    }
}
} // namespace stagemind
