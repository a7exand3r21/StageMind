# ADR 0003: Local resonance detector

## Status

Accepted.

## Context

MVP 3 needs local resonance detection without StageMind Link, background analysis, lookahead, or latency.

## Decision

Use a per-instance `ResonanceDetector` with a 2048-sample Hann-window FFT, fixed internal buffers, time-smoothed magnitudes, and a fixed top-4 peak snapshot. A per-instance `ResonanceLearner` holds live peaks briefly and can learn stable runtime corrections from about four seconds of audio. Suppression is handled by `DynamicEQ` with fixed band storage.

## Consequences

- The plugin remains zero-latency.
- The analyzer path does not allocate or block in `processBlock`.
- CPU cost is bounded, but every active instance can perform FFT work.
- Analyzer quality controls remain reserved for later tuning.
- The GUI reads lossy atomic snapshots; it is not part of DSP truth.
- Learn is runtime-only in MVP 3. It does not create APVTS parameters or preset state yet.
