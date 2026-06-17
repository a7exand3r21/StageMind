# ADR 0016: Guarded Director Auto Commands

## Status

Accepted.

## Context

Local Auto Assist can already resolve conflicts on a Node, but Director only had user-approved commands. That left a workflow gap: Director could see conflicts across the group, while each target Node still had to wait for its own local scan or manual action.

The risky version would let Director silently change any target. That is too broad for this stage.

## Decision

Director uses the existing `auto_assist_mode` parameter.

When Director is set to `Auto`, it may send one bounded Link command for an active unresolved conflict. The command includes:

- Director instance ID;
- target action kind;
- conflict peer instance ID;
- automatic/manual flag.

The target Node consumes the command on its message-thread timer. It applies an automatic command only when the target Node is also set to `Auto`. Sidechain actions go through the existing Auto Link path so the peer activity envelope becomes the control source.

Manual Director commands remain unchanged and conservative.

## Consequences

- No new APVTS parameter is introduced.
- Director Auto cannot override a Node whose Auto Assist is Off or Suggest.
- Automatic sidechain setup can happen without FL manual sidechain routing because it uses StageMind Link control data.
- Director Auto was introduced as a guarded command policy in 0.8.4. In 0.8.5, command selection moved to the Director processor timer so the Director UI does not need to stay open.
- Audio-thread behavior is unchanged except for existing bounded Link snapshot publishing.
