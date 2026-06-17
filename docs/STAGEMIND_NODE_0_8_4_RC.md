# StageMind Node 0.8.4 RC

## What Changed

0.8.4 adds guarded Director Auto correction.

- Director mode now shows the existing `Auto` selector.
- If Director is set to `Auto`, it sends bounded correction commands for active unresolved conflicts.
- Target Nodes accept automatic Director commands only when their own Auto Assist is also `Auto`.
- Automatic sidechain commands carry the conflict peer ID, so the target can use `StageMind Link` as the control source.
- Manual `Apply to #...` keeps the old conservative behavior: Width, Depth, or `SC Mode` only.

## FL Test

1. Put one instance in `Mode: Director`.
2. Put several Nodes in the same Link Group: Drums, Bass, Guitar, Pad/Vocal if available.
3. Set Director `Auto` to `Auto`.
4. Set target Nodes `Auto` to `Auto`.
5. Play a section with overlap.
6. Expected: Director sends auto commands one at a time, rows switch to `resolved`, and sidechain actions on the target use `Trigger: StageMind Link`.
7. Set one target Node `Auto` to `Off`.
8. Expected: Director does not auto-change that Node; manual `Apply to #...` still works as before.

This still does not auto-detect the master insert. Director mode is explicit.
