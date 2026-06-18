# StageMind Node 0.9.2 RC

0.9.2 makes the Character controls easier to hear.

## What changed

- Clean Up now has a dedicated role-aware tone path.
- Resonance suppression responds faster and can ride narrow peaks harder.
- Double now uses stronger asymmetric micro-taps while keeping the dry path zero-latency.
- Existing parameter IDs are unchanged.
- Version shown by the plugin moves to 0.9.2.

## What to test

1. Insert StageMind Node on vocal, guitar, bass, and drums.
2. Confirm the title shows `StageMind Node 0.9.2`.
3. On vocal or guitar, raise `Clean Up` from 0 to 100%.
4. Confirm the source becomes less muddy/harsh without disappearing.
5. Raise `Resonance` while a narrow ringing section plays.
6. Confirm the `Resonances` list appears and the reduction is audible.
7. On a role that allows Double, raise `Double`.
8. Confirm the dry sound is not delayed, but width/thickness changes.
9. Save, close, reopen the FL project.
10. Confirm the Character values restore.

## Expected behavior

- Clean Up should now be audible as tone cleanup, not only as detector sensitivity.
- Resonance is stronger than 0.9.1, but still bounded.
- Double should be most obvious on centered mono vocals, guitars, pads, and backing parts.
- Mono Safe and correlation safety can still reduce risky width/double behavior.

## Known limits

- Clean Up is not a full de-esser or source repair tool.
- Resonance depends on detected or learned narrow peaks.
- Double is still role-limited.

## Verification

- `StageMindDSPTests` passed.
- Debug VST3 target built successfully.
- Release VST3 was packaged into `dist/VST3`.
- `moduleinfo.json` reports version `0.9.2`.
