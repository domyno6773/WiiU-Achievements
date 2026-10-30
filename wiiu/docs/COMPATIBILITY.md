# Compatibility & parity contract

This file defines the native port's relationship to upstream. Do not modify root rules to suit the port.

## Canonical references

- `achievements.xml`: threshold rules.
- `achievements-story.xml`: hero clothing and experimental story/equipment rules.
- `achievements-planned.xml`: catalog entries without executable conditions.
- `achievement-presentation.xml`: rarity, order and editorial overrides.
- `src/Core.cs`: source-of-truth evaluation and local progress semantics.
- `docs/ARCHITECTURE.md`, `docs/VALIDATION.md`, `docs/RARITIES.md`, `docs/PLAYTHROUGH.md`: interpretation and empirical limits.

The source project currently uses a stable set revision `wwhd-alpha-1` for Wind Waker HD. Preserve original IDs, points, rarity, chapter order and labels unless upstream itself revises them. Cemu's exact game fingerprint must not be assumed to identify all physical-disc / installed-title variants without verification.

## Engine semantics to port faithfully

- Never award a condition already fulfilled at initial attachment.
- Require three valid below-threshold samples to arm and three at/above-threshold samples to confirm the crossing, per upstream `AchievementEngine`.
- Clear transient state on invalid readings, time reversals, excessive sample gaps and relevant permanent-value regressions.
- Commit simultaneously satisfied achievements together before showing notifications.
- Never unlock planned/non-executable entries; preserve experimental labels for provisional detectors.
- Keep earned IDs durable across app/game restarts; handle backup and recovery with hardware-appropriate safe writes.
- Preserve per-profile and per-game identity with an explicit compatibility/migration design; do not silently import another user's achievement history.

## Visual and interaction parity

Reference screenshots: `docs/screenshots/*.png`. Reference code: `src/MainForm.Ui.cs`, `src/Theme.cs`, `src/Banner.cs`, `src/Notification.cs`.

Match colors, typography (only properly licensed fonts), medals, completion display, order/filter behavior, unlock text, notifications, animations, sound and accessible focus patterns where feasible. Translate mouse/keyboard patterns to D-pad/GamePad controls without redesigning the user experience. Document any rendering or API limitation rather than claiming pixel identity.

## Live test log template

For every newly tested Wii U reading, record: console environment/version, game title ID/version/region, exact observed event, previous/next interpreted value, in-game vs menu/load behavior, persistence after area transition and restart, test date and whether the evidence was synthetic or physical. Never commit host-private logs, savegames, keys or memory dumps.

## Upstream synchronization policy

Develop only on `domyno6773/WiiU-Achievements:wiiu-native`.
The Windows source of truth is `hoffo1987/WiiU-Achievements:main`.

When upstream changes, inspect its diff and merge/cherry-pick only after evaluating
impacts on source XML, catalog generation, rule semantics and API compatibility.
Never push changes to the upstream repository, update either `main` from the
native port workflow, or open an automatic pull request into upstream `main`.
The owner can continue Windows development independently.

See [UPSTREAM-SYNC.md](UPSTREAM-SYNC.md) for the concrete safe process.
