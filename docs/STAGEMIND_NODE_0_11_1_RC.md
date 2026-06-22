# StageMind Node 0.11.1 RC

0.11.1 adds diagnostic session logging for tester feedback.

## Added

- Shared CSV session log written to `Documents\StageMind\Logs`.
- Per-instance snapshots roughly once per second.
- Explicit events for:
  - instance start/end;
  - prepare/release audio lifecycle;
  - local Stage Gain Analyze;
  - Director Analyze All;
  - Director auto suggestions;
  - Director balance trim commands;
  - Link commands received by Nodes;
  - pending Auto Assist;
  - applied auto Link actions.

## Log Columns

The CSV includes instance ID, mode, role, group, Auto Assist mode, Stage Gain mode, meter mode, target/threshold/ceiling/response, input/output/peak levels, correlation, gain reduction, resonance reduction, Stage Gain values, Link peer counts, Director balance state, transport PPQ/BPM, and a note field.

## Test

1. Open FL Studio and insert several Nodes plus one Director.
2. Play the project for at least one full section where the problem is audible.
3. Trigger the scenario: Auto Assist, Analyze All, sidechain conflict, balance issue, or Stage Gain behavior.
4. Close FL Studio.
5. Send the newest `StageMind-session-*.csv` from `Documents\StageMind\Logs` with a short note about what sounded wrong.

## Limits

- Logging is intentionally simple CSV. It is for diagnosis, not a polished user-facing report.
- The log records control decisions and meters. It does not record audio.
