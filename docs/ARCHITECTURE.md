# Architecture and extension guide

## Runtime flow

`Program.Main` enables WinForms and holds a named mutex derived from the app
folder (`--preview-toast=<png>` renders one popup frame and exits before that). `MainForm` loads `achievements.xml`, then waits for a connection request
(or connects on `--connect`). `--demo` supplies synthetic snapshots with its own
progress directory. `--preview=<absolute PNG path>` closes the UI after rendering
an internal preview; it is a developer aid, not a game screenshot facility.

On connection, `MainForm.Connect`:

1. Finds exactly one Cemu process matching the selected executable path.
2. Reads title, version, RPX hash, memory base, and emulator version from the log.
3. Requires an exact match in the selected catalog entry’s profiles and creates
   that profile’s reader. Only Wind Waker is currently available.
4. Opens Cemu for query/read access and detects its 4 GiB guest reservation.
5. Requires the base from the log to match a discovered reservation.
6. Calls the profile reader. The current Wind Waker reader reads six bytes at
   `guestBase + 0x145B7B80` and validates a snapshot.
7. Opens the selected profile's progress, establishes an engine baseline, then
   starts a WinForms timer with a nominal interval of 100 ms.

Each tick reads a snapshot, runs the engine, persists any awards, updates the UI,
and queues notifications. The interface is custom-drawn (`MainForm.Ui.cs`,
`MainForm.Library.cs`, `Theme.cs`, `Banner.cs`, `Notification.cs`). Its rendering model, animation timer,
input map and asset policy are described in `docs/UI.md`. UI state never feeds
back into the engine. The title fingerprint and memory base are rechecked
from the log approximately once per second. Failures disconnect the reader and
reset pending conditions. They do not erase saved unlocks.

## Types and responsibilities

`ProcessMemory` owns a native process handle and implements `Read`, `Regions`,
and `FindGuestBases`. Addresses are unsigned 64-bit host addresses. Guest offsets
are 32-bit and must be added to the discovered base. Native access is deliberately
limited to `OpenProcess`, `ReadProcessMemory`, `VirtualQueryEx`, and `CloseHandle`.

`Calibration` can scan readable committed MEM2 pages for an observed six-byte
header and filter candidates after values change. It checks alignment, overlaps
scanner chunks, supports cancellation, and caps excessive matches. It is a
research utility, not automatic profile validation and not a currently exposed
GUI workflow.

`GameCatalog` owns game definitions, set revisions, exact profiles and reader
factories. Its collections are immutable. `MainForm` keeps the selected game and
loads that game’s rules before changing selection. See `GAMES.md`.

`GameSnapshot` parses the six-byte big-endian header and provides named fields
to rules. `Observation.Validate` enforces 12–80 quarter-heart capacity, capacity
divisible by four, current health between zero and capacity, and 0–5000 rupees.
Those assumptions are part of the current profile; review them before extending
the game or supporting other titles.

`AchievementSet.Load` requires the revision supplied by the selected game
(the original overload still requires `wwhd-alpha-1`), unique IDs, nonblank
text, positive thresholds/points, and the fields `rupees` and `max_health`.
Unreachable thresholds, fractional heart capacities, unknown child elements
and unsupported condition attributes are rejected. Conditions
are currently a single `>=` threshold. Adding arbitrary XML fields alone does
not add an engine capability.

`AchievementEngine` evaluates snapshots without accessing the process or UI.
For each locked rule it maintains an armed flag and a consecutive-match count:

- A baseline at or above the threshold does not award or arm that rule.
- Three consecutive below-threshold values arm the rule; a single low glitch
  cannot arm an already satisfied save. Below-threshold samples also clear a
  pending match count.
- Three consecutive at/above-threshold samples after arming award it.
- A timestamp reversal or sampling gap longer than one second resets all
  transient state and treats the next sample as a baseline.
- Already persisted IDs never award again in that profile.
- Every award due in the same sample commits together via `AwardMany` before
  any award is returned to the UI. A write failure resets transient state and
  publishes no partial result.
- Null snapshots, missing/non-UTC timestamps and impossible values reset the
  engine and throw, including when callers bypass byte parsing.
- `Status(id)` returns a snapshot of the authoritative phase and confirmation
  count. `StateVersion` changes only when evaluation state changes, so the UI
  updates confirmations even when the numeric game values remain the same.
  The UI no longer maintains its own baseline mirror.

Manual pause stops the sampling timer, clears pending notifications and resets
the engine. Resume resets it again and rechecks session identity before sampling.
Earned progress remains. This is an explicit user-controlled recording boundary,
not automatic gameplay/menu/load/save-slot detection.

This is polling of persistent state, not event hooks or frame-perfect tracking.

## Persistence contract

All paths are relative to the executable folder:

- `data/settings.xml`: local Cemu path, profile name, selected catalog `game` ID, and the optional `sound` and
  `ambient` UI toggles. Missing toggles default to enabled.
- `data/progress/<key>.xml`: earned IDs with UTC timestamps.
- `data/demo/<random ID>/`: isolated synthetic-session progress.

The identity string is exactly:

```text
trimmedProfileName|titleId|titleVersion|rpxHash|setRevision
```

The filename key is the first 24 hexadecimal characters of SHA-256 of that UTF-8
string. XML timestamps use the round-trip UTC format. `SafeXml` prohibits DTDs
and external resolution. Writes use a same-directory temporary file followed
by `File.Replace` (or `File.Move` on the first save). A `.bak` contains the previous
save, so recovery can lose the most recent award if the newest file is damaged.
Damaged originals are preserved before recovery.

Stable IDs are the persistence interface. Cosmetic renames need not change IDs.
Changing IDs or the revision requires an explicit compatibility/migration plan.
The mutex only prevents multiple instances from the **same application folder**;
it does not provide cross-folder coordination or shared-file multi-writer locking.

## Extending without losing the baseline

For a new observed value, document its mapping and validation status first.
Add parsing and meaningful bounds to `GameSnapshot`, expose it in `Value`, and
extend the rule schema/loader only as required. Add regression tests covering
initial loaded state, a real transition, temporary spikes, and disconnect/load
boundaries. Retain existing progress IDs.

For game-state/save-slot detection, introduce explicit validity/identity in the
snapshot/monitoring boundary. Reset the engine when a transition invalidates
continuity. Avoid embedding all future event logic in WinForms timer handlers.

The build embeds `assets/fonts/*.ttf` as resources and references
`Accessibility.dll` for the custom accessible objects.

Profile metadata and the memory adapter now live in `GameCatalog.cs`. The
snapshot fields and threshold schema are still Wind Waker specific: a future
game must add its required data validation and rules with its adapter.
`tests/UiTests.cs` covers catalog navigation, unavailable cards, recording
pause/resume, settings and minimum-size layout. The session-log parser still
lacks dedicated automated coverage; engine/UI tests do not prove live Cemu behavior.
