# Project Context

## Product

We are building a production-grade audio plugin for FL Studio users.

The plugin should feel like a real commercial product:

- Stable.
- Low CPU.
- No audio glitches.
- Clear parameter behavior.
- Polished visual design.
- Fancy but controlled animation.
- Satisfying knobs, meters, glow, depth, and feedback.
- Easy to expand without turning the codebase into spaghetti.

## Technical assumptions

Default stack:

- C++.
- JUCE.
- VST3.
- CMake preferred if the project has no existing build system.
- FL Studio is the primary host.

Adjust this file if the project chooses another framework, build system, or host matrix.

## Architecture language

Use these names consistently unless the codebase already has better names:

- `PluginProcessor`: main audio processor.
- `PluginEditor`: main editor.
- `ParameterLayout`: centralized parameter creation.
- `ParameterIds`: stable IDs.
- `DspEngine`: main DSP coordination layer.
- `PresetState`: save/load state model.
- `Theme`: colors, spacing, typography, shadows, glow tokens.
- `LookAndFeel`: JUCE look-and-feel customization.
- `AnimatedValue`: UI-only animation primitive.
- `MeterModel`: snapshot model for meters, never direct audio-thread UI coupling.

## Product priorities

1. Realtime audio correctness.
2. Host compatibility.
3. Clear parameter model and automation.
4. Maintainable JUCE/C++ architecture.
5. Polished visual identity.
6. Animation and fancy UI that stay within frame/CPU budget.
7. Release QA discipline.

## Non-goals

Avoid these unless explicitly requested:

- Building a huge custom UI framework before the plugin needs it.
- Adding large third-party dependencies for basic UI.
- Treating screenshots/mockups as final without checking usability.
- Sacrificing audio-thread safety for visual effects.
- Implementing undocumented host hacks.
- Shipping with unstable parameter IDs.

## Design direction placeholder

Update after the first visual decision session.

Possible direction:

- Dark premium audio interface.
- High-contrast active states.
- Subtle glass/depth.
- Glowing accent rings around active controls.
- Smooth meters with musical falloff.
- Animated microfeedback on hover, drag, automation, bypass.
- No childish neon overload.
- Controls should look expensive, not random.

## Open decisions

- Plugin type: effect, instrument, utility, analyzer, channel strip, etc.
- Exact DSP feature set.
- Product name.
- Brand colors.
- Whether UI is vector-only or uses image assets.
- Minimum OS versions.
- Validation tools.
- Installer/signing path.
