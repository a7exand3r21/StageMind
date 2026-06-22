# StageMind Node 0.10.0 RC

0.10.0 adds a visible Stage Gain layer inspired by gain-staging plugins such as CodWaves WaveBalance, but implemented inside StageMind's existing Node processing chain.

## Added

- `Stage Gain` mode in Node UI: `Off`, `Static`, `Ride`.
- `Target`, `Threshold`, `Ceiling`, and `Response` controls.
- `Analyze` button for Static Analyze & Hold.
- Applied gain status in the UI.
- Static held gain is saved in the DAW project state.
- DSP tests for Ride and Static hold behavior.

## Test

1. Insert StageMind Node on a stem.
2. Set `Stage Gain` to `Ride`.
3. Play a section with uneven level and confirm the status line shows applied dB.
4. Lower `Ceiling` and confirm large boosts are restrained.
5. Switch to `Static`, play an audible section, press `Analyze`, then stop/start playback.
6. Save/reopen the FL project and confirm the held Static correction remains.

## Not Yet

- LUFS-M/S/I detection.
- Group `Analyze All`.
- True look-ahead limiter.
- Editable per-section gain memory in the UI.
