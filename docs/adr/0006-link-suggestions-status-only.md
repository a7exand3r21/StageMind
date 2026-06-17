# ADR 0006: Link Suggestions Are Status-Only

## Status

Accepted.

## Context

After MVP5, StageMind Node can read peer control data through Link. The next useful step is to surface simple mix suggestions before any automatic correction exists.

This must not turn Node into StageMind Director. It also must not silently change audio.

## Decision

Add a small `LinkSuggestionEngine`.

It reads only lightweight control data:

- current role;
- peer role;
- current width/depth/correlation;
- peer activity;
- peer width.

It returns a suggestion kind, severity, and short UI message. The editor displays the message in the Link diagnostics area.

Rules are intentionally conservative:

- low current correlation plus high width suggests stereo safety;
- bass against kick/drums suggests trying Kick Ducks Bass;
- wide bed against vocal suggests making room for vocal;
- two wide bed peers with high width suggest avoiding a double-wide stack.

## Consequences

- No audio transfer.
- No automatic parameter changes.
- No extra APVTS parameter IDs.
- No new audio-thread allocation or blocking behavior.
- The rules are testable without the editor.

Automatic Link correction needs a separate ADR before it is allowed to affect sound.
