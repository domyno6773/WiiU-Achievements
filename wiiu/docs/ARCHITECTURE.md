# Native Wii U port — engineering plan

Status: **native catalog prototype compiled and launched on a real Wii U**;
the achievements evaluator, WUPS monitoring and overlays are not yet ported or
hardware-tested. Target: Aroma Wii U, with no dependence on Cemu at runtime.
See [hardware test log](HARDWARE-TESTS.md).

## One project, two builds

The console edition is a sibling of the original Windows executable. `src/Core.cs` is the normative implementation for achievement evaluation, `src/Theme.cs`, `src/Banner.cs`, `src/MainForm.Ui.cs`, `src/Notification.cs` and `docs/UI.md` are the normative look-and-feel references, and the root XML documents remain the catalog source. Any conversion to a compact Wii U representation must be generated and checked; never manually fork IDs or rarity metadata.

**App (`.wuhb`)**: catalog and installed-title discovery (subject to API verification), local profile, unlocked list, completion, detail views, controls, UI previews. App start/end must not reset earned progress.

**Plugin (`.wps`)**: process/title lifecycle, robust attachment boundaries, supported-title fingerprinting, memory sampling, engine evaluation, atomic persistent unlocks and notifications. Never assume app remains running while game launches.

**Shared core**: stable IDs, rule parsing/compiled manifest, validation, baseline state, thresholds, experimental flags, deterministic tests and save consistency. Define a stable interchange boundary between app and plugin (e.g. compact versioned storage) and serialize writes. Avoid concurrent access/data corruption when frontend and plugin overlap.

## Source adaptation

| Windows source | Wii U direction |
| --- | --- |
| `Core.cs` | Port pure rules/state machine, verify with shared golden cases |
| `GameCatalog.cs` | Supported-title registry, do not mark installed games as supported automatically |
| `GameSession.cs` | Wii U title lifecycle and version identifiers |
| `MemoryReader.cs` / `WindWakerProgression.cs` | Read-only Wii U adapter, original data-field semantics; test offset assumptions on hardware |
| `MainForm*.cs`, `Theme.cs`, `Banner.cs` | Equivalent Wii U renderer and input/navigation |
| `Notification.cs` | Adapt semantics/style to `libnotifications` where possible |
| Root XML | Canonical data; validate deterministic transformation if precompiled |

## Technical risks to test before claiming support

- Memory permissions, address mapping and the lifetime/identity of the active title in Aroma plugins. A Cemu host pointer or guest base must **never** be copied blindly.
- The exact supported EUR v0 Wind Waker revision, title ID and persistent state layout on hardware.
- Whether libnotifications displays while Wind Waker has focus, presentation position/size/sound and any interference with gameplay.
- Atomic SD card writes, interrupted write recovery, distinct game profiles and coherent load boundaries; do not award progress based on an existing save baseline.
- Resource limits, asynchronous drawing, plugin re-entry and title switches. Avoid heavy allocations or frame work in hooks.
- License of source redistribution: upstream has no explicit license; branch-only collaboration is not a public reuse license. Obtain owner's permission before distributing port builds/code beyond authorized collaboration.

## Verification gates

- **Gate A:** independent reproducible toolchain build; WUHB opens from Wii U Menu; WPS starts without game interference.
- **Gate B:** deterministic parity tests for engine and catalog; match upstream IDs and rarity; verify progress reload/backup.
- **Gate C:** a manually triggered in-game test notification.
- **Gate D:** read-only observation of rupees/hearts on the supported retail title; verify stability during menu/load/reconnect.
- **Gate E:** first genuine unlock with durable progress and visible toast, followed by additional validated rules.
- **Gate F:** fidelity and stress tests across title transitions, sleep/restarts, UI resolutions and input.

No gate is passed just because a build succeeds on a desktop or CI.
