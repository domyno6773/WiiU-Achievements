param([string]$JavaHome = $env:JAVA_HOME)
$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$priorJava = $env:JAVA_HOME
$priorPath = $env:PATH
$priorRules = $env:FIRESTORE_RULES_FILE
try {
    if ($JavaHome) {
        if (-not (Test-Path -LiteralPath (Join-Path $JavaHome 'bin\java.exe'))) { throw 'JavaHome must contain bin\java.exe.' }
        $env:JAVA_HOME = $JavaHome
        $env:PATH = (Join-Path $JavaHome 'bin') + ';' + $env:PATH
    }
    $env:FIRESTORE_RULES_FILE = Join-Path $project 'firestore.rules'
    Push-Location $project
    try {
        # demo- project and loopback guards in the test prevent production writes.
        & npx.cmd -y firebase-tools@latest emulators:exec --only firestore --project demo-wiiu-rules --config (Join-Path $project 'firebase.json') 'node tests/firestore-rules.cjs'
        if ($LASTEXITCODE -ne 0) { throw 'Firestore rules tests failed.' }
    }
    finally { Pop-Location }
}
finally {
    $env:JAVA_HOME = $priorJava
    $env:PATH = $priorPath
    $env:FIRESTORE_RULES_FILE = $priorRules
}
