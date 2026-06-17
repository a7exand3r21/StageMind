# MVP4 / MVP5 Finalization

Date: 2026-06-10

StageMind Node version: 0.5.4

## Decision

MVP4 Spatial and MVP5 Link are finalized for the current Node MVP.

This means the codebase contains explicit implementation for the promised behavior, the implementation matches `docs/PLUGIN_SPEC.md` and `StageMind_TZ_final.md`, the automated DSP/Link tests pass, and the packaged VST3 builds.

This does not mean StageMind Director, automatic Link corrections, audio transfer between instances, or a full 3D room/reverb engine are implemented. Those remain later work.

## MVP4 Spatial Status

Accepted.

Implemented behavior:

- Motion is processed by `MotionProcessor`.
- Motion is role-limited in `PluginProcessor::makeMotionConfig`.
- Motion combines side movement and a small equal-power left/right movement for allowed roles.
- Depth is processed by `DepthProcessor`.
- Depth keeps the dry path undelayed and uses only a small wet/reflection path.
- Pseudo-double is processed by `PseudoDoubleProcessor`.
- Pseudo-double is role-limited in `PluginProcessor::makePseudoDoubleConfig`.
- Correlation safety reduces risky width and also scales motion/pseudo-double through `correlationSafetyScale`.
- Stage View displays effective pan, depth, width, and motion after role/safety/correlation limits.
- Factory programs exist and write existing APVTS parameters.

Evidence in code:

- `Source/DSP/MotionProcessor.cpp`
- `Source/DSP/DepthProcessor.cpp`
- `Source/DSP/PseudoDoubleProcessor.cpp`
- `Source/Plugin/PluginProcessor.cpp`
- `Source/UI/StageView.cpp`
- `Tests/SpatialEnhancementTests.cpp`

User FL check:

- Pan and role-based placement were checked by ear and visually in FL Studio.
- Stage View now reads spatial movement more clearly after the 0.5.4 update.

Known limitation:

- Depth is a lightweight zero-latency spatial cue, not a full reverb or true 3D distance model.
- Pseudo-double is not exposed as a main visible control yet; it is available through APVTS/factory programs and role logic.

## MVP5 Link Status

Accepted as status-only Link.

Implemented behavior:

- Each instance registers with `StageMindLinkRegistry`.
- Link publishes control data only.
- Link reads peers from the same group.
- Link can prefer a source by role.
- `Any Role` falls back to the strongest active peer.
- Missing source role does not silently connect to the wrong role.
- Target routing is supported through target IDs.
- Offline/non-realtime publish is disabled.
- Link diagnostics are shown in the UI.
- Link does not transfer audio.
- Link does not automatically change audio.

Evidence in code:

- `Source/Link/StageMindLinkRegistry.h`
- `Source/Link/StageMindLinkRegistry.cpp`
- `Source/Link/LinkActivityEnvelope.cpp`
- `Source/Plugin/PluginProcessor.cpp`
- `Source/Plugin/PluginEditor.cpp`
- `Tests/StageMindLinkRegistryTests.cpp`
- `Tests/LinkActivityEnvelopeTests.cpp`

User FL check:

- Two/three plugin instances in one FL project were checked.
- Link enables correctly.
- Group saves after project reopen.
- Source role saves after project reopen.
- One Node sees another by role.
- Missing source role shows a warning.
- Export/render does not break.

Known limitation:

- MVP5 Link is diagnostics/status only. It does not correct width, depth, resonance, pan, gain, or sidechain automatically.
- Spectral peer analysis is not implemented yet.

## Verification

Commands run:

```text
scripts\dev-cmd.cmd cmake --build build --target StageMindDSPTests
build\StageMindDSPTests.exe
scripts\build-and-package-vst3.cmd Debug
```

Result:

```text
StageMindDSPTests passed
Packaged runtime VST3: dist\VST3\StageMind Node.vst3
moduleinfo.json version: 0.5.4
```

## Next Recommended Stage

Do not jump straight to StageMind Director.

Recommended next stage:

1. Build a small `0.5.5 RC` package/checkpoint.
2. Clean up tester-facing docs and known limitations.
3. Start Link Suggestions as the bridge toward future automatic corrections.

Link Suggestions should suggest changes first. It should not silently change audio yet.
