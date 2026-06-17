#include "../Source/Link/RideMemory.h"

#include <iostream>
#include <stdexcept>

namespace
{
using namespace stagemind;

void expect(bool condition, const char* message)
{
    if (! condition)
        throw std::runtime_error(message);
}

void mergesRepeatedConflict()
{
    RideMemory memory;
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Presence), 0.4f, false);
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Presence), 0.7f, false);

    const auto snapshot = memory.snapshot();
    expect(snapshot.count == 1, "repeated ride memory event should be merged");
    expect(snapshot.events[0].hits == 2, "merged ride memory event should count hits");
    expect(snapshot.events[0].severity == 0.7f, "merged ride memory event should keep highest severity");
    expect(! snapshot.events[0].resolved, "unresolved ride memory event should stay unresolved");
}

void updatesResolvedState()
{
    RideMemory memory;
    memory.observe(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Low), 0.3f, false);
    memory.markResolved(1, 2, 3, 4, static_cast<int> (RideMemoryBand::Low));

    const auto snapshot = memory.snapshot();
    expect(snapshot.count == 1, "resolved ride memory event should remain in memory");
    expect(snapshot.events[0].resolved, "resolved ride memory event should be marked resolved");
}

void restoresSnapshot()
{
    RideMemory memory;
    memory.setLearning(true);
    memory.observe(2, 5, 6, 7, static_cast<int> (RideMemoryBand::LowMid), 0.6f, true);

    RideMemory restored;
    restored.restore(memory.snapshot());

    const auto snapshot = restored.snapshot();
    expect(snapshot.learning, "ride memory learning state should restore");
    expect(snapshot.count == 1, "ride memory snapshot should restore events");
    expect(snapshot.events[0].group == 2, "ride memory snapshot should restore group");
    expect(snapshot.events[0].targetRole == 5, "ride memory snapshot should restore target role");
    expect(snapshot.events[0].sourceRole == 6, "ride memory snapshot should restore source role");
    expect(snapshot.events[0].actionKind == 7, "ride memory snapshot should restore action kind");
    expect(snapshot.events[0].resolved, "ride memory snapshot should restore resolved state");
}
} // namespace

void runRideMemoryTests()
{
    mergesRepeatedConflict();
    updatesResolvedState();
    restoresSnapshot();
}
