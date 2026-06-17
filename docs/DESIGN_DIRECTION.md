# Design Direction

## Visual target

Premium, dark, musical, tactile.

The UI should feel like a high-end audio tool, not a generic desktop app.

Keywords:

- Dark interface.
- Controlled glow.
- Depth.
- Sharp typography.
- Clean hierarchy.
- Smooth knob motion.
- Rich but not noisy.
- Animated feedback.
- Subtle lighting.
- Clear metering.
- Studio-grade.

## Avoid

- Random neon everywhere.
- Too many gradients fighting each other.
- Tiny unreadable labels.
- Controls without clear active state.
- Heavy shadows that make values harder to read.
- Animation that looks impressive once but annoys users after five minutes.
- GPU/CPU-heavy effects without purpose.
- Copying another plugin’s exact look.

## Theme tokens

Create a centralized theme system.

Suggested tokens:

- Background base.
- Background elevated.
- Panel surface.
- Panel border.
- Text primary.
- Text secondary.
- Text muted.
- Accent primary.
- Accent secondary.
- Warning.
- Error.
- Glow soft.
- Glow strong.
- Shadow low.
- Shadow high.
- Meter cold.
- Meter warm.
- Meter hot.
- Knob track.
- Knob arc inactive.
- Knob arc active.

Never hardcode colors in individual components unless there is a documented reason.

## Layout principles

- Make the most important controls largest.
- Group related controls.
- Leave breathing room.
- Prefer consistent alignment over decorative chaos.
- Use value readouts where precision matters.
- Make bypass and output safety states obvious.
- Keep the plugin usable at the smallest supported size.

## Control design

Knobs:

- Circular or arc-based value indication.
- Clear pointer or active ring.
- Hover glow.
- Drag intensity feedback.
- Double-click reset if implemented.
- Fine adjustment gesture if implemented.
- Value bubble or inline value display.

Sliders:

- Clear filled track.
- Active thumb state.
- Smooth value motion.
- Readable value.

Meters:

- Smooth rise.
- Musical falloff.
- Peak hold if useful.
- Color change near danger zone.
- No direct UI access to audio buffers.

Switches/buttons:

- Strong difference between on/off.
- Hover and press states.
- Bypass should be unmistakable.

## Motion principles

Good motion is short and meaningful.

Suggested ranges:

- Hover fade: 80-160 ms.
- Press feedback: 40-100 ms.
- Panel reveal: 120-240 ms.
- Glow pulse: 300-900 ms depending on purpose.
- Meter falloff: tuned by feel and plugin type.

Use animation to communicate:

- Focus.
- Hover.
- Dragging.
- Automation.
- Modulation.
- Bypass.
- Warnings.

Do not animate just because it is possible.
