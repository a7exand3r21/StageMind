# StageMind Node 0.5.8 RC

Date: 2026-06-10

## Status

0.5.8 clarifies the sidechain status line during FL Studio routing tests.

## What Changed

The sidechain status text now prioritizes audition mode:

- `SC Listen: Sidechain Only` shows `SC listen active` when a sidechain signal is present.
- `SC Mode: Off` no longer reports active dynamic EQ.

This does not change DSP behavior. It only makes the UI line match what the processor is already doing.

## Test Focus

- Bass target with `External Sidechain`, `SC Enable`, `Kick Ducks Bass`, and sidechain signal shows active sidechain behavior.
- Drums/source instance with `SC Mode: Off` does not claim that dynamic EQ is active.
- After confirming routing, `SC Listen` can be returned to `Off` for normal playback.
