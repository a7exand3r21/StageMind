# StageMind Node 0.6.1 RC

Date: 2026-06-10

## Status

0.6.1 adds Link Assist Action Preview.

## What Changed

Link Assist actions now expose a short preview line before the button is pressed.

Examples:

- `Width -> 55%`
- `Width 58%, Depth 48%`
- `SC Mode -> Kick Duck`

Sidechain actions add a guardrail message when needed:

- routing stays manual;
- `SC Enable` stays manual;
- no host sidechain route is created.

## What It Does Not Do

- It does not add Auto Assist.
- It does not apply corrections without a user click.
- It does not transfer audio between instances.
- It does not enable sidechain routing or `SC Enable`.
- It does not create StageMind Director behavior.

## Test Focus

- Suggestion line still names the conflict.
- Preview line matches the action button.
- Sidechain actions warn that routing and `SC Enable` stay manual.
- Pressing the button changes only the expected existing parameter.
- Applied state still disables duplicate clicks.
