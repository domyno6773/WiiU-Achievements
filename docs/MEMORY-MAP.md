# Memory profile — EUR v0 only

## Address model

Cemu 2.6 reserves a contiguous 4 GiB host virtual-address range for the Wii U
address space. `ProcessMemory.FindGuestBases` groups regions by `AllocationBase`,
requires a 4 GiB reservation and readable code/MEM2 pages. `MainForm` cross-checks
the candidate with the base written into the active Cemu log.

```text
host address = current guestBase + Wii U guest address
```

Do not persist or reuse a previously observed host base or process ID. The guest
header address below is still a fixed profile assumption, not a discovered symbol.

## Current fields

- Header guest address: `0x145B7B80`.
- `+0x00`, two bytes, unsigned big-endian: maximum health in quarter hearts.
- `+0x02`, two bytes, unsigned big-endian: current health in quarter hearts.
- `+0x04`, two bytes, unsigned big-endian: current wallet rupees.

Example **synthetic** byte sequences, matching observed values:

```text
00 0C 00 0C 00 00  -> 3/3 hearts, 0 rupees
00 0C 00 0C 00 05  -> 3/3 hearts, 5 rupees
00 0C 00 0C 00 0F  -> 3/3 hearts, 15 rupees
```

These examples are reconstructed from the parsed values, not uploaded game dumps.

## Evidence

The initial address reference came from the public
[GameOffsets.cs mapping](https://github.com/jamesst20/ZeldaWindWakerTrainer-for-CEMU/blob/master/ZeldaWindWakerTrainer/GameOffsets.cs).
The application and memory reader were implemented independently. The observed
session on 2026-10-28 confirmed 0 → 5 → 15 rupees and 12/12 quarter hearts. The
user confirmed the 5-rupee display and the 10-rupee achievement notification.

The address `0x1506B500`, mentioned in other research, was also probed and did
not produce a valid header in that session. It is **not a supported fallback**.
It appears in the optional probe only to distinguish the earlier research cases.

The address model is based on
[Cemu 2.6 MMU.cpp](https://github.com/cemu-project/Cemu/blob/v2.6/src/Cafe/HW/MMU/MMU.cpp)
and
[MemMapperWin.cpp](https://github.com/cemu-project/Cemu/blob/v2.6/src/util/MemMapper/MemMapperWin.cpp).

## Unverified assumptions and research protocol

The fixed header's stability across real Cemu restarts, game loads, all menus,
and the full game remains unverified. No inventory, boss, or stage offset is
validated in this project yet. Do not infer those offsets from the header's
successful rupee test.

For each future field record:

1. Exact emulator/title version, region, Title ID and RPX hash.
2. Guest address or pointer derivation, width, byte order, and bounds.
3. Before/after values and the user-observed action they correspond to.
4. Whether the value is current, a stale copy, UI-only, or saved state.
5. Menu, area transition, save/load, and reconnect behavior.
6. Separate status: hypothesis, observed once, repeatable, or supported.

Use small targeted read-only observations. Keep raw memory and personal log
files out of the shareable project. Record facts and short sanitized examples.
