---
name: juce-plugin-architecture
description: Use when designing, reviewing, or changing the architecture of a JUCE C++ VST3 audio plugin, including AudioProcessor, AudioProcessorEditor, DSP modules, parameter layout, preset/state storage, UI boundaries, CMake/JUCE project structure, and FL Studio host integration. Do not use for generic UI styling unless architecture is affected.
---

# JUCE Plugin Architecture

You are guiding architecture for a production-grade JUCE C++ VST3 plugin.

## Apply this skill when

- Creating a new JUCE plugin project.
- Refactoring plugin structure.
- Adding a new DSP module.
- Adding or changing parameters.
- Changing `AudioProcessor` or `AudioProcessorEditor`.
- Designing state/preset handling.
- Separating UI and DSP responsibilities.
- Reviewing whether the codebase is maintainable.

## First actions

Before modifying code:

1. Read `AGENTS.md`.
2. Read `docs/PROJECT_CONTEXT.md`.
3. Read `docs/PLUGIN_SPEC.md`.
4. Inspect the current file tree.
5. Identify the JUCE version and project layout.
6. Identify build system: CMake, Projucer, or other.
7. Identify the current plugin processor/editor classes.
8. State the architecture assumption before editing.

## Architecture targets

The project should converge toward this separation:

- Processor layer:
  - Owns plugin parameters.
  - Owns DSP engine.
  - Handles prepare/reset/process.
  - Handles state save/load.
  - Contains no visual rendering logic.

- DSP layer:
  - Contains audio algorithms.
  - Can be tested without UI.
  - Uses prepared resources.
  - Allocates outside realtime processing.
  - Has explicit sample-rate and block-size lifecycle.

- Parameter layer:
  - Centralized parameter IDs.
  - Centralized parameter layout.
  - Display formatting near parameter definitions.
  - Automation behavior documented.
  - Stable IDs.

- UI layer:
  - Owns component composition.
  - Owns painting and interaction.
  - Does not own DSP truth.
  - Uses attachments or safe snapshots.
  - Reads theme tokens from a centralized theme system.

- State/preset layer:
  - Versioned.
  - Backward compatible.
  - Independent of editor lifetime.
  - Tested manually or automatically.

## File organization guidance

Prefer a structure like:

```text
Source/
  PluginProcessor.h/.cpp
  PluginEditor.h/.cpp

  Parameters/
    ParameterIds.h
    ParameterLayout.h/.cpp
    ParameterFormatting.h/.cpp

  DSP/
    DspEngine.h/.cpp
    modules/

  State/
    PluginState.h/.cpp
    PresetState.h/.cpp

  UI/
    Theme.h/.cpp
    LookAndFeel/
    Components/
    Controls/
    Animation/
    Layout/

  Tests/
```

Do not force this exact structure if the existing project has a clean equivalent. Preserve working conventions when they are sane.

## Rules

- Keep `processBlock` short and predictable.
- Do not put rendering, layout, or animation in processor code.
- Do not put DSP decisions in custom controls.
- Do not make UI construction depend on audio buffer contents.
- Do not hide parameter IDs inside controls.
- Do not rename parameter IDs without a migration plan.
- Do not introduce global mutable state.
- Prefer clear ownership over clever convenience.

## Change process

When changing architecture:

1. Name the module boundary being changed.
2. Explain why the existing boundary is insufficient.
3. Propose the smallest structural change.
4. List files to touch.
5. Implement.
6. Build.
7. Update docs if the architecture decision matters.

## Output expected from Codex

When using this skill, return:

- Current architecture summary.
- Proposed change.
- Realtime-safety implications.
- Files changed.
- Build/test result.
- Follow-up risks.
