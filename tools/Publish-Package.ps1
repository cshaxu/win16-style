[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$TestResult,
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='publish',
    [ValidateRange(1,65535)][int]$Task=7,
    [switch]$ValidateOnly
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$names=@('demo.exe','ntw16s32.dll','ntw16s64.dll','run16s.exe')
$Package=(Resolve-Path -LiteralPath $Package).Path
$tested=Get-Content -LiteralPath $TestResult -Raw|ConvertFrom-Json
if($tested.status -ne 'passed'){throw 'Only a passed actual-package result may be published'}
$files=@(Get-ChildItem -LiteralPath $Package -Force)
if(@(Compare-Object $names @($files.Name|Sort-Object)).Count){throw 'Expected exactly four candidate files'}
function Hash([string]$Path){(Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()}
foreach($file in $files){
    if($file.PSIsContainer -or ($file.Attributes -band [IO.FileAttributes]::ReparsePoint)){throw 'Candidate must be regular files'}
    if((Hash $file.FullName) -ne $tested.sha256.($file.Name)){throw "Tested hash mismatch: $($file.Name)"}
}
$destinations=@((Join-Path $repo 'assets'),'O:\winnt\system32')
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
if($ValidateOnly){'PUBLISH VALIDATION PASS: tested hashes, exact four files, assets root and fixed system32';return}
$scratch=Join-Path $repo "build/t$Task-package/$RunName"
if(Test-Path -LiteralPath $scratch){throw 'Use a fresh publication run name; preserve prior backups'}
New-Item -ItemType Directory -Path $scratch|Out-Null
$records=[System.Collections.Generic.List[object]]::new()
try{
    # Back up both destinations before replacing any file. Only these four
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
    @{status='passed';tested_package=$Package;test_result=(Resolve-Path -LiteralPath $TestResult).Path;files=@($records)}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    'PUBLISH PASS: four tested files identical at assets root and O:\winnt\system32'
}catch{
    @{status='incomplete';error=$_.Exception.Message;files=@($records)}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    throw
}
