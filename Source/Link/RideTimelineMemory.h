#pragma once

#include "RideMemory.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace stagemind
{
inline constexpr int maxRideTimelineEvents = 64;
inline constexpr double rideTimelineMergeWindowPpq = 1.0;
inline constexpr double rideTimelineEventHalfWindowPpq = 0.25;

struct RideTimelineEvent
{
    bool used = false;
    int group = 0;
    int targetRole = 0;
    int sourceRole = 0;
    int actionKind = 0;
    int band = static_cast<int> (RideMemoryBand::Unknown);
    double startPpq = 0.0;
    double endPpq = 0.0;
    double lastSeenPpq = 0.0;
    float severity = 0.0f;
    int hits = 0;
    bool resolved = false;

    bool matchesKey(int groupToMatch, int targetRoleToMatch, int sourceRoleToMatch, int actionKindToMatch, int bandToMatch) const noexcept
    {
        return used
            && group == groupToMatch
            && targetRole == targetRoleToMatch
            && sourceRole == sourceRoleToMatch
            && actionKind == actionKindToMatch
            && band == bandToMatch;
    }

    bool contains(double ppqPosition, double tolerancePpq = 0.0) const noexcept
    {
        return used
            && std::isfinite(ppqPosition)
            && ppqPosition + tolerancePpq >= startPpq
            && ppqPosition - tolerancePpq <= endPpq;
    }
};

struct RideTimelineSnapshot
{
    bool learning = false;
    int count = 0;
    std::array<RideTimelineEvent, maxRideTimelineEvents> events;
};

class RideTimelineMemory final
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
        int sourceRole,
        int actionKind,
        int band,
        double ppqPosition,
        float severity,
        bool resolved) noexcept
    {
        if (! isValidEventKey(group, targetRole, sourceRole, actionKind) || ! isValidPpq(ppqPosition))
            return;

        band = clampBand(band);
        severity = std::clamp(severity, 0.0f, 1.0f);

        if (auto* event = findNearby(group, targetRole, sourceRole, actionKind, band, ppqPosition))
        {
            extendEvent(*event, ppqPosition);
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
        slot->sourceRole = sourceRole;
        slot->actionKind = actionKind;
        slot->band = band;
        slot->startPpq = std::max(0.0, ppqPosition - rideTimelineEventHalfWindowPpq);
        slot->endPpq = ppqPosition + rideTimelineEventHalfWindowPpq;
        slot->lastSeenPpq = ppqPosition;
        slot->severity = severity;
        slot->hits = 1;
        slot->resolved = resolved;
    }

    void markResolved(
        int group,
        int targetRole,
        int sourceRole,
        int actionKind,
        int band,
        double ppqPosition) noexcept
    {
        if (auto* event = findNearby(group, targetRole, sourceRole, actionKind, clampBand(band), ppqPosition))
            event->resolved = true;
    }

    bool hasPendingEventAt(
        int group,
        int targetRole,
        int sourceRole,
        int actionKind,
        int band,
        double ppqPosition) const noexcept
    {
        if (! isValidEventKey(group, targetRole, sourceRole, actionKind) || ! isValidPpq(ppqPosition))
            return false;

        if (const auto* event = findNearby(group, targetRole, sourceRole, actionKind, clampBand(band), ppqPosition))
            return ! event->resolved;

        return false;
    }

    void restore(const RideTimelineSnapshot& snapshot) noexcept
    {
        learning = snapshot.learning;
        events = {};

        auto copied = 0;
        for (const auto& event : snapshot.events)
        {
            if (! event.used || copied >= maxRideTimelineEvents)
                continue;

            if (! isValidEventKey(event.group, event.targetRole, event.sourceRole, event.actionKind)
                || ! isValidPpq(event.startPpq)
                || ! isValidPpq(event.endPpq))
            {
                continue;
            }

            auto cleaned = event;
            cleaned.band = clampBand(cleaned.band);
            cleaned.startPpq = std::max(0.0, std::min(event.startPpq, event.endPpq));
            cleaned.endPpq = std::max(cleaned.startPpq, std::max(event.startPpq, event.endPpq));
            cleaned.lastSeenPpq = isValidPpq(event.lastSeenPpq)
                ? std::clamp(event.lastSeenPpq, cleaned.startPpq, cleaned.endPpq)
                : cleaned.endPpq;
            cleaned.severity = std::clamp(cleaned.severity, 0.0f, 1.0f);
            cleaned.hits = std::clamp(cleaned.hits, 1, maxHits);
            events[static_cast<size_t> (copied)] = cleaned;
            ++copied;
        }
    }

    RideTimelineSnapshot snapshot() const noexcept
    {
        RideTimelineSnapshot result;
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

    const std::array<RideTimelineEvent, maxRideTimelineEvents>& all() const noexcept
    {
        return events;
    }

private:
    static constexpr int maxHits = 9999;

    static bool isValidEventKey(int group, int targetRole, int sourceRole, int actionKind) noexcept
    {
        return group > 0 && targetRole > 0 && sourceRole > 0 && actionKind > 0;
    }

    static bool isValidPpq(double ppqPosition) noexcept
    {
        return std::isfinite(ppqPosition) && ppqPosition >= 0.0;
    }

    static int clampBand(int band) noexcept
    {
        return std::clamp(band, static_cast<int> (RideMemoryBand::Unknown), static_cast<int> (RideMemoryBand::Air));
    }

    static double distanceToEvent(const RideTimelineEvent& event, double ppqPosition) noexcept
    {
        if (event.contains(ppqPosition))
            return 0.0;

        return std::min(std::abs(ppqPosition - event.startPpq), std::abs(ppqPosition - event.endPpq));
    }

    static void extendEvent(RideTimelineEvent& event, double ppqPosition) noexcept
    {
        event.startPpq = std::max(0.0, std::min(event.startPpq, ppqPosition - rideTimelineEventHalfWindowPpq));
        event.endPpq = std::max(event.endPpq, ppqPosition + rideTimelineEventHalfWindowPpq);
        event.lastSeenPpq = ppqPosition;
    }

    RideTimelineEvent* findNearby(int group, int targetRole, int sourceRole, int actionKind, int band, double ppqPosition) noexcept
    {
        for (auto& event : events)
            if (event.matchesKey(group, targetRole, sourceRole, actionKind, band)
                && distanceToEvent(event, ppqPosition) <= rideTimelineMergeWindowPpq)
            {
                return &event;
            }

        return nullptr;
    }

    const RideTimelineEvent* findNearby(int group, int targetRole, int sourceRole, int actionKind, int band, double ppqPosition) const noexcept
    {
        for (const auto& event : events)
            if (event.matchesKey(group, targetRole, sourceRole, actionKind, band)
                && distanceToEvent(event, ppqPosition) <= rideTimelineMergeWindowPpq)
            {
                return &event;
            }

        return nullptr;
    }

    RideTimelineEvent* findWriteSlot() noexcept
    {
        for (auto& event : events)
            if (! event.used)
                return &event;

        return &*std::min_element(
            events.begin(),
            events.end(),
            [](const RideTimelineEvent& first, const RideTimelineEvent& second)
            {
                const auto firstScore = (first.resolved ? 0 : 10000) + first.hits;
                const auto secondScore = (second.resolved ? 0 : 10000) + second.hits;
                return firstScore < secondScore;
            });
    }

    bool learning = false;
    std::array<RideTimelineEvent, maxRideTimelineEvents> events;
};
} // namespace stagemind
