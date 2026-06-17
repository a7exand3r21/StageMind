# ADR 0008: Link Assist Uses User-Approved Corrections

Date: 2026-06-10

## Status

Accepted.

## Context

StageMind Node can now publish and read Link control data. 0.5.5 added status-only suggestions. 0.5.6 added a user-approved action button. 0.5.7 stopped duplicate apply actions when the target value was already active.

The next step is more useful conflict reporting. Jumping directly to automatic correction would make Node behave like a partial Director without enough proven rules.

## Decision

Implement Link Assist v1 inside Node as a user-approved helper only.

Allowed actions:

- reduce Width for stereo safety or double-wide conflicts;
- set `SC Mode` to `KickDucksBass`;
- set `SC Mode` to `VocalDucksInstrument`;
- set `SC Mode` to `SnareDucksInstrument`;
- set `SC Mode` to `LeadDucksPad`;
- adjust Width and Depth for vocal space.

Every suggestion must expose:

- a conflict label;
- a short reason;
- an action-specific button label;
- an applied state when the current APVTS values already match the action.

Auto Assist is not implemented in this stage.

## Consequences

- Link still does not transfer audio.
- Link still does not change audio without a user click.
- The button writes existing APVTS parameters only.
- No new parameter IDs are introduced.
- Sidechain routing and `SC Enable` remain explicit user decisions.
- Future automatic correction requires a separate ADR.
