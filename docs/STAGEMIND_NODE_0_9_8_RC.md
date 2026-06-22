# StageMind Node 0.9.8 RC

0.9.8 completes Layer 3 as a real role priority loudness model.

## Layer status

- Layer 1: implemented.
- Layer 2: implemented.
- Layer 3: implemented as Role Priority Loudness Model v2.
- Layer 4: implemented as guarded PPQ Balance Timeline Memory v1.
- Layer 5: partial.
- Layer 6: partial.
- Layer 7: not implemented.
- Layer 8: not implemented.

## What changed

- Added `RoleBalanceModel`.
- Replaced the old single `roleBalanceOffsetDb()` switch with role profiles.
- Each role profile now has:
  - target loudness offset;
  - priority weight;
  - deadband;
  - correction gain;
  - max cut and max boost step;
  - activity/RMS threshold.
- Director balance now uses a weighted reference instead of a plain median.
- Vocal, kick, bass, drums, guitar/lead, pads, and FX are treated differently.
- Added unit tests for vocal boost, pad cut, Auto-disabled ignore, and guitar deadband.
- Version bumped to 0.9.8.

## What to test

1. Put Director and all Nodes in `Auto`.
2. Use one lead vocal, drums, bass, guitar, and pad if possible.
3. Make the vocal clearly too quiet under drums/bass.
4. Confirm Director chooses the vocal for a small positive `Output Trim` correction.
5. Make a pad or FX stem as loud as the vocal.
6. Confirm Director cuts the pad/FX rather than forcing the vocal down.
7. Set one Node to `Suggest` or `Off`.
8. Confirm Director does not change that Node.

## Known limits

- It is RMS-based, not LUFS.
- It does not know song intent. A deliberately buried vocal or huge pad may still need manual override.
- It does not yet expose role targets in the UI.
- Full-pass analysis and node/group locks are later layers.
