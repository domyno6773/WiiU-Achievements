# Wind Waker HD — expanded achievement catalog

The three XML files contain **110 targets**: **70 executable objectives** (17
previous rules plus **53 implemented experiments**) and **40 unimplemented targets**. This is an additive expansion
under `wwhd-alpha-1`, not a claim of a working full-game achievement set.
All ten original IDs, thresholds and points remain unchanged. Existing local
progress loads under the same identity and filename, without a migration.

## What can unlock now

The **53 new rules have real decoders and execute in the engine**, at the user's
explicit request to implement batches before they verify them while playing.
They are clearly experimental, not live-certified support. They cover 20 story
milestones, all 21 equipment targets, 8 song/bottle collections, 3 sea-chart
milestones and Grandma's healing/soup quest. See `PLAYTHROUGH.md` for the complete
chronological groups and `MEMORY-MAP.md` for source-derived mapping hypotheses.

The ten original rules plus wallets of 2500/5000 rupees and heart capacities of
5/8/12/18 hearts. These six additions reuse only the existing six-byte header.
Every rule still requires three below-threshold samples, then three matching
samples, and persists through the existing atomic award path. High values on
connection do not grant retrospective awards. Real-game evidence remains limited
to the recorded early rupee test for these thresholds; the new high thresholds
are tested synthetically. The tunic additionally uses an observed event bit,
decoded into boolean `hero_clothes`, with a threshold of one. Its stable absent
baseline and acquisition use the same confirmation/persistence path. Connect
before receiving it; loading an already-clothed save does not unlock it.
Manual pause/disconnect is still required when loading saves. See the current
evidence in `MEMORY-MAP.md` and `VALIDATION.md`.

## What is prepared, with detection still pending

- **5 story targets:** tower rising, the two sage awakenings, Puppet Ganon and
  Ganondorf. Their exact conditions remain unimplemented.
- **8 exploration targets:** chart-based salvage, Ghost Ship, Big Octos and
  the Savage Labyrinth milestones.
- **10 collection targets:** heart pieces, normal/decoded treasure charts,
  Tingle statues and the Nintendo Gallery.
- **17 side-quest targets:** specific minigame/quest rewards still require
  sufficiently precise mappings. Generic hearts or rupees are not substitutes.

Titles and descriptions are original Italian text. IDs describe the exact
intended accomplishment, so later implementation must not silently weaken the
condition. Each pending entry includes a short detection requirement. Points are
proposals, shown as such in the detail card; they do not contribute to earned or
available points. Completion percentage counts only executable objectives.

The UI provides status tabs, a category cycle button (**C**) and chapter cycle
button (**F**). Optional `order`/`chapter` metadata sort the approximate route
without imposing gameplay prerequisites. Selecting a
pending card shows its intended condition and the missing verification. Selecting
the catalog, filters or a detail card never changes recording state.

## Version-specific research and sources

These sources establish gameplay targets, not verified addresses. The guide
authors' gameplay observations were used to cross-check HD changes. Some guides
also cover GameCube: use only their explicitly HD-compatible facts.

- [Link's Hideaway HD walkthrough](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/):
  chapter order and story coverage. Its [Outset chapter](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/outset-island/)
  supports the initial green tunic/telescope/sword/shield and first Niko trial sequence.
- [Dragon Roost Cavern](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/dragon-roost-cavern/),
  [Forbidden Woods](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/forbidden-woods/),
  [Tower of the Gods](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/tower-of-the-gods/),
  [Earth Temple](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/earth-temple/),
  [Wind Temple](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/wind-temple/)
  and [Ganon's Tower](https://www.linkshideaway.com/the-wind-waker-hd-walkthrough/ganons-tower/):
  dungeon tools, boss identities, story events and late-game equipment.
- [Zelda Dungeon's HD chart/heart research](https://www.zeldadungeon.net/the-wind-waker-hd-treasure-charts-and-heart-pieces/):
  HD has 46 normal treasure charts; chart 46 leads to the heart piece moved from
  the bottom of the Savage Labyrinth. The labyrinth instead awards Hero's Charm.
  Mrs. Marie's later pendant reward is chart 45 in HD.
- [Zelda Dungeon's Triforce chapter](https://www.zeldadungeon.net/the-wind-waker-walkthrough/the-triforce/):
  chart 46's Wizzrobe cave and HD heart-piece location.
- [Prima's original HD heart-piece guide](https://primagames.com/news/legend-zelda-wind-waker-hd-pieces-heart-table):
  44 heart pieces and the associated side quests, minigame prizes and letters.
- [Link's Hideaway Tingle statues guide](https://www.linkshideaway.com/the-wind-waker-hd/tingle-statues/):
  five dungeon statues, obtainable with regular bombs in HD.
- [A player's HD completion route](https://gist.github.com/chasestubblefield/10670979):
  cross-check for four bottles, six songs, three HD Triforce charts, equipment
  capacity upgrades and the side-quest sequence. Not used as a memory map.
- [Nintendo's original HD manual](https://csassets.nintendo.com/noaext/image/private/t_KA_PDF/manual-WiiU-The_Legend_of_Zelda_The_Wind_Waker_HD?_a=DATAg1AAZAA0):
  distinction between Sea Chart, dungeon maps and treasure charts.

The catalog intentionally has no Miiverse/Tingle Bottle sharing objective, no
Tingle Tuner dependency, no requirement to acquire eight Triforce charts, no
GameCube-only firefly requirement and no invented no-damage/speedrun detector.
The gallery completion target does not assume an unverified figurine count.

## Promotion contract

`<achievement>` is an executable threshold on a supported field. `<planned>` is
catalog metadata with **no field, threshold or executable condition**. The loader
rejects executable attributes on pending entries. The engine reports
`PendingValidation` and never calls `Met` for them, including in demo mode or
when a progress store contains the ID. Presentation ignores such store entries.

`achievements.xml` contains only the 16 executable thresholds, retaining the
format understood by the older running application. `achievements-planned.xml`
contains the 40 pending targets and is explicitly associated with Wind Waker by
the game catalog. New builds load both, validate the same revision and reject
duplicate IDs across files or executable rules in the pending companion. This
avoids breaking the old root EXE on restart while a separate build is reviewed.
`achievements-story.xml` holds the tunic plus the 53 experimental rules, explicitly associated
with the current reader. The loader rejects duplicate IDs across all three
files, pending entries in active extensions, and active entries in pending files.

To implement a pending target, retain its ID, implement a snapshot field
and condition, add regression coverage, then replace its metadata entry with an
executable definition. The registered field decoder can normalize bits, count
set bits or combine specific conditions before the stable threshold engine.
All new unverified progression fields require `experimental="true"` in the
schema, preventing silent promotion by removing the label. Promotion to verified
support requires the evidence below; the development build is explicitly for
progressive playtesting.

Validation must include the exact EUR v0 RPX, before/after acquisition, menus,
area transitions, save/load, reconnect and Cemu restart. Establish gameplay and
save continuity before awarding story progress; a loaded completed save must not
look like a live accomplishment. Do not infer bosses from heart capacity,
delivered collectibles from current bag quantities, or bit counts from a byte's
numeric magnitude. Do not award a quest solely from a generic heart increase.

The first three inventory candidates can be observed with
`powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\inspect-initial.ps1`.
This manual tool requires exactly one matching live Cemu session and cross-checks
the guest reservation before decoding the selected `0x724`-byte state window. It
prints only interpreted values and the four initial research bytes and
saves no logs/dumps/progress. It is not part of the automated suite and does not
enable achievements. See `MEMORY-MAP.md` for evidence status.

The first story target is **Il colore dell'avventura** (`story-heros-clothes`),
receiving the green tunic from Grandma, before the telescope. The user reported
already receiving it on 2026-10-28 and later repeated the acquisition. The event
bit was observed set, then clear after loading a pre-acquisition save, then set
again after receipt, and stable after leaving the house and opening the menu.
The rule is now active with the same ID. No award is fabricated from a user
report; the engine requires a newly observed live acquisition.
