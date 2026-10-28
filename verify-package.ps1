$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath($PSScriptRoot).TrimEnd('\') + '\'
$manifestPath = Join-Path $PSScriptRoot 'package-manifest.json'
if (-not (Test-Path -LiteralPath $manifestPath)) { throw 'package-manifest.json is absent. Run this script from an extracted package.' }
$manifest = Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
if ($manifest.schemaVersion -ne 1 -or @($manifest.files).Count -eq 0) { throw 'Invalid package manifest.' }
$seen = @{}
foreach ($entry in $manifest.files) {
    $relative = [string]$entry.path
    if ([IO.Path]::IsPathRooted($relative) -or $relative.Contains(':')) { throw ('Invalid manifest path: ' + $relative) }
    $path = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot $relative))
    if (-not $path.StartsWith($root, [StringComparison]::OrdinalIgnoreCase)) { throw 'Manifest path leaves the package.' }
    if ($seen.ContainsKey($path)) { throw ('Duplicate manifest path: ' + $relative) }
    $seen[$path] = $true
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw ('Missing file: ' + $relative) }
    if ((Get-Item -LiteralPath $path).Length -ne $entry.bytes -or
        (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $entry.sha256) {
        throw ('Changed or damaged file: ' + $relative)
    }
}
Write-Output ('Package integrity verified: ' + $seen.Count + ' files. Manifest is an integrity list, not a digital signature.')
