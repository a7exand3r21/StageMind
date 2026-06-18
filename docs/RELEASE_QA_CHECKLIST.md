# Release QA Checklist

Use this before distributing any build.

## Build

- [ ] Clean build succeeds.
- [ ] Release build succeeds.
- [ ] Debug-only code is not present in release behavior.
- [ ] No unexpected compiler warnings.
- [ ] Version number updated.
- [ ] Plugin metadata correct.
- [ ] VST3 output path confirmed.

## Validation

- [ ] Plugin validation tool passes.
- [ ] Plugin loads in FL Studio.
- [ ] Plugin can be inserted, removed, and reinserted.
- [ ] UI opens and closes without crash.
- [ ] DAW project save/reopen restores state.
- [ ] Presets load correctly.
- [ ] Automation writes and reads correctly.
- [ ] No parameter ID was renamed accidentally.
- [ ] StageMind Node 0.9.1 metadata/version is visible in the UI and VST3 module info.

## Audio

- [ ] No clicks when changing parameters.
- [ ] No clicks under automation.
- [ ] Bypass is clean.
- [ ] Output gain is controlled.
- [ ] CPU use is acceptable.
- [ ] No denormal CPU spike during silence.
- [ ] Sample-rate changes work.
- [ ] Buffer-size changes work.
- [ ] Long playback is stable.
- [ ] Offline render works if supported.
- [ ] Link Spectral Control Data does not change audio when Link is on.
- [ ] Link publishing remains suppressed in offline/non-realtime render.
- [ ] Director mode passes audio through unchanged.
- [ ] Director mode does not publish itself as a Node.

## UI

- [ ] All controls show correct values.
- [ ] Node UI shows `Motion Preset` and `Double` in normal Node mode.
- [ ] Knobs/sliders respond smoothly.
- [ ] Hover/active/disabled states work.
- [ ] Animation does not stutter.
- [ ] UI does not cause audio glitches.
- [ ] Meters are readable.
- [ ] Resize behavior works if supported.
- [ ] Text remains readable.
- [ ] Theme is consistent.
- [ ] Link detail shows the peer's strongest coarse band when a peer is active.
- [ ] Link Assist suggestions disappear when the relevant spectral overlap is absent.
- [ ] Link Assist action preview matches the action button.
- [ ] Sidechain Link actions warn that routing and SC Enable stay manual.
- [ ] Link Assist action preview does not flicker while the same conflict is active.
- [ ] Link Assist action preview shows `Resolved` after applying the action.
- [ ] Mode selector switches between Node and Director UI.
- [ ] Director scene shows linked Nodes in the selected group.
- [ ] Director conflict list is stable and observe-only.
- [ ] Director conflict rows remain visible as `resolved` after a conflict clears.
- [ ] Director active conflict rows show correction tips.
- [ ] Director `Apply to #...` changes only the target Node's suggested Width, Depth, or SC Mode.
- [ ] Director sidechain commands still leave routing and SC Enable manual.
- [ ] Director scene draws active conflict arrows between peer and target Nodes.
- [ ] Director group overview shows Groups 1-16 with node/active counts.
- [ ] Director previous/next group buttons jump between linked groups.
- [ ] Auto Assist selector shows Off / Suggest / Auto.
- [ ] Auto mode waits for Link Group with at least two active Nodes before resonance analysis.
- [ ] Auto analysis starts only when input signal is present.
- [ ] Auto mode raises low Clean Up / Resonance values once, then does not fight manual higher values.
- [ ] Learned resonance profile survives FL project save/reopen.
- [ ] Resonance rider releases reduction during silence instead of holding a static notch.
- [ ] Auto mode applies `Kick Ducks Bass` on a Bass Node when a Kick/Suno Drums peer is active and low overlap is present.
- [ ] Auto mode applies `Make Space` on a harmonic instrument Node when a Suno Drums peer conflicts.
- [ ] Auto sidechain actions select `Trigger: StageMind Link`, enable `SC Enable`, and show GR movement without FL manual sidechain routing.
- [ ] Auto mode scans a group with three or more Nodes and applies the best local action even when the compact UI peer is a different instrument.
- [ ] Director marks rows as `resolved` when the target Node already has the suggested Width, Depth, or SC Mode action applied by Auto.
- [ ] Director shows the Auto selector in Director mode.
- [ ] Director Auto sends a guarded command for an active unresolved conflict when both Director and target Node are set to `Auto`.
- [ ] Director Auto sidechain commands make the target use `Trigger: StageMind Link`, enable `SC Enable`, and follow the conflict peer activity.
- [ ] Director Auto does not change a target Node that is not set to `Auto`.
- [ ] Director Auto continues to send guarded commands when the Director UI window is closed.
- [ ] Manual Director `Apply to #...` still changes only Width, Depth, or SC Mode.
- [ ] New Node instances start with Link enabled, Group 1, and Auto Assist Auto.
- [ ] New Director instances start on Group 1 after switching Mode to Director.
- [ ] Director sees newly inserted idle Nodes in Group 1 before playback starts.
- [ ] During playback, idle heartbeat does not overwrite live activity/spectral Link snapshots.
- [ ] Removing an instance from FL makes it disappear from Director after the host destroys it.
- [ ] Clicking a Node in Director Stage View selects it and shows role/pan/depth/width/activity/correlation.
- [ ] Dragging a selected Node in Director changes the target Node's Pan and Depth.
- [ ] Director drag changes are saved after FL project save/reopen because they write target APVTS parameters.
- [ ] Director Stage View horizontal position follows target Pan, and vertical position follows target Depth.
- [ ] Director selected-node panel appears between the scene and correction/status panel.
- [ ] Director selected-node sliders write Pan, Width, Depth, Motion, Clean Up, Resonance, and SC Amount to the target Node.
- [ ] Director conflict/status rows do not flicker away immediately when the same correction is resolved.
- [ ] Director Auto continues applying newly found unresolved corrections after the first correction is applied.
- [ ] Director shows Ride Memory status with event and resolved counts.
- [ ] Director Auto learns Ride Memory events while Auto Assist is Auto.
- [ ] `Learn Mix` keeps Ride Memory learning active.
- [ ] `Clear Memory` removes stored Ride Memory events.
- [ ] Ride Memory survives FL project save/reopen.
- [ ] Ride Memory reapplies a remembered correction when matching target/source roles are present again.
- [ ] Motion Preset changes Stage View movement speed/shape and audible pan/side movement.
- [ ] Double control changes pseudo-double amount only on roles that allow pseudo-double.
- [ ] Depth remains zero-latency on the dry path and adds a more audible room-like tail at higher values.

## Host compatibility

Primary:

- [ ] FL Studio VST3 load.
- [ ] FL Studio automation.
- [ ] FL Studio save/reopen.
- [ ] FL Studio render/export.
- [ ] FL Studio UI open/close stress.

Secondary:

- [ ] Test in one additional host.
- [ ] Document any host-specific issue.

## Packaging

- [ ] Correct plugin binary included.
- [ ] Package contains `StageMind Node.vst3` only.
- [ ] No debug artifacts.
- [ ] License/readme included if needed.
- [ ] Installer/package tested if applicable.
- [ ] Signing/notarization handled if applicable.
- [ ] Final archive name includes product and version.

## Release blockers

List any unresolved blockers here.

- TBD.
