#pragma once

#include "../DSP/ResonanceTypes.h"
#include "../Link/RideMemory.h"
#include <JuceHeader.h>

namespace stagemind
{
struct RestoredPluginState
{
    ResonanceSnapshot learnedResonances;
    RideMemorySnapshot rideMemory;
};

class PluginState
{
public:
    static constexpr int stateVersion = 3;

    static void writeToBlock(
        juce::AudioProcessorValueTreeState& apvts,
        const ResonanceSnapshot& learnedResonances,
        const RideMemorySnapshot& rideMemory,
        juce::MemoryBlock& destination);

    static RestoredPluginState restoreFromData(
        juce::AudioProcessorValueTreeState& apvts,
        const void* data,
        int sizeInBytes);
};
} // namespace stagemind
