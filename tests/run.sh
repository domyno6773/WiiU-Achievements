#!/usr/bin/env bash
set -euo pipefail
project="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
command -v mcs >/dev/null || { echo 'Mono mcs is required.' >&2; exit 1; }
command -v mono >/dev/null || { echo 'Mono is required.' >&2; exit 1; }
test_root="$(mktemp -d "${TMPDIR:-/tmp}/wiiua-tests.XXXXXXXX")"
mcs -langversion:5 -platform:x64 -r:System.Xml.Linq -r:System.Web.Extensions -r:System.Security -out:"$test_root/CoreTests.exe" \
    "$project/src/CloudSync.cs" "$project/tests/GameplayTests.cs" "$project/tests/CloudTests.cs" "$project/src/Gameplay.cs" "$project/src/Core.cs" "$project/src/MemoryReader.cs" "$project/src/GameSession.cs" \
    "$project/src/GameCatalog.cs" "$project/src/WindWakerProgression.cs" "$project/tests/CoreTests.cs" "$project/tests/SystemTests.cs" "$project/tests/ProgressionTests.cs"
cp -- "$project/achievements.xml" "$test_root/achievements.xml"
cp -- "$project/achievements-planned.xml" "$test_root/achievements-planned.xml"
cp -- "$project/achievements-story.xml" "$test_root/achievements-story.xml"
cp -- "$project/achievement-presentation.xml" "$test_root/achievement-presentation.xml"
mono "$test_root/CoreTests.exe" --portable "$test_root/results"
