[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [string]$TestResult,
    [string]$BuildResult,
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='publish',
    [ValidateRange(1,65535)][int]$Task=9,
    [switch]$ValidateOnly
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
. (Join-Path $PSScriptRoot 'Select-PackageCases.ps1')
$contract=Get-Content -LiteralPath (Join-Path $repo 'docs/etc/product-contract.json') -Raw | ConvertFrom-Json
$names=@($contract.themes)+@($contract.components.artifacts.name)
$names=@($names | Where-Object {$_ -ne 'run16s.exe'})+@('run16s.exe')
$Package=(Resolve-Path -LiteralPath $Package).Path
if([bool]$TestResult -eq [bool]$BuildResult){throw 'Provide exactly one of BuildResult or TestResult'}
$gate=@($Package,'--task',$Task)
if($BuildResult){
    $build=Get-Content -LiteralPath $BuildResult -Raw|ConvertFrom-Json
    if($build.status -ne 'passed' -or $build.exit_code -ne 0 -or
       (Resolve-Path -LiteralPath $build.candidate).Path -ne $Package){throw 'A passed matching clean build is required'}
    $buildRoot=Join-Path $build.clean_source 'build/.ninja'
    $gate+=@('--source-root',$build.clean_source,'--built-root',$buildRoot)
    $hashes=[ordered]@{};foreach($name in $names){$hashes[$name]=(Get-FileHash -LiteralPath (Join-Path $Package $name) -Algorithm SHA256).Hash.ToLowerInvariant()}
    $tested=@{status='pending-post-publication';skipped=@();sha256=$hashes}
}else{
    $tested=Get-Content -LiteralPath $TestResult -Raw|ConvertFrom-Json
    if($tested.status -notin @('passed','passed-with-skips','passed-focused','passed-focused-with-skips')){throw 'Only a passed actual-package result may be published'}
    Assert-PackageCoverage -Result $tested
}
$verified=& python -B (Join-Path $repo 'tools/check_package.py') @gate
if($LASTEXITCODE){throw 'Package identity gate failed'}
$manifest=($verified -join "`n") | ConvertFrom-Json -AsHashtable
$baseline=Get-Content -LiteralPath (Join-Path $repo 'docs/etc/release.json') -Raw | ConvertFrom-Json -AsHashtable
$destinations=@((Join-Path $repo 'assets'),'O:\winnt')
$scratch=Join-Path $repo "build/t$Task-package/$RunName"
function Hash([string]$Path){(Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()}
function Assert-RegularPath([string]$Path,[bool]$Directory){
    for($ancestor=$Path;$ancestor;$ancestor=Split-Path $ancestor -Parent){
        if(!(Test-Path -LiteralPath $ancestor)){continue}
        $item=Get-Item -LiteralPath $ancestor -Force
        if(($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -or
           $item.PSIsContainer -ne ($Directory -or $ancestor -ne $Path)){throw "Unexpected path type: $ancestor"}
    }
}
foreach($name in $names){
    $digest=if($manifest.artifacts.Contains($name)){$manifest.artifacts[$name].sha256}else{$manifest.theme_artifacts[$name]}
    if(!$digest -or $digest -ne $tested.sha256.($name)){throw "Tested hash mismatch: $name"}
}
$records=[System.Collections.Generic.List[object]]::new()
Assert-RegularPath $scratch $true
foreach($destination in $destinations){
    if(!(Test-Path -LiteralPath $destination -PathType Container) -or
       [IO.Path]::GetPathRoot($destination) -ine [IO.Path]::GetPathRoot($scratch)){throw "Invalid or cross-volume destination: $destination"}
    foreach($name in ($names+@($contract.retired))){
        $target=Join-Path $destination $name
        Assert-RegularPath $target $false
        $before=if(Test-Path -LiteralPath $target){Hash $target}else{$null}
        $retired=$name -in $contract.retired
        $previous=if($baseline.artifacts.Contains($name)){$baseline.artifacts[$name].sha256}
            elseif($baseline.theme_artifacts.Contains($name)){$baseline.theme_artifacts[$name]}
        if($retired -and $before -and $previous -ne $before){throw "Unrecognized file at retirement path: $target"}
        $after=if(!$retired){$tested.sha256.($name)}else{$null}
        if($name -in $contract.themes -and $before -and $before -ne $after -and $before -ne $previous){
            throw "Unrecognized file at theme destination: $target"
        }
        if($before -ne $after){
            $index=$records.Count
            $records.Add([pscustomobject]@{name=$name;path=$target;before=$before;expected=$after;
                backup=(Join-Path $scratch "before-$index");staged=(Join-Path $scratch "next-$index");
                saved=$false;placed=$false})
        }
    }
}
if($ValidateOnly){'PUBLISH VALIDATION PASS: seven files and retirement ownership verified';return}
if(Test-Path -LiteralPath $scratch){throw 'Use a fresh publication run name; preserve prior backups'}
New-Item -ItemType Directory -Path $scratch | Out-Null
$receipt=Join-Path $scratch 'result.json'
foreach($record in $records){
    if(!$record.expected){continue}
    Copy-Item -LiteralPath (Join-Path $Package $record.name) -Destination $record.staged
    if((Hash $record.staged) -ne $record.expected){throw "Staged digest mismatch: $($record.path)"}
    New-Item -ItemType Directory -Force -Path (Split-Path $record.path -Parent) | Out-Null
}
@{status='prepared';files=@($records)} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $receipt -Encoding utf8
try{
    foreach($record in $records){
        $current=if(Test-Path -LiteralPath $record.path){Hash $record.path}else{$null}
        if($current -ne $record.before){throw "Destination changed since preflight: $($record.path)"}
        if($record.before){[IO.File]::Move($record.path,$record.backup);$record.saved=$true}
        if($record.expected){[IO.File]::Move($record.staged,$record.path);$record.placed=$true}
    }
    foreach($destination in $destinations){
        foreach($name in $names){
            if((Hash (Join-Path $destination $name)) -ne $tested.sha256.($name)){throw "Published digest mismatch: $name"}
        }
        foreach($name in $contract.retired){
            if(Test-Path -LiteralPath (Join-Path $destination $name)){throw "Retired file still present: $name"}
        }
    }
    $evidence=if($TestResult){(Resolve-Path -LiteralPath $TestResult).Path}else{(Resolve-Path -LiteralPath $BuildResult).Path}
    @{status='passed';test_status=$tested.status;package=$Package;evidence=$evidence;files=@($records)} |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $receipt -Encoding utf8
    'PUBLISH PASS: seven files verified and legacy files retired from both release paths'
}catch{
    $reason=$_.Exception.Message
    $rollback=[System.Collections.Generic.List[string]]::new()
    for($i=$records.Count-1;$i -ge 0;--$i){
        $record=$records[$i]
        try{
            if($record.placed){
                if((Hash $record.path) -ne $record.expected){throw 'Replacement changed before rollback'}
                [IO.File]::Move($record.path,$record.staged)
                $record.placed=$false
            }
            if($record.saved){
                [IO.File]::Move($record.backup,$record.path)
                if((Hash $record.path) -ne $record.before){throw 'Rollback digest mismatch'}
                $record.saved=$false
            }
        }catch{$rollback.Add("$($record.path): $($_.Exception.Message)")}
    }
    @{status='failed';error=$reason;rollback_errors=@($rollback);files=@($records)} |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $receipt -Encoding utf8
    throw "Publish failed: $reason. Rollback errors: $($rollback -join ', ')"
}
