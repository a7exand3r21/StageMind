# StageMind Node 0.9.4 RC

0.9.4 is the first audible Space pass after external tester feedback.

## What changed

- Width is stronger and can create audible side width from centered material.
- Depth is more obvious: darker direct tone, stronger early reflections, clearer back placement.
- Clean Up and Resonance have wider audible ranges at high values.
- `Double` is now shown as `Doubler` in the UI.
- Doubler is stronger and now works on vocal roles, not only backing/guitar/pad roles.
- Factory presets are more contrasted.
- Sidechain wording is clearer: `Make Space` is now shown as broad ducking.
- StageMind Link auto ducking uses a stronger default amount.
- Version shown by the plugin moves to 0.9.4.

## What to test

1. Insert StageMind Node on a centered vocal or guitar.
2. Raise `Width` to 100%.
3. Confirm it becomes clearly wider, even if the source is mostly centered.
4. Raise `Depth` to 100%.
5. Confirm the source moves back: darker, less forward, more early-room.
6. Raise `Clean Up` and `Resonance` to high values on a bright vocal/guitar.
7. Confirm the tone change and resonance riding are obvious.
8. Raise `Doubler` on vocal/guitar/backing/pad roles.
9. Confirm the dry attack is not late, but the part is wider/thicker.
10. Step through factory presets and confirm they sound meaningfully different.
11. On Bass vs Drums or Guitar vs Drums, let Auto choose ducking.
12. Confirm GR is easier to see/hear than 0.9.3.

## ShaperBox takeaways applied

- Use obvious musical ranges, not only safe engineering ranges.
- Make sidechain-style processing read as ducking.
- Make presets feel like different workflows.

## Known limits

- StageMind still does not draw custom LFO curves like ShaperBox.
- Link still sends control data only, not audio.
- Sidechain routing in FL Studio is still manual unless StageMind Link Auto is used.
