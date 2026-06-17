# StageMind Node 0.8.9 RC

0.8.9 adds the first Ride Memory foundation for Director Auto.

## What changed

- Director Auto now records compact conflict events while Auto Assist is `Auto`.
- `Learn Mix` starts explicit memory learning.
- `Clear Memory` removes stored conflict events.
- Ride Memory is saved in the FL project state with the normal plugin state.
- Director can reapply remembered corrections when the same target/source role relationship appears again.

## What to test

1. Put Nodes on at least Drums, Bass, Guitar, and Vocal/Instrumental tracks.
2. Keep Nodes on `Link`, `Group 1`, `Auto`.
3. Put one instance in `Director` mode, also on `Group 1`.
4. Start playback and watch `Memory auto` in Director.
5. Check that event count rises when conflicts are detected.
6. Check that corrections apply without pressing `Apply Tip`.
7. Stop playback, save the FL project, reopen it.
8. Confirm that Director still shows memory events.
9. Press `Clear Memory` and confirm the count resets.

## Known limits

- Ride Memory is not timeline-aware yet. It remembers role relationships, not exact song positions.
- It does not store audio.
- It still depends on matching Nodes being visible in the same Director group.
- Target Nodes still must be in `Auto` for automatic corrections.

## Verification

- `StageMindDSPTests` passed for the Ride Memory model and existing DSP/link tests.
- VST3 package should show `StageMind Node 0.8.9` in the UI and module metadata after the release build.
