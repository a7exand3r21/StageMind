# StageMind Node 0.9.0 Session Plan

## Baseline

Current build line: 0.8.x.

Implemented baseline:

- one VST3 plugin with Node and Director modes;
- Link group defaults for fast testing;
- role-aware spatial controls;
- local resonance detection and learned resonance profile;
- Director scene view with linked nodes and conflict lines;
- guarded Auto Assist for known Link corrections;
- Ride Memory for remembered role-to-role conflicts;
- state save/load for plugin parameters, learned resonances, and Ride Memory.

## Session Goal

Move Auto Assist closer to a ride workflow:

1. The group forms.
2. Director observes linked nodes.
3. Conflicts are detected and corrected.
4. Corrections are remembered.
5. On later playback, known corrections can be reused instead of rediscovered from scratch.

## Scope

- Add a timeline-aware memory model for conflict events.
- Capture coarse host transport position without audio transfer between instances.
- Save and restore timeline memory with the project state.
- Show enough status in Director to understand whether memory is idle, learning, or populated.
- Keep all heavy work away from the realtime audio path.
- Keep Link control-data only.

## Non-Goals

- No automatic FL Studio mixer routing.
- No audio stream sharing between plugin instances.
- No cloud analysis.
- No full 3D room model.
- No installer work in this session.

## Implementation Checklist

- [x] Add public product spec/roadmap after repository cleanup.
- [x] Add timeline ride memory model.
- [x] Add unit tests for merge, restore, and resolved states.
- [x] Serialize timeline memory into plugin state.
- [x] Capture transport PPQ position through JUCE playhead snapshots.
- [x] Feed Director Auto observations into timeline memory.
- [x] Add Director UI memory status for timeline events.
- [x] Build and run DSP tests.
- [x] Create a test build and update the release note.

## Manual Test Checklist

- [ ] Open FL Studio project with several linked Nodes and one Director.
- [ ] Confirm every Node defaults to Group 1, Link on, Auto on.
- [ ] Start playback with Director in Auto.
- [ ] Confirm Director conflict list stays visible while conflict exists.
- [ ] Confirm Auto applies known corrections on several Nodes, not only one.
- [ ] Save project, reopen it, and confirm memory counters remain.
- [ ] Stop/start playback and confirm there are no stuck sidechain or gain-reduction states.
- [ ] Render/export and confirm output is stable.
