# StageMind Node 0.7.2 RC

Date: 2026-06-11

## Status

0.7.2 lets Director send a user-approved correction command to a linked Node.

## What Changed

- Director active conflicts now expose an `Apply to #...` button.
- The command is sent through the existing process-local Link registry.
- Target Node instances consume commands on the message thread, not in the audio callback.
- Supported actions are the same as local Link Assist: Width, Depth, and SC Mode changes.
- A Link registry test covers submit/consume behavior.

## What It Does Not Do

- It does not apply anything automatically.
- It does not transfer audio between plugin instances.
- It does not create FL Studio sidechain routing.
- It does not turn on `SC Enable`.
- It does not auto-detect the master insert.

## Test Focus

- Use at least two linked Node instances and one Director in the same Group.
- Create an active conflict so Director shows `Apply to #...`.
- Press the button and confirm the target Node changes the expected parameter.
- For sidechain tips, confirm only `SC Mode` changes.
- Confirm `SC Enable` and FL routing still stay manual.
- Save/reopen the FL project and confirm the target parameter value is preserved.
- Render/export a short section and confirm audio still renders.
