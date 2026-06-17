# StageMind Node 0.8.0 RC

## What Changed

0.8.0 adds the first guarded Auto Assist layer.

- Added `Auto Assist`: Off / Suggest / Auto.
- Auto mode waits for a linked group with at least two active Nodes and local input signal.
- Auto mode starts resonance analysis without pressing `Learn`.
- Auto mode gently raises low `Clean Up` / `Resonance` values once when analysis starts.
- Learned resonance profiles are saved with the DAW project state.
- Resonance suppression now rides learned frequencies, so reduction releases during silence instead of behaving like a static notch.

## Not Included Yet

- No automatic FL Studio sidechain routing.
- No Link-based ducking yet.
- No Director-wide automatic correction policy yet.

## FL Test

1. Put two StageMind Nodes on two active mixer inserts.
2. Enable `Link` on both.
3. Set both to the same Group, for example `1`.
4. Set `Auto Assist` to `Auto` on one Node.
5. Play a busy section.
6. Confirm the status moves from waiting/arming to analyzing, then resonance rider.
7. Save and reopen the FL project.
8. Confirm learned resonances are still visible and the button shows learned/relearn behavior.

Expected: Auto only starts after the group is active. It should not create sidechain routing or silently change unrelated controls.
