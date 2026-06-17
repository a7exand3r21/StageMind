---
name: plugin-ui-fancy-controls
description: Use when designing, implementing, or reviewing premium custom UI controls for a JUCE audio plugin, including animated knobs, sliders, buttons, meters, LED rings, glow states, value readouts, hover/drag/focus states, theme tokens, layout, and visual polish. Do not use for DSP-only work.
---

# Plugin UI Fancy Controls

Build controls that feel premium and musical.

Fancy is allowed. Messy is not.

## Apply this skill when

- Creating knobs.
- Creating sliders.
- Creating buttons/switches.
- Creating meters.
- Creating LED rings or arc indicators.
- Creating custom panels.
- Creating value readouts.
- Adding glow, shadows, depth, gradients, or interaction states.
- Reviewing plugin UI visual quality.

## Design principles

- Controls must be readable first.
- Visual polish must reinforce state.
- Animation must not hide value precision.
- Glow must signal focus/activity, not decorate everything.
- Every control must belong to the same design system.
- Use theme tokens.
- Avoid per-component random colors.
- Avoid visual effects that cost too much to repaint.

## Required states

For each control, define and implement relevant states:

- Idle.
- Hover.
- Pressed.
- Dragging.
- Focused.
- Disabled.
- Automated or externally changed.
- Warning or clipping if applicable.
- Bypassed if applicable.

## Knob behavior

A premium knob should define:

- Value arc.
- Pointer or highlight.
- Drag gesture.
- Fine adjustment gesture if supported.
- Double-click reset if supported.
- Value readout.
- Hover highlight.
- Active glow while dragging.
- Optional automation/modulation indicator.
- Accessible text/value where applicable.

Knob painting must be deterministic and efficient.

## Meter behavior

Meters should define:

- Input source model.
- Rise behavior.
- Falloff behavior.
- Peak hold if applicable.
- Danger threshold.
- Color zones.
- Smoothing.
- Update frequency.
- Snapshot mechanism from audio to UI.

Meters must never read directly from audio buffers on the UI thread unless using a safe snapshot.

## Theme system

Before adding new visuals, check or create:

- `Theme` tokens.
- Color constants.
- Spacing constants.
- Radius constants.
- Shadow/glow constants.
- Typography scale.
- Meter color rules.
- Control dimensions.

All custom controls should consume theme values.

## Layout

Layout must stay clean:

- Align labels and controls.
- Make parameter grouping obvious.
- Keep enough spacing for hover glow and shadows.
- Avoid tiny controls for important parameters.
- Ensure resize behavior is intentional.
- Keep values readable.

## Painting rules

- Avoid unnecessary repainting.
- Clip expensive painting to needed bounds.
- Cache static assets where useful.
- Do not allocate in paint.
- Do not load images in paint.
- Avoid excessive gradients in frequently repainting components unless measured/acceptable.
- Use dirty-region thinking.

## Output expected from Codex

When using this skill, return:

- Visual concept.
- Control states.
- Theme tokens used/added.
- Files changed.
- Repaint/performance considerations.
- Manual UI test notes.
