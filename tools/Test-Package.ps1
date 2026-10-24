[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$BuildRoot,
    [string]$SourceRoot=(Split-Path $PSScriptRoot -Parent),
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='package-test',
    [ValidateRange(1,65535)][int]$Task=8,
    [switch]$FullIntegration,
    [switch]$Worker
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$scratch=Join-Path $repo "build/t$Task-package/$RunName"
$Package=(Resolve-Path -LiteralPath $Package).Path
$BuildRoot=(Resolve-Path -LiteralPath $BuildRoot).Path
$SourceRoot=(Resolve-Path -LiteralPath $SourceRoot).Path
if(!$Worker){
    if(Test-Path -LiteralPath $scratch){throw 'Select a fresh run name; preserve earlier evidence'}
    New-Item -ItemType Directory -Path $scratch | Out-Null
    $arguments=@('-NoProfile','-File',('"'+$PSCommandPath+'"'),'-Worker','-RunName',$RunName,
        '-Task',$Task,'-Package',('"'+$Package+'"'),'-BuildRoot',('"'+$BuildRoot+'"'),'-SourceRoot',('"'+$SourceRoot+'"'))
    if($FullIntegration){$arguments+='-FullIntegration'}
    $job=Start-Process (Join-Path $PSHOME 'pwsh.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $scratch 'worker.log') -RedirectStandardError (Join-Path $scratch 'worker.err')
    @{pid=$job.Id;result=(Join-Path $scratch 'result.json');progress=(Join-Path $scratch 'progress.json');maximum_seconds=1800}|ConvertTo-Json
    return
}
function Package-Hashes {
    $expected=@('demo.exe','ntw16s32.dll','ntw16s64.dll','run16s.exe')
    $files=@(Get-ChildItem -LiteralPath $Package -Force)
    if(@(Compare-Object $expected @($files.Name|Sort-Object)).Count -or
        @($files|Where-Object { $_.PSIsContainer -or ($_.Attributes -band [IO.FileAttributes]::ReparsePoint) }).Count){throw 'Package must contain exactly four regular files'}
    $result=[ordered]@{}
    foreach($file in $files){$result[$file.Name]=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}
    return $result
}
function Case([string]$Name,[string]$Command,[string[]]$Arguments,[string[]]$Markers,[int]$Seconds=120){
    return @{name=$Name;command=$Command;arguments=$Arguments;markers=$Markers;seconds=$Seconds}
}
try{
    New-Item -ItemType Directory -Force -Path $scratch | Out-Null
    $before=Package-Hashes
    $verification=& python (Join-Path $SourceRoot 'tools/check_package.py') $Package --source-root $SourceRoot --built-root $BuildRoot --task $Task
    if($LASTEXITCODE){throw 'Package/source/build identity gate failed'}
    $verification|Set-Content (Join-Path $scratch 'package-before.json') -Encoding utf8
    $python=(Get-Command python.exe -ErrorAction Stop).Source
    $run=Join-Path $Package 'run16s.exe';$d32=Join-Path $Package 'ntw16s32.dll';$d64=Join-Path $Package 'ntw16s64.dll';$demo=Join-Path $Package 'demo.exe'
    $x86=Join-Path $BuildRoot 'x86';$x64=Join-Path $BuildRoot 'x64'
    $pair=@($run,$d32,$d64,(Join-Path $x86 'launch-target.exe'),(Join-Path $x64 'launch-target.exe'))
    $tree=@($run,$d32,$d64,(Join-Path $x86 'propagate-target.exe'),(Join-Path $x64 'propagate-target.exe'))
    $cases=@(
        (Case 'abi-x86' (Join-Path $x86 'core-abi.exe') @($d32) @('ABI PASS width=32')),
        (Case 'abi-x64' (Join-Path $x64 'core-abi.exe') @($d64) @('ABI PASS width=64')),
        (Case 'demo-boundary' $python @('-B',(Join-Path $SourceRoot 'tests/demo-exe/boundary.py'),$demo) @('DEMO BOUNDARY PASS')),
        (Case 'demo-pair' (Join-Path $x64 'demo-observe.exe') @($run,$d64,$d32,$demo) @('DEMO OBSERVE PASS styled=0','DEMO OBSERVE PASS styled=1','DEMO PAIR PASS')),
        (Case 'wait-contract' $python @('-B',(Join-Path $SourceRoot 'tests/run16s-exe/wait.py'),$run,$d32,$d64,(Join-Path $x86 'wait-target.exe'),(Join-Path $x64 'wait-target.exe'),(Join-Path $x64 'wait-gui-target.exe')) @('WAIT CONTRACT PASS 6 cases') 190),
        (Case 'font-layout-x86' (Join-Path $x86 'core-font-layout.exe') @($d32) @('FONT LAYOUT PASS 2 contexts 12 font variants') 180),
        (Case 'font-layout-x64' (Join-Path $x64 'core-font-layout.exe') @($d64) @('FONT LAYOUT PASS 2 contexts 12 font variants') 180),
        (Case 'menu-cancel-x86' (Join-Path $x86 'core-menu-cancel.exe') @($d32) @('MENU CANCEL PASS 15 scenarios','MENU SHADOW SCOPE PASS aware=0','MENU SHADOW SCOPE PASS aware=1') 150),
        (Case 'menu-cancel-x64' (Join-Path $x64 'core-menu-cancel.exe') @($d64) @('MENU CANCEL PASS 15 scenarios','MENU SHADOW SCOPE PASS aware=0','MENU SHADOW SCOPE PASS aware=1') 150),
        (Case 'cli'  $python @('-B',(Join-Path $SourceRoot 'tests/run16s-exe/cli.py'),$run) @('CLI PASS')),
        (Case 'automatic' (Join-Path $x64 'launch-automatic.exe') $pair @('LAUNCH MATRIX PASS')),
        (Case 'propagation' (Join-Path $x64 'propagate-basic.exe') $tree @('PROPAGATION BASIC PASS') 320),
        (Case 'long-path' $python @('-B',(Join-Path $SourceRoot 'tests/run16s-exe/long_path.py'),$run,$d32,$d64,(Join-Path $x86 'launch-failures.exe'),(Join-Path $x64 'launch-failures.exe')) @('LONG PATH PASS width=32','LONG PATH PASS width=64')),
        (Case 'cli-failures' $python (@('-B',(Join-Path $SourceRoot 'tests/run16s-exe/cli_failures.py'))+$pair) @('CLI ROLLBACK PASS width=32','CLI ROLLBACK PASS width=64','CLI IO/EXIT PASS width=32','CLI IO/EXIT PASS width=64'))
    )
    if($FullIntegration){
        $cases+=@(
            (Case 'creation-contract' (Join-Path $x64 'creation-contract-driver.exe') @($run,$d32,$d64,(Join-Path $x86 'creation-contract.exe'),(Join-Path $x64 'creation-contract.exe')) @('CREATION CONTRACT PASS 64') 520),
            (Case 'rollback' (Join-Path $x64 'rollback-contract-driver.exe') @($run,$d32,$d64,(Join-Path $x86 'rollback-contract.exe'),(Join-Path $x64 'rollback-contract.exe')) @('ROLLBACK PASS parent=32','ROLLBACK PASS parent=64') 320),
            (Case 'dialogs' (Join-Path $x64 'launch-automatic.exe') ($pair+@('dialogs')) @('LAUNCH MATRIX PASS') 520),
            (Case 'messages' (Join-Path $x64 'launch-automatic.exe') ($pair+@('messages')) @('LAUNCH MATRIX PASS') 520)
        )
    }
    $records=[System.Collections.Generic.List[object]]::new();$deadline=[DateTime]::UtcNow.AddSeconds(1800)
    foreach($case in $cases){
        $directory=Join-Path $scratch $case.name;New-Item -ItemType Directory -Path $directory | Out-Null
        $start=[Diagnostics.ProcessStartInfo]::new();$start.FileName=$case.command;$start.WorkingDirectory=$directory
        $start.UseShellExecute=$false;$start.CreateNoWindow=$true;$start.WindowStyle='Hidden'
        $start.RedirectStandardOutput=$true;$start.RedirectStandardError=$true
        $start.Environment['WIN16STYLE_CAPTURE_DIR']=$directory
        foreach($argument in $case.arguments){$start.ArgumentList.Add($argument)}
        $process=[Diagnostics.Process]::new();$process.StartInfo=$start
        if(!$process.Start()){throw 'Test process did not start'}
        $stdout=$process.StandardOutput.ReadToEndAsync();$stderr=$process.StandardError.ReadToEndAsync()
        @{name=$case.name;pid=$process.Id;command=$case.command;arguments=$case.arguments;seconds=$case.seconds}|ConvertTo-Json -Depth 4|Set-Content (Join-Path $scratch 'progress.json')
        $budget=[Math]::Min($case.seconds,[Math]::Max(0,($deadline-[DateTime]::UtcNow).TotalSeconds))
        if(!$process.WaitForExit([int]($budget*1000))){$process.Kill($true);$process.WaitForExit();throw "Owned test timed out: $($case.name)"}
        $out=$stdout.GetAwaiter().GetResult();$err=$stderr.GetAwaiter().GetResult()
        [IO.File]::WriteAllText((Join-Path $directory 'stdout.log'),$out)
        [IO.File]::WriteAllText((Join-Path $directory 'stderr.log'),$err)
        $record=@{name=$case.name;exit_code=$process.ExitCode;command=$case.command;arguments=$case.arguments};$records.Add($record)
        $records|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'cases.json')
        if($process.ExitCode){throw "Package test failed: $($case.name), exit $($process.ExitCode)"}
        foreach($marker in $case.markers){if(!($out+$err).Contains($marker)){throw "Missing marker $marker in $($case.name)"}}
        $process.Dispose()
    }
    $after=Package-Hashes
    foreach($name in $before.Keys){if($before[$name] -ne $after[$name]){throw 'Package changed during testing'}}
    @{status='passed';cases=$records.Count;full_integration=[bool]$FullIntegration;package=$Package;sha256=$after;physical_input='not-tested';core_matrix='separate Build-Core matrix required'}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
}catch{
    @{status='failed-or-inconclusive';error=$_.Exception.Message}|ConvertTo-Json|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    exit 1
}
