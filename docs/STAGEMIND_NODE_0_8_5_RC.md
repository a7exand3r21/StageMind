# StageMind Node 0.8.5 RC

## What Changed

0.8.5 makes Director Auto independent of the Director UI window.

- Director Auto command selection now runs from the Director processor timer.
- The audio callback is still unchanged.
- Director sends at most one guarded auto command per cooldown window.
- Target Nodes still apply automatic commands only when their own Auto Assist is `Auto`.
- The editor keeps showing the group map, conflict history, and manual `Apply to #...`, but it is no longer responsible for sending auto commands.

## FL Test

1. Put one instance in `Mode: Director`.
2. Put several Nodes in the same Link Group.
3. Set Director `Auto` to `Auto`.
4. Set target Nodes `Auto` to `Auto`.
5. Start playback, then close the Director UI window.
6. Expected: target Nodes still receive guarded auto corrections after conflicts stabilize.
7. Reopen Director.
8. Expected: corrected rows show as `resolved`.

Manual `Apply to #...` is unchanged.
