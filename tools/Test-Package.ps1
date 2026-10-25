[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$BuildRoot,
    [string]$SourceRoot=(Split-Path $PSScriptRoot -Parent),
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='package-test',
    [ValidateRange(1,65535)][int]$Task=9,
    [switch]$FullIntegration,
    [ValidateRange(1,16)][int]$Jobs=4,
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
    $arguments+=@('-Jobs',$Jobs)
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
        (Case 'startup-errors' (Join-Path $x64 'startup-errors.exe') @($run) @('STARTUP ERRORS PASS 2 formats Windows-Shell/private-console/shared-cmd/hidden-console one_error_channel exit4 jobs_empty') 100),
        (Case 'startup-error-unit-32' (Join-Path $x86 'startup-error-unit.exe') @() @('SHELL UNIT PASS classification=8 delegation=5')),
        (Case 'startup-error-unit-64' (Join-Path $x64 'startup-error-unit.exe') @() @('SHELL UNIT PASS classification=8 delegation=5')),
        (Case 'legacy-rejection' (Join-Path $x64 'legacy-rejection.exe') @($run) @('LEGACY REJECTION PASS 4 cases stderr193 exit4 no_retry_ui exact_owned_processes') 100),
        (Case 'cli'  $python @('-B',(Join-Path $SourceRoot 'tests/run16s-exe/cli.py'),$run) @('CLI PASS')),
        (Case 'automatic' (Join-Path $x64 'launch-automatic.exe') $pair @('LAUNCH MATRIX PASS')),
        (Case 'propagation' (Join-Path $x64 'propagate-basic.exe') $tree @('PROPAGATION BASIC PASS') 320),
        (Case 'cmd-nested' (Join-Path $x64 'cmd-nested.exe') @('--core',$Package) @('CMD NESTED PASS core four native PE edges') 180),
        (Case 'long-path' $python @('-B',(Join-Path $SourceRoot 'tests/run16s-exe/long_path.py'),$run,$d32,$d64,(Join-Path $x86 'launch-failures.exe'),(Join-Path $x64 'launch-failures.exe')) @('LONG PATH PASS width=32','LONG PATH PASS width=64')),
        (Case 'cli-failures' $python (@('-B',(Join-Path $SourceRoot 'tests/run16s-exe/cli_failures.py'))+$pair) @('CLI ROLLBACK PASS width=32','CLI ROLLBACK PASS width=64','CLI IO/EXIT PASS width=32','CLI IO/EXIT PASS width=64'))
    )
    $cases+=@(
        (Case 'legacy-iat-32' (Join-Path $x86 'launch-staging.exe') @('--parent',$Package) @('LEGACY IAT PASS bits=32 zero_oft=0 initialized=1 metadata_restored=1','LEGACY IAT PASS bits=32 zero_oft=1 initialized=1 metadata_restored=1','LEGACY IAT REJECT PASS bits=32 mode=3 error=193')),
        (Case 'legacy-iat-64' (Join-Path $x64 'launch-staging.exe') @('--parent',$Package) @('LEGACY IAT PASS bits=64 zero_oft=0 initialized=1 metadata_restored=1','LEGACY IAT PASS bits=64 zero_oft=1 initialized=1 metadata_restored=1','LEGACY IAT REJECT PASS bits=64 mode=3 error=193')),
        (Case 'uac-unit-32' (Join-Path $x86 'uac-unit.exe') @() @('UAC UNIT PASS cases=8')),
        (Case 'uac-unit-64' (Join-Path $x64 'uac-unit.exe') @() @('UAC UNIT PASS cases=8'))
    )
    $cases+=@(
        (Case 'vb-font-32' (Join-Path $x86 'core-vb-font.exe') @((Join-Path $x86 'vb-fixture/MSVBVM60.dll')) @('VB_FONT_ZERO_OK requested=12','VB_FONT_ZERO_OK requested=14','VB_FONT_ZERO_OK requested=16','VB_OWNERSHIP_OK leases_retired=32 maps_freed=32 sources_preserved=33')),
        (Case 'vb-font-64' (Join-Path $x64 'core-vb-font.exe') @((Join-Path $x64 'vb-fixture/MSVBVM60.dll')) @('VB_FONT_ZERO_OK requested=12','VB_FONT_ZERO_OK requested=14','VB_FONT_ZERO_OK requested=16','VB_OWNERSHIP_OK leases_retired=32 maps_freed=32 sources_preserved=33')),
        (Case 'vb-controls-32' (Join-Path $x86 'core-vb.exe') @($run,(Join-Path $x86 'vb-fixture/MSVBVM60.dll')) @('VB_WINDOWED_OK bits=32 styled=1','VB_EDIT_OK undo=1','VB_EDIT_FONT_PIXELS changed=1','VB_CONTAINER_PAINT_OK label_changed=1','VB_CLEANUP_OK')),
        (Case 'vb-controls-64' (Join-Path $x64 'core-vb.exe') @($run,(Join-Path $x64 'vb-fixture/MSVBVM60.dll')) @('VB_WINDOWED_OK bits=64 styled=1','VB_EDIT_OK undo=1','VB_EDIT_FONT_PIXELS changed=1','VB_CONTAINER_PAINT_OK label_changed=1','VB_CLEANUP_OK'))
    )
    $cases+=@(
        (Case 'control-extensions-32' (Join-Path $x86 'core-control-extensions.exe') @($d32) @('CONTROL_EXTENSION_NATIVE_OK','CONTROL_EXTENSION_IMAGELIST_OK','CONTROL_EXTENSION_OK bits=32 protocol_cases=60')),
        (Case 'control-extensions-64' (Join-Path $x64 'core-control-extensions.exe') @($d64) @('CONTROL_EXTENSION_NATIVE_OK','CONTROL_EXTENSION_IMAGELIST_OK','CONTROL_EXTENSION_OK bits=64 protocol_cases=60')),
        (Case 'control-protocol-owner-32' (Join-Path $x86 'core-control-protocol-owner.exe') @() @('PROTOCOL_OWNER_OK bits=32 allocations=0 bindings=0')),
        (Case 'control-protocol-owner-64' (Join-Path $x64 'core-control-protocol-owner.exe') @() @('PROTOCOL_OWNER_OK bits=64 allocations=0 bindings=0')),
        (Case 'tab-controls-32' (Join-Path $x86 'core-tab-controls.exe') @($d32) @('TAB_CONTROL_OK bits=32','styled=4 preserved=10')),
        (Case 'tab-controls-64' (Join-Path $x64 'core-tab-controls.exe') @($d64) @('TAB_CONTROL_OK bits=64','styled=4 preserved=10')),
        (Case 'tab-controls-dpi-32' (Join-Path $x86 'core-tab-controls.exe') @($d32,'aware') @('TAB_CONTROL_OK bits=32','aware=1 styled=4 preserved=10')),
        (Case 'tab-controls-dpi-64' (Join-Path $x64 'core-tab-controls.exe') @($d64,'aware') @('TAB_CONTROL_OK bits=64','aware=1 styled=4 preserved=10')),
        (Case 'tab-controls-v5-32' (Join-Path $x86 'core-tab-controls-v5.exe') @($d32) @('TAB_CONTROL_OK bits=32','styled=4 preserved=10')),
        (Case 'tab-controls-v5-64' (Join-Path $x64 'core-tab-controls-v5.exe') @($d64) @('TAB_CONTROL_OK bits=64','styled=4 preserved=10')),
        (Case 'tab-controls-v5-dpi-32' (Join-Path $x86 'core-tab-controls-v5.exe') @($d32,'aware') @('TAB_CONTROL_OK bits=32','aware=1 styled=4 preserved=10')),
        (Case 'tab-controls-v5-dpi-64' (Join-Path $x64 'core-tab-controls-v5.exe') @($d64,'aware') @('TAB_CONTROL_OK bits=64','aware=1 styled=4 preserved=10')),
        (Case 'pending-control-owner-32' (Join-Path $x86 'core-pending-control-owner.exe') @($d32) @('PENDING_OWNER_OK bits=32','application_font_valid=1')),
        (Case 'pending-control-owner-64' (Join-Path $x64 'core-pending-control-owner.exe') @($d64) @('PENDING_OWNER_OK bits=64','application_font_valid=1')),
        (Case 'control-visibility-32' (Join-Path $x86 'core-control-visibility.exe') @($d32) @('CONTROL_VISIBILITY_OK bits=32 cases=16')),
        (Case 'control-visibility-64' (Join-Path $x64 'core-control-visibility.exe') @($d64) @('CONTROL_VISIBILITY_OK bits=64 cases=16')),
        (Case 'dialog-scope-32' (Join-Path $x86 'core-dialog-scope.exe') @() @('DIALOG_SCOPE_OWNERSHIP_OK bits=32','allocations=0')),
        (Case 'dialog-scope-64' (Join-Path $x64 'core-dialog-scope.exe') @() @('DIALOG_SCOPE_OWNERSHIP_OK bits=64','allocations=0')),
        (Case 'dialog-show-32' (Join-Path $x86 'core-dialog-show.exe') @($run) @('DIALOG_SHOW_OK bits=32 styled=1','DIALOG_SHOW_CLEANUP_OK') 180),
        (Case 'dialog-show-64' (Join-Path $x64 'core-dialog-show.exe') @($run) @('DIALOG_SHOW_OK bits=64 styled=1','DIALOG_SHOW_CLEANUP_OK') 180),
        (Case 'theme-stage-32' (Join-Path $x86 'core-theme-stage.exe') @() @('THEME_STAGE_OWNERSHIP_OK bits=32','allocations=0 early_proc_unchanged=1')),
        (Case 'theme-stage-64' (Join-Path $x64 'core-theme-stage.exe') @() @('THEME_STAGE_OWNERSHIP_OK bits=64','allocations=0 early_proc_unchanged=1')),
        (Case 'superclass-32' (Join-Path $x86 'core-superclass.exe') @($run) @('SUPER_PROTOCOL_OK bits=32 styled=1','SUPER_RENDER_OK','SUPER_ACTUAL_FILL_OK','SUPER_REJECT_OK','SUPER_CLEANUP_OK')),
        (Case 'superclass-64' (Join-Path $x64 'core-superclass.exe') @($run) @('SUPER_PROTOCOL_OK bits=64 styled=1','SUPER_RENDER_OK','SUPER_ACTUAL_FILL_OK','SUPER_REJECT_OK','SUPER_CLEANUP_OK'))
    )
    if($FullIntegration){
        $cases+=@(
            (Case 'creation-contract' (Join-Path $x64 'creation-contract-driver.exe') @($run,$d32,$d64,(Join-Path $x86 'creation-contract.exe'),(Join-Path $x64 'creation-contract.exe')) @('CREATION CONTRACT PASS 64') 520),
            (Case 'rollback' (Join-Path $x64 'rollback-contract-driver.exe') @($run,$d32,$d64,(Join-Path $x86 'rollback-contract.exe'),(Join-Path $x64 'rollback-contract.exe')) @('ROLLBACK PASS parent=32','ROLLBACK PASS parent=64') 320),
            (Case 'dialogs' (Join-Path $x64 'launch-automatic.exe') ($pair+@('dialogs')) @('LAUNCH MATRIX PASS') 520),
            (Case 'messages' (Join-Path $x64 'launch-automatic.exe') ($pair+@('messages')) @('LAUNCH MATRIX PASS') 520)
        )
    }
    $records=[System.Collections.Generic.List[object]]::new()
    $pending=[System.Collections.Generic.List[object]]::new()
    foreach($case in $cases){$pending.Add($case)}
    $running=[System.Collections.Generic.List[object]]::new()
    $deadline=[DateTime]::UtcNow.AddSeconds(1800);$totalTimer=[Diagnostics.Stopwatch]::StartNew()
    $lastProgress=[DateTime]::MinValue
    try{
        while($pending.Count -or $running.Count){
            $used=0;foreach($entry in $running){$used+=$entry.cost}
            while($pending.Count){
                $case=$pending[0]
                $cost=if($case.name -match '^(font-layout|menu-cancel|dialogs|messages)'){[Math]::Min(2,$Jobs)}else{1}
                if($used+$cost -gt $Jobs){break}
                $pending.RemoveAt(0)
                $directory=Join-Path $scratch $case.name;New-Item -ItemType Directory -Path $directory | Out-Null
                $start=[Diagnostics.ProcessStartInfo]::new();$start.FileName=$case.command;$start.WorkingDirectory=$directory
                $start.UseShellExecute=$false;$start.CreateNoWindow=$true;$start.WindowStyle='Hidden'
                $start.RedirectStandardOutput=$true;$start.RedirectStandardError=$true
                $start.Environment['WIN16STYLE_CAPTURE_DIR']=$directory
                foreach($argument in $case.arguments){$start.ArgumentList.Add($argument)}
                $process=[Diagnostics.Process]::new();$process.StartInfo=$start
                if(!$process.Start()){throw 'Test process did not start'}
                $running.Add(@{case=$case;directory=$directory;process=$process;stdout=$process.StandardOutput.ReadToEndAsync();stderr=$process.StandardError.ReadToEndAsync();timer=[Diagnostics.Stopwatch]::StartNew();cost=$cost})
                $used+=$cost
            }
            if(([DateTime]::UtcNow-$lastProgress).TotalSeconds -ge 1){
                @($running | ForEach-Object {@{name=$_.case.name;pid=$_.process.Id;seconds=$_.timer.Elapsed.TotalSeconds;budget=$_.case.seconds}})|ConvertTo-Json|Set-Content (Join-Path $scratch 'progress.json')
                $lastProgress=[DateTime]::UtcNow
            }
            foreach($entry in $running.ToArray()){
                if($entry.timer.Elapsed.TotalSeconds -gt $entry.case.seconds -or [DateTime]::UtcNow -gt $deadline){
                    if(!$entry.process.HasExited){$entry.process.Kill($true);$entry.process.WaitForExit()}
                    throw "Owned test timed out: $($entry.case.name)"
                }
                if(!$entry.process.HasExited){continue}
                $entry.process.WaitForExit();$entry.timer.Stop()
                $out=$entry.stdout.GetAwaiter().GetResult();$err=$entry.stderr.GetAwaiter().GetResult()
                [IO.File]::WriteAllText((Join-Path $entry.directory 'stdout.log'),$out)
                [IO.File]::WriteAllText((Join-Path $entry.directory 'stderr.log'),$err)
                $record=@{name=$entry.case.name;exit_code=$entry.process.ExitCode;command=$entry.case.command;arguments=$entry.case.arguments;seconds=$entry.timer.Elapsed.TotalSeconds};$records.Add($record)
                $records|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'cases.json')
                if($entry.process.ExitCode){throw "Package test failed: $($entry.case.name), exit $($entry.process.ExitCode)"}
                foreach($marker in $entry.case.markers){if(!($out+$err).Contains($marker)){throw "Missing marker $marker in $($entry.case.name)"}}
                $running.Remove($entry)|Out-Null;$entry.process.Dispose()
            }
            if($running.Count){Start-Sleep -Milliseconds 50}
        }
    }finally{
        foreach($entry in $running){
            if(!$entry.process.HasExited){$entry.process.Kill($true);$entry.process.WaitForExit()}
            [IO.File]::WriteAllText((Join-Path $entry.directory 'stdout.log'),$entry.stdout.GetAwaiter().GetResult())
            [IO.File]::WriteAllText((Join-Path $entry.directory 'stderr.log'),$entry.stderr.GetAwaiter().GetResult())
            $entry.process.Dispose()
        }
    }
    $after=Package-Hashes
    foreach($name in $before.Keys){if($before[$name] -ne $after[$name]){throw 'Package changed during testing'}}
    @{status='passed';seconds=$totalTimer.Elapsed.TotalSeconds;jobs=$Jobs;cases=$records.Count;full_integration=[bool]$FullIntegration;package=$Package;sha256=$after;physical_input='not-tested';core_matrix='separate Build-Core matrix required'}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
}catch{
    @{status='failed-or-inconclusive';error=$_.Exception.Message}|ConvertTo-Json|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    exit 1
}
