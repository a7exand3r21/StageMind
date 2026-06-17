# StageMind Node 0.5.5 RC

Date: 2026-06-10

## Status

0.5.5 is the first RC-style build after MVP4 Spatial and MVP5 status-only Link were finalized.

Main addition:

- Link Suggestions.

## What Changed

Link diagnostics can now show a short suggestion based on peer control data.

Examples:

- `Tip: reduce Width`
- `Tip: Kick Ducks Bass`
- `Tip: make vocal room`
- `Tip: avoid double-wide`

These suggestions do not change audio. They are hints only.

## Still Not Implemented

- Automatic Link corrections.
- StageMind Director.
- Audio transfer between Node instances.
- Spectral peer analysis.

## Test Focus

- Link on/off with two or three instances.
- Source role filtering.
- `Any Role` fallback.
- Suggestions appearing only when a peer is active.
- Suggestions disappearing when Link is off or no peer is found.
- Export/render still stable.
