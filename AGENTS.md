# AGENTS.md

## Project identity

This repository is for a production-grade C++ audio plugin, targeting VST3 usage in FL Studio.

Default assumptions unless the project says otherwise:

- Framework: JUCE.
- Language: modern C++.
- Build system: CMake or JUCE-managed project files.
- Main deliverable: VST3 plugin.
- Host priority: FL Studio first, then at least one secondary DAW for sanity checks.
- Product goal: stable audio behavior, polished visual design, responsive animated UI, clean codebase.

Do not treat this as a toy plugin or demo. Build as if the plugin will be shipped to users.

## How Codex should work in this repo

Before changing code:

1. Read this file.
2. Check `docs/PROJECT_CONTEXT.md`.
3. Check `docs/PLUGIN_SPEC.md`.
4. Check existing ADRs or notes if present.
5. Identify which skills apply.
6. State the planned files to touch before large edits.
7. Keep changes focused.

Use these repo skills aggressively:

- `$juce-plugin-architecture` for plugin structure, JUCE module boundaries, processor/editor separation, state handling.
- `$realtime-audio-safety` before touching audio callback, DSP, parameter reads, smoothing, buffers, allocations, locks, atomics, threading.
- `$dsp-parameter-design` for parameters, automation, preset serialization, smoothing, bypass, gain staging.
- `$cpp-production-engineering` for C++ code quality, ownership, RAII, compile-time hygiene, tests, warnings.
- `$plugin-ui-fancy-controls` for knobs, sliders, visual states, glow, meters, custom controls.
- `$plugin-gui-animation-system` for animation, repaint budgets, microinteractions, frame timing.
- `$host-compat-release-qa` before release, packaging, validation, DAW checks, regression testing.

## Hard rules

### Audio thread

Never do these in the audio callback or realtime DSP path:

- Heap allocation.
- File I/O.
- Network I/O.
- Logging.
- UI calls.
- Blocking locks.
- Waiting on futures/promises.
- Calling code with unknown realtime behavior.
- Creating or destroying heavyweight objects.
- Unbounded loops.
- Throwing exceptions.

If a change might affect the audio thread, apply `$realtime-audio-safety`.

### UI thread

The UI may be fancy, but it must not compromise audio.

Rules:

- UI animation must be timer-driven or host-safe.
- Avoid excessive repaints.
- Do not poll audio data in a way that causes contention.
- Use snapshots, atomics, lock-free FIFOs, or buffered values where needed.
- Keep custom painting predictable.
- Use consistent theme tokens rather than random colors per component.

### C++

Use modern C++ with restrained abstraction.

Prefer:

- RAII.
- Clear ownership.
- `std::unique_ptr` where ownership is exclusive.
- `std::array` or fixed-size storage where possible.
- Small focused classes.
- Compile-time constants for stable design tokens and parameter IDs.
- Explicit units in names when values represent dB, Hz, ms, samples, normalized ranges.

Avoid:

- Clever template machinery unless it pays for itself.
- Global mutable state.
- Raw owning pointers.
- Hidden allocations in hot paths.
- Over-generalized frameworks created before there is real repetition.
- Magic numbers in DSP or UI layout.

### JUCE-specific rules

Keep separation clear:

- `AudioProcessor` owns audio state, parameters, DSP modules, prepare/reset/process.
- `AudioProcessorEditor` owns UI composition only.
- Custom controls own visual behavior and user interaction.
- Theme/design tokens live in their own layer.
- DSP modules must be testable without the editor.
- Parameter definitions must be stable and centralized.
- Preset/state serialization must be versioned.

Use JUCE APIs according to current documentation and project conventions. If unsure, verify before coding.

### Product quality

A feature is not done until:

- It builds.
- It has a focused test or manual verification note.
- It does not violate realtime audio rules.
- Parameter automation works where applicable.
- State save/load works where applicable.
- UI states are covered: idle, hover, active, disabled, focused, automated where relevant.
- The code is understandable to a new developer.

## Prompting convention

When asking Codex to do work, prefer prompts like:

```text
Use $juce-plugin-architecture, $realtime-audio-safety and $dsp-parameter-design.
Implement [feature].
Preserve realtime safety.
Keep UI and DSP separated.
Update docs if decisions change.
```

For UI work:

```text
Use $plugin-ui-fancy-controls and $plugin-gui-animation-system.
Design and implement [control/view].
It should feel premium, dark, responsive, animated, but not expensive to repaint.
```

For release checks:

```text
Use $host-compat-release-qa.
Audit the current plugin for release blockers.
Prioritize FL Studio.
```

## Documentation discipline

Update docs when decisions change:

- `docs/PROJECT_CONTEXT.md` for product/domain context.
- `docs/PLUGIN_SPEC.md` for product behavior.
- `docs/DESIGN_DIRECTION.md` for visual language.
- `docs/CODEX_WORKFLOW.md` for recurring workflow.
- `docs/RELEASE_QA_CHECKLIST.md` for release gates.

If a decision affects architecture, create an ADR in `docs/adr/`.
