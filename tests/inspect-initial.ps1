$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$probeRoot = Join-Path ([IO.Path]::GetTempPath()) ('WiiUAchievements-initial-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $probeRoot | Out-Null
$exe = Join-Path $probeRoot 'InitialItemProbe.exe'
& $compiler /nologo /langversion:5 /target:exe /platform:x64 /utf8output /r:System.Xml.Linq.dll "/out:$exe" (Join-Path $project 'src\Core.cs') (Join-Path $project 'src\WindWakerProgression.cs') (Join-Path $project 'src\MemoryReader.cs') (Join-Path $project 'src\GameSession.cs') (Join-Path $project 'src\GameCatalog.cs') (Join-Path $PSScriptRoot 'InitialItemProbe.cs')
if ($LASTEXITCODE -ne 0) { throw 'Compilazione diagnostica fallita.' }
& $exe
if ($LASTEXITCODE -ne 0) { throw 'Osservazione iniziale non riuscita.' }
