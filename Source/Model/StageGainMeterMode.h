#pragma once

#include <juce_core/juce_core.h>

namespace stagemind
{
enum class StageGainMeterMode
{
    Dbfs = 0,
    Vu,
    Rms,
    LufsMomentary,
    LufsShortTerm,
    LufsIntegrated
};

inline juce::StringArray makeStageGainMeterModeNames()
{
    return { "dBFS", "VU", "RMS", "LUFS M", "LUFS S", "LUFS I" };
}

inline StageGainMeterMode stageGainMeterModeFromIndex(int index) noexcept
{
    switch (index)
    {
        case 1:  return StageGainMeterMode::Vu;
        case 2:  return StageGainMeterMode::Rms;
        case 3:  return StageGainMeterMode::LufsMomentary;
        case 4:  return StageGainMeterMode::LufsShortTerm;
        case 5:  return StageGainMeterMode::LufsIntegrated;
        case 0:
        default: return StageGainMeterMode::Dbfs;
    }
}

inline bool isLufsMode(StageGainMeterMode mode) noexcept
{
    return mode == StageGainMeterMode::LufsMomentary
        || mode == StageGainMeterMode::LufsShortTerm
        || mode == StageGainMeterMode::LufsIntegrated;
}
} // namespace stagemind
