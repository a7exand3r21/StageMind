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
- Width, depth, motion, motion preset, clean-up, resonance, doubler, output trim.
- Role-limited motion and pseudo-double behavior, with vocal/guitar/pad roles allowed to use Doubler.
- Character DSP v2: role-aware Clean Up tone shaping, stronger resonance riding, and asymmetric pseudo-double taps.
- Audible Character pass: Clean Up and Resonance now have wider audible ranges at high values.
- Audible Space pass: high Width can widen centered material through zero-latency side injection, and Depth is a stronger front/back macro.
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
- Auto Balance v1: per-Node level rider that learns a stem loudness target and smooths loud/quiet section jumps before Output Trim.
- Stage Gain v2: visible gain-staging layer with Off, Static, and Ride modes, dBFS/VU/RMS/LUFS-M/LUFS-S/LUFS-I meter modes, target level, threshold, ceiling, response, Analyze & Hold, applied-gain readout, saved Static hold gain, Director Analyze All, and fixed-latency look-ahead ceiling limiting.
- Director Group Balance Assist v1: role-aware group loudness correction through small Output Trim commands.
- Role Priority Loudness Model v2: Director balance uses different targets, weights, deadbands, and trim speeds for vocal, kick, bass, drums, guitar/lead, pads, and FX roles.
- Balance Timeline Memory v2: Director stores coarse PPQ sections where group balance needed correction and labels early buckets as Intro, Verse, Chorus, and Drop.
- Project state save/load for parameters and learned data.
- Editor window size restore through the plugin project state.

## Known Limits

- Room Depth v1 is not HRTF, not binaural 3D, and not late-room convolution.
- Motion is subtle and role-limited.
- Clean Up, Resonance, and Doubler are still mix-starting tools, not full restoration, de-essing, or dedicated chorus/reverb modules.
- Sidechain routing inside FL Studio remains manual.
- Stage Gain LUFS modes are lightweight BS.1770-style practical meters, not certified broadcast QC meters.
- Stage Gain look-ahead limiter is a peak ceiling guard for plugin output, not a mastering limiter with oversampling, ISP detection, or release-shape editing.
- Role Priority Loudness Model v2 is still a starting-balance model. It does not know lyrics, arrangement intent, genre, or final artistic loudness.
- Balance Timeline Memory v2 is coarse and guarded by current live level deviation. Its section labels are automatic PPQ buckets, not real DAW markers or semantic song-part detection.
- Link is control-data only.
- Director depends on Nodes publishing their state.
- Timeline memory is coarse; it remembers rough PPQ areas, not editable song sections.

## Roadmap

- Better section naming and editable correction memory in Director.
- Optional deeper room model if tests show it is worth the CPU and UX cost.
- More complete group workflow for projects with many stems.
- Release QA across clean FL Studio installs.
