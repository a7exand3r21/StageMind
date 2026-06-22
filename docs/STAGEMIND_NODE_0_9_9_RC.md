# StageMind Node 0.9.9 RC

0.9.9 advances Layer 4: Timeline Balance Memory now has automatic sections.

## Layer status

- Layer 1: implemented.
- Layer 2: implemented.
- Layer 3: implemented.
- Layer 4: implemented as Section-aware Balance Timeline Memory v2.
- Layer 5: partial.
- Layer 6: partial.
- Layer 7: not implemented.
- Layer 8: not implemented.

## What changed

- Balance timeline events now store section metadata:
  - section index;
  - section kind;
  - section start/end PPQ.
- Early timeline buckets are labeled:
  - `Intro`;
  - `Verse`;
  - `Chorus`;
  - `Drop`.
- Later buckets are shown as `Section N`.
- Director Memory now shows how many balance sections are remembered.
- Balance event text now includes the section label.
- Balance events across adjacent section boundaries no longer merge into one event.
- Project state saves/restores the section metadata.
- Version bumped to 0.9.9.

## Important limit

These are automatic PPQ buckets, not real FL Studio arrangement markers. The plugin does not yet know whether your actual chorus starts at that exact place. It gives us a stable memory grid for the next layers.

## What to test

1. Put Director and Nodes in `Auto`.
2. Let playback run through several parts of the song.
3. Create one loud/quiet imbalance in the first part.
4. Create another imbalance after the next section boundary.
5. Confirm Director Memory shows section labels such as `Intro`, `Verse`, `Chorus`, `Drop`, or `Section N`.
6. Confirm the Balance count grows and the section count is visible.
7. Save and reopen the FL project.
8. Confirm the remembered balance events still show their section labels.

## Next layer

Layer 5 is next: one decision engine that combines frequency conflict, ducking, width/depth, resonance, and output trim instead of letting separate heuristics fight each other.
