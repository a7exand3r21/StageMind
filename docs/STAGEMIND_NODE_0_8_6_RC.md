# StageMind Node 0.8.6 RC

## What Changed

0.8.6 removes setup friction for normal testing.

- New Nodes start with `Link` enabled.
- New Nodes and Directors start in `Group 1`.
- New instances start with `Auto Assist` set to `Auto`.
- Nodes publish an idle zero-activity Link heartbeat from the processor timer.
- Director can see installed Nodes before playback starts.
- If the host releases a plugin instance's resources, the Node unpublishes from Link until it is prepared/processed again.

Live audio Link snapshots still take priority while audio is processing.

## FL Test

1. Insert StageMind Node on two mixer tracks.
2. Insert one more StageMind Node and switch it to `Mode: Director`.
3. Do not touch Link, Group, or Auto.
4. Expected: Director is already on Group 1 and sees the two Nodes.
5. Start playback.
6. Expected: activity meters/conflicts use live data, not the idle heartbeat.

## VST File Replacement Note

StageMind unregisters from its in-process Link registry when FL actually destroys the plugin instance.

It cannot force FL Studio to unload the VST3 DLL when a slot is disabled. If Windows locks the VST3 file, close FL Studio or test from a new versioned VST3 folder.
