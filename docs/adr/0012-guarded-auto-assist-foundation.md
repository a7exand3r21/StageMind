# ADR 0012: Guarded Auto Assist Foundation

Date: 2026-06-16

## Status

Accepted.

## Context

Manual Link Assist proved useful, but testing showed too much routine work remains: starting resonance learn, choosing basic correction amounts, and preparing sidechain modes by hand.

The tempting version is a fully silent autopilot. That is risky. It would make hidden parameter changes, confuse save/reopen behavior, and make FL render tests harder to reason about.

## Decision

Add `Auto Assist` as a host-facing choice parameter appended after the existing parameter list:

- `Off`
- `Suggest`
- `Auto`

Node 0.8.0 implements only the first guarded automatic layer:

- Auto waits for Link to be enabled, Group > 0, at least two active Nodes, and local input signal.
- Auto starts resonance learning after a short stable window.
- Auto may raise low `Clean Up` and `Resonance` values once, from the processor timer/message thread.
- Learned resonance profiles are saved in plugin state.
- Dynamic resonance suppression rides learned frequencies and releases reduction when the resonance is not present.

The audio callback never writes APVTS parameters. It only updates counters and atomics. Parameter writes remain message-thread host notifications.

## Consequences

- Users get the first automatic routine reduction without needing FL sidechain routing.
- Save/reopen can restore the learned resonance profile.
- Auto Assist still does not create host routing, transfer audio between Nodes, or run Director-wide hidden corrections.
- Link-based ducking and Director auto policy remain separate future decisions.
