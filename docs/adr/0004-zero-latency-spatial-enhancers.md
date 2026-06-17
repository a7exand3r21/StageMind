# ADR 0004: Zero-latency spatial enhancers

## Status

Accepted.

## Context

MVP 4 needs motion, depth, and pseudo-double without lookahead, PDC, full reverb, or host latency.

## Decision

Implement three separate DSP modules:

- `MotionProcessor`: sine modulation after low-end side protection. It combines side modulation with a small equal-power pan movement so centered material has audible motion when the role allows it.
- `DepthProcessor`: dry path remains undelayed, with small presence damping and delayed wet reflections.
- `PseudoDoubleProcessor`: fixed 18 ms wet micro-delay, dry path untouched.

Role and safety limits are resolved in `PluginProcessor`. Correlation safety scales risky widening on following blocks instead of muting audio.

## Consequences

- The plugin continues to report zero latency.
- Delay buffers are allocated in `prepareToPlay`, not in `processBlock`.
- Motion stays zero-latency and allocation-free, but it is no longer strictly side-only.
- Pseudo-double does not change delay time from automation, avoiding transient skips and zipper artifacts.
- Current factory programs reuse existing parameters. No new APVTS IDs are introduced.
