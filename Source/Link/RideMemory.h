#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

namespace stagemind
{
inline constexpr int maxRideMemoryEvents = 32;

enum class RideMemoryBand
{
    Unknown = 0,
    Low,
    LowMid,
    Presence,
    Air
};

struct RideMemoryEvent
{
    bool used = false;
    int group = 0;
    int targetRole = 0;
    int sourceRole = 0;
    int actionKind = 0;
    int band = static_cast<int> (RideMemoryBand::Unknown);
    float severity = 0.0f;
    int hits = 0;
    bool resolved = false;

    bool matches(int groupToMatch, int targetRoleToMatch, int sourceRoleToMatch, int actionKindToMatch, int bandToMatch) const noexcept
    {
        return used
            && group == groupToMatch
            && targetRole == targetRoleToMatch
            && sourceRole == sourceRoleToMatch
            && actionKind == actionKindToMatch
            && band == bandToMatch;
    }
};

struct RideMemorySnapshot
{
    bool learning = false;
    int count = 0;
    std::array<RideMemoryEvent, maxRideMemoryEvents> events;
};

class RideMemory final
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
        float severity,
        bool resolved) noexcept
    {
        if (group <= 0 || targetRole <= 0 || sourceRole <= 0 || actionKind <= 0)
            return;

        band = std::clamp(band, static_cast<int> (RideMemoryBand::Unknown), static_cast<int> (RideMemoryBand::Air));
        severity = std::clamp(severity, 0.0f, 1.0f);

        if (auto* event = find(group, targetRole, sourceRole, actionKind, band))
        {
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
        slot->severity = severity;
        slot->hits = 1;
        slot->resolved = resolved;
    }

    void markResolved(
        int group,
        int targetRole,
        int sourceRole,
        int actionKind,
        int band) noexcept
    {
        if (auto* event = find(group, targetRole, sourceRole, actionKind, band))
            event->resolved = true;
    }

    void restore(const RideMemorySnapshot& snapshot) noexcept
    {
        learning = snapshot.learning;
        events = {};

        auto copied = 0;
        for (const auto& event : snapshot.events)
        {
            if (! event.used || copied >= maxRideMemoryEvents)
                continue;

            if (event.group <= 0 || event.targetRole <= 0 || event.sourceRole <= 0 || event.actionKind <= 0)
                continue;

            auto cleaned = event;
            cleaned.band = std::clamp(
                cleaned.band,
                static_cast<int> (RideMemoryBand::Unknown),
                static_cast<int> (RideMemoryBand::Air));
            cleaned.severity = std::clamp(cleaned.severity, 0.0f, 1.0f);
            cleaned.hits = std::clamp(cleaned.hits, 1, maxHits);
            events[static_cast<size_t> (copied)] = cleaned;
            ++copied;
        }
    }

    RideMemorySnapshot snapshot() const noexcept
    {
        RideMemorySnapshot result;
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

    const std::array<RideMemoryEvent, maxRideMemoryEvents>& all() const noexcept
    {
        return events;
    }

private:
    static constexpr int maxHits = 9999;

    RideMemoryEvent* find(int group, int targetRole, int sourceRole, int actionKind, int band) noexcept
    {
        for (auto& event : events)
            if (event.matches(group, targetRole, sourceRole, actionKind, band))
                return &event;

        return nullptr;
    }

    RideMemoryEvent* findWriteSlot() noexcept
    {
        for (auto& event : events)
            if (! event.used)
                return &event;

        return &*std::min_element(
            events.begin(),
            events.end(),
            [](const RideMemoryEvent& first, const RideMemoryEvent& second)
            {
                const auto firstScore = (first.resolved ? 0 : 10000) + first.hits;
                const auto secondScore = (second.resolved ? 0 : 10000) + second.hits;
                return firstScore < secondScore;
            });
    }

    bool learning = false;
    std::array<RideMemoryEvent, maxRideMemoryEvents> events;
};
} // namespace stagemind
