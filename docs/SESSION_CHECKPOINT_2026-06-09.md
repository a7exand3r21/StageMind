# Session Checkpoint - 2026-06-09

## Current Build

- Product: StageMind Node VST3.
- Current version: 0.4.0.
- Runtime package: `dist/VST3/StageMind Node.vst3`.
- CMake project version and packaged `moduleinfo.json` were checked as `0.4.0`.

Git was not available as a repository from this workspace path:

```text
fatal: not a git repository (or any of the parent directories): .git
```

So this checkpoint is the durable project note for the session.

## Verified Today

- The plugin loads in FL Studio on mixer channels.
- Basic role/width/output behavior works by ear and on meters.
- External sidechain works in FL Studio after manual FL Wrapper mapping:
  `Processing > Input/Connections > Auto map inputs`.
- The sidechain meter reacts after correct host routing.
- The earlier debug assertion from the resonance detector was fixed.
- Resonance display no longer depends only on instant FFT hits; short holding and Learn/Relearn were added.
- UI resize behavior was improved and confirmed in FL Studio.
- MVP4 spatial processing was implemented and packaged.

## Completed Scope

MVP1:

- JUCE/CMake VST3 foundation.
- Stable APVTS parameter layout.
- Role, safety, trigger, width, output gain, meters, basic spatial path.
- Optional sidechain bus.

MVP2:

- External sidechain detector and meter.
- Sidechain dynamic EQ modes.
- Gain reduction meter.
- Sidechain listen mode.

MVP3:

- Local resonance detector.
- Dynamic resonance suppression.
- Resonance list UI.
- Runtime Learn/Relearn action.
- Fixed local-neighborhood array bounds issue found in FL Studio.

MVP4:

- Zero-latency dry-path spatial enhancers.
- Role-limited motion.
- Depth processor with small wet reflection path.
- Pseudo-double wet micro-delay path.
- Correlation safety reduction for risky spatial widening.
- Host factory programs.
- DSP tests for spatial processors.

## Last Verification Commands

```text
build/StageMindDSPTests.exe
```

Result:

```text
StageMindDSPTests passed
```

```text
scripts/build-and-package-vst3.cmd
```

Result:

```text
Build complete.
Packaged VST3: C:\Users\user\Desktop\StageMind\dist\VST3\StageMind Node.vst3
```

## Known Limitations

- `pseudo_double_amount` exists as a host parameter, but it does not yet have a dedicated custom control in the main UI.
- `Learn/Relearn` is runtime-only in MVP3. It is not stored in project state or plugin presets yet.
- MVP4 has passed build/tests and an initial user check, but it still needs a longer FL Studio acceptance pass.
- FL sidechain setup still depends on host wrapper input mapping. The plugin cannot force FL Studio to auto-map wrapper inputs from inside VST3.
- No installer or one-click deployment flow exists yet. Packaging is script-based.

## First Steps Tomorrow

1. Reopen FL Studio and verify the packaged `0.4.0` plugin loads without stale cache confusion.
2. Run a short FL smoke test: multiple instances, project save/load, reopen, resize UI, sidechain route, render/export.
3. Decide whether MVP4 needs UI controls for pseudo-double and advanced spatial parameters before moving on.
4. If MVP4 is accepted, start the next planned stage from `StageMind_TZ_final.md` and keep the same rule: discuss uncertain product ideas before implementing them.

