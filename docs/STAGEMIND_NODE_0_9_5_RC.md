# StageMind Node 0.9.5 RC

0.9.5 starts the balance automation pass.

## What changed

- `Output` is now labeled `Output Trim`.
- The parameter ID is still `output_gain`, so old project state should restore.
- Auto mode now runs Auto Balance in Node mode.
- Auto Balance learns a per-stem RMS target and applies a slow level rider before Output Trim.
- The learned Auto Balance target is saved in the DAW project state.

## Important limit

StageMind still cannot directly move the FL Studio mixer fader through VST3. Output Trim is plugin gain before the DAW channel fader. The DAW fader remains the final mix control.

Auto Balance v1 is per-Node. It helps with sudden loud/quiet section changes inside a stem. It is not yet a full Director timeline loudness map for the whole song.

## What to test

1. Insert StageMind Node on a vocal, guitar, bass, or drums stem.
2. Keep Auto set to `Auto`.
3. Play a loud section for a while, then a noticeably quieter section.
4. Confirm the Auto status can show `Auto level ... dB`.
5. Confirm the quieter section is gently lifted, without silence being boosted.
6. Move `Output Trim` and confirm it behaves like final plugin trim in dB.
7. Save/reopen the FL project and confirm the plugin does not forget the learned balance target.
