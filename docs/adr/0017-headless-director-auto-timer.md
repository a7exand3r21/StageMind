# ADR 0017: Director Auto Runs from Processor Timer

## Status

Accepted.

## Context

0.8.4 added guarded Director Auto commands from the Director editor timer. That proved the policy, but it had a bad product constraint: closing the Director UI stopped group-wide automatic correction.

StageMind should not require an open GUI for core behavior.

## Decision

Move Director Auto command selection into `PluginProcessor::timerCallback` when the instance is in Director mode.

The processor timer:

- checks that Director Auto Assist is `Auto`;
- reads the selected Link Group snapshot;
- evaluates the same Link Suggestion rules used by the UI;
- skips targets whose own Auto Assist is not `Auto`;
- skips actions already applied on the target snapshot;
- sends at most one automatic `LinkCommand` per cooldown window.

The audio callback is not involved.

## Consequences

- Director Auto works with the Director UI closed.
- Editor and processor no longer both send automatic Director commands.
- The UI can lag behind by one Link publish/timer interval, but the action policy stays centralized in the processor.
- No new parameter IDs or state migrations are needed.
