---
name: dsp-parameter-design
description: Use when defining or changing plugin parameters, DSP controls, automation behavior, smoothing, value ranges, units, gain staging, bypass, wet/dry, preset/state serialization, parameter IDs, display formatting, or modulation behavior in a JUCE C++ VST3 plugin.
---

# DSP Parameter Design

Parameters are product API. Treat them as stable contracts.

## Apply this skill when

- Adding a parameter.
- Renaming a parameter.
- Changing a range/default/unit.
- Changing display formatting.
- Connecting UI controls to DSP.
- Adding automation.
- Adding preset/state save-load behavior.
- Adding bypass, wet/dry, input/output gain.
- Adding smoothing.
- Changing gain staging.

## Parameter definition checklist

Every parameter must have:

- Stable ID.
- Human-readable name.
- Unit.
- Minimum value.
- Maximum value.
- Default value.
- Mapping type:
  - linear,
  - logarithmic,
  - skewed,
  - stepped,
  - enum.
- Display formatting.
- Automation support.
- Smoothing policy.
- UI control type.
- Preset/state inclusion.
- Backward compatibility behavior.

## Stable IDs

Rules:

- Do not rename IDs casually.
- Display names may change. IDs are harder to change.
- If an ID must change, write a migration plan.
- Keep IDs centralized.
- Avoid IDs based on visual placement, such as `knob1`.
- Prefer semantic IDs, such as `drive`, `mix`, `attackMs`, `cutoffHz`.

## Ranges

Use musical ranges:

- Gain: usually dB, not raw linear, unless internal.
- Frequency: usually log/skewed.
- Time: ms or seconds, often skewed.
- Mix: percent or normalized.
- Ratio: stepped or constrained.
- Mode: enum/choice.

Avoid meaningless `0.0..1.0` display unless the control is truly abstract.

## Smoothing

Smooth any parameter that can cause audible discontinuity.

Common smoothing candidates:

- Gain.
- Mix.
- Frequency.
- Drive.
- Feedback.
- Delay time.
- Modulation depth.
- Filter resonance.
- Bypass transitions.

Do not smooth parameters where immediate discrete change is musically expected, unless a transition is needed to avoid clicks.

## Automation

Host automation must be predictable:

- Fast automation should not click.
- Display values should match actual behavior.
- UI should reflect automation without fighting user input.
- Parameter changes should not allocate.
- Automation should not require editor to be open.

## State and presets

Rules:

- State must be saved/restored without editor.
- Presets must be versioned.
- New parameters need safe defaults for old presets.
- Removed parameters need migration handling or explicit deprecation.
- Do not store derived runtime-only DSP internals as preset truth unless needed.

## Gain staging

For any effect that changes gain:

- Define input level behavior.
- Define output compensation if relevant.
- Avoid unexpected clipping.
- Consider headroom.
- Make meters meaningful.
- Document whether output is compensated, raw, or user-controlled.

## Output expected from Codex

When using this skill, return:

- Parameter table.
- Range/mapping reasoning.
- Smoothing decisions.
- Automation behavior.
- State/preset impact.
- UI control mapping.
- Risks.
