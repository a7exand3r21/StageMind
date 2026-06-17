#include "PluginState.h"

namespace stagemind
{
namespace
{
constexpr auto resonancePeakCountProperty = "learned_resonance_count";
constexpr auto resonanceFrequencyPrefix = "learned_resonance_frequency_";
constexpr auto resonanceSeverityPrefix = "learned_resonance_severity_";
constexpr auto resonanceQPrefix = "learned_resonance_q_";
constexpr auto resonanceReductionPrefix = "learned_resonance_reduction_";
constexpr auto rideMemoryLearningProperty = "ride_memory_learning";
constexpr auto rideMemoryCountProperty = "ride_memory_count";
constexpr auto rideMemoryGroupPrefix = "ride_memory_group_";
constexpr auto rideMemoryTargetRolePrefix = "ride_memory_target_role_";
constexpr auto rideMemorySourceRolePrefix = "ride_memory_source_role_";
constexpr auto rideMemoryActionPrefix = "ride_memory_action_";
constexpr auto rideMemoryBandPrefix = "ride_memory_band_";
constexpr auto rideMemorySeverityPrefix = "ride_memory_severity_";
constexpr auto rideMemoryHitsPrefix = "ride_memory_hits_";
constexpr auto rideMemoryResolvedPrefix = "ride_memory_resolved_";

juce::Identifier indexedProperty(const char* prefix, int index)
{
    return juce::Identifier(juce::String(prefix) + juce::String(index));
}

void writeLearnedResonances(juce::ValueTree& state, const ResonanceSnapshot& snapshot)
{
    const auto count = juce::jlimit(0, static_cast<int> (maxResonancePeaks), static_cast<int> (snapshot.peakCount));
    state.setProperty(resonancePeakCountProperty, count, nullptr);

    for (int index = 0; index < static_cast<int> (maxResonancePeaks); ++index)
    {
        const auto& peak = snapshot.peaks[static_cast<size_t> (index)];
        const auto isActive = index < count && peak.frequencyHz > 0.0f && peak.suggestedReductionDb > 0.0f;
        state.setProperty(indexedProperty(resonanceFrequencyPrefix, index), isActive ? peak.frequencyHz : 0.0f, nullptr);
        state.setProperty(indexedProperty(resonanceSeverityPrefix, index), isActive ? peak.severity : 0.0f, nullptr);
        state.setProperty(indexedProperty(resonanceQPrefix, index), isActive ? peak.suggestedQ : 1.0f, nullptr);
        state.setProperty(indexedProperty(resonanceReductionPrefix, index), isActive ? peak.suggestedReductionDb : 0.0f, nullptr);
    }
}

ResonanceSnapshot readLearnedResonances(const juce::ValueTree& state)
{
    ResonanceSnapshot snapshot;
    const auto count = juce::jlimit(
        0,
        static_cast<int> (maxResonancePeaks),
        static_cast<int> (state.getProperty(resonancePeakCountProperty, 0)));

    for (int index = 0; index < count; ++index)
    {
        ResonancePeak peak;
        peak.frequencyHz = static_cast<float> (state.getProperty(indexedProperty(resonanceFrequencyPrefix, index), 0.0f));
        peak.severity = static_cast<float> (state.getProperty(indexedProperty(resonanceSeverityPrefix, index), 0.0f));
        peak.suggestedQ = static_cast<float> (state.getProperty(indexedProperty(resonanceQPrefix, index), 1.0f));
        peak.suggestedReductionDb = static_cast<float> (state.getProperty(indexedProperty(resonanceReductionPrefix, index), 0.0f));

        if (peak.frequencyHz <= 0.0f || peak.suggestedReductionDb <= 0.0f)
            continue;

        snapshot.peaks[static_cast<size_t> (snapshot.peakCount)] = peak;
        ++snapshot.peakCount;
    }

    return snapshot;
}

void writeRideMemory(juce::ValueTree& state, const RideMemorySnapshot& snapshot)
{
    state.setProperty(rideMemoryLearningProperty, snapshot.learning, nullptr);

    auto storedCount = 0;
    for (const auto& event : snapshot.events)
    {
        if (! event.used || storedCount >= maxRideMemoryEvents)
            continue;

        state.setProperty(indexedProperty(rideMemoryGroupPrefix, storedCount), event.group, nullptr);
        state.setProperty(indexedProperty(rideMemoryTargetRolePrefix, storedCount), event.targetRole, nullptr);
        state.setProperty(indexedProperty(rideMemorySourceRolePrefix, storedCount), event.sourceRole, nullptr);
        state.setProperty(indexedProperty(rideMemoryActionPrefix, storedCount), event.actionKind, nullptr);
        state.setProperty(indexedProperty(rideMemoryBandPrefix, storedCount), event.band, nullptr);
        state.setProperty(indexedProperty(rideMemorySeverityPrefix, storedCount), event.severity, nullptr);
        state.setProperty(indexedProperty(rideMemoryHitsPrefix, storedCount), event.hits, nullptr);
        state.setProperty(indexedProperty(rideMemoryResolvedPrefix, storedCount), event.resolved, nullptr);
        ++storedCount;
    }

    state.setProperty(rideMemoryCountProperty, storedCount, nullptr);

    for (int index = storedCount; index < maxRideMemoryEvents; ++index)
    {
        state.setProperty(indexedProperty(rideMemoryGroupPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideMemoryTargetRolePrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideMemorySourceRolePrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideMemoryActionPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideMemoryBandPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideMemorySeverityPrefix, index), 0.0f, nullptr);
        state.setProperty(indexedProperty(rideMemoryHitsPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideMemoryResolvedPrefix, index), false, nullptr);
    }
}

RideMemorySnapshot readRideMemory(const juce::ValueTree& state)
{
    RideMemorySnapshot snapshot;
    snapshot.learning = static_cast<bool> (state.getProperty(rideMemoryLearningProperty, false));
    const auto count = juce::jlimit(
        0,
        maxRideMemoryEvents,
        static_cast<int> (state.getProperty(rideMemoryCountProperty, 0)));

    for (int index = 0; index < count; ++index)
    {
        RideMemoryEvent event;
        event.used = true;
        event.group = static_cast<int> (state.getProperty(indexedProperty(rideMemoryGroupPrefix, index), 0));
        event.targetRole = static_cast<int> (state.getProperty(indexedProperty(rideMemoryTargetRolePrefix, index), 0));
        event.sourceRole = static_cast<int> (state.getProperty(indexedProperty(rideMemorySourceRolePrefix, index), 0));
        event.actionKind = static_cast<int> (state.getProperty(indexedProperty(rideMemoryActionPrefix, index), 0));
        event.band = static_cast<int> (state.getProperty(indexedProperty(rideMemoryBandPrefix, index), 0));
        event.severity = static_cast<float> (state.getProperty(indexedProperty(rideMemorySeverityPrefix, index), 0.0f));
        event.hits = static_cast<int> (state.getProperty(indexedProperty(rideMemoryHitsPrefix, index), 1));
        event.resolved = static_cast<bool> (state.getProperty(indexedProperty(rideMemoryResolvedPrefix, index), false));

        snapshot.events[static_cast<size_t> (snapshot.count)] = event;
        ++snapshot.count;
    }

    return snapshot;
}
} // namespace

void PluginState::writeToBlock(
    juce::AudioProcessorValueTreeState& apvts,
    const ResonanceSnapshot& learnedResonances,
    const RideMemorySnapshot& rideMemory,
    juce::MemoryBlock& destination)
{
    auto state = apvts.copyState();
    state.setProperty("state_version", stateVersion, nullptr);
    state.setProperty("plugin_version", JucePlugin_VersionString, nullptr);
    writeLearnedResonances(state, learnedResonances);
    writeRideMemory(state, rideMemory);

    juce::MemoryOutputStream stream { destination, true };
    state.writeToStream(stream);
}

RestoredPluginState PluginState::restoreFromData(
    juce::AudioProcessorValueTreeState& apvts,
    const void* data,
    int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return {};

    auto state = juce::ValueTree::readFromData(data, static_cast<size_t> (sizeInBytes));
    auto restored = RestoredPluginState {};
    if (state.isValid())
    {
        restored.learnedResonances = readLearnedResonances(state);
        restored.rideMemory = readRideMemory(state);
    }

    if (state.isValid())
        apvts.replaceState(state);

    return restored;
}
} // namespace stagemind
