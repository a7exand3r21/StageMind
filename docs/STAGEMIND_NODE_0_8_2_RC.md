# StageMind Node 0.8.2 RC

## What Changed

0.8.2 fixes Auto Assist peer selection in larger groups.

- Auto mode now scans all active peers in the current Link Group.
- It chooses the highest-priority stable local conflict instead of relying on the single peer shown in the compact Link UI.
- Auto sidechain remembers the peer that caused the selected conflict and uses that peer's activity as the `StageMind Link` control envelope.
- Explicit `Source` role filtering is still respected. `Any Role` means the whole group is considered.

## FL Test

1. Put StageMind Node on Drums, Bass, Guitar, Synth, and Vocal.
2. Enable `Link` and set the same Group on all Nodes.
3. Set `Source` to `Any Role` unless you intentionally want to limit Auto.
4. Set `Auto Assist` to `Auto` on Bass, Guitar, Synth, and Vocal.
5. Play a busy section.
6. Expected: each Node should pick its own strongest relevant conflict. For example Bass can pick Drums/Kick, Guitar can pick Drums/Vocal, and Synth can pick Vocal/Lead depending on signal.
7. Check that GR moves on sidechain-style actions even if the compact Link line shows a different active peer.

If you set `Source` to a specific role, Auto should only consider that role.
