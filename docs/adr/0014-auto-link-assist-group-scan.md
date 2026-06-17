# ADR 0014: Auto Link Assist Scans the Whole Group

## Status

Accepted.

## Context

Node 0.8.1 applied Auto Link Assist from one selected peer. In a group with several active instruments, that peer could be the wrong source for a given Node. This made Auto appear inconsistent: some instruments applied corrections, others waited.

## Decision

In Auto mode, each Node scans the fixed-size StageMind Link group snapshot and evaluates Link Assist against every eligible active peer. It then chooses the highest-priority stable local conflict and applies that one local action.

The compact Link UI can still show one representative peer. Auto sidechain stores its own source instance ID so `StageMind Link` dynamic EQ follows the peer that caused the chosen conflict.

## Consequences

- Larger groups behave more predictably.
- No new APVTS parameters are needed.
- The scan is bounded by `maxLinkInstances` and uses existing atomic snapshots.
- A Node still applies one local action at a time, because there is only one active `SC Mode`.
