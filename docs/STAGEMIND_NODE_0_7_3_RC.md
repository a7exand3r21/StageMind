# StageMind Node 0.7.3 RC

Date: 2026-06-11

## Status

0.7.3 makes Director easier to read and easier to navigate across groups.

## What Changed

- Director scene now draws active conflict arrows.
- Arrows point from the peer/source Node to the target Node.
- Scene arrows use short labels such as `Kick/Bass`, `Width`, or `Space`.
- Director shows a compact overview of Groups 1-16.
- Group overview shows node count and active-node count as `nodes/active`.
- Previous/next group buttons jump to linked groups when they exist.

## What It Does Not Do

- It does not add Auto Assist.
- It does not apply commands without a user click.
- It does not transfer audio between instances.
- It does not change FL Studio routing.

## Test Focus

- Put linked Nodes in two different groups.
- Confirm Director shows the group overview for Groups 1-16.
- Use the group arrow buttons and confirm they jump to non-empty groups.
- Create a conflict and confirm a scene arrow appears between the two Node dots.
- Confirm the arrow direction matches the text row: `target <- peer`.
- Clear the conflict and confirm the arrow disappears while the text row can remain `resolved`.
