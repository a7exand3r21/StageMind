# StageMind Node 0.8.3 RC

## What Changed

0.8.3 fixes Director conflict resolution visibility.

- Nodes now publish `SC Mode`, `Trigger`, `SC Enable`, `SC Amount`, and Auto mode in the Link snapshot.
- Director checks the target Node state before marking a conflict active.
- If the target already has the suggested Width, Depth, or `SC Mode`, the row is shown as `resolved`.
- Resolved rows no longer draw active scene arrows and no longer enable `Apply to #...`.

## FL Test

1. Put Director on the master and Nodes on Drums, Bass, Guitar, Vocal, Pad.
2. Use the same Link Group.
3. Set the target Nodes to `Auto`.
4. Play a busy section until Auto applies actions.
5. Expected in Director: rows whose target already has the suggested action switch to `resolved`.
6. Expected: unresolved rows still show `Tip:` and can be applied manually.

If the audio overlap remains but the action is already set, Director should not keep that row active.
