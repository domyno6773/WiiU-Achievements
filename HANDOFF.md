# Start here — handoff for another AI or developer

This folder is the complete source project. No previous chat, account, cloud
service, hidden package, or original workspace path is needed to understand it.
The UI and user guide are Italian; technical handoff documents are English.

## Goal and current result

**Editorial rarity review (2026-10-29):** every target has explicit rarity in
`achievement-presentation.xml`: 53 bronze, 47 silver, 10 gold, zero current
platinum. Rarity is independent of points; platinum is reserved for a future
set-completion objective. Text/order corrections are presentation-only. See
`docs/RARITIES.md`. New isolated build: `build-check/rarity/`. IDs, points,
conditions, experimental flags and progress identity remain unchanged.

**Current development build — playthrough series (2026-10-28):** at the user's
explicit request to implement batches before they playtest them, there are now
70 executable rules, including **53 experimental detectors**, plus 40 still
unimplemented targets (110 total). See `docs/PLAYTHROUGH.md` for the chronological
groups. The new decoders are working code with synthetic coverage, not certified
HD mappings. The tunic and original threshold IDs/progress revision are retained.
`build-check/playthrough/` is the new isolated output; do not overwrite a live EXE.
The earlier sections below record the progression to this build.

The user wants a RetroAchievements-like system for Wii U games on PC, beginning
with **The Legend of Zelda: The Wind Waker HD** in **Cemu 2.6**. They explicitly
requested a fresh implementation, **not one built on EmuHook**.

The current result is an original Windows x64 WinForms application written in
C# 5, using .NET Framework and the Win32 read-only process APIs. It runs beside
Cemu. It reads rupees and hearts, evaluates sixteen simple threshold achievements,
shows a desktop notification, and persists local progress. It is not integrated
with RetroAchievements accounts or servers.

**Catalog expansion (2026-10-28):** the current source has **110 objectives**:
16 executable rupee/heart thresholds (the original ten plus six additive rules),
one tunic acquisition rule and 93 story/equipment/exploration/collection/side-quest targets explicitly marked
pending validation. These 93 cannot unlock, even in demo mode, and do not enter
completion or point totals. Category/status filters and detailed validation notes
are built into the UI. IDs/revision and user progress are preserved. See
`docs/ACHIEVEMENTS.md` and the latest `docs/VALIDATION.md` record. The tunic
adds one targeted byte to production reads after a live before/after,
load, area/menu and handle-reopen check. Other inventory candidates stay research only.

**Scrolling fix (2026-10-28):** `MainForm.Tiles.cs` adds a bounded lazy bitmap
cache for static cards; scroll-only frames invalidate the grid, and introduction
delays depend only on viewport cards. Live progress and unlock animations remain
dynamic. See the latest validation record for timing and regression evidence.
The reviewed build is in `build-check/scroll-fix/`; the user's open earlier EXE
is not overwritten. The tunic was initially pending (the user reproduced its
absence of unlock); it now has an active detector at `0x145B81CE`, mask `0x80`.
It requires a stable absent baseline, so connect/resume BEFORE the delivery.
The user confirmed a real tunic notification in the corrected build; one saved
`story-heros-clothes` entry was verified locally. No manual award was made.
Loading saves still requires manual pause/disconnect; automatic gameplay and
save-slot detection remain unimplemented. Cemu restart coverage is pending.

**Alpha 0.2 (2026-10-28)** replaces the plain WinForms controls with a
custom-drawn, animated interface in the style of the Wii U system menus. It
also adds a layered, click-through unlock popup and a synthesised chime. Session,
engine, memory and persistence semantics are unchanged. The achievement IDs and
set revision are unchanged too. See `docs/UI.md`. This UI was built and
visually checked only on Linux with Mono (see `docs/VALIDATION.md`). **It had not
yet been run on Windows at that handoff.**

**Alpha 0.3 (2026-10-28)** adds a game catalog, exact profile/reader routing,
manual recording pause/resume, centralized per-rule evaluation status, stable
below-threshold arming and atomic batch unlock persistence. The ten original
rules, IDs, XML path, progress identity and revision are unchanged. No game
memory mappings or new achievements were added. See `docs/GAMES.md` and the
alpha 0.3 record in `docs/VALIDATION.md` for current evidence.

The alpha 0.3 code also passed Windows CI: official C# 5/.NET Framework x64 build,
50 core/Win32 checks, 22 UI checks, and a 47-file verified package. The workflow
artifacts contain a Windows-built EXE. This does not replace live Cemu tests on
the user's PC. See the linked run in `docs/VALIDATION.md`.

The first live unlock, **Primi spiccioli** at 10 rupees, was confirmed by the user
on 2026-10-28. It remained unlocked after closing and reopening this application.
That test did **not** restart Cemu. See `docs/VALIDATION.md` for the evidence and
the limits of what has been tested.

## Supported configuration

- OS/runtime: Windows x64, .NET Framework 4.x; the original alpha 0.1 was tested on the
  user's Windows PC. Later build evidence is recorded in `docs/VALIDATION.md`. Windows integration cannot run natively on Linux/macOS.
- Emulator: **Cemu 2.6**. The application deliberately rejects other versions.
- Game: **EUR v0**, Title ID **0005000010143600**, RPX hash **cdc68acd**.
- No game enhancements were enabled in the observed session; language was English.
- Host memory base is discovered per session, never hardcoded.
- Guest save header address is currently **hardcoded** to **0x145B7B80**.
  It was validated in one real Cemu session, not across a full playthrough or
  multiple real emulator restarts.

## Run, build, test

Open this folder in your editor. On Windows, run these commands from its root:

```powershell
# Existing binary: normal UI, explicit connection button
.\WiiUAchievements.exe

# Rebuild without overwriting an app that may currently be running
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1 -OutputDirectory .\build-check

# No Cemu or game required; includes a synthetic Win32 child-process fixture
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run.ps1

# Build, test, and create a clean, shareable archive next to this folder
powershell -NoProfile -ExecutionPolicy Bypass -File .\package.ps1
```

Paths in scripts are resolved relative to the scripts, not the shell's working
directory. Paths containing spaces are supported. Build and tests use the x64
.NET Framework compiler under the current machine's Windows directory, not a
user-specific compiler path. No .NET SDK or package restore is required.

`-ExecutionPolicy Bypass` applies to that PowerShell process only; these commands
do not change the machine's persistent execution-policy configuration. If your
environment already permits local scripts, direct `./build.ps1` invocation works.

The UI's initial Cemu path is only a convenience guess under the current user's
Desktop. Select the real `Cemu.exe` on a different machine. The app also searches
for Cemu's log beside the selected executable and in the current user's
Roaming/Local AppData Cemu directories.

## Reading order and source map

1. `AGENTS.md`: project-wide guidance and invariants.
2. `docs/ARCHITECTURE.md`: control flow, types, persistence, and extension points.
3. `docs/MEMORY-MAP.md`: exact supported profile and evidence requirements.
4. `docs/VALIDATION.md`: what passed, what remains unverified, and live test steps.
5. `NEXT-STEPS.md`: ordered development priorities.
6. `docs/COLLABORATION.md`: contribution boundaries and integration procedure.

Source entry points:

- `src/Program.cs`: GUI entry point and single-instance guard per app folder.
- `src/GameCatalog.cs`: definitions, exact profiles and per-game memory adapters.
- `src/MainForm.Library.cs`: catalog rendering and navigation using the existing theme.
- `src/MainForm.cs`: attach sequence, 100 ms sampling timer, demo, settings, toast queue.
- `src/MainForm.Ui.cs`, `src/MainForm.Tiles.cs`: custom-drawn window, bounded card cache, layout, animations, input, accessibility.
- `src/Theme.cs`, `src/Banner.cs`: palette, embedded Nunito font, vector icons/medals, sea banner.
- `src/Notification.cs`: layered unlock popup and synthesised chime.
- `src/GameSession.cs`: identification from the active Cemu log.
- `src/MemoryReader.cs`: process reader, guest-base discovery, calibration scanner.
- `src/Core.cs`: snapshot parsing, rules, achievement engine, XML progress store.
- `achievements.xml`: 16 thresholds in legacy-compatible format. `achievements-story.xml`: active tunic rule. `achievements-planned.xml`: 93 pending targets; `docs/ACHIEVEMENTS.md` explains promotion.
- `tests/CoreTests.cs`: executable regression suite, including the synthetic child.
- `tests/Probe.cs`: small optional live-memory diagnostic; not the app entry point.

## Recommended next task

**Current priority:** the user can now play normally and check experimental
notifications as they reach each group. Do not require repeated acquisition
rituals for every target. Retain the experimental label until exact EUR evidence
supports promotion. Continue research on the 40 genuinely unimplemented targets
and improve automatic load/gameplay validity. Read `docs/PLAYTHROUGH.md` first.

**Next, on the real PC, validate alpha 0.3 with Cemu.** Run `build.ps1` and
`tests\run.ps1`, then start `--demo`. Check:

- fonts, layout and DPI;
- the layered popup over Cemu, including in fullscreen;
- the chime, the keyboard navigation and screen-reader names.

Also check **Giochi**, **Pausa / Riprendi**, a below-threshold period of at least
three samples and a subsequent live crossing, persistence and reconnects. Record
results in `docs/VALIDATION.md`. The committed alpha 0.2 ZIP is a historical
artifact; use `build.ps1` or a successful alpha 0.3 CI artifact for current code.

**Distinguish active gameplay, menus, loading, and save slots before adding story
achievements.** Manual pause/resume now creates an explicit recording boundary, but the current
app still reads persistent save values without proving
that Link is controllable. Loading a different save can be misread as progress.

Then validate initial item acquisition (telescope, sword, shield) with before/after
observations on the supported EUR version. Add fields and conditions only after
checking the actual session and recording evidence. Do not promote offsets from
an online trainer to verified offsets without testing them.

The expanded catalog is implemented in `build-check/` for local review; the
already-running root EXE is not replaced. The immediate memory task is still
gameplay/save continuity, then the telescope/sword/shield before/after checks.
The user reported no initial items; the three candidate bytes read zero once.
This equipment baseline alone does not validate acquisition or authorize promotion
of telescope/sword/shield. The tunic's separate validated cycle is recorded in
`docs/MEMORY-MAP.md` and `docs/VALIDATION.md`.

The calibration scanner has automated coverage but is not exposed in the UI.
Automatic relocation of the save header, pointer chains, per-frame events,
hardcore mode, online profiles, and other regions are not implemented.

## What to request from the user for live work

Ask them to open the supported game in Cemu, describe the current game state,
and perform a precise action when needed. The absolute install path, current PID,
and host base are local session details, not prerequisites for understanding the
source. Do not request ROM uploads, console keys, passwords, or full memory dumps.

The portable engine suite runs with `bash tests/run.sh` on Mono. Windows
`tests/run.ps1` retains the native process fixture; `tests/run-ui.ps1` exercises
the UI in an isolated directory. GitHub Actions runs both Windows suites and
packages the application. Do not equate synthetic fixture/UI checks to Cemu tests.

For a remote AI without access to the PC, work on code, synthetic tests, and
documented hypotheses. Leave real-game validation explicitly pending.

## Portable handoff and personal state

Share the archive produced by `package.ps1`. It includes source, executable,
rules, documentation, tests, and a per-file SHA-256 manifest. It excludes `data/`,
personal settings, earned progress, logs, ROMs, and temporary artifacts.

To move **your own earned progress** to another PC, close the app and copy
`data/progress/` separately into the new app folder. Re-select Cemu on that PC.
Use the exact same local profile name; its trimmed value is part of progress
identity. Settings need not be copied. Renaming a profile, changing the game
fingerprint, or changing the set revision selects a different progress file.

Keep the app in a writable folder and keep `achievements.xml` beside its EXE.
The original user's data must remain intact during development.

## Licensing status

No redistribution license has been selected by the user yet. This project is
provided as an editable handoff for their collaboration. Do not invent an MIT,
GPL, or other grant, and do not imply that referencing Cemu or a trainer gives
this project that project's license. No external implementation is incorporated.
