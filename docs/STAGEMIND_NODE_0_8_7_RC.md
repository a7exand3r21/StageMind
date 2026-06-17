# StageMind Node 0.8.7 RC

## What Changed

0.8.7 makes Director mode a basic remote stage controller.

- Click a Node in Director Stage View to select it.
- The Director side panel shows selected Node role, pan, depth, width, activity, correlation, and Auto mode.
- Drag the selected Node in Stage View to change that target Node's `pan` and `depth`.
- The target Node applies the move on its processor timer via normal APVTS parameter writes.
- Director Stage View now places Nodes horizontally by effective `pan`, not by role lanes.

Width remains display-only in this build. The horizontal line around the dot shows the target Node's width, but dragging left/right changes pan.

## FL Test

1. Insert StageMind Node on two mixer tracks.
2. Insert one more StageMind Node and switch it to `Mode: Director`.
3. Keep all instances on `Group 1`.
4. Click a Node in the Director Stage View.
5. Expected: the selected Node gets a warm outline and the right panel shows its values.
6. Drag it left/right.
7. Expected: the target Node's pan changes and the sound moves left/right.
8. Drag it front/back.
9. Expected: the target Node's depth changes.
10. Save and reopen the FL project.
11. Expected: moved pan/depth values are restored because the target Node owns the saved parameters.

## Limits

- Director still does not transfer audio.
- Director drag does not control Width yet.
- If two Nodes have the same pan/depth, they may overlap visually. Click selects the nearest drawn Node.
- The scene updates from the target Node's published state, so movement may feel slightly stepped at the current control timer rate.
