$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('WiiUAchievements-tests-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
$exe = Join-Path $testRoot 'CoreTests.exe'
& $compiler /nologo /target:exe /platform:x64 /utf8output /r:System.Xml.Linq.dll "/out:$exe" (Join-Path $project 'src\Core.cs') (Join-Path $project 'src\MemoryReader.cs') (Join-Path $PSScriptRoot 'CoreTests.cs')
if ($LASTEXITCODE -ne 0) { throw 'Compilazione test fallita.' }
& $exe (Join-Path $testRoot 'results')
if ($LASTEXITCODE -ne 0) { throw 'Test falliti.' }
