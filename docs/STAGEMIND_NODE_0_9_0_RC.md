# StageMind Node 0.9.0 RC

0.9.0 starts the timeline-aware Ride Memory layer for Director Auto.

## What changed

- Added Timeline Ride Memory: conflict events can now store a coarse PPQ song position.
- Director Auto writes timeline events while Auto Assist is `Auto` and the host provides transport position.
- Existing Ride Memory still works as the broad role-to-role fallback.
- Timeline memory is saved and restored with the FL project state.
- Director memory status now shows both relationship events and timeline events.
- Director memory status now shows current PPQ position, nearby timeline events, and the nearest remembered timeline event.
- Plugin state format moved to version 4.
- Version shown by the plugin moves to 0.9.0.

## What to test

1. Put Nodes on at least Drums, Bass, Guitar, and Vocal/Instrumental tracks.
2. Keep Nodes on `Link`, `Group 1`, `Auto`.
3. Put one instance in `Director` mode, also on `Group 1`, `Auto`.
4. Start playback from the beginning.
5. Watch Director memory status: the normal event count and `Timeline` count should rise after conflicts are detected.
6. Confirm the memory panel shows `PPQ ...` while FL playback position is available.
7. Let playback pass the same conflict area more than once.
8. Confirm `nearby` becomes non-zero when playback reaches a remembered conflict area.
9. Confirm Auto still applies corrections without pressing `Apply Tip`.
10. Save the FL project, close it, reopen it.
11. Confirm Director still shows memory and timeline counters.
12. Press `Clear Memory` and confirm both counters reset.

## Expected behavior

- If the DAW exposes PPQ position, Timeline count should grow.
- If transport position is unavailable, normal Ride Memory still works.
- Corrections are still guarded: target Nodes must be in `Auto`.
- Link still sends control data only, not audio.

## Known limits

- Timeline memory is coarse. It does not yet show a full song-section editor.
- The Director UI shows counters, not a detailed timeline lane yet.
- Sidechain routing inside FL Studio remains manual.
- Existing relationship memory can still apply as a fallback outside exact timeline positions.

## Verification

- `StageMindDSPTests` passed.
- Debug VST3 target built successfully.
- Release VST3 was packaged into `dist/VST3`.
- `moduleinfo.json` reports version `0.9.0`.
