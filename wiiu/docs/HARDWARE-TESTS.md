# Wii U hardware smoke test log

## 2026-10-30 — First native catalog app, physical Wii U

**Reported by:** the Wii U port developer (first-hand user feedback).

**App:** `WiiUAchievementsNative.wuhb` compiled in upstream Actions run
[`40978345462`](https://github.com/hoffo1987/WiiU-Achievements/actions/runs/40978345462)
from commit `9ac106357c77d20b0d94879b2242725b97ecd0b3`.

**Environment:** real Nintendo Wii U, Aroma homebrew environment as stated by tester.
Firmware, region, Aroma revision and console-model details were not captured.

### Observed and confirmed

- The WUHB application opens on a real Wii U.
- The UI displays *The Legend of Zelda: The Wind Waker HD*.
- The UI displays the achievement catalog.

These observations establish the **first native app startup/display smoke test**.
The Windows game's UI parity, installed-title discovery and foreground/background
monitoring are **not** proven.

### Not yet verified or reported

- Full GamePad input navigation: A, B, X, Y and D-pad.
- TV and GamePad display independently (which screen(s) showed the UI).
- Exit/HOME behavior, restarts and stress tests.
- All 110 details, filters, categories, special characters and layout.
- Running a WUPS plugin on hardware.
- Title memory reads, live unlock detection, persistence or in-game notifications.

**Safety:** The tested WUHB is a read-only catalog viewer. There is no native
unlock engine or game-memory access in this build. The inert WUPS plugin has
**not** been installed/tested as part of this report.

## Next checks

Confirm both displays, basic navigation, B/HOME exit behavior and whether text
wraps or truncates on the GamePad. Record any discrepancies and build SHA.
Do not install the experimental WUPS plugin until its own smoke-test plan
and explicit test build are ready.
