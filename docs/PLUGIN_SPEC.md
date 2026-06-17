# Plugin Specification

This file is the product source of truth. Keep it updated.

## Summary

Build a C++ JUCE VST3 plugin for FL Studio with production-grade audio behavior and a premium animated UI.

## Target host

Primary:

- FL Studio.

Secondary sanity checks:

- Add at least one more DAW before release.
- Use plugin validation tools where available.

## Plugin type

Audio effect.

The MVP artifact is `StageMind Node.vst3`.

## Core user promise

Role-aware space, clean frequency relationships, FL-friendly automation, and sidechain without pain.

StageMind should give a fast starting point for stem mixing. It should not replace the mixer.

## Parameters

MVP and reserved APVTS parameters are defined in `StageMind_TZ_final.md`.

The first eight host-facing parameters are fixed for FL Studio and Patcher:

| Order | ID | Display |
|---:|---|---|
| 1 | `role` | Role |
| 2 | `width` | Width |
| 3 | `depth` | Depth |
| 4 | `motion` | Motion |
| 5 | `clean_up` | Clean Up |
| 6 | `resonance` | Resonance |
| 7 | `safety` | Safety |
| 8 | `output_gain` | Output |

Every parameter must define:

- Stable ID.
- Display name.
- Unit.
- Range.
- Default value.
- Automation behavior.
- Smoothing behavior.
- UI control type.
- Display formatting.
- Whether it is saved in presets.
- Whether it affects latency or needs reset/reprepare.

Template:

| ID | Name | Unit | Range | Default | Smoothing | UI | Notes |
|---|---|---:|---:|---:|---|---|---|
| `inputGain` | Input | dB | -24..24 | 0 | smoothed | knob | Gain before processing |

## State and presets

Rules:

- Preset/state format must be versioned.
- Parameter IDs must never be renamed casually.
- New parameters need backward-compatible defaults.
- Save/load must work after reopening the DAW project.
- State restoration must not depend on UI being open.

## Audio behavior

MVP 1 audio path:

```text
Input
  -> input meter snapshot
  -> pan / spatial prep
  -> mono low-end
  -> side high-pass
  -> Mid/Side width
  -> output gain
  -> output meter snapshot
Output
```

MVP 1 sidechain path:

```text
Optional sidechain input
  -> basic sidechain detector / meter snapshot
  -> no gain reduction yet
```

MVP 2 sidechain path:

```text
Optional sidechain input
  -> sidechain detector / meter snapshot
  -> SidechainDynamicEQ when Trigger is External Sidechain and SC Enable is on
  -> gain-reduction meter snapshot
```

MVP 2 implemented sidechain modes:

| Mode | Bands | Behavior |
|---|---:|---|
| `VocalDucksInstrument` | 1 | Cuts 2-5 kHz in Mid when sidechain is active. |
| `KickDucksBass` | 1 | Cuts 45-100 Hz in Mid when sidechain is active. |
| `SnareDucksInstrument` | 1 | Cuts 1.5-3 kHz in Mid when sidechain is active. |
| `LeadDucksPad` | 2 | Cuts 300-1200 Hz in Mid, and 1200-4000 Hz in Mid/Side. |
| `Custom` | 1 | Uses `sidechain_range_start` and `sidechain_range_end`. |

SidechainDynamicEQ is zero-latency. It does not use FFT, lookahead, full-band compression, or automatic boosts.

`sidechain_amount`, `sidechain_attack`, and `sidechain_release` are read from APVTS and are automatable.

`Sidechain Listen` implements Off and Sidechain Only in MVP 2. `MainOnly` and `Delta` remain reserved.

When `Sidechain Listen` is set to Sidechain Only, the UI status prioritizes audition state over processing status. `SC Mode: Off` is reported as off even if an external sidechain signal is present.

MVP 3 local resonance path:

```text
Main signal after spatial and sidechain processing
  -> Hann-window FFT detector
  -> short held resonance snapshot
  -> optional Learn accumulator
  -> fixed top-4 local resonance snapshot
  -> DynamicEQ resonance suppression
  -> resonance list / gain-reduction snapshots
```

The detector uses a 2048-sample FFT, time-smoothed bin magnitudes, local-neighborhood peak checks, and a fixed-size peak array. It does not allocate, resize vectors, block, call GUI code, or report latency from the audio callback.

Live resonance display uses a short hold window so the GUI does not flicker on and off between FFT frames. The held peaks may also feed suppression until they expire.

`Learn` is a runtime UI action in MVP 3. It is not an APVTS parameter and is not saved in presets yet. When pressed, the processor listens for about four seconds, accumulates stable detected peaks in fixed-size storage, and then uses the strongest learned peaks for DynamicEQ suppression and the Resonance List. Pressing `Relearn` overwrites the runtime learned peaks.

MVP 3 parameter mapping:

| Parameter | Behavior |
|---|---|
| `clean_up` | Lowers the resonance detection threshold together with `resonance_sensitivity`. |
| `resonance` | Scales DynamicEQ resonance suppression amount. |
| `resonance_sensitivity` | Sets detector sensitivity for reserved/advanced control. |
| `max_resonance_reduction` | Caps suggested and applied local resonance cuts. |
| `dynamic_eq_attack` | Controls resonance suppression attack smoothing. |
| `dynamic_eq_release` | Controls resonance suppression release smoothing. |

`clean_up` also slightly scales local suppression intensity after detection. `resonance = 0` still means no local resonance suppression.

MVP 3 limits:

- Maximum active local resonance bands: 4.
- FFT size: 2048.
- Window: Hann.
- No automatic boosts.
- No inter-instance conflict engine.
- No StageMind Link or Director behavior.

MVP 4 spatial enhancement path:

```text
Main signal after MVP 1 spatial prep
  -> role-limited MotionProcessor
  -> DepthProcessor zero-latency dry path with tiny wet reflections
  -> role-limited PseudoDoubleProcessor
  -> sidechain / resonance / output path
```

Motion is a sine LFO applied after low-end side protection. For allowed roles it combines side motion with a small equal-power left/right pan movement so centered material can actually move. Center-critical roles do not receive motion. Mono Safe reduces motion.

Depth is not a full reverb. It applies small presence damping and a tiny delayed wet/reflection path while keeping the dry path undelayed. The plugin still reports zero latency.

Pseudo-double uses a fixed 18 ms wet-path micro-delay. The dry path is not delayed. `pseudo_double_amount` controls wet amount. It is off for center-critical roles and disabled in Mono Safe.

Correlation safety uses the measured output correlation from the previous block to gradually reduce risky width, motion, and pseudo-double. Stage View displays these effective values after role, safety, and correlation limits. It never mutes audio.

MVP 4 factory programs are host programs that write existing APVTS parameters. They do not introduce new parameter IDs.

MVP 5 StageMind Link path:

```text
Each Node instance
  -> registers a runtime Link ID
  -> publishes control data when Link Enable is on and Link Group > 0
  -> reads one active peer from the same group
  -> can filter the source peer by role
  -> exposes Link diagnostics to the UI
  -> does not transfer audio
  -> does not change audio from peer data yet
```

Published control data:

- runtime instance ID;
- group;
- role;
- smoothed activity envelope;
- input and output RMS;
- sidechain envelope;
- correlation;
- width, depth, clean-up, and resonance macro values;
- source/target IDs for manual routing/status.

`link_role` is the preferred source-role filter for reading peer data. When it is set to `Unknown` / `Any Role`, the instance reads the strongest active peer in the group. When it is set to a concrete role, the instance reads only peers publishing that role. Runtime source IDs remain available as a lower-level manual route, but role filtering is preferred because runtime IDs can change after project reload.

The published activity envelope uses a fast rise and slower release so Link status does not flicker on short gaps. It is control data only and does not affect audio.

MVP 5 Link is status-only. It proves inter-instance discovery, publishing, reading, grouping, source-role filtering, and source/target routing without making audio dependent on another instance. Automatic Link conflict correction remains a later step.

The UI diagnostics show group, active node count, peer ID, peer role, activity percentage, peer correlation, and common failure reasons such as missing group, no peers, missing source role, or offline render suppression.

When the host is in non-realtime/offline render mode, Link publishing is disabled. This avoids render-order-dependent behavior. The current instance still processes its own audio normally.

MVP 5.5 Link Suggestions:

```text
Link peer control data
  -> LinkSuggestionEngine
  -> short UI suggestion
  -> no audio change
  -> no parameter write
```

Link Suggestions are hints only. They can suggest stereo safety, Kick/Bass sidechain mode, vocal space, or avoiding a double-wide stack. They do not transfer audio, do not analyze peer audio spectrally, and do not automate parameters.

MVP 5.6 User-approved Link Actions:

```text
Visible Link suggestion
  -> Apply Tip button
  -> user-approved APVTS parameter write
  -> no hidden automation
  -> no audio transfer
```

`Apply Tip` can lower Width, raise Depth, or select a sidechain mode depending on the current suggestion. It does not enable routing, does not turn on sidechain automatically, and does not run from the audio thread.

If the current parameter values already match the suggested action, the suggestion line and button show the action as applied, and the UI does not send another host change gesture.

MVP 5.9 Link Assist v1:

```text
Link peer control data
  -> LinkSuggestionEngine conflict classification
  -> visible conflict label and reason
  -> action-specific user-approved button
  -> existing APVTS parameter write only after user click
```

Supported Link Assist v1 conflicts:

| Conflict | User action | Parameters changed |
|---|---|---|
| Stereo safety | Reduce Width | `width` |
| Kick vs Bass | Set Kick Duck | `sidechain_mode = KickDucksBass` |
| Vocal space | Make Room | `width`, `depth` |
| Vocal mask | Set Vocal Duck | `sidechain_mode = VocalDucksInstrument` |
| Snare bite | Set Snare Duck | `sidechain_mode = SnareDucksInstrument` |
| Lead vs Pad | Set Lead Duck | `sidechain_mode = LeadDucksPad` |
| Double-wide | Reduce Width | `width` |

Auto Assist is intentionally not implemented in Node 0.5.9. Link Assist still does not transfer audio, does not enable sidechain routing, does not turn on sidechain automatically, and does not write parameters without a user click.

MVP 5.10 / Node 0.6.0 Link Spectral Control Data v1:

```text
Main output control analysis
  -> low / low-mid / presence / air band activity
  -> StageMind Link publish snapshot
  -> LinkSuggestionEngine overlap checks
  -> visible user-approved suggestion
```

Each linked Node publishes four normalized control bands:

- `low`;
- `low-mid`;
- `presence`;
- `air`.

The bands are generated by a lightweight one-pole control analyzer in the audio path. It does not use FFT, lookahead, latency, dynamic allocation, file I/O, logging, or UI calls. The data is coarse and intentionally not a mastering analyzer.

Link Assist uses spectral overlap for frequency-specific conflicts:

| Conflict | Required spectral condition |
|---|---|
| Kick vs Bass | low-band overlap |
| Vocal space | vocal peer presence activity |
| Vocal mask | presence-band overlap |
| Snare bite | presence-band overlap |
| Lead vs Pad | low-mid or presence-band overlap |

Double-wide and stereo-safety suggestions remain based on width and correlation. They do not require spectral overlap.

Link Spectral Control Data is still control data only. It does not transfer audio between instances, does not enable sidechain routing, does not auto-write parameters, and is suppressed during non-realtime/offline render together with normal Link publishing.

MVP 5.11 / Node 0.6.1 Link Assist Action Preview:

```text
Visible Link suggestion
  -> action preview line
  -> user-approved action button
  -> existing APVTS parameter write only after user click
```

Every available Link Assist action exposes a short preview of the parameter change before the user clicks. Sidechain actions must also make the guardrail visible: they set `SC Mode` only. They do not enable sidechain routing, do not turn on `SC Enable`, and do not create host routing.

Applied state remains scoped to the action itself. If `SC Mode` is already set, the button may show `Applied`, but the preview line still reminds the user that routing and `SC Enable` are manual.

Node 0.6.2 stabilizes the preview line. The editor may hold the last active Link suggestion briefly to avoid UI flicker from control-band/activity threshold movement. When a user-approved action has been applied, the preview line shows `Resolved` instead of disappearing.

MVP 6 / Node 0.7.0 integrated Director mode:

```text
Mode: Node
  -> normal StageMind Node processing, Link publishing, Link Assist UI

Mode: Director
  -> transparent audio pass-through
  -> Link publishing disabled for this instance
  -> selected Link Group is read on the UI thread
  -> scene overview and conflict list are displayed
```

Director is a mode inside `StageMind Node.vst3`, not a separate plugin artifact. This keeps installation simple and avoids a second VST3 sharing a memory bus.

`plugin_mode` is a host-facing APVTS choice parameter appended after the existing parameters. Existing projects default to `Node`.

Director mode does not auto-detect the master insert. VST3/FL Studio does not provide a reliable, portable "this instance is on the master" signal. The user chooses `Mode: Director` explicitly.

Director mode is observe-only in Node 0.7.0. It does not transfer audio between instances, does not apply corrections, does not enable sidechain, and does not write parameters on other Nodes.

Node 0.7.1 extends Director conflict display. Detected conflicts are kept in a fixed-size UI history. When a conflict is no longer active, the row remains visible and is marked `resolved` instead of disappearing immediately.

Active Director rows may show an observe-only correction hint from the same Link Assist action model, for example `Set Kick Duck` or `Reduce Width`. Director still does not apply those changes to other Nodes in 0.7.1.

Node 0.7.2 adds user-approved Director commands:

```text
Director active conflict row
  -> Apply to #target
  -> one-shot Link registry command
  -> target Node message-thread timer
  -> existing APVTS parameter write
```

The command can apply only the same Link Assist actions as local `Apply Tip`: lower `width`, raise `depth`, or select `sidechain_mode`. It does not transfer audio, does not enable sidechain routing, does not turn on `SC Enable`, and does not auto-run. Target Nodes consume pending commands outside the audio callback.

Node 0.7.3 improves Director navigation and scene readability. Director now draws active conflict arrows on the scene map. The arrow direction is `peer -> target`, matching the conflict text `target <- peer`. Director also shows a compact overview of Groups 1-16 with node and active-node counts, and provides previous/next buttons for jumping between linked groups.

MVP 7.1 / Node 0.8.0 Auto Assist Foundation:

```text
Auto Assist: Auto
  -> waits for Link Enable, Group > 0, at least two active Nodes, and local input signal
  -> arms analysis after a short stable window
  -> starts ResonanceLearner without a manual Learn click
  -> gently raises low Clean Up / Resonance values once on the message thread
  -> saves learned resonance profile in the DAW state
  -> applies resonance suppression as a rider, not a permanent static notch
```

`auto_assist_mode` is appended after existing parameters and has three choices:

| Mode | Behavior |
|---|---|
| `Off` | No automatic analysis or parameter changes. |
| `Suggest` | Reserved/user-readable staging mode. Current Link suggestions still remain manual. |
| `Auto` | Enables guarded automatic resonance analysis for linked groups. |

Auto Assist 0.8.0 does not create FL Studio routing, does not add Link-based sidechain ducking yet, and does not run Director-wide automatic correction. It only automates the local resonance analysis/rider foundation. Any parameter writes happen from the processor timer/message thread, not from the audio callback.

Learned resonance profiles are stored outside APVTS as versioned ValueTree properties together with normal plugin state. State restoration does not require the editor to be open.

MVP 7.2 / Node 0.8.1 Auto Link Assist:

```text
Auto Assist: Auto
  -> evaluates stable Link Assist conflicts
  -> applies the matching local action on the message thread
  -> for sidechain actions, uses StageMind Link as the control source when external routing is not already active
  -> keeps resonance learning/riding behavior from 0.8.0
```

Auto mode can now apply the same local correction model as `Apply Tip` after a short stable window. Width/depth actions write only their existing APVTS parameters. Sidechain actions write `sidechain_mode`, enable `SC Enable`, raise low `SC Amount` to a conservative default, and set `Trigger` to `StageMind Link` unless a working external sidechain is already configured.

`StageMind Link` does not transfer audio between instances. It uses the selected peer's smoothed activity as a control envelope for `SidechainDynamicEQ`. This keeps the path zero-latency and avoids FL Studio manual sidechain routing for the automatic starting point.

Supported Auto Link sidechain starts:

| Conflict | Auto action |
|---|---|
| Bass vs Kick/Suno Drums low overlap | `Kick Ducks Bass` |
| Harmonic instrument vs Suno Drums broad overlap/activity | `Make Space` |
| Harmonic instrument vs Vocal presence overlap | `Vocal Ducks Instrument` |
| Instrument vs Snare presence overlap | `Snare Ducks Instrument` |
| Pad vs Lead overlap | `Lead Ducks Pad` |

In Node 0.8.1-0.8.3 Auto Link Assist is local to the current Node. Director-wide automatic correction starts in Node 0.8.4 and remains guarded by the target Node's own Auto mode.

Node 0.8.2 extends Auto Link Assist peer selection. In Auto mode, a Node scans all active peers in its Link Group, applies the source-role filter only when the user explicitly chose one, and selects the highest-priority stable local conflict. The compact Link UI may still display one representative peer, but Auto sidechain uses the peer selected by the group-wide conflict scan.

Node 0.8.3 extends Director visibility. Nodes publish sidechain mode, trigger mode, sidechain enabled state, sidechain amount, and auto-assist mode in their Link snapshot. Director uses the target snapshot to mark a conflict row as `resolved` when the suggested action is already applied on that target. A spectral overlap can remain visible in the source data, but it no longer keeps an active Director row when the local correction action is already set.

Node 0.8.4 adds guarded Director Auto correction:

```text
Director Auto
  -> reads active unresolved conflict rows
  -> sends one bounded Link command at a time
  -> target Node consumes command on its message-thread timer
  -> target applies only if its own Auto Assist is Auto
```

Automatic Director commands carry both the Director instance ID and the conflict peer ID. Width/depth commands write the same APVTS parameters as local Auto. Sidechain commands use the existing Auto Link action path, so the target can select `StageMind Link`, enable `SC Enable`, set a conservative `SC Amount`, and use the peer activity envelope as the control source.

Manual Director `Apply to #...` remains user-approved and conservative: it still writes only Width, Depth, or `SC Mode`, and it does not enable sidechain routing.

Node 0.8.5 moves Director Auto command selection from the editor timer into the Director processor timer. Director Auto still does not run in the audio callback. It reads the fixed-size Link group snapshot on the message-thread timer and sends at most one guarded command per cooldown window, so group-wide automation does not depend on the Director UI being open.

Node 0.8.6 changes new-instance defaults:

| Parameter | New default |
|---|---:|
| `link_enabled` | `true` |
| `link_group` | `1` |
| `auto_assist_mode` | `Auto` |

Existing saved projects keep their saved values. New Node instances publish an idle zero-activity Link heartbeat from the processor timer. This lets Director see installed Nodes in Group 1 before playback starts. Live audio snapshots take priority while audio blocks are being processed.

When FL Studio releases plugin resources or destroys a plugin instance, the Node unpublishes from the in-process Link registry. Full unregister still happens in the destructor. The plugin cannot force FL Studio to unload the VST3 module or release the DLL file handle when the user merely disables a mixer slot; that is host-owned lifetime behavior.

Node 0.8.7 adds Director scene remote control:

```text
Director Stage View click
  -> selected Node inspector in Director
  -> drag selected point
  -> one-shot Link command with pan/depth
  -> target Node message-thread timer
  -> existing APVTS `pan` and `depth` parameter writes
```

The Director scene now uses the published effective `pan` value for horizontal placement and `depth` for front/back placement. Effective pan includes role defaults. Dragging a Node changes the target Node's own parameters, so the target's audio path, host automation/state, and save/reopen behavior remain the source of truth.

Director drag commands do not transfer audio, do not run in the audio callback, and do not bypass the target Node. Width is shown as the horizontal spread marker, but dragging does not change Width in 0.8.7.

Node 0.8.8 expands Director control and stabilizes conflict memory:

```text
Director Stage View
  -> smaller scene map
  -> selected Node remote-control panel
  -> correction/status panel
```

The selected Node panel can write Pan, Width, Depth, Motion, Clean Up, Resonance, and SC Amount to the target Node through one-shot Link commands. The target still owns its APVTS parameters, DSP, automation, and saved state.

Director conflict history now holds more rows and keeps resolved rows visible longer. Director Auto also uses a shorter guarded command cooldown so it can move through newly found unresolved conflicts instead of appearing stuck on one visible row.

This is not yet a full arrangement timeline memory. StageMind does not store playhead-positioned conflict events across the whole song in 0.8.8. Applied APVTS corrections persist through normal DAW save/reopen; a future Ride Memory stage should add playhead-aware conflict capture if per-section behavior is needed.

Node 0.8.9 adds the first Ride Memory foundation:

```text
Director Auto / Learn Mix
  -> observes group conflicts as target role + source role + correction + coarse band
  -> stores event severity, hit count, and resolved state
  -> saves the fixed-size memory in the DAW project state
  -> reapplies remembered corrections when matching Nodes are present again
```

Director Auto now learns conflict memory automatically while it is enabled. The `Learn Mix` button keeps learning active even when the user wants to observe first. `Clear Memory` removes stored conflict events.

Ride Memory is deliberately not playhead-positioned in 0.8.9. It does not yet know verse/chorus sections or exact timeline locations. It remembers stable mix relationships inside the selected Director group and sends guarded Link commands from the Director processor timer. Target Nodes still apply commands only from their own message-thread timer and only when their Auto Assist mode is `Auto`.

Saved memory includes only compact event metadata, not audio. Audio is never copied between instances, and no memory work runs in the realtime audio callback.

Rules:

- No clicks under automation.
- No denormal CPU spikes.
- Bypass must be clean.
- Output must avoid accidental clipping where possible.
- Prepare/reset must handle sample-rate and block-size changes.
- DSP must behave predictably across common buffer sizes.

## UI behavior

Every interactive control must cover:

- Idle.
- Hover.
- Drag/active.
- Keyboard focus where applicable.
- Disabled.
- Automated/modulated if applicable.
- Reset/default gesture.
- Fine adjustment gesture if supported.

UI should feel polished:

- Smooth knob movement.
- Clear value readout.
- Subtle glow on active controls.
- Consistent spacing.
- Consistent typography.
- No stutter during audio playback.
- Repaint budget respected.

## Animation behavior

Animation must be UI-only.

Rules:

- Never animate from the audio thread.
- Never block audio for visual updates.
- Use smoothed UI values.
- Meter falloff must be musical and readable.
- Glow/pulse should support interaction feedback, not distract.

## Acceptance criteria for any feature

A feature is done when:

- Code builds.
- Feature works manually.
- Realtime audio rules are not violated.
- Parameters are centralized and stable.
- State save/load works if relevant.
- UI has all required interaction states.
- The implementation is easy to explain.
- Docs are updated if product behavior changed.
