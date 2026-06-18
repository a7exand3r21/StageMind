# StageMind Product Spec

## Product

StageMind Node is a FL Studio-first VST3 insert effect for stem-based mixing.

The user places one instance on each stem, chooses a role, and lets the plugin create a safer starting point for space, width, low-end control, resonances, and inter-track conflicts.

The plugin should help quickly. It should not fight manual mixing decisions.

## Current Shape

The product is one plugin with two modes:

- Node: placed on a normal mixer channel and processes that channel.
- Director: placed on a master or bus channel and reads Link control data from Nodes.

Link does not transfer audio between instances. It only shares control data:

- role;
- group;
- activity;
- spectral band energy;
- width, pan, depth, motion;
- correction status.

## Working Features

- Role selector.
- Width, depth, motion, motion preset, clean-up, resonance, double, output gain.
- Role-limited motion and pseudo-double behavior.
- Character DSP v2: role-aware Clean Up tone shaping, stronger resonance riding, and asymmetric pseudo-double taps.
- Room Depth v1: zero-latency dry path with damped early reflections and short feedback.
- Zero-latency dry path.
- Correlation safety for risky width.
- Optional external sidechain bus.
- StageMind Link group view.
- Director scene map.
- Director remote control for selected Node parameters.
- Auto Assist suggestions and guarded automatic commands.
- Local resonance learning.
- Ride Memory for remembered relationship corrections.
- Project state save/load for parameters and learned data.

## Known Limits

- Room Depth v1 is not HRTF, not binaural 3D, and not late-room convolution.
- Motion is subtle and role-limited.
- Clean Up, Resonance, and Double are still mix-starting tools, not full restoration, de-essing, or dedicated chorus/reverb modules.
- Sidechain routing inside FL Studio remains manual.
- Link is control-data only.
- Director depends on Nodes publishing their state.
- Timeline memory is coarse; it remembers rough PPQ areas, not editable song sections.

## Roadmap

- Section-aware correction status in Director.
- Better Director memory display.
- Optional deeper room model if tests show it is worth the CPU and UX cost.
- More complete group workflow for projects with many stems.
- Release QA across clean FL Studio installs.
