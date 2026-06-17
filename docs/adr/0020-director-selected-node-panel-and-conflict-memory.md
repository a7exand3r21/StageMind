# ADR 0020: Director Selected Node Panel and Conflict Memory

## Status

Accepted.

## Context

Director Stage View could move Nodes, but selection feedback lived inside the same text area as conflict diagnostics. In FL this made it hard to tell whether the click worked. The correction/status list also changed too quickly because it reflected the current analysis window more than the accumulated mixing decision.

Full Ride Memory needs playhead-aware conflict capture. That is bigger than a UI patch because it needs host position handling, storage shape, and rules for section-specific automation.

## Decision

Split Director UI into three areas:

- Stage View;
- selected Node remote controls;
- correction/status history.

Expand one-shot Link commands so the Director can remotely write Pan, Width, Depth, Motion, Clean Up, Resonance, and SC Amount to the selected target Node. The target Node still performs normal APVTS parameter writes on its timer.

Increase runtime Director conflict history and keep resolved rows visible longer. Shorten Director Auto cooldown so it can move through unresolved conflicts faster while still sending bounded commands.

## Consequences

- Selection is visible without reading the conflict list.
- Target Node state remains the source of truth and survives DAW save/reopen.
- No audio is transferred between instances.
- The audio callback does not wait on Director UI or Link commands.
- Conflict history is session/runtime memory, not full playhead-timeline Ride Memory.
