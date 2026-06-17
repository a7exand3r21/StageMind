# StageMind Node 0.5.7 RC

Date: 2026-06-10

## Status

0.5.7 is a Link Actions UX fix.

## What Changed

`Apply Tip` now checks the current parameter values before enabling itself.

If the visible suggestion is already active, the suggestion line and button show `Applied`, and the button stays disabled. Example: when a Bass node already has `SC Mode` set to `Kick Ducks Bass`, the UI no longer offers a duplicate apply action.

## What It Does Not Do

- It does not add automatic Link correction.
- It does not transfer audio between instances.
- It does not change sidechain routing.
- It does not change DSP behavior.

## Test Focus

- Two linked instances still see each other.
- `Apply Tip` is enabled only when a suggestion has a pending action.
- The suggestion line and `Apply Tip` show `Applied` when the current controls already match the action.
- Saved FL projects still restore Link group, Source role, and changed parameters.
