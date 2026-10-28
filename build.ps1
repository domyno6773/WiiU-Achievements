param([string]$OutputDirectory = $PSScriptRoot)
$ErrorActionPreference = 'Stop'
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
if (-not (Test-Path -LiteralPath $compiler)) { throw 'Compilatore .NET Framework a 64 bit non trovato.' }
$output = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $output -Force | Out-Null
$sources = Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'src') -Filter '*.cs' | ForEach-Object FullName
& $compiler /nologo /target:winexe /platform:x64 /optimize+ /utf8output /r:System.Xml.Linq.dll /r:System.Windows.Forms.dll /r:System.Drawing.dll "/out:$output\WiiUAchievements.exe" $sources
if ($LASTEXITCODE -ne 0) { throw 'Compilazione applicazione fallita.' }
if ($output.TrimEnd('\') -ne $PSScriptRoot.TrimEnd('\')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'achievements.xml') -Destination $output -Force
}
Write-Output ('Applicazione compilata: ' + (Join-Path $output 'WiiUAchievements.exe'))
