# Instructions for coding agents

These instructions apply to this entire project. Read `HANDOFF.md` first, then
`docs/ARCHITECTURE.md`, `docs/MEMORY-MAP.md`, and `docs/VALIDATION.md` as needed.

- User goal: an independent achievement system for Wii U games emulated on PC.
- Build the implementation from scratch. Do not introduce EmuHook as a dependency.
- First target is Cemu 2.6, Wind Waker HD EUR v0, RPX `cdc68acd`.
- This development build has 70 executable objectives (including 53 explicitly experimental detectors requested for progressive playtesting) and 40 unimplemented targets. Only the recorded live findings are verified; see `docs/PLAYTHROUGH.md` and `docs/VALIDATION.md`.
- Production process access is read-only. Preserve that design while extending it.
- Do not include game files, console keys, real memory dumps, personal logs,
  `data/`, or other user state in shared archives or source control.
- Keep C# 5 / .NET Framework compatibility and x64. There is no NuGet dependency.
- Build: `powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1`.
- Test: `powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run.ps1` (Windows x64, no game needed).
- Package: `powershell -NoProfile -ExecutionPolicy Bypass -File .\package.ps1`.
- The app may be running beside Cemu. Build to a separate output directory using
  `build.ps1 -OutputDirectory <path>` instead of disrupting the live session.
- Never treat a guest address as a host process address: host = guestBase + guest.
- Keep verified findings separate from hypotheses. New offsets need live evidence
  for the exact region/version, plus menu/load/reconnect checks before release.
- Preserve achievement IDs and existing progress. Changing set revision changes
  progress identity; document and implement migration if that is intentional.
- Synthetic tests do not prove a complete real game set or restart compatibility.
- Add regression checks for engine, persistence, or memory-reader changes; do not
  require a real ROM or a running Cemu process for automated tests.
- Update `docs/VALIDATION.md` with what was actually checked, and update the
  handoff/roadmap when the supported configuration or next task changes.
- Read `docs/COLLABORATION.md` before combining contributions from other agents.
- The UI is custom-drawn (`docs/UI.md`). Keep artwork and sounds original and
  code-generated. Do not add Nintendo images, logos, Miis, sounds or system fonts.
  The only third-party asset is Nunito (SIL OFL 1.1): keep `assets/fonts/OFL.txt`.
- UI state must not change engine, memory or persistence semantics. Keep sampling
  in `MainForm.cs` and presentation in `MainForm.Ui.cs`.
