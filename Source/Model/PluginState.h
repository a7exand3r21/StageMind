#pragma once

#include "../DSP/ResonanceTypes.h"
#include "../Link/BalanceTimelineMemory.h"
#include "../Link/RideMemory.h"
#include "../Link/RideTimelineMemory.h"
#include <JuceHeader.h>

namespace stagemind
{
struct RestoredPluginState
{
    ResonanceSnapshot learnedResonances;
    RideMemorySnapshot rideMemory;
    RideTimelineSnapshot rideTimelineMemory;
    BalanceTimelineSnapshot balanceTimelineMemory;
    float levelRiderTargetRms = 0.0f;
    float levelRiderHeldGainDb = 0.0f;
    bool levelRiderHasHeldGain = false;
};

class PluginState
{
public:
    static constexpr int stateVersion = 10;

    static void writeToBlock(
        juce::AudioProcessorValueTreeState& apvts,
        const ResonanceSnapshot& learnedResonances,
        const RideMemorySnapshot& rideMemory,
        const RideTimelineSnapshot& rideTimelineMemory,
        const BalanceTimelineSnapshot& balanceTimelineMemory,
        float levelRiderTargetRms,
        float levelRiderHeldGainDb,
        bool levelRiderHasHeldGain,
        juce::MemoryBlock& destination);

    static RestoredPluginState restoreFromData(
        juce::AudioProcessorValueTreeState& apvts,
        const void* data,
        int sizeInBytes);
};
} // namespace stagemind
