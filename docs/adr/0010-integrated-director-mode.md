# ADR 0010: Integrated Director Mode

Date: 2026-06-11

## Status

Accepted.

## Context

The first Director prototype was a second VST3 target. That created two installation artifacts and pushed us toward a shared-memory registry so the two plugin modules could see each other.

That is more moving parts than the product needs right now. FL Studio users should install one plugin. The existing process-local Link registry is enough when every instance comes from the same `StageMind Node.vst3` module.

Automatic master-insert detection was considered. It is not reliable enough as a product rule. VST3 hosts do not expose a stable cross-host "this instance is on the master" signal, and FL-specific guessing would be fragile.

## Decision

StageMind Director is implemented as a selectable mode inside `StageMind Node.vst3`.

Add a host-facing `plugin_mode` choice parameter at the end of the existing APVTS layout:

- `Node`
- `Director`

In `Node` mode, processing is unchanged.

In `Director` mode, the processor passes audio through unchanged, disables Link publishing for this instance, and lets the editor read the selected Link group on the UI thread. The Director view is observe-only.

The Link registry stays process-local and fixed-size as described in ADR 0005. No shared-memory registry is used for this stage.

## Consequences

- Users install one VST3 artifact.
- Existing projects default to `Node`.
- Director mode can be placed on any insert, including master, but the user chooses the mode manually.
- Director mode cannot see Node instances loaded from a different plugin binary or host process.
- No audio is transferred through Link.
- No other Node is controlled or edited by Director in 0.7.0.
