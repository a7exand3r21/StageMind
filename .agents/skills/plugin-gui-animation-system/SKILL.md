---
name: plugin-gui-animation-system
description: Use when adding or reviewing animations in a JUCE audio plugin UI, including hover fades, knob motion, glow pulses, meter falloff, modulation highlights, panel transitions, repaint throttling, timers, easing, frame budget, and UI responsiveness. Must be used when animation could affect performance or audio stability.
---

# Plugin GUI Animation System

Animation should make the plugin feel alive without making audio unstable.

## Apply this skill when

- Adding hover transitions.
- Adding glow pulses.
- Smoothing UI values.
- Animating knobs/sliders.
- Animating meters.
- Adding modulation visualization.
- Adding panel transitions.
- Changing timer frequency.
- Reviewing repaint performance.
- Debugging UI stutter.

## Rules

- Animation is UI-only.
- Audio thread must never drive animation directly.
- Audio thread may publish lightweight snapshots.
- UI thread consumes snapshots at a controlled rate.
- No allocation in animation tick where avoidable.
- No heavy calculations in `paint`.
- No unbounded repaint loops.
- No animation that changes DSP state unless caused by a real parameter change.

## Animation primitives

Prefer a small shared animation layer:

- `AnimatedFloat`.
- `SmoothedUiValue`.
- `HoverState`.
- `GlowState`.
- `MeterBallistics`.
- `FrameTimer` or centralized timer if suitable.

Avoid each component inventing a different timing system.

## Timing guidance

Suggested starting points:

- Hover fade: 80-160 ms.
- Press response: 40-100 ms.
- Knob active glow: immediate rise, 120-240 ms release.
- Panel transition: 120-240 ms.
- Meter rise: fast.
- Meter falloff: musical and readable.
- Peak hold: optional, plugin-dependent.

Tune by feel and performance.

## Repaint budget

Before adding animation, define:

- Which component repaints.
- How often it repaints.
- Whether repaint bounds are limited.
- Whether static background is cached.
- Whether multiple controls animate independently.
- Whether timer frequency is justified.

Avoid repainting the whole editor for one blinking control.

## Audio/UI data flow

For animated meters or analyzers:

1. Audio thread computes cheap level/snapshot.
2. Audio thread writes to atomic or lock-free buffer.
3. UI timer reads snapshot.
4. UI applies visual smoothing.
5. UI repaints only meter region.

Never reverse this dependency.

## Easing

Use simple easing:

- Linear for meters where needed.
- Ease-out for hover release.
- Ease-in-out for panel transitions.
- Avoid bouncy/cartoon easing unless product direction calls for it.

## Output expected from Codex

When using this skill, return:

- Animation purpose.
- Timing values.
- Data flow.
- Repaint scope.
- Performance risks.
- Manual test notes.
