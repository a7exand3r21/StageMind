# StageMind Node 0.11.2 RC

0.11.2 is a diagnostic-log fix pass after the first 0.11.1 test session.

## Found In Log

- Director/Memory sent hundreds of repeated ducking commands to the same targets.
- Some targets flipped between `Vocal Duck` and `Broad Duck`.
- Director Balance pushed one target Output Trim up to `+8 dB`.
- Local Stage Gain was already boosting some targets by more than `+10 dB`.
- Final `output_peak_db` could exceed 0 dB because Output Trim was applied after the Stage Gain ceiling limiter.
- Director snapshot rows did not show group node counts.

## Changed

- Any already-active ducking path now blocks automatic switching to another ducking mode on the same target.
- Local Auto Assist keeps the existing ducking source instead of changing it when ducking is already active.
- Director Memory replay commands are logged explicitly as `director_timeline_memory_command` and `director_ride_memory_command`.
- Director Balance boost is capped at `+3 dB`.
- Director Balance skips positive boost when the target's local Stage Gain is already applying a strong boost.
- Link snapshots now publish each node's current Stage Gain dB for balance decisions.
- Output Trim is applied before the Stage Gain ceiling limiter so positive trims are included in peak control.
- Director diagnostic snapshots now include group node counts in `link_nodes` and active balance nodes in `active_peers`.

## Test

1. Open a project with several Nodes and one Director.
2. Keep Nodes in `Auto`.
3. Play the same section that produced the previous log.
4. Confirm one target does not bounce between `Vocal Duck` and `Broad Duck`.
5. Confirm Director Balance does not push Output Trim above `+3 dB`.
6. Confirm the newest CSV has `director_timeline_memory_command` / `director_ride_memory_command` if memory replay is active.
7. Confirm `output_peak_db` stays near the selected ceiling when Stage Gain is active and Output Trim is positive.
