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
$names=@('demo.exe','ntw16s32.dll','ntw16s64.dll','ntw2ks32.dll','ntw2ks64.dll','ntwshk32.dll','ntwshk64.dll','run16s.exe')
$Package=(Resolve-Path -LiteralPath $Package).Path
if([bool]$TestResult -eq [bool]$BuildResult){throw 'Provide exactly one of BuildResult or TestResult'}
if($BuildResult){
    $build=Get-Content -LiteralPath $BuildResult -Raw|ConvertFrom-Json
    if($build.status -ne 'passed' -or $build.exit_code -ne 0 -or
       (Resolve-Path -LiteralPath $build.candidate).Path -ne $Package){throw 'A passed matching clean build is required'}
    $buildRoot=Join-Path $build.clean_source 'build/.ninja'
    & python (Join-Path $repo 'tools/check_package.py') $Package --source-root $build.clean_source --built-root $buildRoot --task $Task
    if($LASTEXITCODE){throw 'Package/source/build identity gate failed'}
    $hashes=[ordered]@{};foreach($name in $names){$hashes[$name]=(Get-FileHash -LiteralPath (Join-Path $Package $name) -Algorithm SHA256).Hash.ToLowerInvariant()}
    $tested=@{status='pending-post-publication';skipped=@();sha256=$hashes}
}else{
    $tested=Get-Content -LiteralPath $TestResult -Raw|ConvertFrom-Json
    if($tested.status -notin @('passed','passed-with-skips','passed-focused','passed-focused-with-skips')){throw 'Only a passed actual-package result may be published'}
    Assert-PackageCoverage -Result $tested
}
$files=@(Get-ChildItem -LiteralPath $Package -Force)
if(@(Compare-Object $names @($files.Name|Sort-Object)).Count){throw 'Expected exactly eight candidate files'}
function Hash([string]$Path){(Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()}
foreach($file in $files){
    if($file.PSIsContainer -or ($file.Attributes -band [IO.FileAttributes]::ReparsePoint)){throw 'Candidate must be regular files'}
    if((Hash $file.FullName) -ne $tested.sha256.($file.Name)){throw "Tested hash mismatch: $($file.Name)"}
}
$destinations=@((Join-Path $repo 'assets'),'O:\winnt')
foreach($destination in $destinations){
    if(!(Test-Path -LiteralPath $destination -PathType Container)){throw "Destination absent: $destination"}
    for($ancestor=$destination;$ancestor;$ancestor=Split-Path $ancestor -Parent){
        if((Get-Item -LiteralPath $ancestor -Force).Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Destination traverses a reparse point'}
    }
    foreach($name in $names){
        $target=Join-Path $destination $name
        if((Test-Path -LiteralPath $target) -and ((Get-Item -LiteralPath $target -Force).Attributes -band ([IO.FileAttributes]::ReparsePoint -bor [IO.FileAttributes]::Directory))){throw 'Destination is not a regular file'}
    }
}
if($ValidateOnly){'PUBLISH VALIDATION PASS: package hashes, exact eight files, assets root and fixed O:\winnt';return}
$scratch=Join-Path $repo "build/t$Task-package/$RunName"
if(Test-Path -LiteralPath $scratch){throw 'Use a fresh publication run name; preserve prior backups'}
New-Item -ItemType Directory -Path $scratch|Out-Null
$records=[System.Collections.Generic.List[object]]::new()
try{
    # Back up both destinations before replacing any file. Only these six
    # project-owned names are read or written; occupied files cause a stop.
    for($index=0;$index -lt $destinations.Count;++$index){
        $backup=Join-Path $scratch "before-$index";New-Item -ItemType Directory -Path $backup|Out-Null
        foreach($name in $names){
            $target=Join-Path $destinations[$index] $name
            $before=if(Test-Path -LiteralPath $target){Hash $target}else{$null}
            if($before){Copy-Item -LiteralPath $target -Destination (Join-Path $backup $name);if((Hash (Join-Path $backup $name)) -ne $before){throw 'Backup hash mismatch'}}
            $records.Add(@{path=$target;before=$before;after=$null;expected=$tested.sha256.$name;backup_directory=$backup})
        }
    }
    foreach($record in $records){
        $name=Split-Path $record.path -Leaf
        Copy-Item -LiteralPath (Join-Path $Package $name) -Destination $record.path -Force
        $record.after=Hash $record.path
        if($record.after -ne $record.expected){throw "Published hash mismatch: $($record.path)"}
    }
    $evidence=if($TestResult){(Resolve-Path -LiteralPath $TestResult).Path}else{(Resolve-Path -LiteralPath $BuildResult).Path}
    @{status='passed';test_status=$tested.status;skipped=@($tested.skipped);package=$Package;evidence=$evidence;
      tested_package=$(if($TestResult){$Package}else{$null});test_result=$(if($TestResult){$evidence}else{$null});files=@($records)}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    'PUBLISH PASS: eight package files identical at assets root and O:\winnt'
}catch{
    @{status='incomplete';error=$_.Exception.Message;files=@($records)}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    throw
}
