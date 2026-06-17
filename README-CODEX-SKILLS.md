# Codex MD Pack for JUCE VST Plugin

This pack contains project-level Codex instructions and repo-scoped skills for building a production-grade C++ JUCE VST3 plugin with polished animated UI.

## Where to put this

Copy all files into the root of your project.

Expected structure:

```text
AGENTS.md
README-CODEX-SKILLS.md
docs/
  PROJECT_CONTEXT.md
  PLUGIN_SPEC.md
  DESIGN_DIRECTION.md
  CODEX_WORKFLOW.md
  RELEASE_QA_CHECKLIST.md
.agents/
  skills/
    juce-plugin-architecture/
      SKILL.md
    realtime-audio-safety/
      SKILL.md
    dsp-parameter-design/
      SKILL.md
    cpp-production-engineering/
      SKILL.md
    plugin-ui-fancy-controls/
      SKILL.md
    plugin-gui-animation-system/
      SKILL.md
    host-compat-release-qa/
      SKILL.md
```

Codex scans repo skills from `.agents/skills` when launched inside the repository.

## First Codex prompt to use

```text
Read AGENTS.md, docs/PROJECT_CONTEXT.md and docs/PLUGIN_SPEC.md.
Use $juce-plugin-architecture, $dsp-parameter-design and $realtime-audio-safety.
Inspect the repo and propose the initial production architecture for a JUCE VST3 plugin targeting FL Studio.
Do not implement yet. Return the plan and files that need to exist.
```

## For fancy UI work

```text
Use $plugin-ui-fancy-controls and $plugin-gui-animation-system.
Design the first visual system for this plugin: theme tokens, knob behavior, glow states, meter behavior and animation rules.
Update docs/DESIGN_DIRECTION.md.
Do not implement until the design direction is clear.
```

## For release audit

```text
Use $host-compat-release-qa, $realtime-audio-safety and $cpp-production-engineering.
Audit the plugin for release blockers, prioritizing FL Studio VST3 behavior.
```
