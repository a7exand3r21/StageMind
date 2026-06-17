# ADR 0021: Ride Memory Foundation

## Status

Accepted for Node 0.8.9.

## Context

Director Auto could react to the best current conflict, but it did not remember what it had already found. During playback this made corrections feel momentary. It also made save/reopen behavior unclear for analysis results that were not normal APVTS parameter values.

The user goal is closer to a ride workflow: listen, find recurring mix conflicts, remember them, and apply the known correction when the mix reaches that relationship again.

## Decision

Add a fixed-size Ride Memory model owned by `PluginProcessor`.

Each event stores:

- Director group.
- Target role.
- Source role.
- Link correction action.
- Coarse spectral band.
- Severity.
- Hit count.
- Resolved state.

Director Auto records memory events automatically while Auto Assist is `Auto`. `Learn Mix` can also keep learning active. `Clear Memory` clears the event store.

Ride Memory is serialized into the plugin ValueTree state alongside learned resonances. Target Nodes still own their own APVTS parameters and apply received commands on their message-thread timer.

## Consequences

- No Ride Memory work runs in the realtime audio callback.
- No audio is stored or transferred between plugin instances.
- The first version is not timeline-aware. It remembers role relationships, not exact bars or song sections.
- The implementation can be tested without opening the editor because the model is independent from UI.
