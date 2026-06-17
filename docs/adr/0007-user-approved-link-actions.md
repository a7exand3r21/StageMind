# ADR 0007: User-Approved Link Actions

## Status

Accepted.

## Context

StageMind Node 0.5.5 introduced Link Suggestions as status-only hints. The next step is to let the user apply a small safe action from a visible suggestion.

This is not automatic correction. The plugin must not change audio just because Link found a peer.

## Decision

Add an `Apply Tip` button in the Link diagnostics area.

The button is enabled only when `LinkSuggestionEngine` returns a suggestion with an available action.

Current actions:

- Stereo safety: reduce `width`.
- Kick/Bass: select `Kick Ducks Bass` sidechain mode.
- Vocal space: reduce `width` and raise `depth`.
- Double-wide stack: reduce `width`.

The button writes existing APVTS parameters through normal host gestures. It does not add parameters and does not run from the audio thread.

## Consequences

- The user remains in control.
- Host state/save/load sees the applied parameter changes.
- No hidden automation is introduced.
- No Link action changes routing, enables sidechain, or transfers audio.
- Full automatic correction still needs a separate ADR.
