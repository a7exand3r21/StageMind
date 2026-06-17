# Toolchain

StageMind targets Windows first for the MVP.

Required local tools:

- Visual Studio Build Tools 2022.
- MSVC x64/x86 tools.
- Windows SDK.
- CMake tools for Visual Studio.
- Ninja.
- Network access for the first CMake configure, because JUCE is fetched through CMake `FetchContent`.

Use the repo wrapper instead of relying on the global PATH:

```bat
scripts\verify-toolchain.cmd
```

Run a command inside the MSVC developer environment:

```bat
scripts\dev-cmd.cmd cmake --version
scripts\dev-cmd.cmd ninja --version
```

Open an interactive developer shell:

```bat
scripts\dev-cmd.cmd
```

The wrapper locates Visual Studio through `vswhere.exe`, then calls `vcvars64.bat`.
This keeps the build scripts independent of the actual Visual Studio install path.

The current project pins JUCE to `8.0.13`.
