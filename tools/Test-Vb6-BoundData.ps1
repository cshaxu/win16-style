[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$FixtureDir,
    [Parameter(Mandatory)][string]$Observer,
    [Parameter(Mandatory)][string]$Launcher,
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='bound-data'
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$scratchRoot=Join-Path $repo 'build/t18-s64-vb-data'
if(!(Test-Path -LiteralPath $FixtureDir -PathType Container)){Write-Output 'BOUND_DATA_SKIP missing=fixture-directory';exit 2}
$fixture=(Resolve-Path -LiteralPath $FixtureDir).Path
$observerPath=(Resolve-Path -LiteralPath $Observer).Path
$launcherPath=(Resolve-Path -LiteralPath $Launcher).Path
$files=[ordered]@{
    'research.exe'='3d46e6384cfd8d458031059f636160c17ff25eeedb4be6fddaf9bb3f85013f92'
    'LICENSE.txt'='9315466538cc1e4703d5d5eb63291b9a0552b597f35edb615fe4cb641df32757'
    'dao_bridge.dll'='ff6afd4cec3922aa7666c5323ba41a61eaf7bf91fa08aea8db6dbfa2abaadbb5'
    'dao350.dll'='a4b20735be317a924d2e36707baaf911fbae890ca53c5044fb506f15d33bcb6d'
    'VB5DB.DLL'='d7cf39e673a87fac5d5abaf81c572d422675b9f0fbe18d4eb4e7c20f3d3038cd'
    'mscomctl.ocx'='152121a5d1ac8f12002c18afc294bb1ebcecc1d61deec6211df586c11acde9b6'
    'Data01/Biblio.mdb'='dfb0194a5b9d7986d5a5ce2ca86567c245ebb97a3792dd1142b1327a077b599c'
}
foreach($relative in $files.Keys){
    $source=Join-Path $fixture $relative
    if(!(Test-Path -LiteralPath $source -PathType Leaf)){Write-Output "BOUND_DATA_SKIP missing=$relative";exit 2}
    for($part=$source;$part;$part=Split-Path $part -Parent){if((Get-Item -LiteralPath $part -Force).Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Fixture traverses a reparse point'}}
    if((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant() -ne $files[$relative]){throw "Unexpected fixture identity: $relative"}
}
$run=Join-Path $scratchRoot $RunName
if(Test-Path -LiteralPath $run){throw 'Use a fresh run name'}
New-Item -ItemType Directory -Path $run | Out-Null
foreach($relative in $files.Keys){$dest=Join-Path $run $relative;New-Item -ItemType Directory -Force -Path (Split-Path $dest -Parent) | Out-Null;Copy-Item -LiteralPath (Join-Path $fixture $relative) -Destination $dest}
$before=$env:WIN16STYLE_CAPTURE_DIR
Push-Location $run
try {
    foreach($mode in @('bare','styled')){
        $capture=Join-Path $run $mode;New-Item -ItemType Directory -Path $capture | Out-Null
        $env:WIN16STYLE_CAPTURE_DIR=$capture
        $launch=if($mode -eq 'bare'){'bare'}else{$launcherPath}
        & $observerPath $launch (Join-Path $run 'research.exe') 'vb6-bound-data' *> (Join-Path $capture 'output.txt')
        if($LASTEXITCODE){throw "Bound sample failed: $mode exit=$LASTEXITCODE"}
        $output=Get-Content -Raw -LiteralPath (Join-Path $capture 'output.txt')
        if($output -notmatch 'BOUND_NAV_OK first=1 next=2 last=727 previous=726 restored=1' -or $output -notmatch "SAMPLE_PASS styled=$(if($mode -eq 'bare'){0}else{1}) count=23 active=0"){throw "Bound sample markers absent: $mode"}
        if((Get-FileHash -LiteralPath (Join-Path $run 'Data01/Biblio.mdb') -Algorithm SHA256).Hash.ToLowerInvariant() -ne $files['Data01/Biblio.mdb']){throw "Private database changed: $mode"}
    }
} finally {Pop-Location;$env:WIN16STYLE_CAPTURE_DIR=$before}
Write-Output 'BOUND_DATA_PAIR_OK native=1 styled=1 next=2 last=727 previous=726 restored=1 private_db_unchanged=1 normal_exit=1'
