# Game catalog and adapters (alpha 0.3)

The **Giochi** button, or **G**, opens the catalog. The selected game's card
opens its achievements. **B / Esc** returns. Browsing the catalog does not stop,
resume, or reconnect a recording session. New selections require disconnection.
Unavailable cards are explicitly disabled, including for accessibility clients.

The only available entry remains **The Wind Waker HD**, EUR v0, Cemu 2.6,
Title ID `0005000010143600`, RPX `cdc68acd`. The **Altri giochi** card is an
unavailable placeholder, not an announced title, downloader, or generic reader.
No new achievements or memory offsets were added in alpha 0.3.

![Games catalog](screenshots/games.png)

## Registration boundary

`src/GameCatalog.cs` defines:

- `GameDefinition`: stable catalog ID, title, description, rules path, set
  revision and an immutable collection of supported profiles.
- `GameProfile`: exact emulator version, Title ID, title version and RPX hash,
  plus a reader factory. All four identifiers must match before process access.
- `IGameMemoryReader`: reads a snapshot for the selected profile. Its first
  implementation, `WindWakerMemoryReader`, uses the original six-byte header.

`MainForm.Connect` uses the selected definition and profile instead of its own
hardcoded game check and memory address. An unsupported configuration is rejected
before opening the process for reading. The catalog does not discover support
from a title name or a superficially plausible memory value.

The original XML path and `wwhd-alpha-1` revision are preserved. Progress uses
the actual session fingerprint and the selected game's revision. Its existing
identity formula and saved IDs do not change. Settings remember a stable catalog
ID; a missing, removed or unavailable saved ID falls back to the default game.
Selecting another game loads its rules before changing the active selection,
clears the displayed old game's state, and leaves all progress files intact.

## Adding a supported game later

1. Record exact region/version/fingerprint and live evidence in `MEMORY-MAP.md`.
2. Implement a read-only adapter. Extend snapshot fields, validation, rule schema,
   evaluator and presentation together where the new game requires new data.
   **The existing snapshot and threshold fields remain specific to Wind Waker.**
   This change provides the registration and routing boundary, not a universal
   condition language or automatic support for another game.
3. Give the game a stable ID, rules file, set revision and exact profiles in the
   catalog. An entry without profiles remains unavailable. Do not copy another
   game's reader without verifying the mapping.
4. Add synthetic rules, reader and isolation tests, then do real-game menu,
   load, save-slot, reconnect and emulator-restart validation.
5. Add original artwork appropriate to the game. The existing achievement banner
   remains Wind Waker's sea artwork; the catalog uses a neutral icon for other
   entries. Do not add Nintendo assets.
6. Add new set/assets to `package.ps1`'s explicit allowlist.

The catalog automatically lays out registered cards, adds rows when needed,
supports scrolling and keyboard focus, and blocks switching to another game
while the current session or demo is attached. No unverified game is enabled.
