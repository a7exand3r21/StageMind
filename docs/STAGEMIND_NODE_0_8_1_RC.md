# StageMind Node 0.8.1 RC

## What Changed

0.8.1 extends Auto Assist from resonance-only into guarded Link actions.

- Auto mode can apply stable Link Assist suggestions without pressing `Apply Tip`.
- Bass against Kick/Suno Drums can auto-select `Kick Ducks Bass`.
- Harmonic instruments against Suno Drums can auto-select `Make Space`.
- Auto sidechain actions enable `SC Enable`, choose `StageMind Link`, and set a conservative `SC Amount` if it was too low.
- `StageMind Link` now drives `SidechainDynamicEQ` from peer activity control data. It still does not transfer audio between plugin instances.
- UI status shows when an Auto action has been applied.

## Not Included Yet

- No automatic Director-wide correction policy.
- No automatic master-insert detection.
- No true inter-instance audio bus.

## FL Test

1. Put StageMind Node on Drums, Bass, and Guitar mixer inserts.
2. Enable `Link` on all three and set the same Group, for example `1`.
3. Set `Auto Assist` to `Auto` on Bass and Guitar.
4. Play a section where Drums and Bass overlap.
5. Expected on Bass: `SC Mode` becomes `Kick Ducks Bass`, `Trigger` becomes `StageMind Link`, `SC Enable` turns on, and GR moves.
6. Play a section where Drums and Guitar overlap.
7. Expected on Guitar: `SC Mode` becomes `Make Space`, `Trigger` becomes `StageMind Link`, `SC Enable` turns on, and GR moves.
8. Save/reopen the FL project and confirm the chosen parameters are restored.

If a track has no stable signal or no active peer in the group, Auto should wait instead of forcing a correction.
