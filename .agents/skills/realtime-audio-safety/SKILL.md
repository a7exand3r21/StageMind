---
name: realtime-audio-safety
description: Use before editing or reviewing realtime audio code in a C++ JUCE plugin, including processBlock, DSP modules, parameter smoothing, audio/UI communication, buffers, atomics, locks, allocations, denormals, bypass, and host automation. Must be used whenever audio glitches, clicks, CPU spikes, threading, or performance regressions are possible.
---

# Realtime Audio Safety

You are protecting the audio thread.

In an audio plugin, correctness means more than compiling. Code must behave under realtime constraints.

## Apply this skill when

- Editing `processBlock`.
- Editing DSP modules.
- Reading parameters in audio code.
- Adding smoothing.
- Adding bypass.
- Adding meters or analyzers.
- Sharing data between audio and UI.
- Adding logging, diagnostics, or assertions near audio code.
- Investigating clicks, crackles, CPU spikes, denormals, or automation issues.
- Reviewing code before release.

## Hard bans in realtime code

Do not perform these in the audio callback or hot DSP path:

- Heap allocation.
- File I/O.
- Network I/O.
- Logging.
- String formatting.
- Blocking locks.
- Waiting on futures/promises.
- UI calls.
- Message-manager calls.
- Creating or destroying heavyweight objects.
- Throwing exceptions.
- Calling functions with unknown realtime behavior.
- Unbounded loops.
- Reallocating vectors.
- Resizing buffers.
- Loading images/fonts/assets.
- Parsing JSON/XML.
- Accessing DAW/project state in a blocking way.

If existing code violates this, flag it as a blocker before expanding it.

## Required review questions

For every audio-path change, answer:

1. Does this allocate?
2. Can this block?
3. Can this call UI code?
4. Can this log or format strings?
5. Is the loop bounded by block size or fixed constants?
6. Is parameter smoothing handled?
7. Is bypass click-safe?
8. Does prepare/reset allocate everything needed?
9. Does it survive sample-rate changes?
10. Does it survive buffer-size changes?
11. Are denormals handled?
12. Are shared values read safely?

## Parameter handling

Rules:

- Read host parameters in a stable, cheap way.
- Smooth parameters that can cause clicks.
- Use appropriate smoothing time per parameter.
- Use log/skew mapping for frequency-like parameters.
- Avoid reading UI components from audio code.
- Avoid writing to UI components from audio code.
- Keep automation responsive but not clicky.

## UI communication

For meters/analyzers:

- Audio thread may write lightweight snapshots.
- UI thread may read snapshots.
- Prefer atomics, lock-free FIFO, or double-buffered data.
- Never let UI wait on audio thread.
- Never let audio wait on UI thread.
- Meter data may be lossy; audio correctness is not.

## Prepare/reset lifecycle

In `prepareToPlay` or equivalent:

- Allocate buffers.
- Prepare DSP modules.
- Reset smoothers.
- Store sample rate.
- Store maximum block size.
- Validate assumptions.

In `releaseResources`:

- Release safely.
- Do not rely on release being called at a predictable musical time.

## Denormals

Use denormal protection where silence or near-silence can hit recursive filters, feedback paths, nonlinear processing, or envelope followers.

## Bypass

Bypass must not click.

Options:

- Smoothed wet/dry.
- Short crossfade.
- State-aware bypass ramp.
- Host-provided bypass parameter handling if suitable.

Document chosen bypass behavior in `docs/PLUGIN_SPEC.md`.

## Output expected from Codex

When using this skill, return:

- Realtime risks found.
- Whether any hard-ban violation exists.
- What changed.
- How smoothing/bypass/threading are handled.
- Manual or automated verification.
