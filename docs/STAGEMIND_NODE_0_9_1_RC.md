# StageMind Node 0.9.1 RC

0.9.1 improves the spatial controls in the normal Node UI.

## What changed

- Added `Motion Preset` with four movement shapes: `Slow Drift`, `Orbit`, `Pulse`, `Wide Sweep`.
- Stage View preview now follows the selected Motion preset speed.
- Added a visible `Double` knob in the main Node UI for `pseudo_double_amount`.
- Depth now uses a stronger Room Depth v1 path: damped early reflections, cross-channel taps, and short feedback.
- Dry depth path stays zero-latency.
- Plugin state format moved to version 5.
- Version shown by the plugin moves to 0.9.1.

## What to test

1. Insert StageMind Node on a few tracks.
2. Confirm the title shows `StageMind Node 0.9.1`.
3. In Node mode, confirm `Motion Preset` and `Double` are visible.
4. Play a centered source and raise `Motion`.
5. Switch between `Slow Drift`, `Orbit`, `Pulse`, and `Wide Sweep`.
6. Confirm Stage View movement and audible motion change.
7. Raise `Depth` on a vocal, guitar, pad, or percussion loop.
8. Confirm the source feels slightly farther/roomier, but the dry hit is not delayed.
9. Try `Double` on roles that allow pseudo-double and confirm it changes the width/thickness.
10. Save the FL project, close it, reopen it.
11. Confirm Motion preset and Double value are restored.

## Expected behavior

- `Slow Drift` is subtle.
- `Orbit` adds a rounder left/right movement.
- `Pulse` moves faster.
- `Wide Sweep` gives the widest movement.
- `Depth` is still not a full reverb. It is an insert-safe room-depth layer with early reflections.
- Mono/correlation safety can still reduce risky width, motion, and double.

## Known limits

- This is not HRTF, not binaural 3D, and not a convolution room.
- Motion remains role-limited.
- Double remains role-limited.

## Verification

- `StageMindDSPTests` passed.
- Debug VST3 target built successfully.
- Release VST3 was packaged into `dist/VST3`.
- `moduleinfo.json` reports version `0.9.1`.
