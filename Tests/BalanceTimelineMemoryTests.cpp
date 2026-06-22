#include "../Source/Link/BalanceTimelineMemory.h"

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

void mergesNearbyBalanceEvents()
{
    BalanceTimelineMemory memory;
    memory.observe(1, 2, 12.0, -0.5f, 0.4f, false);
    memory.observe(1, 2, 12.8, -0.8f, 0.7f, false);

    const auto snapshot = memory.snapshot();
    expect(snapshot.count == 1, "nearby balance events should merge");
    expect(snapshot.events[0].hits == 2, "merged balance event should count hits");
    expect(snapshot.events[0].severity == 0.7f, "merged balance event should keep highest severity");
    expect(snapshot.events[0].correctionDb < -0.55f, "merged balance event should smooth correction amount");
}

void separatesDistantBalanceEvents()
{
    BalanceTimelineMemory memory;
    memory.observe(1, 2, 8.0, -0.4f, 0.4f, false);
    memory.observe(1, 2, 16.0, -0.4f, 0.4f, false);

    expect(memory.snapshot().count == 2, "distant balance events should stay separate");
}

void separatesAdjacentSections()
{
    BalanceTimelineMemory memory;
    memory.observe(1, 2, balanceTimelineSectionLengthPpq - 0.2, -0.4f, 0.4f, false);
    memory.observe(1, 2, balanceTimelineSectionLengthPpq + 0.2, -0.4f, 0.4f, false);

    const auto snapshot = memory.snapshot();
    expect(snapshot.count == 2, "adjacent balance sections should not merge into one event");
    expect(snapshot.events[0].sectionIndex == 0, "first section should be Intro bucket");
    expect(snapshot.events[1].sectionIndex == 1, "second section should be Verse bucket");
}

void tracksResolvedBalancePosition()
{
    BalanceTimelineMemory memory;
    memory.observe(1, 2, 20.0, 0.5f, 0.6f, false);

    expect(
        memory.hasPendingEventAt(1, 2, 20.1),
        "unresolved balance event should be pending near its position");

    memory.markResolved(1, 2, 20.1);

    expect(
        ! memory.hasPendingEventAt(1, 2, 20.1),
        "resolved balance event should stop being pending");
}

void restoresBalanceSnapshot()
{
    BalanceTimelineMemory memory;
    memory.setLearning(true);
    memory.observe(2, 5, 32.0, -0.7f, 0.8f, true);

    BalanceTimelineMemory restored;
    restored.restore(memory.snapshot());

    const auto snapshot = restored.snapshot();
    expect(snapshot.learning, "balance timeline learning state should restore");
    expect(snapshot.count == 1, "balance timeline snapshot should restore events");
    expect(snapshot.events[0].group == 2, "balance timeline snapshot should restore group");
    expect(snapshot.events[0].targetRole == 5, "balance timeline snapshot should restore target role");
    expect(snapshot.events[0].sectionIndex == 0, "balance timeline snapshot should restore section index");
    expect(
        snapshot.events[0].sectionKind == static_cast<int> (BalanceTimelineSectionKind::Intro),
        "balance timeline snapshot should restore section kind");
    expect(snapshot.events[0].resolved, "balance timeline snapshot should restore resolved state");
    expect(std::abs(snapshot.events[0].correctionDb + 0.7f) < 0.001f, "balance timeline snapshot should restore correction");
    expect(std::abs(snapshot.events[0].lastSeenPpq - 32.0) < 0.001, "balance timeline snapshot should restore position");
}
} // namespace

void runBalanceTimelineMemoryTests()
{
    mergesNearbyBalanceEvents();
    separatesDistantBalanceEvents();
    separatesAdjacentSections();
    tracksResolvedBalancePosition();
    restoresBalanceSnapshot();
}
