#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace stagemind
{
inline constexpr int maxBalanceTimelineEvents = 64;
inline constexpr double balanceTimelineMergeWindowPpq = 1.0;
inline constexpr double balanceTimelineEventHalfWindowPpq = 0.35;
inline constexpr double balanceTimelineSectionLengthPpq = 64.0;

enum class BalanceTimelineSectionKind
{
    Intro = 1,
    Verse = 2,
    Chorus = 3,
    Drop = 4,
    Section = 5
};

inline int balanceTimelineSectionIndexFor(double ppqPosition) noexcept
{
    if (! std::isfinite(ppqPosition) || ppqPosition <= 0.0)
        return 0;

    return static_cast<int> (std::floor(ppqPosition / balanceTimelineSectionLengthPpq));
}

inline BalanceTimelineSectionKind balanceTimelineSectionKindForIndex(int sectionIndex) noexcept
{
    switch (sectionIndex)
    {
        case 0:  return BalanceTimelineSectionKind::Intro;
        case 1:  return BalanceTimelineSectionKind::Verse;
        case 2:  return BalanceTimelineSectionKind::Chorus;
        case 3:  return BalanceTimelineSectionKind::Drop;
        default: return BalanceTimelineSectionKind::Section;
    }
}

inline const char* balanceTimelineSectionName(BalanceTimelineSectionKind kind) noexcept
{
    switch (kind)
    {
        case BalanceTimelineSectionKind::Intro:   return "Intro";
        case BalanceTimelineSectionKind::Verse:   return "Verse";
        case BalanceTimelineSectionKind::Chorus:  return "Chorus";
        case BalanceTimelineSectionKind::Drop:    return "Drop";
        case BalanceTimelineSectionKind::Section:
        default:                                  return "Section";
    }
}

struct BalanceTimelineEvent
{
    bool used = false;
    int group = 0;
    int targetRole = 0;
    int sectionIndex = 0;
    int sectionKind = static_cast<int> (BalanceTimelineSectionKind::Intro);
    double sectionStartPpq = 0.0;
    double sectionEndPpq = balanceTimelineSectionLengthPpq;
    double startPpq = 0.0;
    double endPpq = 0.0;
    double lastSeenPpq = 0.0;
    float correctionDb = 0.0f;
    float severity = 0.0f;
    int hits = 0;
    bool resolved = false;

    bool matchesKey(int groupToMatch, int targetRoleToMatch, int sectionIndexToMatch) const noexcept
    {
        return used
            && group == groupToMatch
            && targetRole == targetRoleToMatch
            && sectionIndex == sectionIndexToMatch;
    }

    bool contains(double ppqPosition, double tolerancePpq = 0.0) const noexcept
    {
        return used
            && std::isfinite(ppqPosition)
            && ppqPosition + tolerancePpq >= startPpq
            && ppqPosition - tolerancePpq <= endPpq;
    }
};

struct BalanceTimelineSnapshot
{
    bool learning = false;
    int count = 0;
    std::array<BalanceTimelineEvent, maxBalanceTimelineEvents> events;
};

class BalanceTimelineMemory final
{
public:
    void clear() noexcept
    {
        learning = false;
        events = {};
    }

    void setLearning(bool shouldLearn) noexcept
    {
        learning = shouldLearn;
    }

    bool isLearning() const noexcept
    {
        return learning;
    }

    void observe(
        int group,
        int targetRole,
        double ppqPosition,
        float correctionDb,
        float severity,
        bool resolved) noexcept
    {
        if (! isValidEventKey(group, targetRole) || ! isValidPpq(ppqPosition) || ! std::isfinite(correctionDb))
            return;

        correctionDb = std::clamp(correctionDb, -1.2f, 1.2f);
        severity = std::clamp(severity, 0.0f, 1.0f);
        const auto sectionIndex = balanceTimelineSectionIndexFor(ppqPosition);

        if (auto* event = findNearby(group, targetRole, ppqPosition))
        {
            extendEvent(*event, ppqPosition);
            event->correctionDb = event->hits > 0
                ? event->correctionDb * 0.72f + correctionDb * 0.28f
                : correctionDb;
            event->severity = std::max(event->severity, severity);
            event->hits = std::min(event->hits + 1, maxHits);
            event->resolved = resolved;
            return;
        }

        auto* slot = findWriteSlot();
        if (slot == nullptr)
            return;

        *slot = {};
        slot->used = true;
        slot->group = group;
        slot->targetRole = targetRole;
        slot->sectionIndex = sectionIndex;
        slot->sectionKind = static_cast<int> (balanceTimelineSectionKindForIndex(sectionIndex));
        slot->sectionStartPpq = static_cast<double> (sectionIndex) * balanceTimelineSectionLengthPpq;
        slot->sectionEndPpq = slot->sectionStartPpq + balanceTimelineSectionLengthPpq;
        slot->startPpq = std::max(0.0, ppqPosition - balanceTimelineEventHalfWindowPpq);
        slot->endPpq = ppqPosition + balanceTimelineEventHalfWindowPpq;
        slot->lastSeenPpq = ppqPosition;
        slot->correctionDb = correctionDb;
        slot->severity = severity;
        slot->hits = 1;
        slot->resolved = resolved;
    }

    void markResolved(int group, int targetRole, double ppqPosition) noexcept
    {
        if (auto* event = findNearby(group, targetRole, ppqPosition))
            event->resolved = true;
    }

    void markNearbyResolved(int group, double ppqPosition) noexcept
    {
        if (group <= 0 || ! isValidPpq(ppqPosition))
            return;

        for (auto& event : events)
            if (event.used && event.group == group && event.contains(ppqPosition, balanceTimelineMergeWindowPpq))
                event.resolved = true;
    }

    bool hasPendingEventAt(int group, int targetRole, double ppqPosition) const noexcept
    {
        if (! isValidEventKey(group, targetRole) || ! isValidPpq(ppqPosition))
            return false;

        if (const auto* event = findNearby(group, targetRole, ppqPosition))
            return ! event->resolved;

        return false;
    }

    void restore(const BalanceTimelineSnapshot& snapshot) noexcept
    {
        learning = snapshot.learning;
        events = {};

        auto copied = 0;
        for (const auto& event : snapshot.events)
        {
            if (! event.used || copied >= maxBalanceTimelineEvents)
                continue;

            if (! isValidEventKey(event.group, event.targetRole)
                || ! isValidPpq(event.startPpq)
                || ! isValidPpq(event.endPpq)
                || ! std::isfinite(event.correctionDb))
            {
                continue;
            }

            auto cleaned = event;
            const auto sectionIndex = cleaned.sectionIndex >= 0
                ? cleaned.sectionIndex
                : balanceTimelineSectionIndexFor(event.lastSeenPpq);
            cleaned.sectionIndex = sectionIndex;
            cleaned.sectionKind = cleanSectionKind(cleaned.sectionKind, sectionIndex);
            cleaned.sectionStartPpq = isValidPpq(cleaned.sectionStartPpq)
                ? cleaned.sectionStartPpq
                : static_cast<double> (sectionIndex) * balanceTimelineSectionLengthPpq;
            cleaned.sectionEndPpq = isValidPpq(cleaned.sectionEndPpq) && cleaned.sectionEndPpq > cleaned.sectionStartPpq
                ? cleaned.sectionEndPpq
                : cleaned.sectionStartPpq + balanceTimelineSectionLengthPpq;
            cleaned.startPpq = std::max(0.0, std::min(event.startPpq, event.endPpq));
            cleaned.endPpq = std::max(cleaned.startPpq, std::max(event.startPpq, event.endPpq));
            cleaned.lastSeenPpq = isValidPpq(event.lastSeenPpq)
                ? std::clamp(event.lastSeenPpq, cleaned.startPpq, cleaned.endPpq)
                : cleaned.endPpq;
            cleaned.correctionDb = std::clamp(cleaned.correctionDb, -1.2f, 1.2f);
            cleaned.severity = std::clamp(cleaned.severity, 0.0f, 1.0f);
            cleaned.hits = std::clamp(cleaned.hits, 1, maxHits);
            events[static_cast<size_t> (copied)] = cleaned;
            ++copied;
        }
    }

    BalanceTimelineSnapshot snapshot() const noexcept
    {
        BalanceTimelineSnapshot result;
        result.learning = learning;
        result.events = events;
        result.count = count();
        return result;
    }

    int count() const noexcept
    {
        auto result = 0;
        for (const auto& event : events)
            if (event.used)
                ++result;

        return result;
    }

    int resolvedCount() const noexcept
    {
        auto result = 0;
        for (const auto& event : events)
            if (event.used && event.resolved)
                ++result;

        return result;
    }

private:
    static constexpr int maxHits = 9999;

    static bool isValidEventKey(int group, int targetRole) noexcept
    {
        return group > 0 && targetRole > 0;
    }

    static bool isValidPpq(double ppqPosition) noexcept
    {
        return std::isfinite(ppqPosition) && ppqPosition >= 0.0;
    }

    static int cleanSectionKind(int sectionKind, int sectionIndex) noexcept
    {
        const auto minimum = static_cast<int> (BalanceTimelineSectionKind::Intro);
        const auto maximum = static_cast<int> (BalanceTimelineSectionKind::Section);
        if (sectionKind < minimum || sectionKind > maximum)
            return static_cast<int> (balanceTimelineSectionKindForIndex(sectionIndex));

        return sectionKind;
    }

    static double distanceToEvent(const BalanceTimelineEvent& event, double ppqPosition) noexcept
    {
        if (event.contains(ppqPosition))
            return 0.0;

        return std::min(std::abs(ppqPosition - event.startPpq), std::abs(ppqPosition - event.endPpq));
    }

    static void extendEvent(BalanceTimelineEvent& event, double ppqPosition) noexcept
    {
        event.startPpq = std::max(0.0, std::min(event.startPpq, ppqPosition - balanceTimelineEventHalfWindowPpq));
        event.endPpq = std::max(event.endPpq, ppqPosition + balanceTimelineEventHalfWindowPpq);
        event.lastSeenPpq = ppqPosition;
    }

    BalanceTimelineEvent* findNearby(int group, int targetRole, double ppqPosition) noexcept
    {
        const auto sectionIndex = balanceTimelineSectionIndexFor(ppqPosition);
        for (auto& event : events)
            if (event.matchesKey(group, targetRole, sectionIndex)
                && distanceToEvent(event, ppqPosition) <= balanceTimelineMergeWindowPpq)
            {
                return &event;
            }

        return nullptr;
    }

    const BalanceTimelineEvent* findNearby(int group, int targetRole, double ppqPosition) const noexcept
    {
        const auto sectionIndex = balanceTimelineSectionIndexFor(ppqPosition);
        for (const auto& event : events)
            if (event.matchesKey(group, targetRole, sectionIndex)
                && distanceToEvent(event, ppqPosition) <= balanceTimelineMergeWindowPpq)
            {
                return &event;
            }

        return nullptr;
    }

    BalanceTimelineEvent* findWriteSlot() noexcept
    {
        for (auto& event : events)
            if (! event.used)
                return &event;

        return &*std::min_element(
            events.begin(),
            events.end(),
            [](const BalanceTimelineEvent& first, const BalanceTimelineEvent& second)
            {
                const auto firstScore = (first.resolved ? 0 : 10000) + first.hits;
                const auto secondScore = (second.resolved ? 0 : 10000) + second.hits;
                return firstScore < secondScore;
            });
    }

    bool learning = false;
    std::array<BalanceTimelineEvent, maxBalanceTimelineEvents> events;
};
} // namespace stagemind
