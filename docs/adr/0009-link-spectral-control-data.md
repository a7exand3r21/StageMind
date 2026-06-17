# ADR 0009: Link Spectral Control Data

Date: 2026-06-10

## Status

Accepted.

## Context

Link Assist v1 uses role, activity, width, depth, and correlation. That is useful, but too coarse for frequency conflicts. A Bass/Kick or Vocal/Instrument hint should not appear only because the roles match.

The tempting version is a shared FFT or a hidden inter-instance sidechain. That is too much for Node at this stage. It risks CPU cost, host-order dependence, and user confusion about whether audio is being transferred.

## Decision

Add Link Spectral Control Data v1.

Each Node computes four normalized control bands from its own processed output:

- low;
- low-mid;
- presence;
- air.

The analyzer uses lightweight one-pole filters. No FFT, no lookahead, no latency, no heap allocation in the audio callback, and no UI calls.

The registry publishes the bands with the existing Link snapshot. The suggestion engine uses overlap gates for frequency-specific conflicts, while stereo-safety and double-wide stay based on correlation and width.

## Consequences

- Suggestions should be less role-only and less noisy.
- Link still transfers control data only.
- Offline render behavior stays deterministic because Link publish remains suppressed in non-realtime render.
- This is not a spectral matching system. A later Director stage can add richer analysis if the product needs it.
