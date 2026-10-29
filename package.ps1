param(
    [string]$DestinationPath = (Join-Path (Split-Path -Parent $PSScriptRoot) 'WiiUAchievements-handoff.zip'),
    [switch]$SkipTests
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
$tempPrefix = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\') + '\'
$stageRoot = Join-Path $tempPrefix ('WiiUAchievements-package-' + [guid]::NewGuid().ToString('N'))
$stage = Join-Path $stageRoot 'WiiUAchievements'
$destination = [IO.Path]::GetFullPath($DestinationPath)
New-Item -ItemType Directory -Path $stage -Force | Out-Null
try {
    # Deliberate allowlist: local settings, earned progress and new unreviewed files
    # cannot accidentally enter an archive simply because they are in the folder.
    $names = @('AGENTS.md', 'HANDOFF.md', 'README.md', 'NEXT-STEPS.md', '.gitignore',
        'build.ps1', 'package.ps1', 'verify-package.ps1',
        'src\CloudProject.cs', 'src\Gameplay.cs', 'src\CloudSync.cs', 'src\GameCatalog.cs', 'src\MainForm.Library.cs', 'src\MainForm.Tiles.cs', 'src\Core.cs', 'src\GameSession.cs', 'src\MainForm.cs', 'src\MainForm.Ui.cs', 'src\MemoryReader.cs', 'src\Program.cs',
        'src\Theme.cs', 'src\Banner.cs', 'src\Notification.cs', 'src\WindWakerProgression.cs',
        'assets\fonts\Nunito-Regular.ttf', 'assets\fonts\Nunito-Bold.ttf', 'assets\fonts\Nunito-ExtraBold.ttf', 'assets\fonts\OFL.txt',
        'docs\UI.md', 'docs\GAMES.md', 'docs\screenshots\games.png', 'docs\screenshots\paused.png', 'docs\screenshots\main.png', 'docs\screenshots\unlock.png', 'docs\screenshots\detail.png', 'docs\screenshots\settings.png', 'docs\screenshots\toast.png',
        'tests\GameplayTests.cs', 'tests\CloudTests.cs', 'tests\CoreTests.cs', 'tests\SystemTests.cs', 'tests\ProgressionTests.cs', 'tests\UiTests.cs', 'tests\Probe.cs', 'tests\InitialItemProbe.cs', 'tests\inspect-initial.ps1', 'tests\run.ps1', 'tests\run.sh', 'tests\run-ui.ps1',
        'docs\GROK-REVIEW-2026-10-29.md', 'docs\AUTOMATIC-LOADS.md', 'docs\CLOUD.md', 'docs\ARCHITECTURE.md', 'docs\ACHIEVEMENTS.md', 'docs\PLAYTHROUGH.md', 'docs\RARITIES.md', 'docs\MEMORY-MAP.md', 'docs\VALIDATION.md', 'docs\COLLABORATION.md',
        '.cursor\rules\project.mdc', '.github\workflows\validate.yml')
    foreach ($name in $names) {
        $source = Join-Path $PSScriptRoot $name
        if (-not (Test-Path -LiteralPath $source)) { throw ('Missing package input: ' + $name) }
        $target = Join-Path $stage $name
        New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
        Copy-Item -LiteralPath $source -Destination $target -Force
    }
    & (Join-Path $PSScriptRoot 'build.ps1') -OutputDirectory $stage
    if (-not $SkipTests) { & (Join-Path $PSScriptRoot 'tests\run.ps1') }
    $entries = @(Get-ChildItem -LiteralPath $stage -File -Recurse -Force | Sort-Object FullName | ForEach-Object {
        [ordered]@{
            path = $_.FullName.Substring($stage.Length + 1).Replace('\', '/')
            bytes = $_.Length
            sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        }
    })
    $manifest = [ordered]@{
        schemaVersion = 1
        project = 'Wii U Achievements'
        version = 'alpha-0.3-playthrough-experimental'
        generatedAtUtc = [DateTime]::UtcNow.ToString('o')
        testsPassedDuringPackaging = (-not $SkipTests.IsPresent)
        personalDataIncluded = $false
        files = $entries
    }
    $manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $stage 'package-manifest.json') -Encoding UTF8
    & (Join-Path $stage 'verify-package.ps1')
    # Build solely from packaged sources, catching omitted source dependencies.
    & (Join-Path $stage 'build.ps1') -OutputDirectory (Join-Path $stageRoot 'rebuild-check')
    $temporaryZip = Join-Path $stageRoot 'package.zip'
    [IO.Compression.ZipFile]::CreateFromDirectory($stage, $temporaryZip)
    New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
    Move-Item -LiteralPath $temporaryZip -Destination $destination -Force
    Write-Output ('Shareable package: ' + $destination)
    Write-Output ('SHA256: ' + (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash)
}
finally {
    # Delete only the private directory created above, under this machine's temp.
    $resolved = [IO.Path]::GetFullPath($stageRoot)
    if ($resolved.StartsWith($tempPrefix, [StringComparison]::OrdinalIgnoreCase) -and
        ([IO.Path]::GetFileName($resolved) -match '^WiiUAchievements-package-[0-9a-f]{32}$')) {
        Remove-Item -LiteralPath $resolved -Recurse -Force
    }
    else { throw 'Unexpected staging directory; automatic cleanup refused.' }
}
