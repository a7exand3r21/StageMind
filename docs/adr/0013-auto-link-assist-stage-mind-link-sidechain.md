# ADR 0013: Auto Link Assist Uses StageMind Link Control Sidechain

## Status

Accepted.

## Context

Auto Assist 0.8.0 learned local resonances but did not apply Link suggestions. That left common cases, such as Bass vs Drums or Guitar vs Drums, waiting for manual `Apply Tip` and manual FL Studio sidechain routing.

Full inter-instance audio transfer would add host-ordering, buffering, realtime, and render-consistency risk. It is too large for this stage.

## Decision

Node 0.8.1 keeps Link audio-free. Auto mode evaluates stable Link suggestions from existing control data and writes local APVTS parameters on the message thread.

For sidechain-style actions, Auto sets `Trigger` to `StageMind Link` when no working external sidechain is already active. The DSP then uses the selected peer's smoothed activity as the `SidechainDynamicEQ` control envelope.

## Consequences

- Bass/Drums and Guitar/Drums can produce audible automatic ducking without FL manual sidechain routing.
- The path remains zero-latency and does not transfer peer audio.
- The result is control ducking, not spectral analysis of the peer's raw audio.
- Director-wide automatic correction remains a later policy decision.
