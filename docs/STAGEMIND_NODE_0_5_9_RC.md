# StageMind Node 0.5.9 RC

Date: 2026-06-10

## Status

0.5.9 adds Link Assist v1.

## What Changed

Link suggestions now show conflict labels instead of generic tips.

The action button now names the expected action:

- `Reduce Width`
- `Make Room`
- `Set Kick Duck`
- `Set Vocal Duck`
- `Set Snare Duck`
- `Set Lead Duck`

The suggestion engine also stores a short reason for each conflict, so the UI can explain why the action exists.

## What It Does Not Do

- It does not add Auto Assist.
- It does not apply corrections without a user click.
- It does not transfer audio between instances.
- It does not enable sidechain routing or `SC Enable`.
- It does not create StageMind Director behavior.

## Test Focus

- Link conflict labels appear only when a peer is active.
- Action-specific button labels match the expected parameter change.
- Applied actions still show `Applied` when the current values already match.
- The new LinkSuggestionEngine cases pass tests.
