---
name: cpp-production-engineering
description: Use when writing, refactoring, or reviewing modern C++ code for the JUCE VST3 plugin, especially ownership, RAII, headers, compile times, warnings, tests, error handling, CMake/JUCE build hygiene, naming, constants, and maintainability. Use before committing non-trivial C++ changes.
---

# C++ Production Engineering

Write boring, clear, shippable C++.

## Apply this skill when

- Adding C++ classes.
- Refactoring C++ modules.
- Reviewing generated code.
- Fixing warnings.
- Changing headers.
- Changing CMake/build setup.
- Adding tests.
- Simplifying LLM-generated code.
- Preparing a commit.

## C++ rules

Prefer:

- RAII.
- Value types where practical.
- `std::unique_ptr` for exclusive ownership.
- `std::array` or fixed-size storage in performance-sensitive paths.
- `std::vector` with allocation outside realtime paths.
- `constexpr` for constants.
- `enum class` for modes.
- Small classes with clear responsibility.
- Clear names with units: `attackMs`, `cutoffHz`, `gainDb`.

Avoid:

- Raw owning pointers.
- Global mutable state.
- Deep inheritance trees.
- Hidden allocations.
- Unnecessary templates.
- Overloaded abstractions created too early.
- Catch-all exception handling around realtime code.
- Macros unless required by JUCE/project convention.

## Header hygiene

- Keep headers minimal.
- Forward declare where reasonable.
- Avoid including heavy JUCE headers in every custom header unless project convention requires it.
- Keep implementation details in `.cpp`.
- Avoid circular dependencies.
- Prefer stable interfaces between DSP, parameters, and UI.

## Error handling

- In realtime code, prevent invalid states before processing.
- In setup/config code, fail clearly.
- Do not throw from audio callback.
- Do not hide errors with silent defaults unless documented.
- Use assertions for programmer errors, but do not depend on assertions for release safety.

## Testing

Add tests where practical for:

- Parameter mapping.
- State serialization.
- DSP helper functions.
- Preset migration.
- Value formatting.
- Bypass/mix math.
- Envelope/smoothing behavior.

If no test framework exists, write a manual verification note and suggest a minimal test approach.

## Build quality

Before declaring done:

- Build the project.
- Report exact build command used.
- Report warnings.
- Fix new warnings unless there is a documented reason.
- Avoid adding dependencies without need.
- Avoid changing formatting across unrelated files.

## Review checklist

Before final response:

1. Does the code compile?
2. Are ownership rules clear?
3. Are headers clean?
4. Are names meaningful?
5. Are units explicit?
6. Is realtime code safe?
7. Is UI code separated from DSP?
8. Are tests or verification included?
9. Did scope creep happen?
10. Are docs updated if behavior changed?

## Output expected from Codex

When using this skill, return:

- C++ changes made.
- Build command/result.
- Warnings fixed or remaining.
- Tests/verification.
- Any technical debt introduced.
