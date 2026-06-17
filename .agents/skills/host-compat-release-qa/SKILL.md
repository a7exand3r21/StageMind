---
name: host-compat-release-qa
description: Use before releasing or packaging a JUCE C++ VST3 plugin, or when checking FL Studio compatibility, DAW save/load, automation, plugin validation, CPU stability, sample-rate changes, buffer-size changes, UI open-close behavior, installer/package readiness, and release blockers.
---

# Host Compatibility and Release QA

A plugin is not ready because it builds. It is ready when hosts behave.

## Apply this skill when

- Preparing a release.
- Testing FL Studio.
- Testing VST3 packaging.
- Checking plugin validation.
- Testing automation.
- Testing presets/state.
- Testing UI open/close.
- Testing sample rate or buffer size changes.
- Investigating host-specific bugs.
- Creating release notes or QA checklist.

## Primary host

FL Studio is the primary target.

Always prioritize:

- Plugin scan/load.
- Insert/remove/reinsert.
- UI open/close.
- Automation recording/playback.
- Project save/reopen.
- Render/export.
- Preset changes.
- Bypass.
- CPU stability.

## Validation checklist

Before release:

1. Clean build.
2. Release build.
3. Plugin metadata correct.
4. VST3 bundle/path correct.
5. Plugin validation passes.
6. FL Studio loads plugin.
7. Audio passes through or generates sound as expected.
8. Parameters appear correctly.
9. Automation works.
10. State save/load works.
11. Presets work.
12. UI open/close is stable.
13. No obvious CPU spikes.
14. Sample-rate changes work.
15. Buffer-size changes work.
16. Offline render works if relevant.
17. Secondary DAW sanity check done.

## FL Studio manual test script

Use this script unless the project defines a better one:

1. Open FL Studio.
2. Add plugin to mixer insert or channel rack depending on plugin type.
3. Confirm plugin UI opens.
4. Move every visible control.
5. Check values update correctly.
6. Record automation for key controls.
7. Playback automation.
8. Save project.
9. Close FL Studio.
10. Reopen project.
11. Confirm state restored.
12. Export/render a short section.
13. Confirm output is correct.
14. Open/close UI repeatedly during playback.
15. Remove plugin and undo if possible.
16. Test bypass.

## Regression checks

After every significant change:

- No parameter IDs changed accidentally.
- Old presets still load if any exist.
- DSP still initializes correctly.
- UI does not require processor state that is unavailable during construction.
- No debug logs in realtime path.
- No assertions during common host operations.

## Release blocker severity

Critical:

- Crash.
- Audio dropouts caused by plugin.
- State not saved/restored.
- Automation broken.
- Plugin fails validation.
- CPU spikes during silence or normal playback.
- Realtime safety violation.

High:

- UI freezes.
- Controls show wrong values.
- Presets unreliable.
- Resize breaks layout.
- Output clips unexpectedly.

Medium:

- Visual polish issues.
- Minor animation stutter.
- Missing tooltip/readout.
- Non-blocking warning.

## Output expected from Codex

When using this skill, return:

- Release readiness status.
- Critical blockers.
- High/medium issues.
- Tests performed.
- Tests still needed.
- Recommended next fixes.
