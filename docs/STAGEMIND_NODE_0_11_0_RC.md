# StageMind Node 0.11.0 RC

0.11.0 completes the first Stage Gain feature pass: LUFS-style modes, group Analyze All, and a real look-ahead ceiling path.

## Added

- Stage Gain meter selector:
  - `dBFS`
  - `VU`
  - `RMS`
  - `LUFS M`
  - `LUFS S`
  - `LUFS I`
- Fixed-latency look-ahead ceiling limiter after Stage Gain.
- Host latency reporting for the look-ahead path.
- Director `Analyze All` button.
- Link command support for remote Stage Gain Static Analyze.
- DSP tests for LUFS-style metering and look-ahead peak ceiling.

## Test

1. Insert Nodes on several stems and put them in `Group 1`.
2. Add one Director and choose `Group 1`.
3. Press `Analyze All` while playback is running.
4. Nodes should switch to Static Stage Gain and capture `Hold ... dB`.
5. On one Node, switch meter modes and confirm Ride/Static still reacts.
6. Push a loud signal into Stage Gain with a low `Ceiling`, for example `-6 dB`; output peaks should stay near that ceiling.
7. Save/reopen and confirm held Static gain remains.

## Limits

- LUFS modes are practical Stage Gain meters, not certified broadcast loudness measurement.
- The limiter is a safety ceiling, not a full mastering limiter.
- `Analyze All` analyzes the current playback moment. It is not yet an offline full-song scan.
