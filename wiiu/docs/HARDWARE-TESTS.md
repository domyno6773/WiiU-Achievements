# Native Wii U hardware validation log

## 2026-10-30 — confirmed earlier technical app

The user tested the initial read-only `OSScreen` WUHB on a real Wii U
running Aroma. It launched and displayed the Wind Waker HD catalog.
No monitoring/plugin behavior was tested. GamePad buttons, TV/DRC
individual output and exit behavior were not fully reported.

## 2026-10-30 — new high-fidelity GPU edition

A new `app/src/gpu_ui.c` frontend uses SDL2 accelerated GX2 and two
screen-specific layouts, embedded pre-rendered assets and original
Nunito. Cross-compilation and two host layout previews pass in CI.

**Physical GPU runtime status: NOT YET TESTED.** The earlier successful
OSScreen boot does not prove this version starts.

Required feedback: does the WUHB open, does the image appear on both
TV and GamePad, does moving the D-pad highlight another card without
lag, do A/B/X/Y/L/R/PLUS work, can B exit, are there visual/aural
glitches or missing resources? Compare against the original Windows
screenshot supplied in the development conversation.

The plugin is deliberately inactive and **must not be installed**.
