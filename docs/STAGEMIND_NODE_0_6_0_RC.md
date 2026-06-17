# StageMind Node 0.6.0 RC

Date: 2026-06-10

## Status

0.6.0 adds Link Spectral Control Data v1.

## What Changed

Each linked Node now publishes four coarse control bands:

- low;
- low-mid;
- presence;
- air.

Link Assist uses those bands for frequency-overlap conflicts. For example, Kick/Bass needs low-band overlap, Vocal Mask needs presence overlap, and Lead/Pad needs low-mid or presence overlap.

The Link detail line now shows the strongest peer band beside peer activity and correlation.

## What It Does Not Do

- It does not transfer audio between plugin instances.
- It does not add Auto Assist.
- It does not apply corrections without a user click.
- It does not enable sidechain routing or `SC Enable`.
- It does not add FFT-based inter-instance analysis.
- It does not create StageMind Director behavior.

## Test Focus

- Link spectral bands publish through the registry and are clamped to 0..1.
- The control analyzer reacts differently to low and presence test tones.
- Frequency-specific Link Assist suggestions require matching band overlap.
- The VST3 still builds and packages for FL Studio.
