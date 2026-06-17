#include "ResonanceLearner.h"

#include <algorithm>
#include <cmath>

namespace stagemind
{
namespace
{
constexpr double learnSeconds = 4.0;
constexpr double holdSeconds = 1.8;
} // namespace

void ResonanceLearner::prepare(double sampleRate) noexcept
{
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    holdSamples = std::max(1, static_cast<int> (safeSampleRate * holdSeconds));
    learnTotalSamples = std::max(1, static_cast<int> (safeSampleRate * learnSeconds));
    resetRuntime();
}

void ResonanceLearner::reset() noexcept
{
    status = Status::Idle;
    learnedReady = false;
    learnSamplesRemaining = 0;
    learnedSnapshot = {};
    buildSnapshot = {};
    buildRanks.fill(0.0f);
    heldPeaks.fill({});
    learnCandidates.fill({});
}

void ResonanceLearner::resetRuntime() noexcept
{
    const auto hadLearnedSnapshot = learnedReady;
    const auto savedSnapshot = learnedSnapshot;

    reset();

    if (hadLearnedSnapshot)
        setLearnedSnapshot(savedSnapshot);
}

void ResonanceLearner::beginLearn() noexcept
{
    status = Status::Learning;
    learnedReady = false;
    learnSamplesRemaining = learnTotalSamples;
    learnedSnapshot = {};
    learnCandidates.fill({});
}

void ResonanceLearner::setLearnedSnapshot(const ResonanceSnapshot& snapshot) noexcept
{
    learnedSnapshot = {};

    const auto count = std::min(static_cast<int> (snapshot.peakCount), static_cast<int> (maxResonancePeaks));
    for (int i = 0; i < count; ++i)
    {
        const auto& peak = snapshot.peaks[static_cast<size_t> (i)];
        if (peak.frequencyHz <= 0.0f || peak.suggestedReductionDb <= 0.0f)
            continue;

        auto& target = learnedSnapshot.peaks[static_cast<size_t> (learnedSnapshot.peakCount)];
        target.frequencyHz = std::clamp(peak.frequencyHz, 80.0f, 16000.0f);
        target.severity = clamp01(peak.severity);
        target.suggestedQ = std::clamp(peak.suggestedQ, 1.0f, 12.0f);
        target.suggestedReductionDb = std::clamp(peak.suggestedReductionDb, 0.0f, 7.0f);
        ++learnedSnapshot.peakCount;

        if (learnedSnapshot.peakCount >= maxResonancePeaks)
            break;
    }

    learnedReady = learnedSnapshot.peakCount > 0;
    status = learnedReady ? Status::Learned : Status::Idle;
    learnSamplesRemaining = 0;
}

ResonanceSnapshot ResonanceLearner::process(const ResonanceSnapshot& liveSnapshot, int numSamples) noexcept
{
    updateHeldSnapshot(liveSnapshot, std::max(0, numSamples));

    if (status == Status::Learning)
    {
        for (int i = 0; i < static_cast<int> (liveSnapshot.peakCount); ++i)
            addLearnPeak(liveSnapshot.peaks[static_cast<size_t> (i)]);

        learnSamplesRemaining -= std::max(0, numSamples);

        if (learnSamplesRemaining <= 0)
            finalizeLearn();
    }

    if (learnedReady)
        return buildRidingSnapshot(buildHeldSnapshot());

    return buildHeldSnapshot();
}

float ResonanceLearner::getProgress() const noexcept
{
    if (status == Status::Learning)
        return clamp01(1.0f - static_cast<float> (learnSamplesRemaining) / static_cast<float> (std::max(1, learnTotalSamples)));

    return learnedReady ? 1.0f : 0.0f;
}

bool ResonanceLearner::frequenciesMatch(float firstHz, float secondHz) noexcept
{
    if (firstHz <= 0.0f || secondHz <= 0.0f)
        return false;

    const auto toleranceHz = std::max(45.0f, std::max(firstHz, secondHz) * 0.045f);
    return std::abs(firstHz - secondHz) <= toleranceHz;
}

float ResonanceLearner::clamp01(float value) noexcept
{
    return std::max(0.0f, std::min(1.0f, value));
}

void ResonanceLearner::updateHeldSnapshot(const ResonanceSnapshot& liveSnapshot, int numSamples) noexcept
{
    for (auto& slot : heldPeaks)
    {
        if (slot.samplesRemaining > 0)
            slot.samplesRemaining = std::max(0, slot.samplesRemaining - numSamples);
    }

    for (int i = 0; i < static_cast<int> (liveSnapshot.peakCount); ++i)
        mergeHeldPeak(liveSnapshot.peaks[static_cast<size_t> (i)]);
}

void ResonanceLearner::mergeHeldPeak(const ResonancePeak& peak) noexcept
{
    if (peak.frequencyHz <= 0.0f)
        return;

    for (auto& slot : heldPeaks)
    {
        if (slot.samplesRemaining > 0 && frequenciesMatch(slot.peak.frequencyHz, peak.frequencyHz))
        {
            slot.peak.frequencyHz = slot.peak.frequencyHz * 0.75f + peak.frequencyHz * 0.25f;
            slot.peak.severity = std::max(slot.peak.severity * 0.92f, peak.severity);
            slot.peak.suggestedQ = slot.peak.suggestedQ * 0.75f + peak.suggestedQ * 0.25f;
            slot.peak.suggestedReductionDb = std::max(slot.peak.suggestedReductionDb * 0.9f, peak.suggestedReductionDb);
            slot.samplesRemaining = holdSamples;
            return;
        }
    }

    auto* target = &heldPeaks.front();

    for (auto& slot : heldPeaks)
    {
        if (slot.samplesRemaining <= 0)
        {
            target = &slot;
            break;
        }

        if (slot.peak.severity < target->peak.severity)
            target = &slot;
    }

    target->peak = peak;
    target->samplesRemaining = holdSamples;
}

ResonanceSnapshot ResonanceLearner::buildHeldSnapshot() const noexcept
{
    ResonanceSnapshot snapshot;

    for (const auto& slot : heldPeaks)
    {
        if (slot.samplesRemaining <= 0 || slot.peak.frequencyHz <= 0.0f)
            continue;

        snapshot.peaks[static_cast<size_t> (snapshot.peakCount)] = slot.peak;
        ++snapshot.peakCount;

        if (snapshot.peakCount >= maxResonancePeaks)
            break;
    }

    return snapshot;
}

ResonanceSnapshot ResonanceLearner::buildRidingSnapshot(const ResonanceSnapshot& heldSnapshot) const noexcept
{
    ResonanceSnapshot snapshot;

    for (int learnedIndex = 0; learnedIndex < static_cast<int> (learnedSnapshot.peakCount); ++learnedIndex)
    {
        auto peak = learnedSnapshot.peaks[static_cast<size_t> (learnedIndex)];
        auto rideAmount = 0.0f;

        for (int heldIndex = 0; heldIndex < static_cast<int> (heldSnapshot.peakCount); ++heldIndex)
        {
            const auto& heldPeak = heldSnapshot.peaks[static_cast<size_t> (heldIndex)];
            if (! frequenciesMatch(peak.frequencyHz, heldPeak.frequencyHz))
                continue;

            rideAmount = std::max(rideAmount, clamp01(heldPeak.severity * 1.25f));
            peak.frequencyHz = peak.frequencyHz * 0.85f + heldPeak.frequencyHz * 0.15f;
            peak.suggestedQ = peak.suggestedQ * 0.85f + heldPeak.suggestedQ * 0.15f;
        }

        peak.severity = rideAmount > 0.0f ? std::max(peak.severity * 0.35f, rideAmount) : peak.severity * 0.18f;
        peak.suggestedReductionDb *= rideAmount;

        snapshot.peaks[static_cast<size_t> (snapshot.peakCount)] = peak;
        ++snapshot.peakCount;

        if (snapshot.peakCount >= maxResonancePeaks)
            break;
    }

    return snapshot;
}

void ResonanceLearner::addLearnPeak(const ResonancePeak& peak) noexcept
{
    if (peak.frequencyHz <= 0.0f)
        return;

    for (auto& candidate : learnCandidates)
    {
        if (candidate.hits > 0 && frequenciesMatch(candidate.peak.frequencyHz, peak.frequencyHz))
        {
            const auto oldHits = static_cast<float> (candidate.hits);
            const auto newHits = oldHits + 1.0f;
            candidate.peak.frequencyHz = (candidate.peak.frequencyHz * oldHits + peak.frequencyHz) / newHits;
            candidate.peak.severity = std::max(candidate.peak.severity * 0.96f, peak.severity);
            candidate.peak.suggestedQ = (candidate.peak.suggestedQ * oldHits + peak.suggestedQ) / newHits;
            candidate.peak.suggestedReductionDb = std::max(candidate.peak.suggestedReductionDb * 0.96f, peak.suggestedReductionDb);
            candidate.score += std::max(0.05f, peak.severity);
            ++candidate.hits;
            return;
        }
    }

    auto* target = &learnCandidates.front();

    for (auto& candidate : learnCandidates)
    {
        if (candidate.hits == 0)
        {
            target = &candidate;
            break;
        }

        if (candidate.score < target->score)
            target = &candidate;
    }

    target->peak = peak;
    target->score = std::max(0.05f, peak.severity);
    target->hits = 1;
}

void ResonanceLearner::finalizeLearn() noexcept
{
    learnedSnapshot = {};
    buildSnapshot = {};
    buildRanks.fill(0.0f);

    for (const auto& candidate : learnCandidates)
    {
        if (candidate.hits <= 0)
            continue;

        auto peak = candidate.peak;
        peak.severity = clamp01(candidate.score / static_cast<float> (candidate.hits));
        peak.suggestedReductionDb = std::max(peak.suggestedReductionDb, peak.severity * 2.0f);
        insertLearnedPeak(peak, candidate.score);
    }

    learnedSnapshot = buildSnapshot;
    learnedReady = learnedSnapshot.peakCount > 0;
    status = learnedReady ? Status::Learned : Status::Idle;
    learnSamplesRemaining = 0;
}

void ResonanceLearner::insertLearnedPeak(ResonancePeak peak, float rank) noexcept
{
    auto insertIndex = static_cast<int> (maxResonancePeaks);

    for (int i = 0; i < static_cast<int> (maxResonancePeaks); ++i)
    {
        if (rank > buildRanks[static_cast<size_t> (i)])
        {
            insertIndex = i;
            break;
        }
    }

    if (insertIndex >= static_cast<int> (maxResonancePeaks))
        return;

    for (int i = static_cast<int> (maxResonancePeaks) - 1; i > insertIndex; --i)
    {
        buildSnapshot.peaks[static_cast<size_t> (i)] = buildSnapshot.peaks[static_cast<size_t> (i - 1)];
        buildRanks[static_cast<size_t> (i)] = buildRanks[static_cast<size_t> (i - 1)];
    }

    buildSnapshot.peaks[static_cast<size_t> (insertIndex)] = peak;
    buildRanks[static_cast<size_t> (insertIndex)] = rank;
    buildSnapshot.peakCount = static_cast<uint8_t> (
        std::min(static_cast<int> (maxResonancePeaks), static_cast<int> (buildSnapshot.peakCount) + 1));
}
} // namespace stagemind
