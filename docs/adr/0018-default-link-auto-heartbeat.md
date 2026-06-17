# ADR 0018: Default Link/Auto and Idle Link Heartbeat

## Status

Accepted.

## Context

Manual setup slowed FL testing. Every new Node needed Link enabled, a group number, and Auto Assist changed to Auto before Director could do useful work.

Director also depended on Nodes publishing during audio processing. A newly inserted Node could be invisible until playback or processing happened.

## Decision

Change new-instance defaults:

- `link_enabled = true`;
- `link_group = 1`;
- `auto_assist_mode = Auto`.

Add an idle Link heartbeat from the processor timer. It publishes role, group, sidechain state, Auto mode, and macro values with zero activity. Audio processing publishes live snapshots and suppresses the idle heartbeat for a short timer window so live activity is not overwritten by idle state.

## Consequences

- New Nodes are visible to Director immediately in Group 1.
- New Director instances start in the same Group 1 after switching mode.
- Existing saved projects keep their stored values.
- Conflict detection still needs real activity and spectral data.
- If the host releases plugin resources, the instance unpublishes from Link until it is prepared/processed again.
- The plugin cannot force FL Studio to unload its VST3 module; host lifetime remains host-owned.
