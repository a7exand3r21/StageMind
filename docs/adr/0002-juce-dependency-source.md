# ADR 0002: JUCE Dependency Source

## Status

Accepted

## Context

The repository did not contain JUCE source code. MVP 1 needs a reproducible CMake-based VST3 build.

## Decision

Use CMake `FetchContent` to download JUCE from the official `juce-framework/JUCE` GitHub repository.

The pinned tag is `8.0.13`.

## Consequences

The first configure step needs network access.

The repository stays small and does not vendor JUCE source.

Builds depend on the pinned JUCE tag rather than a moving branch.
