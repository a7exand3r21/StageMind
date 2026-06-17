# ADR 0005: StageMind Link Registry

## Status

Accepted.

## Context

MVP 5 needs StageMind Node instances to discover each other and exchange control data. The Link layer must not transfer audio, block the audio thread, allocate in `processBlock`, or make offline renders depend on host instance order.

## Decision

Use a process-local fixed-size registry with 128 atomic slots.

Each plugin instance receives a runtime ID from the registry. During audio processing, an instance may publish a small control-data snapshot when `link_enabled` is on and `link_group` is greater than zero. Other instances may read snapshots from the same group using bounded fixed-size scans.

The registry stores only scalar data. It does not store processor pointers, editor pointers, buffers, or callbacks.

`link_role` is used as a preferred source-role filter when reading peers. Role filtering is preferred over runtime source IDs for ordinary use because runtime IDs are assigned per process and may change after project reload.

MVP 5 is status-only. Link data is visible in the UI, but it does not automatically change audio yet.

Publishing is disabled when the host reports non-realtime/offline processing.

## Consequences

- Audio is never transferred between instances.
- `processBlock` uses only atomics and bounded loops for Link.
- There are no audio-thread locks or heap allocations in the Link path.
- Runtime IDs are not stable project identities. Project reload is safe, but manual source/target IDs may need checking after reopening.
- Source-role filtering gives a stable user-facing route while keeping IDs available for low-level testing.
- Automatic conflict correction needs a later ADR before it is allowed to affect audio.
