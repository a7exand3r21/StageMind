# StageMind Node 0.9.7 RC

0.9.7 adds the first Balance Timeline Memory layer for Director Auto.

## What changed

- Added `BalanceTimelineMemory` for coarse PPQ balance events.
- Project state now saves/restores Balance Timeline Memory.
- `Learn Mix` now starts spectral ride memory, spectral timeline memory, and balance timeline memory.
- `Clear Memory` clears all three memory layers.
- Director Memory shows separate `Timeline` and `Balance` counts.
- Director shows the current live balance correction target, correction amount, and deviation.
- Director Auto gives remembered balance sections priority over weak suggestions, while strong spectral conflicts still win.
- Version bumped to 0.9.7.

## Balance memory behavior

Balance memory is guarded. It remembers that a section needed trim, but it does not blindly keep turning `Output Trim` every time playback crosses that PPQ area. The current live group balance still has to confirm the problem.

That guard is intentional. Without it, remembered trim could keep stacking until the Node hits the trim limit.

## What to test

1. Put Director in `Auto`.
2. Put at least two normal Nodes in the same Group and set them to `Auto`.
3. Play a section where one stem is clearly louder or quieter than the group.
4. Confirm Director Memory shows `Balance` count above zero.
5. Confirm the target Node receives small `Output Trim` changes, not huge jumps.
6. Save and reopen the FL project.
7. Confirm Director Memory still shows the learned balance event.
8. Let the same section play after the group is balanced.
9. Confirm the event becomes `resolved` or no longer keeps applying trim.

## Known limits

- Balance memory uses PPQ windows, not named song sections.
- It uses RMS and role offsets, not LUFS.
- It cannot move the FL Studio mixer fader.
- It is still a guarded assist layer, not a full automatic mix engine.
