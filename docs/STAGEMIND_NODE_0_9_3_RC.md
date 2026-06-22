# StageMind Node 0.9.3 RC

0.9.3 fixes editor window size restore.

## What changed

- The plugin stores `editor_width` and `editor_height` in project state.
- Reopening the plugin editor should restore the last resized window size.
- Saving and reopening the FL project should also restore the saved editor size.
- Version shown by the plugin moves to 0.9.3.
- State format moved to version 6.

## What to test

1. Insert StageMind Node.
2. Resize the plugin window.
3. Close the plugin window.
4. Open the same instance again.
5. Confirm the previous size is restored.
6. Save the FL project.
7. Close and reopen the project.
8. Open the same plugin instance.
9. Confirm the window size is still restored.

## Feedback captured for next DSP/UX pass

- Width needs a more obvious range.
- Depth needs a clearer front/back effect.
- Motion is understandable enough for now.
- Clean Up / Resonance are more understandable than Space/Double.
- Double needs a clearer audible identity and UI explanation.
- Factory presets should sound more different.
- Sidechain / Make Space needs clearer wording and workflow.
