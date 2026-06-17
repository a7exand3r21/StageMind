# ADR 0011: Director Uses User-Approved Commands

Date: 2026-06-11

## Status

Accepted.

## Context

Director can now see linked Nodes, show a scene, and keep conflict history. The tempting next step is automatic correction, but the rules are not mature enough for silent cross-instance changes.

We still want Director to become useful before full auto-assist. The safer step is an explicit command: the user sees the active conflict, sees the target instance ID, and clicks a button.

## Decision

Director may send a one-shot command through the existing process-local Link registry.

The target Node consumes the command from its processor timer and writes normal APVTS parameters on the message thread. The audio callback never consumes commands and never writes parameters.

Allowed command actions are the existing Link Assist actions:

- reduce Width;
- adjust Width and Depth for space;
- select a sidechain mode.

Sidechain actions set `SC Mode` only. Routing and `SC Enable` remain manual.

## Consequences

- Director can now control a linked Node, but only after a user click.
- No new parameter IDs are introduced.
- No audio is transferred through Link.
- Commands work only between active instances in the same plugin process.
- Future automatic correction still requires a separate decision and stronger guardrails.
