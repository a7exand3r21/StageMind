# StageMind Node 0.6.2 RC

Date: 2026-06-10

## Status

0.6.2 stabilizes Link Assist Action Preview.

## What Changed

The preview line now holds the last active Link suggestion briefly, so it does not flicker when activity or spectral overlap moves around the threshold.

After a user-approved action is applied, the preview line shows:

```text
Resolved
```

The action button still changes only the expected existing parameter.

## What It Does Not Do

- It does not add Auto Assist.
- It does not apply corrections without a user click.
- It does not transfer audio between instances.
- It does not enable sidechain routing or `SC Enable`.

## Test Focus

- Preview line stays visible while the same conflict is active.
- Preview line shows `Resolved` after pressing the action button.
- Sidechain routing and `SC Enable` remain manual.
- No audio behavior changes.
