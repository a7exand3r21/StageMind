# StageMind Node 0.9.6 RC

0.9.6 starts the Director group-balance layer.

## What changed

- Link snapshots now publish the current `Output Trim` value.
- Link commands can write `Output Trim`.
- Director Auto compares active Nodes in the selected group by role-aware loudness.
- Director Auto can send small Output Trim corrections to Auto-enabled Nodes.
- Severe spectral/link conflicts still take priority over balance trim.

## Balance model

This does not force every stem to the same RMS. Lead vocals are allowed to sit forward. Pads, FX, and wide beds are expected lower. Bass, kick/drums, and snare have their own offsets.

## What to test

1. Put Director in `Auto`.
2. Put at least two Nodes in the same Group, also set to `Auto`.
3. Make one active stem obviously louder than the others.
4. Confirm the target Node's `Output Trim` moves gradually, not in huge jumps.
5. Set one Node to `Suggest` or `Off`.
6. Confirm Director does not change that Node.
7. Check that strong ducking/width conflict corrections still happen before balance trim.

## Known limits

- This is live group balance, not full song-section memory yet.
- It uses RMS, not LUFS.
- It cannot move the FL Studio mixer fader.
