# ADR 0015: Director Resolves Rows from Target Action State

## Status

Accepted.

## Context

Director detects conflicts by re-running Link Suggestion logic over published group snapshots. After Auto Link Assist was added, Director could still display active conflicts because it only saw roles, activity, bands, width, depth, and correlation. It did not know whether the target Node had already applied the suggested sidechain mode.

## Decision

Nodes publish a small set of action-state fields in the Link snapshot:

- sidechain mode;
- trigger mode;
- sidechain enabled;
- sidechain amount;
- auto-assist mode.

Director checks the target Node against the `LinkSuggestionAction`. If the suggested Width, Depth, or SC Mode is already set, the conflict row is marked `resolved` even if the spectral overlap is still present.

## Consequences

- Director reflects Auto-applied local actions.
- Resolved conflicts remain visible as history but do not draw active arrows.
- No new APVTS parameters are introduced.
- The audio thread only performs bounded atomic stores/loads.
