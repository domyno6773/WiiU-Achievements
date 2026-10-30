# Upstream synchronization — fork-only development

## Repositories and branch guarantees

- **Original / upstream**: `hoffo1987/WiiU-Achievements`, branch `main`.
- **Our fork / origin**: `domyno6773/WiiU-Achievements`, branch `wiiu-native`.
- `main` in either repository is **not** a target for port commits.
- There are **no** automatic pushes, merges, pull requests or force updates
  to upstream from the Wii U port.

This fork retains ancestry with the original project. Changes to Windows/Cemu
source, canonical XML, achievements metadata and cloud logic should be reviewed
for impact before adapting the native port. Updating from upstream does **not**
mean the Wii U now supports Windows-only features such as Windows DPAPI or the
Firebase cloud client.

## Initial synchronization

- 2026-10-30: merged upstream `main` at
  `05cb1ca51acda3bf0c5602029919d0746e4f063e`
  into the fork's `wiiu-native`, preserving the original port files.
- The upstream change added Firebase project rules/configuration and Windows
  UI/login updates. **No native Firebase integration is implied.**
- The native port started from `f9d2453e382c30af70c5ad9c9c1b6cf48258f091`;
  the new merge brings the original source history up to date.

## Safe manual sync (for developers using Git)

Always verify the current repository and working branch before making changes.
If `upstream` isn't configured:

```bash
git remote add upstream https://github.com/hoffo1987/WiiU-Achievements.git
```

Then:

```bash
git fetch upstream main
git checkout wiiu-native
git status --short
git log --oneline --max-count=10 wiiu-native..upstream/main
git diff --stat wiiu-native...upstream/main
# After reviewing changes and confirming a clean working tree:
git merge --no-ff upstream/main
python3 -m unittest discover -s wiiu/tests -p 'test_*.py' -v
# Only after reviewing merge results, platform build and diffs:
git push origin wiiu-native
```

If there are conflicts, resolve only with a reviewed mapping from the updated
canonical rules to the port; do not silently overwrite Windows logic or native
code. For significant XML/engine changes, regenerate the Wii U C catalog and
revalidate behavior. The GitHub Actions native build runs only on our
`wiiu-native` branch if Actions are enabled on the fork.

**Never** use `git push upstream`, `git push origin main`, or force-push an
existing shared branch as part of this process. Never merge `wiiu-native` into
upstream without an explicit collaboration decision.

## AI-assisted sessions

At the start of each continuation, read both repository heads and compare
`upstream/main` against the fork's `wiiu-native`. Only touch
`domyno6773/WiiU-Achievements:wiiu-native`; treat upstream as read-only.
Describe and review new upstream changes before syncing, then use a
non-forced commit preserving both histories. Confirm the final branch diff
contains native changes only and run the available tests.

This check happens **when a development session is requested**, not continuously
in the background.
