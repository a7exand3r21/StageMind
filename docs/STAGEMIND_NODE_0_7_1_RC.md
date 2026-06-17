# StageMind Node 0.7.1 RC

Date: 2026-06-11

## Status

0.7.1 improves Director conflict readability and adds observe-only correction hints.

## What Changed

- Director keeps a fixed-size conflict history.
- Active conflicts show the roles, conflict type, strongest peer band, and a `Tip`.
- Cleared conflicts stay visible and switch to `resolved`.
- Conflict memory resets when the Director group changes.

## What It Does Not Do

- It does not apply Director tips automatically.
- It does not write parameters on other Node instances.
- It does not transfer audio between instances.
- It does not add master-insert auto-detection.

## Test Focus

- Create a conflict between two linked Nodes.
- Confirm Director shows the active conflict and a `Tip`.
- Change Width/Depth/role/audio so the conflict clears.
- Confirm the row remains visible and says `resolved`.
- Change Director Group and confirm the old conflict list clears.
- Confirm Director mode still passes audio unchanged.
