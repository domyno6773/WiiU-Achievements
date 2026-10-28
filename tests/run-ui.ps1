param([string]$OutputDirectory = (Join-Path ([IO.Path]::GetTempPath()) ('WiiUAchievements-ui-' + [guid]::NewGuid().ToString('N'))))
$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$output = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $output -Force | Out-Null
& (Join-Path $project 'build.ps1') -OutputDirectory $output
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$exe = Join-Path $output 'UiTests.exe'
$application = Join-Path $output 'WiiUAchievements.exe'
& $compiler /nologo /langversion:5 /target:exe /platform:x64 /utf8output /r:System.Drawing.dll /r:System.Windows.Forms.dll /r:Accessibility.dll "/r:$application" "/out:$exe" (Join-Path $PSScriptRoot 'UiTests.cs')
if ($LASTEXITCODE -ne 0) { throw 'Compilazione test interfaccia fallita.' }
& $exe (Join-Path $output 'previews')
if ($LASTEXITCODE -ne 0) { throw 'Test interfaccia falliti.' }
