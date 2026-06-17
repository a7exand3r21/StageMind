# StageMind Node 0.5.6 RC

Date: 2026-06-10

## Status

0.5.6 adds user-approved Link Actions.

## What Changed

The Link diagnostics area now has an `Apply Tip` button.

When a Link Suggestion is active, the button can apply a small safe change:

- `Tip: reduce Width` lowers Width.
- `Tip: Kick Ducks Bass` selects the Kick Ducks Bass sidechain mode.
- `Tip: make vocal room` lowers Width and raises Depth.
- `Tip: avoid double-wide` lowers Width.

## What It Does Not Do

- It does not apply anything automatically.
- It does not enable sidechain routing.
- It does not transfer audio between instances.
- It does not create Director behavior.

## Test Focus

- Button is disabled when no suggestion is active.
- Button becomes enabled when a suggestion appears.
- Clicking it changes only the expected visible parameter.
- Project save/reopen preserves applied parameter values.
- Render/export is unchanged.
