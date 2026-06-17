# ADR 0019: Director Stage Remote Control

## Status

Accepted.

## Context

Director mode was useful for observing group conflicts, but it could not inspect or move individual Nodes from the scene. A stage map that cannot control the target Nodes forces the user back into each insert window for basic positioning.

The tempting alternative is to make Director own a global audio stage. That would require audio routing between plugin instances, host-dependent ordering, or shared audio buffers. It conflicts with the current product rule that StageMind Link carries control data only.

## Decision

Use Director as a remote controller for target Node parameters.

- Nodes publish `pan` together with the existing Link control snapshot.
- Director Stage View uses effective `pan` for horizontal position and `depth` for front/back position.
- Clicking a scene Node selects it and shows a compact inspector in Director.
- Dragging a selected scene Node sends a one-shot Link command containing `pan` and `depth`.
- The target Node consumes the command on its processor timer and writes its own existing APVTS parameters.

## Consequences

- The audio still comes only from each target Node's normal processing path.
- Save/reopen works through normal target Node state.
- Effective pan includes role defaults, while the target still saves the existing APVTS pan offset.
- The audio callback does not wait on Director, the editor, or the Link registry.
- Director movement can be slightly stepped because commands are consumed on the message-thread timer.
- Width is deliberately not changed by drag in 0.8.7; dragging left/right means pan.
