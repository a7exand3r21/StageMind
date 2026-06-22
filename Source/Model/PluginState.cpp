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
constexpr auto rideTimelineLearningProperty = "ride_timeline_learning";
constexpr auto rideTimelineCountProperty = "ride_timeline_count";
constexpr auto rideTimelineGroupPrefix = "ride_timeline_group_";
constexpr auto rideTimelineTargetRolePrefix = "ride_timeline_target_role_";
constexpr auto rideTimelineSourceRolePrefix = "ride_timeline_source_role_";
constexpr auto rideTimelineActionPrefix = "ride_timeline_action_";
constexpr auto rideTimelineBandPrefix = "ride_timeline_band_";
constexpr auto rideTimelineStartPrefix = "ride_timeline_start_ppq_";
constexpr auto rideTimelineEndPrefix = "ride_timeline_end_ppq_";
constexpr auto rideTimelineLastSeenPrefix = "ride_timeline_last_seen_ppq_";
constexpr auto rideTimelineSeverityPrefix = "ride_timeline_severity_";
constexpr auto rideTimelineHitsPrefix = "ride_timeline_hits_";
constexpr auto rideTimelineResolvedPrefix = "ride_timeline_resolved_";
constexpr auto balanceTimelineLearningProperty = "balance_timeline_learning";
constexpr auto balanceTimelineCountProperty = "balance_timeline_count";
constexpr auto balanceTimelineGroupPrefix = "balance_timeline_group_";
constexpr auto balanceTimelineTargetRolePrefix = "balance_timeline_target_role_";
constexpr auto balanceTimelineSectionIndexPrefix = "balance_timeline_section_index_";
constexpr auto balanceTimelineSectionKindPrefix = "balance_timeline_section_kind_";
constexpr auto balanceTimelineSectionStartPrefix = "balance_timeline_section_start_ppq_";
constexpr auto balanceTimelineSectionEndPrefix = "balance_timeline_section_end_ppq_";
constexpr auto balanceTimelineStartPrefix = "balance_timeline_start_ppq_";
constexpr auto balanceTimelineEndPrefix = "balance_timeline_end_ppq_";
constexpr auto balanceTimelineLastSeenPrefix = "balance_timeline_last_seen_ppq_";
constexpr auto balanceTimelineCorrectionPrefix = "balance_timeline_correction_db_";
constexpr auto balanceTimelineSeverityPrefix = "balance_timeline_severity_";
constexpr auto balanceTimelineHitsPrefix = "balance_timeline_hits_";
constexpr auto balanceTimelineResolvedPrefix = "balance_timeline_resolved_";
constexpr auto levelRiderTargetRmsProperty = "level_rider_target_rms";
constexpr auto levelRiderHeldGainDbProperty = "level_rider_held_gain_db";
constexpr auto levelRiderHasHeldGainProperty = "level_rider_has_held_gain";

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

void writeRideTimelineMemory(juce::ValueTree& state, const RideTimelineSnapshot& snapshot)
{
    state.setProperty(rideTimelineLearningProperty, snapshot.learning, nullptr);

    auto storedCount = 0;
    for (const auto& event : snapshot.events)
    {
        if (! event.used || storedCount >= maxRideTimelineEvents)
            continue;

        state.setProperty(indexedProperty(rideTimelineGroupPrefix, storedCount), event.group, nullptr);
        state.setProperty(indexedProperty(rideTimelineTargetRolePrefix, storedCount), event.targetRole, nullptr);
        state.setProperty(indexedProperty(rideTimelineSourceRolePrefix, storedCount), event.sourceRole, nullptr);
        state.setProperty(indexedProperty(rideTimelineActionPrefix, storedCount), event.actionKind, nullptr);
        state.setProperty(indexedProperty(rideTimelineBandPrefix, storedCount), event.band, nullptr);
        state.setProperty(indexedProperty(rideTimelineStartPrefix, storedCount), event.startPpq, nullptr);
        state.setProperty(indexedProperty(rideTimelineEndPrefix, storedCount), event.endPpq, nullptr);
        state.setProperty(indexedProperty(rideTimelineLastSeenPrefix, storedCount), event.lastSeenPpq, nullptr);
        state.setProperty(indexedProperty(rideTimelineSeverityPrefix, storedCount), event.severity, nullptr);
        state.setProperty(indexedProperty(rideTimelineHitsPrefix, storedCount), event.hits, nullptr);
        state.setProperty(indexedProperty(rideTimelineResolvedPrefix, storedCount), event.resolved, nullptr);
        ++storedCount;
    }

    state.setProperty(rideTimelineCountProperty, storedCount, nullptr);

    for (int index = storedCount; index < maxRideTimelineEvents; ++index)
    {
        state.setProperty(indexedProperty(rideTimelineGroupPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideTimelineTargetRolePrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideTimelineSourceRolePrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideTimelineActionPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideTimelineBandPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideTimelineStartPrefix, index), 0.0, nullptr);
        state.setProperty(indexedProperty(rideTimelineEndPrefix, index), 0.0, nullptr);
        state.setProperty(indexedProperty(rideTimelineLastSeenPrefix, index), 0.0, nullptr);
        state.setProperty(indexedProperty(rideTimelineSeverityPrefix, index), 0.0f, nullptr);
        state.setProperty(indexedProperty(rideTimelineHitsPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(rideTimelineResolvedPrefix, index), false, nullptr);
    }
}

RideTimelineSnapshot readRideTimelineMemory(const juce::ValueTree& state)
{
    RideTimelineSnapshot snapshot;
    snapshot.learning = static_cast<bool> (state.getProperty(rideTimelineLearningProperty, false));
    const auto count = juce::jlimit(
        0,
        maxRideTimelineEvents,
        static_cast<int> (state.getProperty(rideTimelineCountProperty, 0)));

    for (int index = 0; index < count; ++index)
    {
        RideTimelineEvent event;
        event.used = true;
        event.group = static_cast<int> (state.getProperty(indexedProperty(rideTimelineGroupPrefix, index), 0));
        event.targetRole = static_cast<int> (state.getProperty(indexedProperty(rideTimelineTargetRolePrefix, index), 0));
        event.sourceRole = static_cast<int> (state.getProperty(indexedProperty(rideTimelineSourceRolePrefix, index), 0));
        event.actionKind = static_cast<int> (state.getProperty(indexedProperty(rideTimelineActionPrefix, index), 0));
        event.band = static_cast<int> (state.getProperty(indexedProperty(rideTimelineBandPrefix, index), 0));
        event.startPpq = static_cast<double> (state.getProperty(indexedProperty(rideTimelineStartPrefix, index), 0.0));
        event.endPpq = static_cast<double> (state.getProperty(indexedProperty(rideTimelineEndPrefix, index), 0.0));
        event.lastSeenPpq = static_cast<double> (state.getProperty(indexedProperty(rideTimelineLastSeenPrefix, index), event.endPpq));
        event.severity = static_cast<float> (state.getProperty(indexedProperty(rideTimelineSeverityPrefix, index), 0.0f));
        event.hits = static_cast<int> (state.getProperty(indexedProperty(rideTimelineHitsPrefix, index), 1));
        event.resolved = static_cast<bool> (state.getProperty(indexedProperty(rideTimelineResolvedPrefix, index), false));

        snapshot.events[static_cast<size_t> (snapshot.count)] = event;
        ++snapshot.count;
    }

    return snapshot;
}

void writeBalanceTimelineMemory(juce::ValueTree& state, const BalanceTimelineSnapshot& snapshot)
{
    state.setProperty(balanceTimelineLearningProperty, snapshot.learning, nullptr);

    auto storedCount = 0;
    for (const auto& event : snapshot.events)
    {
        if (! event.used || storedCount >= maxBalanceTimelineEvents)
            continue;

        state.setProperty(indexedProperty(balanceTimelineGroupPrefix, storedCount), event.group, nullptr);
        state.setProperty(indexedProperty(balanceTimelineTargetRolePrefix, storedCount), event.targetRole, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionIndexPrefix, storedCount), event.sectionIndex, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionKindPrefix, storedCount), event.sectionKind, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionStartPrefix, storedCount), event.sectionStartPpq, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionEndPrefix, storedCount), event.sectionEndPpq, nullptr);
        state.setProperty(indexedProperty(balanceTimelineStartPrefix, storedCount), event.startPpq, nullptr);
        state.setProperty(indexedProperty(balanceTimelineEndPrefix, storedCount), event.endPpq, nullptr);
        state.setProperty(indexedProperty(balanceTimelineLastSeenPrefix, storedCount), event.lastSeenPpq, nullptr);
        state.setProperty(indexedProperty(balanceTimelineCorrectionPrefix, storedCount), event.correctionDb, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSeverityPrefix, storedCount), event.severity, nullptr);
        state.setProperty(indexedProperty(balanceTimelineHitsPrefix, storedCount), event.hits, nullptr);
        state.setProperty(indexedProperty(balanceTimelineResolvedPrefix, storedCount), event.resolved, nullptr);
        ++storedCount;
    }

    state.setProperty(balanceTimelineCountProperty, storedCount, nullptr);

    for (int index = storedCount; index < maxBalanceTimelineEvents; ++index)
    {
        state.setProperty(indexedProperty(balanceTimelineGroupPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineTargetRolePrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionIndexPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionKindPrefix, index), static_cast<int> (BalanceTimelineSectionKind::Intro), nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionStartPrefix, index), 0.0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSectionEndPrefix, index), balanceTimelineSectionLengthPpq, nullptr);
        state.setProperty(indexedProperty(balanceTimelineStartPrefix, index), 0.0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineEndPrefix, index), 0.0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineLastSeenPrefix, index), 0.0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineCorrectionPrefix, index), 0.0f, nullptr);
        state.setProperty(indexedProperty(balanceTimelineSeverityPrefix, index), 0.0f, nullptr);
        state.setProperty(indexedProperty(balanceTimelineHitsPrefix, index), 0, nullptr);
        state.setProperty(indexedProperty(balanceTimelineResolvedPrefix, index), false, nullptr);
    }
}

BalanceTimelineSnapshot readBalanceTimelineMemory(const juce::ValueTree& state)
{
    BalanceTimelineSnapshot snapshot;
    snapshot.learning = static_cast<bool> (state.getProperty(balanceTimelineLearningProperty, false));
    const auto count = juce::jlimit(
        0,
        maxBalanceTimelineEvents,
        static_cast<int> (state.getProperty(balanceTimelineCountProperty, 0)));

    for (int index = 0; index < count; ++index)
    {
        BalanceTimelineEvent event;
        event.used = true;
        event.group = static_cast<int> (state.getProperty(indexedProperty(balanceTimelineGroupPrefix, index), 0));
        event.targetRole = static_cast<int> (state.getProperty(indexedProperty(balanceTimelineTargetRolePrefix, index), 0));
        event.startPpq = static_cast<double> (state.getProperty(indexedProperty(balanceTimelineStartPrefix, index), 0.0));
        event.endPpq = static_cast<double> (state.getProperty(indexedProperty(balanceTimelineEndPrefix, index), 0.0));
        event.lastSeenPpq = static_cast<double> (state.getProperty(indexedProperty(balanceTimelineLastSeenPrefix, index), event.endPpq));
        const auto inferredSectionIndex = balanceTimelineSectionIndexFor(event.lastSeenPpq);
        event.sectionIndex = static_cast<int> (state.getProperty(indexedProperty(balanceTimelineSectionIndexPrefix, index), inferredSectionIndex));
        event.sectionKind = static_cast<int> (state.getProperty(
            indexedProperty(balanceTimelineSectionKindPrefix, index),
            static_cast<int> (balanceTimelineSectionKindForIndex(event.sectionIndex))));
        event.sectionStartPpq = static_cast<double> (state.getProperty(
            indexedProperty(balanceTimelineSectionStartPrefix, index),
            static_cast<double> (event.sectionIndex) * balanceTimelineSectionLengthPpq));
        event.sectionEndPpq = static_cast<double> (state.getProperty(
            indexedProperty(balanceTimelineSectionEndPrefix, index),
            event.sectionStartPpq + balanceTimelineSectionLengthPpq));
        event.correctionDb = static_cast<float> (state.getProperty(indexedProperty(balanceTimelineCorrectionPrefix, index), 0.0f));
        event.severity = static_cast<float> (state.getProperty(indexedProperty(balanceTimelineSeverityPrefix, index), 0.0f));
        event.hits = static_cast<int> (state.getProperty(indexedProperty(balanceTimelineHitsPrefix, index), 1));
        event.resolved = static_cast<bool> (state.getProperty(indexedProperty(balanceTimelineResolvedPrefix, index), false));

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
    const RideTimelineSnapshot& rideTimelineMemory,
    const BalanceTimelineSnapshot& balanceTimelineMemory,
    float levelRiderTargetRms,
    float levelRiderHeldGainDb,
    bool levelRiderHasHeldGain,
    juce::MemoryBlock& destination)
{
    auto state = apvts.copyState();
    state.setProperty("state_version", stateVersion, nullptr);
    state.setProperty("plugin_version", JucePlugin_VersionString, nullptr);
    writeLearnedResonances(state, learnedResonances);
    writeRideMemory(state, rideMemory);
    writeRideTimelineMemory(state, rideTimelineMemory);
    writeBalanceTimelineMemory(state, balanceTimelineMemory);
    state.setProperty(levelRiderTargetRmsProperty, juce::jlimit(0.0f, 0.6f, levelRiderTargetRms), nullptr);
    state.setProperty(levelRiderHeldGainDbProperty, juce::jlimit(-36.0f, 24.0f, levelRiderHeldGainDb), nullptr);
    state.setProperty(levelRiderHasHeldGainProperty, levelRiderHasHeldGain, nullptr);

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
        restored.rideTimelineMemory = readRideTimelineMemory(state);
        restored.balanceTimelineMemory = readBalanceTimelineMemory(state);
        restored.levelRiderTargetRms = static_cast<float> (state.getProperty(levelRiderTargetRmsProperty, 0.0f));
        restored.levelRiderHeldGainDb = static_cast<float> (state.getProperty(levelRiderHeldGainDbProperty, 0.0f));
        restored.levelRiderHasHeldGain = static_cast<bool> (state.getProperty(levelRiderHasHeldGainProperty, false));
    }

    if (state.isValid())
        apvts.replaceState(state);

    return restored;
}
} // namespace stagemind
