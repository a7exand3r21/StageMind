# StageMind Node 0.8.8 RC

## What Changed

0.8.8 fixes the Director selection workflow and makes the correction view less jumpy.

- Director layout now has three zones: Stage View, selected Node controls, correction/status.
- Clicking a Node selects it and fills the middle control panel.
- The selected Node panel can remotely control Pan, Width, Depth, Motion, Clean Up, Resonance, and SC Amount.
- The Stage View hitbox is wider and includes the width line, so selecting a Node should be easier.
- Director conflict history keeps more rows and keeps resolved rows visible longer.
- Director Auto uses a shorter guarded cooldown so it can continue through newly found unresolved conflicts faster.

## FL Test

1. Insert several StageMind Nodes on mixer tracks.
2. Insert one more StageMind Node and switch it to `Mode: Director`.
3. Keep all instances on `Group 1`.
4. Click a point or its width line in Director Stage View.
5. Expected: the middle panel shows the selected Node and enables sliders.
6. Move Pan/Width/Depth/Motion/Clean Up/Resonance/SC Amount in the middle panel.
7. Expected: the target Node changes, not the Director instance.
8. Let playback run with Auto enabled.
9. Expected: correction rows should not blink away instantly; resolved rows stay visible long enough to read.

## Ride Note

This build is a practical intermediate step, not full song-timeline Ride Memory.

Applied corrections are saved as normal target Node parameter state. Director also keeps a runtime history of found/resolved conflicts. It does not yet store conflict events against host playhead positions across the whole arrangement.
