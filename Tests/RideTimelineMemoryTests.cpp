#include "../Source/Link/RideTimelineMemory.h"

#include <cmath>
#include <stdexcept>

namespace
{
using namespace stagemind;

void expect(bool condition, const char* message)
{
    if (! condition)
        throw std::runtime_error(message);
}

void mergesNearbyTimelineEvents()
{
    RideTimelineMemory memory;
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Presence), 12.0, 0.4f, false);
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Presence), 12.75, 0.7f, false);

    const auto snapshot = memory.snapshot();
    expect(snapshot.count == 1, "nearby timeline ride events should merge");
    expect(snapshot.events[0].hits == 2, "merged timeline ride event should count hits");
    expect(snapshot.events[0].severity == 0.7f, "merged timeline ride event should keep highest severity");
    expect(snapshot.events[0].startPpq <= 11.75, "merged timeline ride event should keep early window");
    expect(snapshot.events[0].endPpq >= 13.0, "merged timeline ride event should extend late window");
}

void separatesDistantTimelineEvents()
{
    RideTimelineMemory memory;
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Low), 8.0, 0.4f, false);
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Low), 16.0, 0.4f, false);

    expect(memory.snapshot().count == 2, "distant timeline ride events should stay separate");
}

void tracksResolvedStateAtTimelinePosition()
{
    RideTimelineMemory memory;
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::LowMid), 20.0, 0.6f, false);

    expect(
        memory.hasPendingEventAt(1, 2, 3, 4, static_cast<int> (RideMemoryBand::LowMid), 20.1),
        "unresolved timeline ride event should be pending near its position");

    memory.markResolved(1, 2, 3, 4, static_cast<int> (RideMemoryBand::LowMid), 20.1);

    expect(
        ! memory.hasPendingEventAt(1, 2, 3, 4, static_cast<int> (RideMemoryBand::LowMid), 20.1),
        "resolved timeline ride event should stop being pending");
}

void restoresTimelineSnapshot()
{
    RideTimelineMemory memory;
    memory.setLearning(true);
    memory.observe(2, 5, 6, 7, static_cast<int> (RideMemoryBand::Air), 32.0, 0.8f, true);

    RideTimelineMemory restored;
    restored.restore(memory.snapshot());

    const auto snapshot = restored.snapshot();
    expect(snapshot.learning, "timeline ride learning state should restore");
    expect(snapshot.count == 1, "timeline ride snapshot should restore events");
    expect(snapshot.events[0].group == 2, "timeline ride snapshot should restore group");
    expect(snapshot.events[0].targetRole == 5, "timeline ride snapshot should restore target role");
    expect(snapshot.events[0].sourceRole == 6, "timeline ride snapshot should restore source role");
    expect(snapshot.events[0].actionKind == 7, "timeline ride snapshot should restore action kind");
    expect(snapshot.events[0].resolved, "timeline ride snapshot should restore resolved state");
    expect(std::abs(snapshot.events[0].lastSeenPpq - 32.0) < 0.001, "timeline ride snapshot should restore position");
}
} // namespace

void runRideTimelineMemoryTests()
{
    mergesNearbyTimelineEvents();
    separatesDistantTimelineEvents();
    tracksResolvedStateAtTimelinePosition();
    restoresTimelineSnapshot();
}
