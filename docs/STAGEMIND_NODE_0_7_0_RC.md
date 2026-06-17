# StageMind Node 0.7.0 RC

Date: 2026-06-11

## Status

0.7.0 adds Director as a mode inside `StageMind Node.vst3`.

## What Changed

- Added `Mode: Node / Director`.
- Removed the separate `StageMind Director.vst3` build/package path.
- Kept Link group reading through the existing fixed-size registry.
- Removed the shared-memory registry prototype.
- Added a Director scene view for linked Nodes in the selected group.
- Added a Director conflict list using the existing Link Assist engine.

## Director Behavior

Director mode passes audio through unchanged.

It does not publish itself as a Node. It reads the selected `Group` from the UI thread and displays the active linked Nodes.

It is observe-only. It does not apply corrections, route sidechain, transfer audio, or write parameters on other instances.

## What It Does Not Do

- It does not auto-detect the master insert.
- It does not replace Node mode.
- It does not add Auto Assist.
- It does not make Link audio-dependent.

## Test Focus

- One plugin artifact: `StageMind Node.vst3`.
- `Mode` defaults to `Node` in old/new projects.
- Switching to `Director` changes the UI to the scene overview.
- Director mode passes audio unchanged.
- Director mode sees linked Nodes in the same group.
- Director mode disappears from the group if the same instance previously published as Node.
- Project save/reopen restores `Mode` and `Group`.
- Export/render still works.
