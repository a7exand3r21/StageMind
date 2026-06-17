# MVP5 FL Studio QA Checklist

StageMind Node version: 0.7.0.

Use this checklist before moving from MVP5 Link diagnostics to automatic Link corrections.

Finalization note: MVP4 Spatial and MVP5 status-only Link are accepted for StageMind Node 0.5.4. See `docs/MVP4_MVP5_FINALIZATION.md`.

0.5.5 note: Link Suggestions are status-only hints. They do not change audio.
0.5.6 note: Apply Tip performs only user-approved parameter changes.
0.5.7 note: the suggestion line and Apply Tip button show Applied, and the button stays disabled when the suggested action is already active.
0.5.8 note: Sidechain status prioritizes listen/audition mode and does not call SC Mode Off active dynamic EQ.
0.5.9 note: Link Assist v1 adds conflict labels, reasons, and action-specific user-approved buttons.
0.6.0 note: Link Spectral Control Data v1 adds low, low-mid, presence, and air control bands for spectral-aware Link Assist suggestions.
0.6.1 note: Link Assist action preview shows the parameter change before a user-approved click.
0.6.2 note: Link Assist action preview is held against flicker and shows Resolved after applying an action.
0.7.0 note: Director is now an observe-only mode inside `StageMind Node.vst3`, not a second plugin.

## From The Spec

MVP1 foundation:

- [ ] VST3 builds.
- [ ] Plugin opens in FL Studio.
- [ ] Main mono/stereo input and output work.
- [ ] Optional sidechain bus exists.
- [ ] Audio passes without sidechain.
- [ ] Parameters are visible in FL Studio.
- [ ] Parameter order starts with Role, Width, Depth, Motion, Clean Up, Resonance, Safety, Output.
- [ ] Role selector exposes all selectable roles.
- [ ] Width works through Mid/Side.
- [ ] Mono low-end works.
- [ ] Side high-pass works.
- [ ] Output gain works.
- [ ] Basic GUI works.
- [ ] Input, output, sidechain meters exist.
- [ ] Sidechain meter detects signal.
- [ ] State saves and restores in FL project.
- [ ] Plugin reports zero latency.

MVP2 sidechain:

- [ ] Sidechain dynamic EQ works.
- [ ] VocalDucksInstrument mode works.
- [ ] KickDucksBass mode works.
- [ ] SnareDucksInstrument mode works.
- [ ] LeadDucksPad mode works.
- [ ] Sidechain Listen Off works.
- [ ] Sidechain Listen SidechainOnly works.
- [ ] Gain reduction meter works.
- [ ] SC Amount is automatable.
- [ ] Attack and Release react correctly.
- [ ] No obvious clicks with automation.
- [ ] Plugin renders correctly in FL.
- [ ] Max active sidechain bands is 2.

MVP3 resonance:

- [ ] Local resonance detector works.
- [ ] Detector finds narrow peaks on obvious resonant material.
- [ ] Dynamic resonance suppression works.
- [ ] Max active resonance bands is 4.
- [ ] Resonance macro controls amount.
- [ ] Clean Up affects detection or amount.
- [ ] Resonance list is shown in GUI.
- [ ] Default settings do not overprocess normal material.

MVP4 spatial:

- [x] Motion works.
- [x] Motion is role-limited.
- [x] Depth works as a zero-latency wet-path effect.
- [x] Stage View reflects effective pan/depth/width after role/safety/correlation limits.
- [x] Pseudo-double works for allowed roles.
- [x] Correlation safety reduces risky width.
- [x] Factory presets exist and change expected parameters.

MVP5 Link:

- [x] Multiple Node instances can be enabled for Link.
- [x] Instances in the same Group can see each other.
- [x] Group value saves and restores after FL project reopen.
- [x] Source role filter saves and restores after FL project reopen.
- [x] Link Enable saves and restores after FL project reopen.
- [x] Source role filter reads only matching roles.
- [x] Any Role reads the strongest active peer in the group.
- [x] A missing Source role shows a warning instead of connecting to the wrong role.
- [x] Link status shows group, node count, peer id, peer role, activity, and peer correlation.
- [x] Link status can show the strongest peer spectral band.
- [x] Link Assist frequency suggestions use coarse spectral overlap.
- [x] Link does not transfer audio.
- [x] Link does not automatically change audio yet.
- [x] Offline/non-realtime render does not depend on Link peer order.

Director mode:

- [ ] `Mode` defaults to `Node`.
- [ ] Switching `Mode` to `Director` shows the Director scene UI.
- [ ] Director mode passes audio through unchanged.
- [ ] Director mode does not publish itself as a linked Node.
- [ ] Director mode sees Nodes in the selected Group.
- [ ] Director conflict list updates without changing any Node parameters.
- [ ] Mode and Group restore after FL project reopen.

## My FL Studio Checklist

Basic load:

- [ ] Copy `dist/VST3/StageMind Node.vst3` to the test VST3 location or point FL Plugin Manager to `dist/VST3`.
- [ ] Rescan in FL Plugin Manager.
- [ ] Confirm the plugin window title shows `StageMind Node 0.7.3`.
- [ ] Insert one instance on a normal mixer insert.
- [ ] Play audio and confirm input/output meters move.
- [ ] Open and close the UI during playback 10 times.
- [ ] Resize the UI small and large; no important control should disappear.

State save/reopen:

- [ ] Set Role, Safety, Width, Depth, Clean Up, Resonance, Output.
- [ ] Enable Link.
- [ ] Set Group by typing a number.
- [ ] Set Source to a concrete role.
- [ ] Save the FL project.
- [ ] Close FL Studio.
- [ ] Reopen the project.
- [ ] Confirm all values restored.

Link group behavior:

- [x] Add two StageMind Node instances on two active inserts.
- [x] Enable Link on both.
- [x] Set both to Group 1.
- [x] Set instance A Role to Lead Vocal.
- [x] Set instance B Source to Lead Vocal.
- [x] Confirm B shows a peer with Lead Vocal.
- [x] Change A Role to Bass.
- [x] Confirm B warns that Lead Vocal source is missing.
- [ ] Change B Source to Any Role.
- [ ] Confirm B sees the strongest active peer.
- [ ] Put A and B into different groups.
- [ ] Confirm they no longer see each other.

Link diagnostics:

- [x] With Link off, status shows Link off.
- [x] With Link on and Group 0, status asks for a group.
- [x] With one node in Group 1, status says one node/no peers.
- [x] With matching peer, status shows peer id and role.
- [x] Activity percentage rises when the source plays.
- [x] Activity percentage falls smoothly after the source stops.
- [x] Correlation value appears for the peer.
- [ ] Strongest peer band appears when a peer is active.
- [ ] Suggestion line appears under Link detail.
- [ ] Suggestion line names the conflict, not only a generic tip.
- [ ] Action button names the expected action, such as `Reduce Width` or `Set Kick Duck`.
- [ ] Action preview line names the exact parameter change before pressing the button.
- [ ] Action preview line stays visible while the same conflict is active.
- [ ] Suggestions disappear when Link is off.
- [ ] Suggestions disappear when no peer is found.
- [ ] Suggestions do not move any plugin parameter.
- [ ] Action button is disabled when no suggestion is active.
- [ ] Suggestion line and action button show `Applied` when the suggested action is already active.
- [ ] Action button changes only the expected visible parameter.
- [ ] Action button changes save and restore after FL project reopen.
- [ ] Kick/Bass, Vocal, Snare, and Lead/Pad suggestions disappear when the relevant spectral overlap is absent.
- [ ] Sidechain actions show that routing and SC Enable stay manual.
- [ ] After pressing the action button, the preview line shows `Resolved`.
- [ ] Auto Assist is not present and no Link correction happens without a user click.

Spatial regression:

- [ ] On `Suno Drums`, set Motion high and confirm Stage View shows that Motion is role-locked.
- [ ] On `Suno Synth Pad`, `Suno Percussion`, `Backing Vocal`, `Pad`, or `FX`, set Motion high and confirm Stage View moves left/right.
- [ ] On an allowed Motion role, use centered/mono-like material and confirm Motion creates audible left/right movement.
- [ ] Raise Width and confirm Stage View spread gets wider.
- [ ] Lower Width and confirm Stage View spread narrows.
- [ ] Raise Depth and confirm Stage View moves toward Back.
- [ ] Lower Depth and confirm Stage View moves toward Front.
- [ ] On roles with default pan, such as `HiHat`, `Percussion`, `Rhythm Guitar Pair Left`, or `Rhythm Guitar Pair Right`, confirm Stage View shifts left/right.
- [ ] If correlation drops into warning, confirm Stage View warns and width/motion become less aggressive.

Sidechain regression:

- [ ] Route sidechain in FL.
- [ ] Use Wrapper Processing > Input/Connections > Auto map inputs.
- [ ] Confirm sidechain meter reacts.
- [ ] Enable External Sidechain and SC Enable.
- [ ] Confirm gain reduction meter reacts in a sidechain mode.
- [ ] Confirm Link status still works while sidechain is active.
- [ ] With `SC Listen: Sidechain Only`, confirm status says `SC listen active` when signal is present.
- [ ] With `SC Mode: Off`, confirm status does not say dynamic EQ is active.

Render/export:

- [x] Export a short section with one instance.
- [x] Export a short section with multiple linked instances.
- [x] Render does not crash.
- [x] Render has audio.
- [x] Render does not contain obvious clicks or silence caused by Link.

Stress:

- [ ] Test at least 6 StageMind Node instances.
- [ ] Use two different Link groups.
- [ ] Change Source role during playback.
- [ ] Enable/disable Link during playback.
- [ ] Save/reopen after the stress setup.
- [ ] CPU stays reasonable.
- [ ] No assertions or runtime dialogs appear.

Report template:

```text
FL Studio version:
Windows version:
StageMind Node version:
Number of instances:
Buffer size:
Sample rate:
What was being tested:
What happened:
Expected result:
Screenshot/video:
Project file saved: yes/no
```
