# Codex Workflow

## Initial setup

After placing this pack in the project root, Codex should discover:

- Root instructions from `AGENTS.md`.
- Repo skills from `.agents/skills/*/SKILL.md`.

If a skill does not appear, restart Codex.

## Typical feature workflow

Use this order:

1. Clarify product behavior in `docs/PLUGIN_SPEC.md`.
2. Use `$juce-plugin-architecture` to decide where the feature belongs.
3. Use `$dsp-parameter-design` if the feature touches parameters, automation, state, presets, smoothing, gain, bypass, or DSP controls.
4. Use `$realtime-audio-safety` if the feature touches audio callback, DSP, buffers, threading, atomics, or communication between audio and UI.
5. Implement the smallest working slice.
6. Build.
7. Test manually or add focused tests.
8. Use `$cpp-production-engineering`.
9. Use `$code-review-and-quality` if installed from external skill packs.
10. Update docs.

## UI feature workflow

Use this order:

1. Update `docs/DESIGN_DIRECTION.md` if visual language changes.
2. Use `$plugin-ui-fancy-controls` for custom knobs, sliders, buttons, panels, meters.
3. Use `$plugin-gui-animation-system` for animation timing, repaint behavior, glow, meter motion.
4. Implement one visual component at a time.
5. Check repaint cost and UI responsiveness.
6. Confirm audio is not affected.
7. Test small and large plugin window sizes if resizing is supported.

## Release workflow

Use this order:

1. Use `$host-compat-release-qa`.
2. Build clean.
3. Validate plugin.
4. Test FL Studio.
5. Test state save/load.
6. Test automation.
7. Test UI open/close while audio is playing.
8. Test sample-rate and buffer-size changes.
9. Check CPU usage.
10. Check installer/package if applicable.

## Suggested Codex prompts

### Start project architecture

```text
Use $juce-plugin-architecture, $dsp-parameter-design and $realtime-audio-safety.
Review the current project and propose the initial architecture for a production JUCE VST3 plugin.
Update docs/PROJECT_CONTEXT.md and docs/PLUGIN_SPEC.md if needed.
Do not implement yet.
```

### Implement a DSP feature

```text
Use $dsp-parameter-design, $realtime-audio-safety and $cpp-production-engineering.
Implement [feature].
Keep DSP testable without the editor.
Preserve realtime safety.
Build and report verification.
```

### Implement a fancy knob

```text
Use $plugin-ui-fancy-controls and $plugin-gui-animation-system.
Implement a premium rotary knob with hover glow, active arc, value readout and smooth drag feedback.
Keep repaint cost controlled.
Do not touch DSP.
```

### Pre-release audit

```text
Use $host-compat-release-qa, $realtime-audio-safety and $cpp-production-engineering.
Audit this plugin for release blockers, prioritizing FL Studio VST3 behavior.
Return blockers first, then fixes.
```
