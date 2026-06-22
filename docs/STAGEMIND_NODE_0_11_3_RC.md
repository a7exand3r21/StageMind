# StageMind Node 0.11.3 RC

0.11.3 is a manual-intent fix pass after the second diagnostic log.

## Found In Log

- `Suno Guitar` received 113 Director Balance commands.
- 108 of those commands were cuts.
- The guitar's Output Trim reached `-12 dB`.
- The user switched Stage Gain to `Static`, but Director Balance still kept sending `set_output=1`.
- Periodic diagnostic snapshots continued while playback was stopped.

## Changed

- Director Balance now trims only Nodes in `Stage Gain: Ride`.
- `Stage Gain: Static` and `Off` are treated as manual/held modes for Director Balance.
- Director Output Trim automation is capped to `-3 dB` / `+3 dB`.
- If `SC Enable` is turned off on an already configured ducking path, Auto treats that as a manual bypass and does not turn ducking back on.
- Director auto processing pauses while the host transport is stopped.
- Periodic diagnostic snapshots pause while the host transport is stopped. Lifecycle events still log.

## Test

1. Put a guitar Node in `Auto`.
2. Let Director find balance conflicts.
3. Switch the guitar Stage Gain to `Static`.
4. Raise guitar Output Trim manually.
5. Confirm Director no longer sends `set_output=1` to that guitar.
6. Turn `SC Enable` off after Auto configured ducking.
7. Confirm Auto does not re-enable it.
8. Stop playback and confirm the CSV does not keep growing with `node_snapshot` / `director_snapshot` rows.
