[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$BuildRoot,
    [string]$SourceRoot=(Split-Path $PSScriptRoot -Parent),
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='package-test',
    [ValidateRange(1,65535)][int]$Task=9,
    [switch]$FullIntegration,
    [AllowEmptyString()][string]$Focus,
    [ValidateRange(1,16)][int]$Jobs=4,
    [switch]$Worker
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
. (Join-Path $PSScriptRoot 'Set-PackageProgress.ps1')
. (Join-Path $PSScriptRoot 'Select-PackageCases.ps1')
$focusSpecified=$PSBoundParameters.ContainsKey('Focus')
if ($focusSpecified -and $FullIntegration) { throw 'Focused and full integration modes cannot be combined' }
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
    if($focusSpecified){$arguments+=@('-Focus',$Focus)}
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
        (Case 'progress-sharing' (Join-Path $PSHOME 'pwsh.exe') @('-NoProfile','-File',(Join-Path $SourceRoot 'tests/tooling/package_progress_lock.ps1')) @('PACKAGE_PROGRESS_LOCK PASS old-fails locked-skip recovered nonsharing-fails')),
        (Case 'plan-selection' (Join-Path $PSHOME 'pwsh.exe') @('-NoProfile','-File',(Join-Path $SourceRoot 'tests/tooling/package_case_selection.ps1')) @('PACKAGE_PLAN PASS default focused unknown duplicate empty missing-required rejected')),
        (Case 'abi-x86' (Join-Path $x86 'core-abi.exe') @($d32) @('ABI PASS width=32')),
        (Case 'abi-x64' (Join-Path $x64 'core-abi.exe') @($d64) @('ABI PASS width=64')),
        (Case 'demo-boundary' $python @('-B',(Join-Path $SourceRoot 'tests/demo-exe/boundary.py'),$demo) @('DEMO BOUNDARY PASS')),
        (Case 'demo-pair' (Join-Path $x64 'demo-observe.exe') @($run,$d64,$d32,$demo) @('DEMO OBSERVE PASS styled=0','DEMO OBSERVE PASS styled=1','DEMO DIALOG ENTRIES PASS open=3 save=3 folder=3','DEMO EXTRA ENTRIES PASS 7','DEMO DIALOG LAYOUT PASS 27 visible nonoverlapping buttons','DEMO EXTRA DIALOG PASS opened=7 cancelled=7 shell_tree=1','DEMO DIALOG PATHS PASS opened=9 cancelled=9','DEMO DIALOG SELECT PASS legacy=1 bridge=1 save=1 cleanup=1','DEMO FOLDER TREE SELECT PASS filesystem=1','DEMO PAIR PASS')),
        (Case 'wait-contract' $python @('-B',(Join-Path $SourceRoot 'tests/run16s-exe/wait.py'),$run,$d32,$d64,(Join-Path $x86 'wait-target.exe'),(Join-Path $x64 'wait-target.exe'),(Join-Path $x64 'wait-gui-target.exe')) @('WAIT CONTRACT PASS 6 cases') 190),
        (Case 'font-layout-x86' (Join-Path $x86 'core-font-layout.exe') @($d32) @('FONT WARMUP SELFTEST PASS transient=1 rising=1 real_gdi_leak=1 cleanup=1','FONT LAYOUT PASS 2 contexts 12 font variants') 180),
        (Case 'font-layout-x64' (Join-Path $x64 'core-font-layout.exe') @($d64) @('FONT WARMUP SELFTEST PASS transient=1 rising=1 real_gdi_leak=1 cleanup=1','FONT LAYOUT PASS 2 contexts 12 font variants') 180),
        (Case 'menu-cancel-x86' (Join-Path $x86 'core-menu-cancel.exe') @($d32) @('MENU CANCEL PASS 15 scenarios','MENU FRAME CLIP WITNESS dpi=96 pixel=775325','MENU FRAME CLIP WITNESS dpi=144 pixel=775325','MENU FRAME FULL PAINT dpi=96 pixel=f0c8a4','MENU FRAME FULL PAINT dpi=144 pixel=f0c8a4','MENU SHADOW SCOPE PASS aware=0','MENU SHADOW SCOPE PASS aware=1') 35),
        (Case 'menu-cancel-x64' (Join-Path $x64 'core-menu-cancel.exe') @($d64) @('MENU CANCEL PASS 15 scenarios','MENU FRAME CLIP WITNESS dpi=96 pixel=775325','MENU FRAME CLIP WITNESS dpi=144 pixel=775325','MENU FRAME FULL PAINT dpi=96 pixel=f0c8a4','MENU FRAME FULL PAINT dpi=144 pixel=f0c8a4','MENU SHADOW SCOPE PASS aware=0','MENU SHADOW SCOPE PASS aware=1') 35),
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
    # Each native dialog target is owned by a timeout job and observes its own
    # UI thread. The driver runs the actual candidate launcher and both DLLs.
    $dialogDriver=Join-Path $x64 'dialog-test-driver.exe'
    foreach($arch in @('x86','x64')) {
        $bits=if($arch -eq 'x86'){'32'}else{'64'}
        $scrollTarget=Join-Path (Join-Path $BuildRoot $arch) 'scroll-frame-target.exe'
        $hiddenNativeTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-hidden-native-target.exe'
        $eventsTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-events-target.exe'
        $navTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-nav-target.exe'
        $rootFolderTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-root-folder-target.exe'
        $redirectTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-redirect-target.exe'
        $initialRedirectTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-initial-redirect-target.exe'
        $initialVetoTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-initial-veto-target.exe'
        $liveTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-live-target.exe'
        $liveFolderTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-folder-live-target.exe'
        $linkTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-link-target.exe'
        $oldEventTarget=Join-Path (Join-Path $BuildRoot $arch) 'old-file-event-target.exe'
        $oldNestedTarget=Join-Path (Join-Path $BuildRoot $arch) 'old-dialog-nested-target.exe'
        $creationWndprocTarget=Join-Path (Join-Path $BuildRoot $arch) 'creation-wndproc-target.exe'
        $creationDll=if($arch -eq 'x86'){$d32}else{$d64}
        $multiTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-multi-target.exe'
        $identityTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-identity-target.exe'
        $comTarget=Join-Path (Join-Path $BuildRoot $arch) 'com-dialog-target.exe'
        $modernTarget=Join-Path (Join-Path $BuildRoot $arch) 'modern-dialog-target.exe'
        $fileTarget=Join-Path (Join-Path $BuildRoot $arch) 'file-dialog-target.exe'
        foreach($encoding in @('a','w')) {
            $cases+=Case "dialog-file-custom-filter-$arch-$encoding" $dialogDriver @($run,$fileTarget,'open',$encoding,'1','customfilter') @('CUSTOM_FILTER changed=1 index_ok=1',"FILE_DIALOG bits=$bits",'old=1','ok=1','same=1','DIALOG_DRIVER exit=0') 45
            $cases+=Case "dialog-file-spaced-$arch-$encoding" $dialogDriver @($run,$fileTarget,'open',$encoding,'1','spaced') @('SPACED_FILE listed=1 same=1 cleanup=1',"FILE_DIALOG bits=$bits",'old=1','ok=1','DIALOG_DRIVER exit=0') 30
        }
        $folderTarget=Join-Path (Join-Path $BuildRoot $arch) 'folder-dialog-target.exe'
        $treeFolderTarget=Join-Path (Join-Path $BuildRoot $arch) 'tree-folder-target.exe'
        foreach($kind in @('shell','com')){foreach($action in @('accept','cancel')){
            $cases+=Case "dialog-tree-$arch-$kind-$action" $dialogDriver @($run,$treeFolderTarget,$kind,$action) @("TREE_FOLDER bits=$bits",'tree=1','windows=1','result=1','worker=0','DIALOG_DRIVER exit=0') 30
        }}
        foreach($kind in @('shell-simple','shell-simple-callback')){foreach($action in @('accept','cancel')){
            $cases+=Case "dialog-tree-$arch-$kind-$action" $dialogDriver @($run,$treeFolderTarget,$kind,$action) @("TREE_FOLDER bits=$bits",'tree=1','windows=1','result=1','direct=1','worker=0','DIALOG_DRIVER exit=0') 30
        }}
        $cases+=Case "dialog-tree-$arch-shell-validate" $dialogDriver @($run,$treeFolderTarget,'shell','validate') @("TREE_FOLDER bits=$bits",'result=1','validated=1','direct=1','worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-tree-$arch-shell-recover" $dialogDriver @($run,$treeFolderTarget,'shell','recover') @("TREE_FOLDER bits=$bits",'result=1','validated=1','direct=1','worker=0','DIALOG_DRIVER exit=0') 30
        $findTarget=Join-Path (Join-Path $BuildRoot $arch) 'find-dialog-target.exe'
        $commonTarget=Join-Path (Join-Path $BuildRoot $arch) 'common-dialog-target.exe'
        $printTarget=Join-Path (Join-Path $BuildRoot $arch) 'print-dialog-target.exe'
        $cases+=Case "dialog-print-ranges-$arch" (Join-Path (Join-Path $BuildRoot $arch) 'print-ranges-unit.exe') @() @('PRINT RANGES PASS order duplicates overlap bounds overflow atomic') 15
        $sheetTarget=Join-Path (Join-Path $BuildRoot $arch) 'sheet-dialog-target.exe'
        $taskTarget=Join-Path (Join-Path $BuildRoot $arch) 'task-dialog-target.exe'
        foreach($kind in @('open','save','folder')) {
            foreach($action in @('accept','cancel')) {
                $cases+=Case "dialog-com-$arch-$kind-$action" $dialogDriver @($run,$comTarget,$kind,$action) @("COM_BRIDGE bits=$bits",'old=1','DIALOG_DRIVER exit=0') 30
            }
        }
        $cases+=Case "dialog-com-ex-$arch" $dialogDriver @($run,$comTarget,'folder','accept','ex') @('COM_EX identity=1 partial=00080012',"COM_BRIDGE bits=$bits",'folder=1','old=1','same=1','DIALOG_DRIVER exit=0') 30
        foreach($kind in @('open','save','folder')){
            $cases+=Case "dialog-com-missing-interface-$arch-$kind" $dialogDriver @($run,$comTarget,$kind,'cancel','missing') @('COM_MISSING_QUERY hr=80004002 null=1',"COM_BRIDGE bits=$bits",'old=1','hr=800704c7','DIALOG_DRIVER exit=0') 30
        }
        foreach($kind in @('open','save','folder')){
            $cases+=Case "dialog-com-missing-control-$arch-$kind" $dialogDriver @($run,$comTarget,$kind,'cancel','missing-control') @('COM_MISSING_CONTROL index=0 hr=80070057','COM_MISSING_CONTROL index=1 hr=80070057','COM_MISSING_CONTROL index=2 hr=80070057','COM_MISSING_CONTROL index=3 hr=80070057','COM_MISSING_CONTROL index=4 hr=80070057',"COM_BRIDGE bits=$bits",'old=1','hr=800704c7','DIALOG_DRIVER exit=0') 30
        }
        $cases+=Case "dialog-com-foreign-interface-$arch" $dialogDriver @($run,$comTarget,'open','cancel','foreign') @('COM_FOREIGN_QUERY hr=00000000 present=1',"COM_BRIDGE bits=$bits",'old=0','hr=800704c7','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-factory-$arch" $dialogDriver @($run,$comTarget,'open','accept','factory') @('COM_FACTORY identity=1 hr=00000000',"COM_BRIDGE bits=$bits",'old=1','same=1','DIALOG_DRIVER exit=0') 30
        foreach($labelKind in @('open','save','folder')) {
            $cases+=Case "dialog-com-label-$arch-$labelKind" $dialogDriver @($run,$comTarget,$labelKind,'accept','labels') @("COM_BRIDGE bits=$bits",'old=1','same=1','labels=1','DIALOG_DRIVER exit=0') 30
            foreach($action in @('accept','cancel')) {
                $cases+=Case "dialog-com-cancel-label-$arch-$labelKind-$action" $dialogDriver @($run,$comTarget,$labelKind,$action,'cancel-label') @("COM_BRIDGE bits=$bits",'old=1','COM_CANCEL_LABEL seen=1','DIALOG_DRIVER exit=0') 30
            }
        }
        $cases+=Case "dialog-com-multi-$arch" $dialogDriver @($run,$multiTarget,'accept','spaces') @("COM_MULTI bits=$bits",'selected=2','count=2 paths=11','single=8000ffff','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-multi-single-$arch" $dialogDriver @($run,$multiTarget,'accept','spaces','single') @("COM_MULTI bits=$bits",'selected=1','count=1 paths=10','single=00000000','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-multi-cancel-$arch" $dialogDriver @($run,$multiTarget,'cancel','spaces') @("COM_MULTI bits=$bits",'selected=2','hr=800704c7','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-multi-vetocancel-native-$arch" $dialogDriver @('--direct',$multiTarget,'vetocancel','spaces','both','events','modern') @('MULTI_EVENTS calls=1 counts=1 paths=1 refs=1',"COM_MULTI bits=$bits",'hr=800704c7 array=80004005','single=8000ffff','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-multi-vetocancel-styled-$arch" $dialogDriver @($run,$multiTarget,'vetocancel','spaces','both','events','modern') @('MULTI_EVENTS calls=1 counts=1 paths=1 refs=1',"COM_MULTI bits=$bits",'hr=800704c7 array=80004005','single=8000ffff','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-old-event-$arch" $dialogDriver @($run,$oldEventTarget) @("OLD_FILE_EVENT bits=$bits",'calls=2 valid=2 kept=1 result=1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-old-nested-$arch" $dialogDriver @($run,$oldNestedTarget) @("NESTED_OLD bits=$bits",'opens=3 checks=3 attached=3 failures=0 cancelled=111','DIALOG_DRIVER exit=0') 30
        $cases+=Case "creation-wndproc-$arch" $dialogDriver @('--direct',$creationWndprocTarget,$creationDll) @("NC_REPLACE bits=$bits",'attached_before=1 attached_after=1 ended=1 installed=1','failure=0','DIALOG_DRIVER exit=0') 30
        foreach($eventMode in @('normal','release','self','remove')) {
            $cases+=Case "dialog-com-event-$arch-$eventMode" $dialogDriver @($run,$eventsTarget,$eventMode) @("COM_EVENTS bits=$bits",'unadvise=00000000 refs=1','DIALOG_DRIVER exit=0') 30
        }
        $cases+=Case "dialog-com-folder-event-native-$arch" $dialogDriver @('--direct',$eventsTarget,'folder') @("COM_EVENTS bits=$bits seen=1 old=0 calls=2 correct=2 identity=2",'FOLDER_EVENT_FINAL correct=1 hr=00000000','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-event-styled-$arch" $dialogDriver @($run,$eventsTarget,'folder') @("COM_EVENTS bits=$bits seen=1 old=0 calls=2 correct=2 identity=2",'FOLDER_EVENT_FINAL correct=1 hr=00000000','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-native-$arch" $dialogDriver @('--direct',$eventsTarget,'overwrite') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE count=2 correct=2 ok=1 kept=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-styled-$arch" $dialogDriver @($run,$eventsTarget,'overwrite') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE count=2 correct=2 ok=1 kept=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-multi-native-$arch" $dialogDriver @('--direct',$eventsTarget,'overwrite-multi') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE_MULTI other=0 ok=2 refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-multi-styled-$arch" $dialogDriver @($run,$eventsTarget,'overwrite-multi') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE_MULTI other=0 ok=2 refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-notimpl-native-$arch" $dialogDriver @('--direct',$eventsTarget,'overwrite-multi-notimpl') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE_MULTI other=1 ok=2 refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-notimpl-styled-$arch" $dialogDriver @($run,$eventsTarget,'overwrite-multi-notimpl') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE_MULTI other=1 ok=2 refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-denied-native-$arch" $dialogDriver @('--direct',$eventsTarget,'overwrite-multi-denied') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE_MULTI other=1 ok=2 refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-denied-styled-$arch" $dialogDriver @($run,$eventsTarget,'overwrite-multi-denied') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE_MULTI other=1 ok=2 refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-overwrite-ext-native-$arch" $dialogDriver @('--direct',$eventsTarget,'overwrite-ext') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE count=2 correct=2 ok=1 kept=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-$arch-multi-notimpl-native" $dialogDriver @('--direct',$eventsTarget,'multi-notimpl') @("COM_EVENTS bits=$bits seen=1 old=0",'COM_EVENT_MULTI_ERROR enabled=0 notimpl=1','COM_EVENT_MULTI_VETO first=1 other=1','refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-$arch-multi-notimpl-styled" $dialogDriver @($run,$eventsTarget,'multi-notimpl') @("COM_EVENTS bits=$bits seen=1 old=0",'COM_EVENT_MULTI_ERROR enabled=0 notimpl=1','COM_EVENT_MULTI_VETO first=1 other=1','refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-$arch-multi-error-native" $dialogDriver @('--direct',$eventsTarget,'multi-error') @("COM_EVENTS bits=$bits seen=1 old=0",'COM_EVENT_MULTI_ERROR enabled=1','COM_EVENT_MULTI_VETO first=2 other=1 kept=1 refs=1,1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-$arch-multi-error-styled" $dialogDriver @($run,$eventsTarget,'multi-error') @("COM_EVENTS bits=$bits seen=1 old=0",'COM_EVENT_MULTI_ERROR enabled=1','COM_EVENT_MULTI_VETO first=2 other=1 kept=1 refs=1,1','DIALOG_DRIVER exit=0') 30
        foreach($kind in @('fileok-error','fileok-notimpl','fileok-success2')) {
            $isNotimpl=if($kind -eq 'fileok-notimpl'){'1'}else{'0'}
            $calls=if($kind -eq 'fileok-notimpl'){'1'}else{'2'}
            $markers=@("COM_EVENTS bits=$bits seen=1 old=0", "COM_FILEOK_HRESULT notimpl=$isNotimpl calls=$calls expected=$calls",'refs=1 worker=0','DIALOG_DRIVER exit=0')
            $cases+=Case "dialog-com-$arch-$kind-native" $dialogDriver @('--direct',$eventsTarget,$kind) $markers 30
            $cases+=Case "dialog-com-$arch-$kind-styled" $dialogDriver @($run,$eventsTarget,$kind) $markers 30
        }
        $cases+=Case "dialog-com-overwrite-ext-styled-$arch" $dialogDriver @($run,$eventsTarget,'overwrite-ext') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_OVERWRITE count=2 correct=2 ok=1 kept=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-share-native-$arch" $dialogDriver @('--direct',$eventsTarget,'share') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_SHARE count=1 correct=1 ok=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-share-styled-$arch" $dialogDriver @($run,$eventsTarget,'share') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_SHARE count=1 correct=1 ok=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-share-refuse-native-$arch" $dialogDriver @('--direct',$eventsTarget,'share-refuse') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_SHARE count=2 correct=2 ok=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-share-refuse-styled-$arch" $dialogDriver @($run,$eventsTarget,'share-refuse') @("COM_EVENTS bits=$bits",'old=0','COM_EVENT_SHARE count=2 correct=2 ok=1 refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-root-native-$arch" $dialogDriver @('--direct',$rootFolderTarget) @("ROOT_FOLDER bits=$bits old=0 hr=00000000",'match=1','ROOT_RESULT hr=800704c7 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-root-styled-$arch" $dialogDriver @($run,$rootFolderTarget,'old') @("ROOT_FOLDER bits=$bits old=1 hr=00000000",'match=1','ROOT_RESULT hr=800704c7 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-hidden-native-$arch" $dialogDriver @('--direct',$hiddenNativeTarget) @("COM_HIDDEN_NAV bits=$bits",'PROBE_VIEW hr=00000000','cleanup=1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-nav-native-$arch" $dialogDriver @('--direct',$navTarget) @("NAV_COM bits=$bits",'root=1 child=1 veto_root=1 set_ok=2','refs=1 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-nav-styled-$arch" $dialogDriver @($run,$navTarget) @("NAV_COM bits=$bits",'root=1 child=1 veto_root=1 set_ok=2','refs=1 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        foreach($folderKind in @('file','folder')) {
            $args=@('multi');if($folderKind -eq 'folder'){$args+=@('folder')}
            $markers=@("NAV_COM bits=$bits",'NAV_MULTI first=2 second=2 refs=1,1','root=2 child=2 veto_root=1 set_ok=2','NAV_CLEANUP child=1 root=1','DIALOG_DRIVER exit=0')
            $cases+=Case "dialog-com-nav-multi-$arch-$folderKind-native" $dialogDriver (@('--direct',$navTarget)+$args) $markers 30
            $cases+=Case "dialog-com-nav-multi-$arch-$folderKind-styled" $dialogDriver (@($run,$navTarget)+$args) $markers 30
        }
        foreach($folderKind in @('file','folder')) {
            $args=@('notimpl');if($folderKind -eq 'folder'){$args+=@('folder')}
            $markers=@("NAV_COM bits=$bits",'NAV_NOTIMPL child_changes=2 post_first=1','root=1 child=2 veto_root=1 set_ok=2','refs=1 wait=0 worker=0','DIALOG_DRIVER exit=0')
            $cases+=Case "dialog-com-nav-notimpl-$arch-$folderKind-native" $dialogDriver (@('--direct',$navTarget)+$args) $markers 30
            $cases+=Case "dialog-com-nav-notimpl-$arch-$folderKind-styled" $dialogDriver (@($run,$navTarget)+$args) $markers 30
        }
        foreach($folderKind in @('file','folder')) {
            $args=@('success2');if($folderKind -eq 'folder'){$args+=@('folder')}
            $markers=@("NAV_COM bits=$bits",'NAV_SUCCESS2 first_rejected=1','root=1 child=1 veto_root=1 set_ok=2','NAV_CLEANUP child=1 root=1','refs=1 wait=0 worker=0','DIALOG_DRIVER exit=0')
            $cases+=Case "dialog-com-nav-success2-$arch-$folderKind-native" $dialogDriver (@('--direct',$navTarget)+$args) $markers 30
            $cases+=Case "dialog-com-nav-success2-$arch-$folderKind-styled" $dialogDriver (@($run,$navTarget)+$args) $markers 30
        }
        $cases+=Case "dialog-com-folder-nav-native-$arch" $dialogDriver @('--direct',$navTarget,'folder') @('FOLDER_NAV_TYPE count=0',"NAV_COM bits=$bits",'root=1 child=1 veto_root=1 set_ok=2','refs=1 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-nav-styled-$arch" $dialogDriver @($run,$navTarget,'folder') @('FOLDER_NAV_TYPE count=0',"NAV_COM bits=$bits",'root=1 child=1 veto_root=1 set_ok=2','refs=1 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-redirect-native-$arch" $dialogDriver @('--direct',$redirectTarget) @("REDIRECT_RESULT bits=$bits",'old=0','requested=1 arrived=1 child=0','refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-redirect-styled-$arch" $dialogDriver @($run,$redirectTarget) @("REDIRECT_RESULT bits=$bits",'old=0','requested=1 arrived=1 child=0','refs=1 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-initial-redirect-native-$arch" $dialogDriver @('--direct',$initialRedirectTarget) @("REDIRECT_INITIAL bits=$bits old=0 sequence=6 errors=0",'DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-initial-redirect-styled-$arch" $dialogDriver @($run,$initialRedirectTarget) @("REDIRECT_INITIAL bits=$bits old=0 sequence=6 errors=0",'DIALOG_DRIVER exit=0') 30
        foreach($closeKind in @('close','close-ok','close-changed','close-changed-ok')) {
            $isSuccess=if($closeKind -like '*-ok'){'1'}else{'0'}
            $showHr=if($closeKind -like '*-ok'){'00000000'}else{'80004004'}
            foreach($folderKind in @('file','folder')) {
                $args=@($closeKind);if($folderKind -eq 'folder'){$args+=@('folder')}
                $markers=@("CLOSE_INITIAL bits=$bits success=$isSuccess hr=$showHr close=00000000 result=8000ffff item=0 calls=1 refs=1 worker=0 cleanup=1 changed=1",'DIALOG_DRIVER exit=0')
                $cases+=Case "dialog-com-initial-$arch-$folderKind-$closeKind-native" $dialogDriver (@('--direct',$initialRedirectTarget)+$args) $markers 30
                $cases+=Case "dialog-com-initial-$arch-$folderKind-$closeKind-styled" $dialogDriver (@($run,$initialRedirectTarget)+$args) $markers 30
            }
        }
        $cases+=Case "dialog-com-folder-redirect-native-$arch" $dialogDriver @('--direct',$initialRedirectTarget,'folder') @("REDIRECT_INITIAL bits=$bits old=0 sequence=6 errors=0",'DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-redirect-styled-$arch" $dialogDriver @($run,$initialRedirectTarget,'folder') @("REDIRECT_INITIAL bits=$bits old=0 sequence=6 errors=0",'DIALOG_DRIVER exit=0') 30
        foreach($vetoMode in @('once','twice')) {
            $vetoArg=if($vetoMode -eq 'twice'){@('twice')}else{@()}
            $cases+=Case "dialog-com-initial-veto-native-$arch-$vetoMode" $dialogDriver (@('--direct',$initialVetoTarget)+$vetoArg) @("VETO_ASSERT bits=$bits old=0 fallback=1 final_ok=1 refs=1 worker=0",'DIALOG_DRIVER exit=0') 30
            $cases+=Case "dialog-com-initial-veto-styled-$arch-$vetoMode" $dialogDriver (@($run,$initialVetoTarget)+$vetoArg) @("VETO_ASSERT bits=$bits old=0 fallback=1 final_ok=1 refs=1 worker=0",'DIALOG_DRIVER exit=0') 30
        }
        $cases+=Case "dialog-com-folder-veto-native-$arch" $dialogDriver @('--direct',$initialVetoTarget,'folder') @("VETO_ASSERT bits=$bits old=0 fallback=1 final_ok=1 refs=1 worker=0",'DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-veto-styled-$arch" $dialogDriver @($run,$initialVetoTarget,'folder') @("VETO_ASSERT bits=$bits old=0 fallback=1 final_ok=1 refs=1 worker=0",'DIALOG_DRIVER exit=0') 30
        foreach($folderMode in @('native','styled')) {
            $folderLaunch=if($folderMode -eq 'native'){'--direct'}else{$run}
            $folderOld=if($folderMode -eq 'native'){'old=0'}else{'old=1'}
            $cases+=Case "dialog-com-live-folder-$arch-$folderMode" $dialogDriver @($folderLaunch,$liveFolderTarget) @("LIVE_FOLDER_RESULT bits=$bits",$folderOld,'initial=1 final=1 stable=1 v2=1 parent=1','phase=3','worker=0','DIALOG_DRIVER exit=0') 30
            foreach($folderKind in @('open','save')) {
                $cases+=Case "dialog-com-set-folder-$arch-$folderKind-$folderMode" $dialogDriver @($folderLaunch,$liveFolderTarget,'set',$folderKind) @("LIVE_FOLDER_RESULT bits=$bits",$folderOld,'initial=1 final=1 stable=1 v2=1 parent=1 name=1 filter=1 phase=3','worker=0','DIALOG_DRIVER exit=0') 30
                $cases+=Case "dialog-com-live-filter-$arch-$folderKind-$folderMode" $dialogDriver @($folderLaunch,$liveFolderTarget,'filter',$folderKind) @('LIVE_FILTER selected=1 set=00000000 remains=1',"LIVE_FOLDER_RESULT bits=$bits",$folderOld,'filter=1 phase=3','worker=0 livefilter=1','DIALOG_DRIVER exit=0') 30
            }
        }
        foreach($liveKind in @('open','save')) {
            foreach($liveMode in @('native','styled')) {
                $liveLaunch=if($liveMode -eq 'native'){'--direct'}else{$run}
                $expectedOld=if($liveMode -eq 'native'){'old=0'}else{'old=1'}
                $cases+=Case "dialog-com-live-$arch-$liveKind-$liveMode" $dialogDriver @($liveLaunch,$liveTarget,$liveKind,'accept') @('LIVE_NAME initial=1 updated=1 visible=1 user=1 v2=1',$expectedOld,"LIVE_RESULT bits=$bits",'result=1','DIALOG_DRIVER exit=0') 30
            }
        }
        foreach($linkMode in @('keep','follow')) {
            foreach($linkBackend in @('native','styled')) {
                $linkLaunch=if($linkBackend -eq 'native'){'--direct'}else{$run}
                $linkOld=if($linkBackend -eq 'native'){'old=0'}else{'old=1'}
                $linkExpected=if($linkMode -eq 'keep'){'link=1 target=0'}else{'link=0 target=1'}
                $cases+=Case "dialog-com-link-$arch-$linkMode-$linkBackend" $dialogDriver @($linkLaunch,$linkTarget,$linkMode) @("LINK_RESULT bits=$bits",$linkOld,$linkExpected,'worker=0','DIALOG_DRIVER exit=0') 30
            }
            $multiMode=if($linkMode -eq 'keep'){'linkkeep'}else{'linkfollow'}
            $cases+=Case "dialog-com-multi-link-$arch-$linkMode" $dialogDriver @($run,$multiTarget,'accept',$multiMode) @("COM_MULTI bits=$bits",'count=2 paths=11','DIALOG_DRIVER exit=0') 30
        }
        $cases+=Case "dialog-com-identity-$arch" $dialogDriver @($run,$identityTarget) @("COM_CUSTOM_IDENTITY bits=$bits hr=00000000 same=1","COM_V2_IDENTITY bits=$bits hr=00000000 same=1",'DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-v2-$arch" $dialogDriver @($run,$comTarget,'folder','accept','v2') @("COM_BRIDGE bits=$bits",'folder=1','old=1','array=1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-custom-$arch" $dialogDriver @($run,$comTarget,'open','cancel','custom') @("COM_BRIDGE bits=$bits",'old=0','hr=800704c7','DIALOG_DRIVER exit=0') 30
        $cases+=Case "scroll-frame-$arch" $dialogDriver @($run,$scrollTarget) @("FRAME_HOST bits=$bits attached=0 installed=1",'FRAME_AXIS axis=0 action=hover diff=0','FRAME_AXIS axis=0 action=leave diff=0','FRAME_AXIS axis=1 action=hover diff=0','FRAME_AXIS axis=1 action=leave diff=0','FRAME_DRAG axis=0 captured=1 moved=1','FRAME_DRAG axis=1 captured=1 moved=1','FRAME_SCROLL_RESULT horizontal=0 vertical=0','DIALOG_DRIVER exit=0') 30
        foreach($scrollKind in @('list','edit')) {
            $cases+=Case "scroll-$scrollKind-$arch" $dialogDriver @($run,$scrollTarget,$scrollKind) @("FRAME_HOST bits=$bits attached=1 installed=1",'FRAME_AXIS axis=0 action=hover diff=0','FRAME_AXIS axis=0 action=leave diff=0','FRAME_AXIS axis=1 action=hover diff=0','FRAME_AXIS axis=1 action=leave diff=0','FRAME_DRAG axis=0 captured=1 moved=1','FRAME_DRAG axis=1 captured=1 moved=1','FRAME_SCROLL_RESULT horizontal=0 vertical=0','DIALOG_DRIVER exit=0') 30
        }
        foreach($kind in 1..3) {
            $cases+=Case "dialog-modern-api-$arch-$kind" $dialogDriver @($run,$modernTarget,"$kind") @("MODERN_API bits=$bits kind=$kind seen=1 modern=1 frame=1 hr=800704c7 path_empty=1",'DIALOG_DRIVER exit=0') 30
        }
        $cases+=Case "dialog-task-$arch" $dialogDriver @($run,$taskTarget) @("TASK_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
        foreach($encoding in @('ansi','wide')) {
            $cases+=Case "dialog-print-ex-apply-state-$arch-$encoding" $dialogDriver @($run,$printTarget,'ex-apply-state',$encoding) @("EX_APPLY_STATE bits=$bits",'initial=1','changed=1','applied=1','again=1','applied_again=1','copies=4','old=1','valid=1','DIALOG_DRIVER exit=0') 30
        }
        foreach($mode in @('ex-device-print','ex-device-apply','ex-device-cancel','ex-device-locked')) {foreach($encoding in @('ansi','wide')) {
            $cases+=Case "dialog-print-$mode-$arch-$encoding" $dialogDriver @($run,$printTarget,$mode,$encoding) @("EX_DEVICE bits=$bits",'initial=1','old=1','valid=1','DIALOG_DRIVER exit=0') 30
        }}
        foreach($encoding in @('ansi','wide')) {
            $cases+=Case "dialog-print-ex-initial-all-$arch-$encoding" $dialogDriver @($run,$printTarget,'ex-initial-all',$encoding) @("EX_INITIAL bits=$bits",'initial=1','ranges=0','old=1','valid=1','DIALOG_DRIVER exit=0') 30
        }
        foreach($mode in @('ex-initial-print','ex-initial-apply','ex-initial-cancel')) {foreach($encoding in @('ansi','wide')) {
            $cases+=Case "dialog-print-$mode-$arch-$encoding" $dialogDriver @($run,$printTarget,$mode,$encoding) @("EX_INITIAL bits=$bits",'initial=1','ranges=2','old=1','valid=1','DIALOG_DRIVER exit=0') 30
        }}
        foreach($mode in @('ex-ansi-cancel','ex-ansi-fallback')) {
            $cases+=Case "dialog-print-$mode-$arch" $dialogDriver @($run,$printTarget,$mode,'ansi') @("EX_ANSI_CANCEL bits=$bits",'untouched=1','frame=1','DIALOG_DRIVER exit=0') 30
        }
        foreach($mode in @('ex-ansi-ranges','ex-ansi-apply')) {
            $cases+=Case "dialog-print-$mode-$arch" $dialogDriver @($run,$printTarget,$mode,'ansi') @("EX_ANSI bits=$bits",'ranges=2','copies=3','ansi_layout=1','old=1','frame=1','valid=1','DIALOG_DRIVER exit=0') 30
        }
        foreach($mode in @('ex-nopages-print','ex-nopages-apply','ex-nopages-cancel','ex-options-print','ex-options-apply','ex-options-cancel')) {
            $cases+=Case "dialog-print-$mode-$arch" $dialogDriver @($run,$printTarget,$mode,'wide') @("EX_NOPAGES bits=$bits",'old=1','frame=1','valid=1','DIALOG_DRIVER exit=0') 30
        }
        foreach($mode in @('ex-cancel','ex-fallback')) {
            $cases+=Case "dialog-print-$mode-$arch" $dialogDriver @($run,$printTarget,$mode,'wide') @("EX_CANCEL bits=$bits",'untouched=1','frame=1','DIALOG_DRIVER exit=0') 30
        }
        foreach($mode in @('ex-ranges','ex-apply')) {
            $cases+=Case "dialog-print-$mode-$arch" $dialogDriver @($run,$printTarget,$mode,'wide') @("PRINT_EX_RANGE bits=$bits",'ranges=2','frame=1','valid=1','old=1','DIALOG_DRIVER exit=0') 30
        }
        $cases+=Case "dialog-print-ex-invalid-$arch" $dialogDriver @($run,$printTarget,'invalid','wide') @("PRINT_EX_INVALID bits=$bits",'DIALOG_DRIVER exit=0') 30
        foreach($encoding in @('ansi','wide')) {
            foreach($action in @('accept','cancel')) {
                $cases+=Case "dialog-print-classic-$arch-$encoding-$action" $dialogDriver @($run,$printTarget,'classic',$encoding,$action) @("PRINT_CLASSIC bits=$bits",'old=1','frame=1','clean=1','error=0','DIALOG_DRIVER exit=0') 30
            }
            $cases+=Case "dialog-sheet-$arch-$encoding" $dialogDriver @($run,$sheetTarget,$encoding) @("SHEET_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
            foreach($kind in @('find','replace')) {
                $cases+=Case "dialog-$kind-$arch-$encoding" $dialogDriver @($run,$findTarget,$kind,$encoding) @("FIND_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
            }
            foreach($kind in @('font','color')) { foreach($action in @('ok','cancel')) {
                $cases+=Case "dialog-$kind-$arch-$encoding-$action" $dialogDriver @($run,$commonTarget,$kind,$encoding,$action) @("COMMON_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
            }}
            $cases+=Case "dialog-color-$arch-$encoding-expand" $dialogDriver @($run,$commonTarget,'color',$encoding,'expand') @("COLOR_EXPANDED",'DIALOG_DRIVER exit=0') 30
            foreach($kind in @('print','page')) {
                $cases+=Case "dialog-$kind-$arch-$encoding" $dialogDriver @($run,$printTarget,$kind,$encoding) @("PRINT_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
            }
        }
        $cases+=Case "dialog-file-negative-$arch" $dialogDriver @('--direct',$fileTarget,'open','wide','0','cancel') @("FILE_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-file-oldhook-accept-native-$arch" $dialogDriver @('--direct',$fileTarget,'open','wide','1','oldhookaccept') @("FILE_DIALOG bits=$bits",'old=1','ok=1','same=1','custom_hook=1 hook_seen=1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-file-oldhook-accept-styled-$arch" $dialogDriver @($run,$fileTarget,'open','wide','1','oldhookaccept') @("FILE_DIALOG bits=$bits",'old=1','ok=1','same=1','custom_hook=1 hook_seen=1','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-file-oldhook-dynamic-native-$arch" $dialogDriver @('--direct',$fileTarget,'open','wide','1','oldhookdynamic') @("FILE_DYNAMIC bits=$bits set=1 same=1",'DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-file-oldhook-dynamic-styled-$arch" $dialogDriver @($run,$fileTarget,'open','wide','1','oldhookdynamic') @("FILE_DYNAMIC bits=$bits set=1 same=1",'DIALOG_DRIVER exit=0') 30
        foreach($kind in @('open','save')) {
            foreach($encoding in @('ansi','wide')) {
                $cases+=Case "dialog-file-$arch-$kind-$encoding" $dialogDriver @($run,$fileTarget,$kind,$encoding,'1') @("FILE_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
            }
        }
        foreach($encoding in @('ansi','wide')) {
            foreach($mode in @('v400','default','oldhook')) {
                $cases+=Case "dialog-file-$arch-$encoding-$mode" $dialogDriver @($run,$fileTarget,'open',$encoding,'1',$mode) @("FILE_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
            }
        }
        foreach($mode in @('cancel','hook')) {
            $layout=if($mode -eq 'hook'){'0'}else{'1'}
            $cases+=Case "dialog-file-$arch-$mode" $dialogDriver @($run,$fileTarget,'open','wide',$layout,$mode) @("FILE_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 30
        }
        foreach($fileKind in @('open','save')) {
            $cases+=Case "dialog-com-select-folder-$arch-$fileKind" $dialogDriver @($run,$liveFolderTarget,'select',$fileKind) @("LIVE_FOLDER_RESULT bits=$bits old=1",'LIVE_FOLDER_SELECT before_enter=1','LIVE_FOLDER_SELECT before_parent=1','initial=1 final=1 stable=1','worker=0','DIALOG_DRIVER exit=0') 30
        }
        $cases+=Case "dialog-com-folder-create-$arch" $dialogDriver @($run,$eventsTarget,'folder-noevents') @("COM_EVENTS bits=$bits seen=1 old=1 calls=0",'FOLDER_EVENT_FINAL correct=1','COM_FOLDER_CREATE invalid=0 prompt=1 rejected=0 made=1 cleaned=1 wrong_directory=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-create-selected-$arch" $dialogDriver @($run,$eventsTarget,'folder-select-noevents') @("COM_EVENTS bits=$bits seen=1 old=1 calls=0",'COM_FOLDER_SELECT enabled=1','FOLDER_EVENT_FINAL correct=1','COM_FOLDER_CREATE invalid=0 prompt=1 rejected=0 made=1 cleaned=1 wrong_directory=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-create-cwd-$arch" $dialogDriver @($run,$eventsTarget,'folder-cwd-noevents') @("COM_EVENTS bits=$bits seen=1 old=1 calls=0",'FOLDER_EVENT_FINAL correct=1','COM_FOLDER_CREATE invalid=0 prompt=1 rejected=0 made=1 cleaned=1 wrong_directory=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-create-existing-$arch" $dialogDriver @($run,$eventsTarget,'folder-existing-noevents') @("COM_EVENTS bits=$bits seen=1 old=1 calls=0",'COM_FOLDER_EXISTING exists=1 rejected=1 marker=1','COM_FOLDER_CREATE invalid=0 prompt=1 rejected=1 made=1 cleaned=1 wrong_directory=0','hr=800704c7','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-com-folder-create-invalid-$arch" $dialogDriver @($run,$eventsTarget,'folder-invalid-noevents') @("COM_EVENTS bits=$bits seen=1 old=1 calls=0",'COM_FOLDER_CREATE invalid=1 prompt=1 rejected=1 made=0 cleaned=1 wrong_directory=0','hr=800704c7','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-folder-create-typed-$arch" $dialogDriver @($run,$folderTarget,'wide','newclassictyped') @("FOLDER_DIALOG bits=$bits",'FOLDER_CLASSIC_ENTER navigated=1 typed=1','FOLDER_CLASSIC_NEW cancel=0 prompt=1 callback=1 cleaned=1 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        $cases+=Case "dialog-folder-create-enter-$arch" $dialogDriver @($run,$folderTarget,'wide','newclassicenter') @("FOLDER_DIALOG bits=$bits",'FOLDER_CLASSIC_ENTER navigated=1','FOLDER_CLASSIC_NEW cancel=0 prompt=1 callback=1 cleaned=1 wait=0 worker=0','DIALOG_DRIVER exit=0') 30
        foreach($encoding in @('ansi','wide')) {
            foreach($mode in @('newclassic','newclassiccancel')) {
                $cancelFlag=if($mode -eq 'newclassiccancel'){'1'}else{'0'}
                $cases+=Case "dialog-folder-create-$arch-$encoding-$mode" $dialogDriver @($run,$folderTarget,$encoding,$mode) @("FOLDER_DIALOG bits=$bits","FOLDER_CLASSIC_NEW cancel=$cancelFlag prompt=1 callback=1 cleaned=1 wait=0 worker=0",'DIALOG_DRIVER exit=0') 30
            }
        }
        foreach($encoding in @('ansi','wide')) {
            foreach($mode in @('select','cancel')) {
                $dialogArgs=@($run,$folderTarget,$encoding)
                if($mode -eq 'cancel'){$dialogArgs+=@('cancel')}
                $cases+=Case "dialog-folder-$arch-$encoding-$mode" $dialogDriver $dialogArgs @("FOLDER_DIALOG bits=$bits",'DIALOG_DRIVER exit=0') 45
            }
        }
        foreach($mode in @('same','newfolder','controls')) {
            $markers=@("FOLDER_DIALOG bits=$bits",'DIALOG_DRIVER exit=0')
            if($mode -eq 'controls'){$markers+=@('FOLDER_RIGHT_VISUAL_PASS settled_equals_repaint=1','FOLDER_SCREEN_PRESS read=1 repaint=1 diff=0')}
            $cases+=Case "dialog-folder-$arch-$mode" $dialogDriver @($run,$folderTarget,'wide',$mode) $markers 45
        }
        $cases+=Case "dialog-folder-$arch-theme" $dialogDriver @($run,$folderTarget,'wide','theme') @("FOLDER_THEME_TIMERS bits=$bits count=0",'DIALOG_DRIVER exit=0') 45
    }
    $cases+=@(
        (Case 'legacy-iat-32' (Join-Path $x86 'launch-staging.exe') @('--parent',$Package) @('LEGACY IAT PASS bits=32 zero_oft=0 initialized=1 metadata_restored=1','LEGACY IAT PASS bits=32 zero_oft=1 initialized=1 metadata_restored=1','LEGACY IAT REJECT PASS bits=32 mode=3 error=193')),
        (Case 'legacy-iat-64' (Join-Path $x64 'launch-staging.exe') @('--parent',$Package) @('LEGACY IAT PASS bits=64 zero_oft=0 initialized=1 metadata_restored=1','LEGACY IAT PASS bits=64 zero_oft=1 initialized=1 metadata_restored=1','LEGACY IAT REJECT PASS bits=64 mode=3 error=193')),
        (Case 'theme-track-32' (Join-Path $x86 'core-theme-track.exe') @() @('THEME_TRACK_OK shared=1 alias=1 combo=1 failed_close=1 nested=1 overlap=1 capacity=1')),
        (Case 'theme-track-64' (Join-Path $x64 'core-theme-track.exe') @() @('THEME_TRACK_OK shared=1 alias=1 combo=1 failed_close=1 nested=1 overlap=1 capacity=1')),
        (Case 'theme-track-cancel-32' (Join-Path $x86 'core-theme-track.exe') @('cancel') @('THEME_TRACK_CANCEL_OK')),
        (Case 'theme-track-cancel-64' (Join-Path $x64 'core-theme-track.exe') @('cancel') @('THEME_TRACK_CANCEL_OK')),
        (Case 'uac-unit-32' (Join-Path $x86 'uac-unit.exe') @() @('UAC UNIT PASS cases=8')),
        (Case 'uac-unit-64' (Join-Path $x64 'uac-unit.exe') @() @('UAC UNIT PASS cases=8'))
    )
    $cases+=@(
        (Case 'vb5-checklist-dpi144-32' (Join-Path $x86 'core-vb-checklist-dpi144.exe') @($run,(Join-Path $x86 'vb-checklist-fixture/MSVBVM50.dll')) @('VB_CHECKLIST_144_OK bits=32 vb5=1 styled=0 states=4 invalid_geometry=1 text_scope=0','VB_CHECKLIST_144_OK bits=32 vb5=1 styled=1 states=4 invalid_geometry=1 text_scope=1')),
        (Case 'vb6-checklist-dpi144-32' (Join-Path $x86 'core-vb-checklist-dpi144.exe') @($run,(Join-Path $x86 'vb-checklist-fixture/MSVBVM60.dll')) @('VB_CHECKLIST_144_OK bits=32 vb5=0 styled=0 states=4 invalid_geometry=1 text_scope=0','VB_CHECKLIST_144_OK bits=32 vb5=0 styled=1 states=4 invalid_geometry=1 text_scope=1')),
        (Case 'vb5-checklist-dpi144-64' (Join-Path $x64 'core-vb-checklist-dpi144.exe') @($run,(Join-Path $x64 'vb-checklist-fixture/MSVBVM50.dll')) @('VB_CHECKLIST_144_OK bits=64 vb5=1 styled=0 states=4 invalid_geometry=1 text_scope=0','VB_CHECKLIST_144_OK bits=64 vb5=1 styled=1 states=4 invalid_geometry=1 text_scope=1')),
        (Case 'vb6-checklist-dpi144-64' (Join-Path $x64 'core-vb-checklist-dpi144.exe') @($run,(Join-Path $x64 'vb-checklist-fixture/MSVBVM60.dll')) @('VB_CHECKLIST_144_OK bits=64 vb5=0 styled=0 states=4 invalid_geometry=1 text_scope=0','VB_CHECKLIST_144_OK bits=64 vb5=0 styled=1 states=4 invalid_geometry=1 text_scope=1'))
    )
    $cases+=@(
        (Case 'vb-checklist-32' (Join-Path $x86 'core-vb-checklist.exe') @($run,(Join-Path $x86 'vb-checklist-fixture/MSVBVM60.dll')) @('VB_CHECKLIST_OK bits=32 styled=1 selected=1 restored=1 wrong_geometry=1 wrong_source=1 wrong_caller=1 disabled_states=1 mismatched_state_native=1 reentrant_disable_native=1 reentrant_style_native=1 reentrant_reparent_lifecycle=1 system_text=1 unicode_fallback=1 source_font_preserved=1 role_retirement=1 mixed_fallback=1')),
        (Case 'vb-user-edge-32' (Join-Path $x86 'core-vb-user-edge.exe') @($run,(Join-Path $x86 'vb-container-fixture/MSVBVM60.dll')) @('VB_USER_EDGE_OK bits=32 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=6 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=32 label_pixels=','native_preserved=1')),
        (Case 'vb-data-border-32' (Join-Path $x86 'core-vb-data-border.exe') @($run,(Join-Path $x86 'vb-data-fixture/MSVBVM60.dll')) @('VB_DATA_BORDER_OK bits=32 states=4 client_preserved=1 input=1 wrong_owner=1 unsupported=1 reparent=1')),
        (Case 'vb-data-arrows-32' (Join-Path $x86 'core-vb-data-arrows.exe') @($run,(Join-Path $x86 'vb-data-arrows-fixture/MSVBVM60.dll')) @('VB_DATA_ARROW_OK bits=32 states=4 middle_preserved=1 input=4 wrong_caller=1 clip=1 wrong_owner=1 unsupported=1 reparent=1')),
        (Case 'vb-data-arrows-64' (Join-Path $x64 'core-vb-data-arrows.exe') @($run,(Join-Path $x64 'vb-data-arrows-fixture/MSVBVM60.dll')) @('VB_DATA_ARROW_OK bits=64 states=4 middle_preserved=1 input=4 wrong_caller=1 clip=1 wrong_owner=1 unsupported=1 reparent=1')),
        (Case 'vb-data-border-64' (Join-Path $x64 'core-vb-data-border.exe') @($run,(Join-Path $x64 'vb-data-fixture/MSVBVM60.dll')) @('VB_DATA_BORDER_OK bits=64 states=4 client_preserved=1 input=1 wrong_owner=1 unsupported=1 reparent=1')),
        (Case 'vb-user-edge-64' (Join-Path $x64 'core-vb-user-edge.exe') @($run,(Join-Path $x64 'vb-container-fixture/MSVBVM60.dll')) @('VB_USER_EDGE_OK bits=64 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=6 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=64 label_pixels=','native_preserved=1')),
        (Case 'vb5-user-edge-32' (Join-Path $x86 'core-vb-user-edge.exe') @($run,(Join-Path $x86 'vb-container-fixture/MSVBVM50.dll')) @('VB_USER_EDGE_OK bits=32 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=5 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=32 label_pixels=','native_preserved=1')),
        (Case 'vb5-user-edge-64' (Join-Path $x64 'core-vb-user-edge.exe') @($run,(Join-Path $x64 'vb-container-fixture/MSVBVM50.dll')) @('VB_USER_EDGE_OK bits=64 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=5 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=64 label_pixels=','native_preserved=1')),
        (Case 'coolbar-lifetime-32' $run @('--wait',(Join-Path $x86 'core-coolbar-lifetime.exe')) @('LIFETIME_START attached=1','LIFETIME_NESTED hits=1 attached=1','LIFETIME_EXCEPTION caught=1 attached=1','LIFETIME_BYPASS remove_hits=0 attached=1','LIFETIME_BYPASS reparent_hits=0 attached=1','LIFETIME_REMOVE hits=1 removed=1 attached=0 alive=1','LIFETIME_REPARENT hits=1 attached=0 parent=1','LIFETIME_FALLBACK kind=0 painted=0 content=1 removed=1','LIFETIME_FALLBACK kind=1 painted=0 content=1 removed=1','LIFETIME_FALLBACK kind=2 painted=0 content=1 removed=1','LIFETIME_FALLBACK kind=3 painted=0 content=1 removed=1','LIFETIME_WRONG_OWNER attached=0','LIFETIME_WRONG_PROC attached=0','LIFETIME_OK'))
        (Case 'coolbar-lifetime-64' $run @('--wait',(Join-Path $x64 'core-coolbar-lifetime.exe')) @('LIFETIME_START attached=1','LIFETIME_NESTED hits=1 attached=1','LIFETIME_EXCEPTION caught=1 attached=1','LIFETIME_BYPASS remove_hits=0 attached=1','LIFETIME_BYPASS reparent_hits=0 attached=1','LIFETIME_REMOVE hits=1 removed=1 attached=0 alive=1','LIFETIME_REPARENT hits=1 attached=0 parent=1','LIFETIME_FALLBACK kind=0 painted=0 content=1 removed=1','LIFETIME_FALLBACK kind=1 painted=0 content=1 removed=1','LIFETIME_FALLBACK kind=2 painted=0 content=1 removed=1','LIFETIME_FALLBACK kind=3 painted=0 content=1 removed=1','LIFETIME_WRONG_OWNER attached=0','LIFETIME_WRONG_PROC attached=0','LIFETIME_OK'))
        (Case 'vcl-mdi-ref-32' (Join-Path $PSHOME 'pwsh.exe') @('-NoProfile','-File',(Join-Path $SourceRoot 'tests/ntw16s-dll/vcl_mdi_ref.ps1'),$run,$d32,(Join-Path $x86 'core-vcl-mdi-ref.exe'),(Join-Path $x86 'libvcl-mdi-fixture.dll')) @('MDI_REF observed_before=0 after=0 forced=1','MDI_REF_OK')),
        (Case 'vcl-mdi-ref-64' (Join-Path $PSHOME 'pwsh.exe') @('-NoProfile','-File',(Join-Path $SourceRoot 'tests/ntw16s-dll/vcl_mdi_ref.ps1'),$run,$d64,(Join-Path $x64 'core-vcl-mdi-ref.exe'),(Join-Path $x64 'libvcl-mdi-fixture.dll')) @('MDI_REF observed_before=0 after=0 forced=1','MDI_REF_OK')),
        (Case 'vcl-mdi-32' $run @('--wait',(Join-Path $x86 'core-vcl-mdi.exe'),(Join-Path $x86 'libvcl-mdi-fixture.dll')) @('FIXTURE initial=1 root=1','FIXTURE bad_host=0','FIXTURE after_reparent=0','FIXTURE after_role_change=0','FIXTURE after_host_remove=0','MDI_FIX_OK')),
        (Case 'vcl-mdi-64' $run @('--wait',(Join-Path $x64 'core-vcl-mdi.exe'),(Join-Path $x64 'libvcl-mdi-fixture.dll')) @('FIXTURE initial=1 root=1','FIXTURE bad_host=0','FIXTURE after_reparent=0','FIXTURE after_role_change=0','FIXTURE after_host_remove=0','MDI_FIX_OK')),
        (Case 'vb5-mdi-32' (Join-Path $x86 'core-vb-mdi.exe') @($run,(Join-Path $x86 'vb-container-fixture/MSVBVM50.dll')) @('VB_MDI_OK bits=32 vb=5 styled=1 attached=1 wrong_parent_native=1 button_maximize=1 maximize_restore=1 destroyed=1')),
        (Case 'vb6-mdi-32' (Join-Path $x86 'core-vb-mdi.exe') @($run,(Join-Path $x86 'vb-container-fixture/MSVBVM60.dll')) @('VB_MDI_OK bits=32 vb=6 styled=1 attached=1 wrong_parent_native=1 button_maximize=1 maximize_restore=1 destroyed=1')),
        (Case 'vb5-mdi-64' (Join-Path $x64 'core-vb-mdi.exe') @($run,(Join-Path $x64 'vb-container-fixture/MSVBVM50.dll')) @('VB_MDI_OK bits=64 vb=5 styled=1 attached=1 wrong_parent_native=1 button_maximize=1 maximize_restore=1 destroyed=1')),
        (Case 'vb6-mdi-64' (Join-Path $x64 'core-vb-mdi.exe') @($run,(Join-Path $x64 'vb-container-fixture/MSVBVM60.dll')) @('VB_MDI_OK bits=64 vb=6 styled=1 attached=1 wrong_parent_native=1 button_maximize=1 maximize_restore=1 destroyed=1')),
        (Case 'vb5-checklist-32' (Join-Path $x86 'core-vb-checklist.exe') @($run,(Join-Path $x86 'vb-checklist-fixture/MSVBVM50.dll')) @('VB_CHECKLIST_OK bits=32 styled=1 selected=1 restored=1 wrong_geometry=1 wrong_source=1 wrong_caller=1 disabled_states=1 mismatched_state_native=1 reentrant_disable_native=1 reentrant_style_native=1 reentrant_reparent_lifecycle=1 system_text=1 unicode_fallback=1 source_font_preserved=1 role_retirement=1 mixed_fallback=1')),
        (Case 'vb-checklist-64' (Join-Path $x64 'core-vb-checklist.exe') @($run,(Join-Path $x64 'vb-checklist-fixture/MSVBVM60.dll')) @('VB_CHECKLIST_OK bits=64 styled=1 selected=1 restored=1 wrong_geometry=1 wrong_source=1 wrong_caller=1 disabled_states=1 mismatched_state_native=1 reentrant_disable_native=1 reentrant_style_native=1 reentrant_reparent_lifecycle=1 system_text=1 unicode_fallback=1 source_font_preserved=1 role_retirement=1 mixed_fallback=1')),
        (Case 'vb5-checklist-64' (Join-Path $x64 'core-vb-checklist.exe') @($run,(Join-Path $x64 'vb-checklist-fixture/MSVBVM50.dll')) @('VB_CHECKLIST_OK bits=64 styled=1 selected=1 restored=1 wrong_geometry=1 wrong_source=1 wrong_caller=1 disabled_states=1 mismatched_state_native=1 reentrant_disable_native=1 reentrant_style_native=1 reentrant_reparent_lifecycle=1 system_text=1 unicode_fallback=1 source_font_preserved=1 role_retirement=1 mixed_fallback=1')),
        (Case 'vb5-owner-check-32' (Join-Path $x86 'core-vb-graphical.exe') @($run,(Join-Path $x86 'vb-graphical-fixture/MSVBVM50.dll'),(Join-Path $x86 'vb-queue-witness.dll')) @('VB5_GRAPHICAL_CHECK_OK ownerdraw=1 checked_frame=1 native_image=1','VB5_GRAPHICAL_OPTION_OK ownerdraw=1 checked_frame=1 native_image=1','VB_GRAPHICAL_CORE_OK bits=32 styled=1')),
        (Case 'vb5-owner-check-64' (Join-Path $x64 'core-vb-graphical.exe') @($run,(Join-Path $x64 'vb-graphical-fixture/MSVBVM50.dll'),(Join-Path $x64 'vb-queue-witness.dll')) @('VB5_GRAPHICAL_CHECK_OK ownerdraw=1 checked_frame=1 native_image=1','VB5_GRAPHICAL_OPTION_OK ownerdraw=1 checked_frame=1 native_image=1','VB_GRAPHICAL_CORE_OK bits=64 styled=1')),
        (Case 'vb-owner-option-32' (Join-Path $x86 'core-vb-graphical.exe') @($run,(Join-Path $x86 'vb-graphical-fixture/MSVBVM60.dll'),(Join-Path $x86 'vb-queue-witness.dll')) @('VB6_GRAPHICAL_OPTION_OK ownerdraw=1 checked_frame=1 native_image=1','VB6_GRAPHICAL_CHECK_OK ownerdraw=1 checked_frame=1 native_image=1','VB_GRAPHICAL_CORE_OK bits=32 styled=1')),
        (Case 'vb-owner-option-64' (Join-Path $x64 'core-vb-graphical.exe') @($run,(Join-Path $x64 'vb-graphical-fixture/MSVBVM60.dll'),(Join-Path $x64 'vb-queue-witness.dll')) @('VB6_GRAPHICAL_OPTION_OK ownerdraw=1 checked_frame=1 native_image=1','VB6_GRAPHICAL_CHECK_OK ownerdraw=1 checked_frame=1 native_image=1','VB_GRAPHICAL_CORE_OK bits=64 styled=1')),
        (Case 'vb-graphical-choice-32' $run @('--wait',(Join-Path $x86 'core-vb-graphical-choice.exe'),$run,(Join-Path $x86 'vb-fixture/MSVBVM60.dll')) @('VB_GRAPHICAL_PICTURE_OK borrowed=1 state=1 disabled=1 native_retire=1','VB_GRAPHICAL_ICON_OK borrowed=1 state=1 disabled=1 native_retire=1','VB_GRAPHICAL_IMAGE_PIXELS_OK bitmap=256 icon=256 checked=1','VB_GRAPHICAL_ARRAY_OK independent_state=1 sibling_retire=1 recreate=1')),
        (Case 'vb-graphical-choice-64' $run @('--wait',(Join-Path $x64 'core-vb-graphical-choice.exe'),$run,(Join-Path $x64 'vb-fixture/MSVBVM60.dll')) @('VB_GRAPHICAL_PICTURE_OK borrowed=1 state=1 disabled=1 native_retire=1','VB_GRAPHICAL_ICON_OK borrowed=1 state=1 disabled=1 native_retire=1','VB_GRAPHICAL_IMAGE_PIXELS_OK bitmap=256 icon=256 checked=1','VB_GRAPHICAL_ARRAY_OK independent_state=1 sibling_retire=1 recreate=1')),
        (Case 'vb-queue-32' (Join-Path $x86 'vb-queue-test.exe') @((Join-Path $x86 'vb-queue-witness.dll')) @('VB_QUEUE_UNIT_OK bits=32 wrong_tag_rejected=1 notification=1 no_hook_negative=1 resources_closed=1')),
        (Case 'vb-queue-64' (Join-Path $x64 'vb-queue-test.exe') @((Join-Path $x64 'vb-queue-witness.dll')) @('VB_QUEUE_UNIT_OK bits=64 wrong_tag_rejected=1 notification=1 no_hook_negative=1 resources_closed=1')),
        (Case 'vb-control-text-32' (Join-Path $x86 'vb-control-text-test.exe') @() @('VB_CONTROL_TEXT_OK bits=32 cross_process=1 unicode=1 invalid_window=1 cleanup=1')),
        (Case 'vb-control-text-64' (Join-Path $x64 'vb-control-text-test.exe') @() @('VB_CONTROL_TEXT_OK bits=64 cross_process=1 unicode=1 invalid_window=1 cleanup=1')),
        (Case 'vb-font-32' (Join-Path $x86 'core-vb-font.exe') @((Join-Path $x86 'vb-fixture/MSVBVM60.dll')) @('VB_FONT_ZERO_OK requested=12','VB_FONT_ZERO_OK requested=14','VB_FONT_ZERO_OK requested=16','VB_OWNERSHIP_OK leases_retired=32 maps_freed=32 sources_preserved=33','REPORT_TEXT_OWNERSHIP_OK exact_dc=1 barrier=1 cancelled=1')),
        (Case 'vb-font-64' (Join-Path $x64 'core-vb-font.exe') @((Join-Path $x64 'vb-fixture/MSVBVM60.dll')) @('VB_FONT_ZERO_OK requested=12','VB_FONT_ZERO_OK requested=14','VB_FONT_ZERO_OK requested=16','VB_OWNERSHIP_OK leases_retired=32 maps_freed=32 sources_preserved=33','REPORT_TEXT_OWNERSHIP_OK exact_dc=1 barrier=1 cancelled=1')),
        (Case 'vb-controls-32' (Join-Path $x86 'core-vb.exe') @($run,(Join-Path $x86 'vb-fixture/MSVBVM60.dll')) @('VB_WINDOWED_OK bits=32 styled=1','VB_EDIT_OK undo=1','VB_EDIT_FONT_PIXELS changed=1','VB_CONTAINER_PAINT_OK label_changed=1','VB_CLEANUP_OK')),
        (Case 'vb-controls-64' (Join-Path $x64 'core-vb.exe') @($run,(Join-Path $x64 'vb-fixture/MSVBVM60.dll')) @('VB_WINDOWED_OK bits=64 styled=1','VB_EDIT_OK undo=1','VB_EDIT_FONT_PIXELS changed=1','VB_CONTAINER_PAINT_OK label_changed=1','VB_CLEANUP_OK'))
    )
    $cases+=@(
        (Case 'control-extensions-32' (Join-Path $x86 'core-control-extensions.exe') @($d32) @('CONTROL_EXTENSION_NATIVE_OK','CONTROL_EXTENSION_IMAGELIST_OK','CONTROL_EXTENSION_OK bits=32 protocol_cases=60')),
        (Case 'control-extensions-64' (Join-Path $x64 'core-control-extensions.exe') @($d64) @('CONTROL_EXTENSION_NATIVE_OK','CONTROL_EXTENSION_IMAGELIST_OK','CONTROL_EXTENSION_OK bits=64 protocol_cases=60')),
        (Case 'control-protocol-owner-32' (Join-Path $x86 'core-control-protocol-owner.exe') @() @('PROTOCOL_OWNER_OK bits=32 allocations=0 bindings=0')),
        (Case 'control-protocol-owner-64' (Join-Path $x64 'core-control-protocol-owner.exe') @() @('PROTOCOL_OWNER_OK bits=64 allocations=0 bindings=0')),
        (Case 'toolbar-bounds-v5-32' (Join-Path $x86 'core-toolbar-text-bounds.exe') @() @('TOOLBAR_SHORT_OK','TOOLBAR_BOUNDS_OK')),
        (Case 'toolbar-bounds-v5-64' (Join-Path $x64 'core-toolbar-text-bounds.exe') @() @('TOOLBAR_SHORT_OK','TOOLBAR_BOUNDS_OK')),
        (Case 'toolbar-bounds-v6-32' (Join-Path $x86 'core-toolbar-text-bounds-v6.exe') @() @('TOOLBAR_SHORT_OK','TOOLBAR_BOUNDS_OK')),
        (Case 'toolbar-bounds-v6-64' (Join-Path $x64 'core-toolbar-text-bounds-v6.exe') @() @('TOOLBAR_SHORT_OK','TOOLBAR_BOUNDS_OK')),
        (Case 'tab-controls-32' (Join-Path $x86 'core-tab-controls.exe') @($d32) @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=32','styled=6 preserved=15')),
        (Case 'tab-controls-64' (Join-Path $x64 'core-tab-controls.exe') @($d64) @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=64','styled=6 preserved=15')),
        (Case 'tab-controls-dpi-32' (Join-Path $x86 'core-tab-controls.exe') @($d32,'aware') @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=32','aware=1 styled=6 preserved=15')),
        (Case 'tab-controls-dpi-64' (Join-Path $x64 'core-tab-controls.exe') @($d64,'aware') @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=64','aware=1 styled=6 preserved=15')),
        (Case 'tab-controls-v5-32' (Join-Path $x86 'core-tab-controls-v5.exe') @($d32) @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=32','styled=6 preserved=15')),
        (Case 'tab-controls-v5-64' (Join-Path $x64 'core-tab-controls-v5.exe') @($d64) @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=64','styled=6 preserved=15')),
        (Case 'tab-controls-v5-dpi-32' (Join-Path $x86 'core-tab-controls-v5.exe') @($d32,'aware') @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=32','aware=1 styled=6 preserved=15')),
        (Case 'tab-controls-v5-dpi-64' (Join-Path $x64 'core-tab-controls-v5.exe') @($d64,'aware') @('TAB_OS_ROOT_CLEANUP process_exit=1','TAB_CONTROL_OK bits=64','aware=1 styled=6 preserved=15')),
        (Case 'pending-control-owner-32' (Join-Path $x86 'core-pending-control-owner.exe') @($d32) @('PENDING_OWNER_OK bits=32','application_font_valid=1')),
        (Case 'pending-control-owner-64' (Join-Path $x64 'core-pending-control-owner.exe') @($d64) @('PENDING_OWNER_OK bits=64','application_font_valid=1')),
        (Case 'header-v5-32' (Join-Path $x86 'core-header-v5.exe') @($d32) @('HEADER_CONTROL_OK bits=32','callback_cases=5 roles_released=1')),
        (Case 'header-v5-64' (Join-Path $x64 'core-header-v5.exe') @($d64) @('HEADER_CONTROL_OK bits=64','callback_cases=5 roles_released=1')),
        (Case 'header-v5-dpi-32' (Join-Path $x86 'core-header-v5.exe') @($d32,'aware') @('HEADER_CONTROL_OK bits=32','aware=1','callback_cases=5 roles_released=1')),
        (Case 'header-v5-dpi-64' (Join-Path $x64 'core-header-v5.exe') @($d64,'aware') @('HEADER_CONTROL_OK bits=64','aware=1','callback_cases=5 roles_released=1')),
        (Case 'header-v6-32' (Join-Path $x86 'core-header-v6.exe') @($d32) @('HEADER_CONTROL_OK bits=32','callback_cases=5 roles_released=1')),
        (Case 'header-v6-64' (Join-Path $x64 'core-header-v6.exe') @($d64) @('HEADER_CONTROL_OK bits=64','callback_cases=5 roles_released=1')),
        (Case 'header-v6-dpi-32' (Join-Path $x86 'core-header-v6.exe') @($d32,'aware') @('HEADER_CONTROL_OK bits=32','aware=1','callback_cases=5 roles_released=1')),
        (Case 'header-v6-dpi-64' (Join-Path $x64 'core-header-v6.exe') @($d64,'aware') @('HEADER_CONTROL_OK bits=64','aware=1','callback_cases=5 roles_released=1')),
        (Case 'list-report-v5-32' (Join-Path $x86 'core-list-report-v5.exe') @($run) @('LIST_REPORT_OK bits=32 styled=1','LIST_REPORT_SYSTEM_MASK_OK','LIST_REPORT_CALLBACK_RETIRE_OK','LIST_REPORT_CLEANUP_OK')),
        (Case 'list-report-v5-64' (Join-Path $x64 'core-list-report-v5.exe') @($run) @('LIST_REPORT_OK bits=64 styled=1','LIST_REPORT_SYSTEM_MASK_OK','LIST_REPORT_CALLBACK_RETIRE_OK','LIST_REPORT_CLEANUP_OK')),
        (Case 'list-report-v6-32' (Join-Path $x86 'core-list-report-v6.exe') @($run) @('LIST_REPORT_OK bits=32 styled=1','LIST_REPORT_SYSTEM_MASK_OK','LIST_REPORT_CALLBACK_RETIRE_OK','LIST_REPORT_CLEANUP_OK')),
        (Case 'list-report-v6-64' (Join-Path $x64 'core-list-report-v6.exe') @($run) @('LIST_REPORT_OK bits=64 styled=1','LIST_REPORT_SYSTEM_MASK_OK','LIST_REPORT_CALLBACK_RETIRE_OK','LIST_REPORT_CLEANUP_OK')),
        (Case 'pending-frame-owner-32' (Join-Path $x86 'core-pending-frame-owner.exe') @($d32) @('PENDING_FRAME_OWNER_OK failures=0')),
        (Case 'tree-v5-32' (Join-Path $x86 'core-tree-v5.exe') @($run) @('TREE_CONTROL_OK bits=32 styled=1','TREE_CALLBACK_RETIRE_OK','TREE_INPUT_STATE_OK ctrl_preserved=1 neutral_moves=1 focus=1 control=0000 shift=0000 alt=0000 restored=1','TREE_CLEANUP_OK')),
        (Case 'tree-v6-32' (Join-Path $x86 'core-tree-v6.exe') @($run) @('TREE_CONTROL_OK bits=32 styled=1','TREE_CALLBACK_RETIRE_OK','TREE_INPUT_STATE_OK ctrl_preserved=1 neutral_moves=1 focus=1 control=0000 shift=0000 alt=0000 restored=1','TREE_CLEANUP_OK')),
        (Case 'pending-frame-owner-64' (Join-Path $x64 'core-pending-frame-owner.exe') @($d64) @('PENDING_FRAME_OWNER_OK failures=0')),
        (Case 'tree-v5-64' (Join-Path $x64 'core-tree-v5.exe') @($run) @('TREE_CONTROL_OK bits=64 styled=1','TREE_CALLBACK_RETIRE_OK','TREE_INPUT_STATE_OK ctrl_preserved=1 neutral_moves=1 focus=1 control=0000 shift=0000 alt=0000 restored=1','TREE_CLEANUP_OK')),
        (Case 'tree-v6-64' (Join-Path $x64 'core-tree-v6.exe') @($run) @('TREE_CONTROL_OK bits=64 styled=1','TREE_CALLBACK_RETIRE_OK','TREE_INPUT_STATE_OK ctrl_preserved=1 neutral_moves=1 focus=1 control=0000 shift=0000 alt=0000 restored=1','TREE_CLEANUP_OK')),
        (Case 'updown-v5-32' (Join-Path $x86 'core-updown-v5.exe') @($run) @('UPDOWN_CONTROL_OK bits=32 styled=1','UPDOWN_SHARED_THEME_OK','UPDOWN_PASSIVE_OK','UPDOWN_RETIRE_OK','UPDOWN_RESOURCE_OBSERVED','UPDOWN_CLEANUP_OK')),
        (Case 'updown-v6-32' (Join-Path $x86 'core-updown-v6.exe') @($run) @('UPDOWN_CONTROL_OK bits=32 styled=1','UPDOWN_SHARED_THEME_OK','UPDOWN_PASSIVE_OK','UPDOWN_RETIRE_OK','UPDOWN_RESOURCE_OBSERVED','UPDOWN_CLEANUP_OK')),
        (Case 'updown-owner-32' (Join-Path $x86 'core-updown-owner.exe') @() @('UPDOWN_OWNER_OK bits=32','owned_dc_handles_released=165','UPDOWN_DC_CAPACITY_OK')),
        (Case 'startup-hooks-32' (Join-Path $x86 'launch-startup-hooks.exe') @() @('STARTUP_CAPACITY_OK slots=56 overflow57=1 abort_restored=57','C STARTUP HOOK PASS width=32')),
        (Case 'updown-v5-64' (Join-Path $x64 'core-updown-v5.exe') @($run) @('UPDOWN_CONTROL_OK bits=64 styled=1','UPDOWN_SHARED_THEME_OK','UPDOWN_PASSIVE_OK','UPDOWN_RETIRE_OK','UPDOWN_RESOURCE_OBSERVED','UPDOWN_CLEANUP_OK')),
        (Case 'updown-v6-64' (Join-Path $x64 'core-updown-v6.exe') @($run) @('UPDOWN_CONTROL_OK bits=64 styled=1','UPDOWN_SHARED_THEME_OK','UPDOWN_PASSIVE_OK','UPDOWN_RETIRE_OK','UPDOWN_RESOURCE_OBSERVED','UPDOWN_CLEANUP_OK')),
        (Case 'updown-owner-64' (Join-Path $x64 'core-updown-owner.exe') @() @('UPDOWN_OWNER_OK bits=64','owned_dc_handles_released=165','UPDOWN_DC_CAPACITY_OK')),
        (Case 'startup-hooks-64' (Join-Path $x64 'launch-startup-hooks.exe') @() @('STARTUP_CAPACITY_OK slots=56 overflow57=1 abort_restored=57','C STARTUP HOOK PASS width=64')),
        (Case 'trackbar-v5-32' (Join-Path $x86 'core-trackbar-v5.exe') @($run) @('TRACKBAR_CONTROL_OK bits=32 styled=1','bootstrap_frame_attached=1','TRACKBAR_CLEANUP_OK','TRACKBAR_RETIRE_FALLBACK_OK','TRACKBAR_RESOURCE_OK')),
        (Case 'trackbar-v6-32' (Join-Path $x86 'core-trackbar-v6.exe') @($run) @('TRACKBAR_CONTROL_OK bits=32 styled=1','bootstrap_frame_attached=1','TRACKBAR_CLEANUP_OK','TRACKBAR_RETIRE_FALLBACK_OK','TRACKBAR_RESOURCE_OK')),
        (Case 'trackbar-v5-64' (Join-Path $x64 'core-trackbar-v5.exe') @($run) @('TRACKBAR_CONTROL_OK bits=64 styled=1','bootstrap_frame_attached=1','TRACKBAR_CLEANUP_OK','TRACKBAR_RETIRE_FALLBACK_OK','TRACKBAR_RESOURCE_OK')),
        (Case 'trackbar-v6-64' (Join-Path $x64 'core-trackbar-v6.exe') @($run) @('TRACKBAR_CONTROL_OK bits=64 styled=1','bootstrap_frame_attached=1','TRACKBAR_CLEANUP_OK','TRACKBAR_RETIRE_FALLBACK_OK','TRACKBAR_RESOURCE_OK')),
        (Case 'progress-v5-32' (Join-Path $x86 'core-progress-v5.exe') @($run) @('PROGRESS_CONTROL_OK bits=32 styled=1','bootstrap_frame_attached=1','PROGRESS_V5_BORDER_OK bits=32','PROGRESS_CLEANUP_OK')),
        (Case 'progress-color-v5-32' (Join-Path $x86 'core-progress-color-v5.exe') @() @('COLOR_WITNESS bits=32 version=5.','create_setters=2','direct_hook_observed=0','COLOR_WITNESS_CLEANUP_OK')),
        (Case 'progress-detour-v5-32' (Join-Path $x86 'core-progress-detour-v5.exe') @() @('NATIVE_DETOUR_INSTALL bits=32 error=0','NATIVE_DETOUR_DIRECT bits=32','captured=1','COLOR_WITNESS_CLEANUP_OK')),
        (Case 'progress-v6-32' (Join-Path $x86 'core-progress-v6.exe') @($run) @('PROGRESS_CONTROL_OK bits=32 styled=1','bootstrap_frame_attached=1','PROGRESS_CLEANUP_OK','PROGRESS_QUERY_RETIRE_OK','PROGRESS_BOUNDARY_OK')),
        (Case 'progress-v5-64' (Join-Path $x64 'core-progress-v5.exe') @($run) @('PROGRESS_CONTROL_OK bits=64 styled=1','bootstrap_frame_attached=1','PROGRESS_V5_BORDER_OK bits=64','PROGRESS_CLEANUP_OK')),
        (Case 'progress-color-v5-64' (Join-Path $x64 'core-progress-color-v5.exe') @() @('COLOR_WITNESS bits=64 version=5.','create_setters=2','direct_hook_observed=0','COLOR_WITNESS_CLEANUP_OK')),
        (Case 'progress-detour-v5-64' (Join-Path $x64 'core-progress-detour-v5.exe') @() @('NATIVE_DETOUR_INSTALL bits=64 error=0','NATIVE_DETOUR_DIRECT bits=64','captured=1','COLOR_WITNESS_CLEANUP_OK')),
        (Case 'progress-v6-64' (Join-Path $x64 'core-progress-v6.exe') @($run) @('PROGRESS_CONTROL_OK bits=64 styled=1','bootstrap_frame_attached=1','PROGRESS_CLEANUP_OK','PROGRESS_QUERY_RETIRE_OK','PROGRESS_BOUNDARY_OK')),
        (Case 'header-owner-32' (Join-Path $x86 'core-header-owner.exe') @() @('HEADER_OWNER_OK bits=32 cases=16 allocations=0 dc_restored=1')),
        (Case 'header-owner-64' (Join-Path $x64 'core-header-owner.exe') @() @('HEADER_OWNER_OK bits=64 cases=16 allocations=0 dc_restored=1')),
        (Case 'control-visibility-32' (Join-Path $x86 'core-control-visibility.exe') @($d32) @('CONTROL_VISIBILITY_OK bits=32 cases=16')),
        (Case 'control-visibility-64' (Join-Path $x64 'core-control-visibility.exe') @($d64) @('CONTROL_VISIBILITY_OK bits=64 cases=16')),
        (Case 'dialog-scope-32' (Join-Path $x86 'core-dialog-scope.exe') @() @('DIALOG_SCOPE_OWNERSHIP_OK bits=32','allocations=0')),
        (Case 'dialog-scope-64' (Join-Path $x64 'core-dialog-scope.exe') @() @('DIALOG_SCOPE_OWNERSHIP_OK bits=64','allocations=0')),
        (Case 'dialog-show-32' (Join-Path $x86 'core-dialog-show.exe') @($run) @('DIALOG_SHOW_OK bits=32 styled=1','DIALOG_SHOW_CLEANUP_OK') 180),
        (Case 'dialog-show-64' (Join-Path $x64 'core-dialog-show.exe') @($run) @('DIALOG_SHOW_OK bits=64 styled=1','DIALOG_SHOW_CLEANUP_OK') 180),
        (Case 'theme-stage-32' (Join-Path $x86 'core-theme-stage.exe') @() @('THEME_STAGE_OWNERSHIP_OK bits=32','allocations=0 early_proc_unchanged=1')),
        (Case 'theme-stage-64' (Join-Path $x64 'core-theme-stage.exe') @() @('THEME_STAGE_OWNERSHIP_OK bits=64','allocations=0 early_proc_unchanged=1')),
        (Case 'superclass-32' (Join-Path $x86 'core-superclass.exe') @($run) @('SUPER_PROTOCOL_OK bits=32 styled=1','SUPER_VCL_CHOICE_OK native_superclass=3 state_click=1 styled=1 detach_pixels=1','SUPER_VCL_EDIT_OK native_superclass=2 multiline_undo_readonly_scroll=1 styled=1 detach_pixels=1','SUPER_VCL_LIST_OK item_data_selection=1 multiselect=1 ownerdraw_rejected=1 detach_pixels=1 styled=1','SUPER_DBLIST_OK selection=1 data=1 ownerdraw_native=1 detach_pixels=1 styled=1','SUPER_RENDER_OK','SUPER_ACTUAL_FILL_OK','SUPER_REJECT_OK','SUPER_OS_CLEANUP','SUPER_CLEANUP_OK')),
        (Case 'superclass-64' (Join-Path $x64 'core-superclass.exe') @($run) @('SUPER_PROTOCOL_OK bits=64 styled=1','SUPER_VCL_CHOICE_OK native_superclass=3 state_click=1 styled=1 detach_pixels=1','SUPER_VCL_EDIT_OK native_superclass=2 multiline_undo_readonly_scroll=1 styled=1 detach_pixels=1','SUPER_VCL_LIST_OK item_data_selection=1 multiselect=1 ownerdraw_rejected=1 detach_pixels=1 styled=1','SUPER_DBLIST_OK selection=1 data=1 ownerdraw_native=1 detach_pixels=1 styled=1','SUPER_RENDER_OK','SUPER_ACTUAL_FILL_OK','SUPER_REJECT_OK','SUPER_OS_CLEANUP','SUPER_CLEANUP_OK')),
        (Case 'checklist-32' (Join-Path $x86 'core-checklist.exe') @($run) @('CHECKLIST_MODEL_OK bits=32 styled=0','CHECKLIST_MODEL_OK bits=32 styled=1 checked_mismatch=0','CHECKLIST_PUSHED_OK bits=32','CHECKLIST_CLEANUP_OK')),
        (Case 'checklist-64' (Join-Path $x64 'core-checklist.exe') @($run) @('CHECKLIST_MODEL_OK bits=64 styled=0','CHECKLIST_MODEL_OK bits=64 styled=1 checked_mismatch=0','CHECKLIST_PUSHED_OK bits=64','CHECKLIST_CLEANUP_OK'))
    )
    $cases+=@(
        (Case 'vcl-static-32' $run @('--wait',(Join-Path $x86 'core-vcl-static.exe')) @('VCL_STATIC_OK bits=32 font_preserved=1 fake_rejected=1 nested_native=1')),
        (Case 'vcl-static-64' $run @('--wait',(Join-Path $x64 'core-vcl-static.exe')) @('VCL_STATIC_OK bits=64 font_preserved=1 fake_rejected=1 nested_native=1')),
        (Case 'vcl-static-bare-32' (Join-Path $x86 'core-vcl-static.exe') @('bare') @('VCL_STATIC_BARE_OK bits=32 font_preserved=1')),
        (Case 'vcl-static-bare-64' (Join-Path $x64 'core-vcl-static.exe') @('bare') @('VCL_STATIC_BARE_OK bits=64 font_preserved=1')),
        (Case 'vcl-mask-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-mask-module.exe'),(Join-Path $x86 'vcl-mask-module-fixture.dll')) @('VCL_MASK_MODEL_OK bits=32 fake=1 selection=1 readonly=1 frame=1 removal=1 ansi_dbedit=1 foreign=1 cleanup=1')),
        (Case 'vcl-mask-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-mask-module.exe'),(Join-Path $x64 'vcl-mask-module-fixture.dll')) @('VCL_MASK_MODEL_OK bits=64 fake=1 selection=1 readonly=1 frame=1 removal=1 ansi_dbedit=1 foreign=1 cleanup=1')),
        (Case 'vcl-memo-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-edit-module.exe'),(Join-Path $x86 'vcl-memo-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=32 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 frame_pixels=1 nc_clip=1 noedge=1 cleanup=1')),
        (Case 'vcl-memo-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-edit-module.exe'),(Join-Path $x64 'vcl-memo-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=64 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 frame_pixels=1 nc_clip=1 noedge=1 cleanup=1')),
        (Case 'vcl-linklabel-32' $run @('--wait',(Join-Path $x86 'core-vcl-linklabel.exe'),(Join-Path $x86 'vcl-linklabel-fixture.dll')) @('VCL_LINK_OK bits=32 fake=1 foreign=1 font=1 visited=1 mouse=1 keyboard=1 disabled=1 restore=1 reentrant=1 delayed=1 fresh_witness=1 cleanup=1')),
        (Case 'vcl-linklabel-64' $run @('--wait',(Join-Path $x64 'core-vcl-linklabel.exe'),(Join-Path $x64 'vcl-linklabel-fixture.dll')) @('VCL_LINK_OK bits=64 fake=1 foreign=1 font=1 visited=1 mouse=1 keyboard=1 disabled=1 restore=1 reentrant=1 delayed=1 fresh_witness=1 cleanup=1')),
        (Case 'vcl-spinedit-32' $run @('--wait',(Join-Path $x86 'core-vcl-spinedit.exe'),(Join-Path $x86 'vcl-spinedit-fixture.dll')) @('SPINEDIT_LAYOUT_OK reserved=1 font_change=1','VCL_LABELED_EDIT_OK bits=32 fake_paint_rejected=1 foreign_caller_rejected=1 clip=1 selection=1 readonly=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-spinedit-64' $run @('--wait',(Join-Path $x64 'core-vcl-spinedit.exe'),(Join-Path $x64 'vcl-spinedit-fixture.dll')) @('SPINEDIT_LAYOUT_OK reserved=1 font_change=1','VCL_LABELED_EDIT_OK bits=64 fake_paint_rejected=1 foreign_caller_rejected=1 clip=1 selection=1 readonly=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-modern-spin-32' $run @('--wait',(Join-Path $x86 'core-vcl-modern-spin.exe'),(Join-Path $x86 'vcl-modern-spin-fixture.dll')) @('SPIN_UNWIND caught=1 later_paint=1','SPIN_COLOR_GLYPH pixels=9 unchanged=9','SPIN_OTHER_THEME sampled=00fdfdfd expected=00fdfdfd','SPIN_FAILED_THEME sampled=00fdfdfd expected=00fdfdfd','SPIN_SOURCE_CLIP sampled=00fdfdfd expected=00fdfdfd','SPIN_SOURCE_HOLE sampled=00fdfdfd expected=00fdfdfd','SPIN_MODEL_PASS nested=1 retained=1 glyph=1 detach=1 cleanup=1')),
        (Case 'vcl-modern-spin-64' $run @('--wait',(Join-Path $x64 'core-vcl-modern-spin.exe'),(Join-Path $x64 'vcl-modern-spin-fixture.dll')) @('SPIN_UNWIND caught=1 later_paint=1','SPIN_COLOR_GLYPH pixels=9 unchanged=9','SPIN_OTHER_THEME sampled=00fdfdfd expected=00fdfdfd','SPIN_FAILED_THEME sampled=00fdfdfd expected=00fdfdfd','SPIN_SOURCE_CLIP sampled=00fdfdfd expected=00fdfdfd','SPIN_SOURCE_HOLE sampled=00fdfdfd expected=00fdfdfd','SPIN_MODEL_PASS nested=1 retained=1 glyph=1 detach=1 cleanup=1')),
        (Case 'vcl-numberbox-32' $run @('--wait',(Join-Path $x86 'core-vcl-labeled-edit.exe'),(Join-Path $x86 'vcl-numberbox-fixture.dll')) @('VCL_LABELED_EDIT_OK bits=32 fake_paint_rejected=1 foreign_caller_rejected=1 clip=1 selection=1 readonly=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-numberbox-64' $run @('--wait',(Join-Path $x64 'core-vcl-labeled-edit.exe'),(Join-Path $x64 'vcl-numberbox-fixture.dll')) @('VCL_LABELED_EDIT_OK bits=64 fake_paint_rejected=1 foreign_caller_rejected=1 clip=1 selection=1 readonly=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-labeled-edit-32' $run @('--wait',(Join-Path $x86 'core-vcl-labeled-edit.exe'),(Join-Path $x86 'vcl-labeled-edit-fixture.dll')) @('VCL_LABELED_EDIT_OK bits=32 fake_paint_rejected=1 foreign_caller_rejected=1 clip=1 selection=1 readonly=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-labeled-edit-64' $run @('--wait',(Join-Path $x64 'core-vcl-labeled-edit.exe'),(Join-Path $x64 'vcl-labeled-edit-fixture.dll')) @('VCL_LABELED_EDIT_OK bits=64 fake_paint_rejected=1 foreign_caller_rejected=1 clip=1 selection=1 readonly=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-edit-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-edit-module.exe'),(Join-Path $x86 'vcl-edit-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=32 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 frame_pixels=1 nc_clip=1 noedge=1 cleanup=1')),
        (Case 'vcl-edit-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-edit-module.exe'),(Join-Path $x64 'vcl-edit-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=64 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 frame_pixels=1 nc_clip=1 noedge=1 cleanup=1')),
        (Case 'vcl-trackbar-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-trackbar-module.exe'),(Join-Path $x86 'vcl-trackbar-module-fixture.dll')) @('VCL_TRACK_MODULE_OK bits=32 fake=1 selection=1 nested=1 callbacks=1 apppost=1 appskip=1 pixels=1 clip=1 disabled=1 reentrant_remove=1 destroy=1 gdi_sample=1 cleanup=1')),
        (Case 'vcl-trackbar-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-trackbar-module.exe'),(Join-Path $x64 'vcl-trackbar-module-fixture.dll')) @('VCL_TRACK_MODULE_OK bits=64 fake=1 selection=1 nested=1 callbacks=1 apppost=1 appskip=1 pixels=1 clip=1 disabled=1 reentrant_remove=1 destroy=1 gdi_sample=1 cleanup=1')),
        (Case 'vcl-drive-combo-32' $run @('--wait',(Join-Path $x86 'core-vcl-color-module.exe'),(Join-Path $x86 'vcl-drive-combo-fixture.dll')) @('COLOR_OS_ROOT_CLEANUP process_exit=1','VCL_COLOR_MODULE_OK bits=32 fake=1 selection=1 keyboard=1 popup=1 ownerdraw=1 nested=1 pixels=1 clip=1 dc_state=1 print_resources=1 unwind=1 retire=1 destroy=1 cleanup=1')),
        (Case 'vcl-drive-combo-64' $run @('--wait',(Join-Path $x64 'core-vcl-color-module.exe'),(Join-Path $x64 'vcl-drive-combo-fixture.dll')) @('COLOR_OS_ROOT_CLEANUP process_exit=1','VCL_COLOR_MODULE_OK bits=64 fake=1 selection=1 keyboard=1 popup=1 ownerdraw=1 nested=1 pixels=1 clip=1 dc_state=1 print_resources=1 unwind=1 retire=1 destroy=1 cleanup=1')),
        (Case 'vcl-color-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-color-module.exe'),(Join-Path $x86 'vcl-color-module-fixture.dll')) @('COLOR_OS_ROOT_CLEANUP process_exit=1','VCL_COLOR_MODULE_OK bits=32 fake=1 selection=1 keyboard=1 popup=1 ownerdraw=1 nested=1 pixels=1 clip=1 dc_state=1 print_resources=1 unwind=1 retire=1 destroy=1 cleanup=1')),
        (Case 'vcl-color-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-color-module.exe'),(Join-Path $x64 'vcl-color-module-fixture.dll')) @('COLOR_OS_ROOT_CLEANUP process_exit=1','VCL_COLOR_MODULE_OK bits=64 fake=1 selection=1 keyboard=1 popup=1 ownerdraw=1 nested=1 pixels=1 clip=1 dc_state=1 print_resources=1 unwind=1 retire=1 destroy=1 cleanup=1')),
        (Case 'vcl-status-pane-font-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-pane-font.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('PANE_FONT_OK ascii=1 unicode=1 ownerdraw=1 noborder=1 icon=1 mutation=1 unwind=1 cleanup=1')),
        (Case 'vcl-status-pane-font-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-pane-font.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('PANE_FONT_OK ascii=1 unicode=1 ownerdraw=1 noborder=1 icon=1 mutation=1 unwind=1 cleanup=1')),
        (Case 'month-model-32' $run @('--wait',(Join-Path $x86 'core-month-model.exe'),(Join-Path $x86 'month-fixture.dll')) @('MONTH_MODEL_OK multi=1 owner=1 fake=1 caller=1 witness=1 unwind=1 stale=1 query=1 date=1 layout=1 colors=1 theme=1 selection=1 header=1 font=1 fontbounds=1 fontlease=1 dpi=1 selectfail=1 text=1 fallback=1 pixels=1 cleanup=1')),
        (Case 'month-model-64' $run @('--wait',(Join-Path $x64 'core-month-model.exe'),(Join-Path $x64 'month-fixture.dll')) @('MONTH_MODEL_OK multi=1 owner=1 fake=1 caller=1 witness=1 unwind=1 stale=1 query=1 date=1 layout=1 colors=1 theme=1 selection=1 header=1 font=1 fontbounds=1 fontlease=1 dpi=1 selectfail=1 text=1 fallback=1 pixels=1 cleanup=1')),
        (Case 'inplace-model-32' $run @('--wait',(Join-Path $x86 'core-inplace-model.exe'),(Join-Path $x86 'inplace-fixture.dll')) @('INPLACE_MODEL_OK owner=1 fake=1 unwind=1 normal=1 pressed=1 hotfallback=1 geometry=1 foreign=1 clip=1 disabled=1 pixel=1 glyph=1 dropdown=1 dropclip=1 dropforeign=1 drophot=1 dropdisabled=1 droppressedfallback=1 click=1 reparent=1 cleanup=1','LOOKUP_MODEL_OK owner=1 edge=1 fake=1 theme=1 ansi=1 unicode=1 glyph=1 reparent=1 cleanup=1')),
        (Case 'inplace-model-64' $run @('--wait',(Join-Path $x64 'core-inplace-model.exe'),(Join-Path $x64 'inplace-fixture.dll')) @('INPLACE_MODEL_OK owner=1 fake=1 unwind=1 normal=1 pressed=1 hotfallback=1 geometry=1 foreign=1 clip=1 disabled=1 pixel=1 glyph=1 dropdown=1 dropclip=1 dropforeign=1 drophot=1 dropdisabled=1 droppressedfallback=1 click=1 reparent=1 cleanup=1','LOOKUP_MODEL_OK owner=1 edge=1 fake=1 theme=1 ansi=1 unicode=1 glyph=1 reparent=1 cleanup=1')),
        (Case 'dbnav-model-32' $run @('--wait',(Join-Path $x86 'core-dbnav-model.exe'),(Join-Path $x86 'dbnav-fixture.dll')) @('DBNAV_MODEL_OK width=32 wrong_parent=1 foreign=1 origin=1 normal=1 pressed=1 disabled=1 hot_fallback=1 glyph=1 clip=1 click=1 reparent=1 media=1 media_foreign=1 media_module=1 media_fallback=1 media_clip=1 media_disabled=1 media_focus=1 focus_pixels=1 cleanup=1')),
        (Case 'dbnav-model-64' $run @('--wait',(Join-Path $x64 'core-dbnav-model.exe'),(Join-Path $x64 'dbnav-fixture.dll')) @('DBNAV_MODEL_OK width=64 wrong_parent=1 foreign=1 origin=1 normal=1 pressed=1 disabled=1 hot_fallback=1 glyph=1 clip=1 click=1 reparent=1 media=1 media_foreign=1 media_module=1 media_fallback=1 media_clip=1 media_disabled=1 media_focus=1 focus_pixels=1 cleanup=1')),
        (Case 'lookup-popup-model-32' $run @('--wait',(Join-Path $x86 'core-lookup-popup-model.exe')) @('LOOKUP_POP_MODEL_OK width=32 scoped=1 alt_down=1 wrong_key=1 wrong_owner=1 outside=1 owner_change=1 source_destroy=1 exception=1 cleanup=1')),
        (Case 'lookup-popup-model-64' $run @('--wait',(Join-Path $x64 'core-lookup-popup-model.exe')) @('LOOKUP_POP_MODEL_OK width=64 scoped=1 alt_down=1 wrong_key=1 wrong_owner=1 outside=1 owner_change=1 source_destroy=1 exception=1 cleanup=1')),
        (Case 'vcl-directory-model-32' $run @('--wait',(Join-Path $x86 'core-vcl-directory-model.exe')) @('DIR_OK')),
        (Case 'vcl-directory-model-64' $run @('--wait',(Join-Path $x64 'core-vcl-directory-model.exe')) @('DIR_OK')),
        (Case 'vcl-button-group-model-32' $run @('--wait',(Join-Path $x86 'core-vcl-button-group-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'vcl-button-group-model-64' $run @('--wait',(Join-Path $x64 'core-vcl-button-group-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'vcl-category-buttons-model-32' $run @('--wait',(Join-Path $x86 'core-vcl-category-buttons-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'vcl-category-buttons-model-64' $run @('--wait',(Join-Path $x64 'core-vcl-category-buttons-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'vcl-category-external-32' $run @('--wait',(Join-Path $x86 'core-vcl-category-external.exe'),(Join-Path $x86 'vcl-category-external-fixture.dll')) @('CATEGORY_EXTERNAL_OK positive=1 owner=1 noedge=1 reparent=1 client=1 root_os_cleanup=1')),
        (Case 'vcl-category-external-64' $run @('--wait',(Join-Path $x64 'core-vcl-category-external.exe'),(Join-Path $x64 'vcl-category-external-fixture.dll')) @('CATEGORY_EXTERNAL_OK positive=1 owner=1 noedge=1 reparent=1 client=1 root_os_cleanup=1')),
        (Case 'vcl-outline-model-32' $run @('--wait',(Join-Path $x86 'core-vcl-outline-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'vcl-outline-model-64' $run @('--wait',(Join-Path $x64 'core-vcl-outline-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'popup-list-model-32' $run @('--wait',(Join-Path $x86 'core-popup-list-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'popup-list-model-64' $run @('--wait',(Join-Path $x64 'core-popup-list-model.exe')) @('POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1')),
        (Case 'controlbar-model-32' $run @('--wait',(Join-Path $x86 'core-controlbar-model.exe'),(Join-Path $x86 'controlbar-fixture.dll')) @('CONTROLBAR_MODEL_OK owner=1 edge=1 geometry=1 caller=1 fake=1 dc=1 clip=1 transition=1 adjusted=1 variants=1 plain=1 dynamic=1 pixels=1 button=1 reparent=1 cleanup=1')),
        (Case 'controlbar-model-64' $run @('--wait',(Join-Path $x64 'core-controlbar-model.exe'),(Join-Path $x64 'controlbar-fixture.dll')) @('CONTROLBAR_MODEL_OK owner=1 edge=1 geometry=1 caller=1 fake=1 dc=1 clip=1 transition=1 adjusted=1 variants=1 plain=1 dynamic=1 pixels=1 button=1 reparent=1 cleanup=1')),
        (Case 'hotkey-model-32' $run @('--wait',(Join-Path $x86 'core-hotkey-model.exe'),(Join-Path $x86 'hotkey-fixture.dll')) @('HOTKEY_MODEL_OK owner=1 fake=1 caller=1 witness=1 unwind=1 stale=1 role=1 key=1 input=1 pixels=1 cleanup=1')),
        (Case 'hotkey-model-64' $run @('--wait',(Join-Path $x64 'core-hotkey-model.exe'),(Join-Path $x64 'hotkey-fixture.dll')) @('HOTKEY_MODEL_OK owner=1 fake=1 caller=1 witness=1 unwind=1 stale=1 role=1 key=1 input=1 pixels=1 cleanup=1')),
        (Case 'box-model-32' $run @('--wait',(Join-Path $x86 'core-box-model.exe'),(Join-Path $x86 'box-fixture.dll')) @('BOX_MODEL_OK owner=1 fake=1 caller=1 witness=1 unwind=1 stale=1 reparent=1 role=1 pixels=1 cleanup=1')),
        (Case 'box-model-64' $run @('--wait',(Join-Path $x64 'core-box-model.exe'),(Join-Path $x64 'box-fixture.dll')) @('BOX_MODEL_OK owner=1 fake=1 caller=1 witness=1 unwind=1 stale=1 reparent=1 role=1 pixels=1 cleanup=1')),
        (Case 'rich-wide-32' $run @('--wait',(Join-Path $x86 'core-rich-model.exe'),(Join-Path $x86 'rich-wide-fixture.dll')) @('RICH_MODEL_OK wrong_owner=1 wrong_caller=1 fake=1 stale_witness=1 witness=1 selection=1 format=1 scroll=1 link=1 long=1 unwind=1 reparent=1 role=1 pixels=1 cleanup=1')),
        (Case 'rich-wide-64' $run @('--wait',(Join-Path $x64 'core-rich-model.exe'),(Join-Path $x64 'rich-wide-fixture.dll')) @('RICH_MODEL_OK wrong_owner=1 wrong_caller=1 fake=1 stale_witness=1 witness=1 selection=1 format=1 scroll=1 link=1 long=1 unwind=1 reparent=1 role=1 pixels=1 cleanup=1')),
        (Case 'rich-model-32' $run @('--wait',(Join-Path $x86 'core-rich-model.exe'),(Join-Path $x86 'rich-fixture.dll')) @('RICH_MODEL_OK wrong_owner=1 wrong_caller=1 fake=1 stale_witness=1 witness=1 selection=1 format=1 scroll=1 link=1 long=1 unwind=1 reparent=1 role=1 pixels=1 cleanup=1')),
        (Case 'rich-model-64' $run @('--wait',(Join-Path $x64 'core-rich-model.exe'),(Join-Path $x64 'rich-fixture.dll')) @('RICH_MODEL_OK wrong_owner=1 wrong_caller=1 fake=1 stale_witness=1 witness=1 selection=1 format=1 scroll=1 link=1 long=1 unwind=1 reparent=1 role=1 pixels=1 cleanup=1')),
        (Case 'vcl-status-pane-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-pane.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('PANE_CLIP changed=','PANE mode=0 changed=','PANE mode=1 changed=','PANE mode=2 changed=','PANE mode=3 changed=0','VCL_STATUS_PANE_OK')),
        (Case 'vcl-status-pane-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-pane.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('PANE_CLIP changed=','PANE mode=0 changed=','PANE mode=1 changed=','PANE mode=2 changed=','PANE mode=3 changed=0','VCL_STATUS_PANE_OK')),
        (Case 'vcl-status-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-module.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'vcl-status-module-dpi-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-module.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll'),'aware') @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'vcl-status-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-module.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'vcl-status-module-dpi-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-module.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll'),'aware') @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'owner-draw-transition-32' $run @('--wait',(Join-Path $x86 'core-owner-draw-transition.exe')) @('OWNERDRAW_TRANSITION_PASS')),
        (Case 'owner-draw-transition-64' $run @('--wait',(Join-Path $x64 'core-owner-draw-transition.exe')) @('OWNERDRAW_TRANSITION_PASS')),
        (Case 'vcl-report-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-report-module.exe'),(Join-Path $x86 'vcl-report-fixture.dll')) @('VCL_REPORT_MODULE_OK bits=32 fake=1 ownerdraw=1 ownerdata=1 reentrant=1 views=3 reparent=1 cleanup=1')),
        (Case 'vcl-report-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-report-module.exe'),(Join-Path $x64 'vcl-report-fixture.dll')) @('VCL_REPORT_MODULE_OK bits=64 fake=1 ownerdraw=1 ownerdata=1 reentrant=1 views=3 reparent=1 cleanup=1')),
        (Case 'vcl-page-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-tab-module.exe'),(Join-Path $x86 'vcl-page-fixture.dll')) @('TAB_STABLE=1','TAB_PIXELS_DIFFER=1','VCL_TAB_MODULE_OK bits=32 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-page-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-tab-module.exe'),(Join-Path $x64 'vcl-page-fixture.dll')) @('TAB_STABLE=1','TAB_PIXELS_DIFFER=1','VCL_TAB_MODULE_OK bits=64 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-notebook-32' $run @('--wait',(Join-Path $x86 'core-vcl-tab-module.exe'),(Join-Path $x86 'vcl-notebook-fixture.dll')) @('TAB_STABLE=1','TAB_PIXELS_DIFFER=1','VCL_TAB_MODULE_OK bits=32 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-notebook-64' $run @('--wait',(Join-Path $x64 'core-vcl-tab-module.exe'),(Join-Path $x64 'vcl-notebook-fixture.dll')) @('TAB_STABLE=1','TAB_PIXELS_DIFFER=1','VCL_TAB_MODULE_OK bits=64 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-tab-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-tab-module.exe'),(Join-Path $x86 'vcl-tab-module-fixture.dll')) @('TAB_STABLE=1','TAB_PIXELS_DIFFER=1','VCL_TAB_MODULE_OK bits=32 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-tab-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-tab-module.exe'),(Join-Path $x64 'vcl-tab-module-fixture.dll')) @('TAB_STABLE=1','TAB_PIXELS_DIFFER=1','VCL_TAB_MODULE_OK bits=64 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-toolbar-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-toolbar-module.exe'),(Join-Path $x86 'vcl-toolbar-module-fixture.dll')) @('VCL_TOOLBAR_MODULE_OK bits=32 fake=1 zero=1 duplicate=1 image=1 click=1 callback=1 app_skip=1 reparent=1 clipped=1 cap64=1 cap65=1 removal=1 cleanup=1')),
        (Case 'vcl-toolbar-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-toolbar-module.exe'),(Join-Path $x64 'vcl-toolbar-module-fixture.dll')) @('VCL_TOOLBAR_MODULE_OK bits=64 fake=1 zero=1 duplicate=1 image=1 click=1 callback=1 app_skip=1 reparent=1 clipped=1 cap64=1 cap65=1 removal=1 cleanup=1')),
        (Case 'vcl-grid-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-grid-module.exe'),(Join-Path $x86 'vcl-grid-module-fixture.dll')) @('VCL_GRID_MODULE_OK bits=32 invalid=1 both=1 scroll=1 vertical=1 client=1 removal=1 reparent=1 small=1 transition=1 pixels=1 cleanup=1')),
        (Case 'vcl-grid-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-grid-module.exe'),(Join-Path $x64 'vcl-grid-module-fixture.dll')) @('VCL_GRID_MODULE_OK bits=64 invalid=1 both=1 scroll=1 vertical=1 client=1 removal=1 reparent=1 small=1 transition=1 pixels=1 cleanup=1')),
        (Case 'vcl-panel-aware-32' $run @('--wait',(Join-Path $x86 'core-vcl-panel-frame.exe'),(Join-Path $x86 'vcl-panel-frame-fixture.dll'),'--aware') @('PANEL_DPI','OUTSIDE_AND_NONE_PASS','CUSTOM_AND_INCOMPLETE_PASS','WRONG_CALLER_PASS','INTERIOR_CLIP_FALLBACK_PASS','PAIR_AND_CLIP_PASS','REPARENT_RETIRE_PASS','MIDPAIR_REPARENT_PASS','REENTRANT_PAINT_PASS','DISABLED_TRANSITION_PASS','DISABLED_MIDPAIR_REJECT_PASS','WRONG_OWNER_PASS','PANEL_MODEL_PASS') 15),
        (Case 'vcl-panel-aware-64' $run @('--wait',(Join-Path $x64 'core-vcl-panel-frame.exe'),(Join-Path $x64 'vcl-panel-frame-fixture.dll'),'--aware') @('PANEL_DPI','OUTSIDE_AND_NONE_PASS','CUSTOM_AND_INCOMPLETE_PASS','WRONG_CALLER_PASS','INTERIOR_CLIP_FALLBACK_PASS','PAIR_AND_CLIP_PASS','REPARENT_RETIRE_PASS','MIDPAIR_REPARENT_PASS','REENTRANT_PAINT_PASS','DISABLED_TRANSITION_PASS','DISABLED_MIDPAIR_REJECT_PASS','WRONG_OWNER_PASS','PANEL_MODEL_PASS') 15),
        (Case 'vcl-scroll-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-scroll-module.exe'),(Join-Path $x86 'vcl-scroll-module-fixture.dll')) @('SCROLL_DPI','SCROLL_CLIP_PASS','SCROLL_FIXTURE_PASS fake=1 wrong_caller=1 horizontal=1 vertical=1 notifications=1 clip=1 mouse=1 disabled_mouse=1 disabled_key=1 enabled=1 reparent=1 reentrant_remove=1 cleanup=1') 15),
        (Case 'vcl-scroll-module-aware-32' $run @('--wait',(Join-Path $x86 'core-vcl-scroll-module.exe'),(Join-Path $x86 'vcl-scroll-module-fixture.dll'),'--aware') @('SCROLL_DPI','SCROLL_CLIP_PASS','SCROLL_FIXTURE_PASS fake=1 wrong_caller=1 horizontal=1 vertical=1 notifications=1 clip=1 mouse=1 disabled_mouse=1 disabled_key=1 enabled=1 reparent=1 reentrant_remove=1 cleanup=1') 15),
        (Case 'vcl-scroll-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-scroll-module.exe'),(Join-Path $x64 'vcl-scroll-module-fixture.dll')) @('SCROLL_DPI','SCROLL_CLIP_PASS','SCROLL_FIXTURE_PASS fake=1 wrong_caller=1 horizontal=1 vertical=1 notifications=1 clip=1 mouse=1 disabled_mouse=1 disabled_key=1 enabled=1 reparent=1 reentrant_remove=1 cleanup=1') 15),
        (Case 'vcl-scroll-module-aware-64' $run @('--wait',(Join-Path $x64 'core-vcl-scroll-module.exe'),(Join-Path $x64 'vcl-scroll-module-fixture.dll'),'--aware') @('SCROLL_DPI','SCROLL_CLIP_PASS','SCROLL_FIXTURE_PASS fake=1 wrong_caller=1 horizontal=1 vertical=1 notifications=1 clip=1 mouse=1 disabled_mouse=1 disabled_key=1 enabled=1 reparent=1 reentrant_remove=1 cleanup=1') 15),
        (Case 'vcl-panel-frame-32' $run @('--wait',(Join-Path $x86 'core-vcl-panel-frame.exe'),(Join-Path $x86 'vcl-panel-frame-fixture.dll')) @('OUTSIDE_AND_NONE_PASS','CUSTOM_AND_INCOMPLETE_PASS','WRONG_CALLER_PASS','INTERIOR_CLIP_FALLBACK_PASS','PAIR_AND_CLIP_PASS','REPARENT_RETIRE_PASS','MIDPAIR_REPARENT_PASS','REENTRANT_PAINT_PASS','DISABLED_TRANSITION_PASS','DISABLED_MIDPAIR_REJECT_PASS','WRONG_OWNER_PASS','PANEL_MODEL_PASS') 15),
        (Case 'vcl-panel-frame-64' $run @('--wait',(Join-Path $x64 'core-vcl-panel-frame.exe'),(Join-Path $x64 'vcl-panel-frame-fixture.dll')) @('OUTSIDE_AND_NONE_PASS','CUSTOM_AND_INCOMPLETE_PASS','WRONG_CALLER_PASS','INTERIOR_CLIP_FALLBACK_PASS','PAIR_AND_CLIP_PASS','REPARENT_RETIRE_PASS','MIDPAIR_REPARENT_PASS','REENTRANT_PAINT_PASS','DISABLED_TRANSITION_PASS','DISABLED_MIDPAIR_REJECT_PASS','WRONG_OWNER_PASS','PANEL_MODEL_PASS') 15),
        (Case 'vcl-popup-aware-32' $run @('--wait',(Join-Path $x86 'core-vcl-popup-scope.exe'),(Join-Path $x86 'vcl-popup-wrong-owner.dll'),'--aware') @('POPUP_DPI','VCL_POPUP_SCOPE_OK callbacks=1 ownerdraw=1 nested=1 destroy=1 wrong_owner=1 restored=1') 15),
        (Case 'vcl-popup-aware-64' $run @('--wait',(Join-Path $x64 'core-vcl-popup-scope.exe'),(Join-Path $x64 'vcl-popup-wrong-owner.dll'),'--aware') @('POPUP_DPI','VCL_POPUP_SCOPE_OK callbacks=1 ownerdraw=1 nested=1 destroy=1 wrong_owner=1 restored=1') 15),
        (Case 'vcl-popup-scope-32' $run @('--wait',(Join-Path $x86 'core-vcl-popup-scope.exe'),(Join-Path $x86 'vcl-popup-wrong-owner.dll')) @('VCL_POPUP_SCOPE_OK callbacks=1 ownerdraw=1 nested=1 destroy=1 wrong_owner=1 restored=1') 15),
        (Case 'vcl-popup-scope-64' $run @('--wait',(Join-Path $x64 'core-vcl-popup-scope.exe'),(Join-Path $x64 'vcl-popup-wrong-owner.dll')) @('VCL_POPUP_SCOPE_OK callbacks=1 ownerdraw=1 nested=1 destroy=1 wrong_owner=1 restored=1') 15),
        (Case 'vcl-radio-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-group-module.exe'),(Join-Path $x86 'vcl-radio-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=32 fake=1 native=1 background=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
        (Case 'vcl-radio-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-group-module.exe'),(Join-Path $x64 'vcl-radio-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=64 fake=1 native=1 background=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
        (Case 'vcl-radio-frame-32' $run @('--wait',(Join-Path $x86 'core-vcl-group-frame.exe'),(Join-Path $x86 'vcl-radio-frame-fixture.dll')) @('NO_BACKGROUND_AND_OUTSIDE_PAINT_OK','MATCHED_BACKGROUND_PAINT_OK','CLIP_PRESERVED_OK','REPARENT_RETIRES_OK','REENTRANT_REPARENT_OK','EMPTY_CAPTION_OK','DISABLED_CLIP_OK','DISABLED_TRANSITION_OK','DISABLED_EMPTY_OK','DISABLED_REENTRANT_STATE_REJECTED','WRONG_PARENT_OWNER_OK','GROUP_SCOPE_FIXTURE_OK')),
        (Case 'vcl-radio-frame-64' $run @('--wait',(Join-Path $x64 'core-vcl-group-frame.exe'),(Join-Path $x64 'vcl-radio-frame-fixture.dll')) @('NO_BACKGROUND_AND_OUTSIDE_PAINT_OK','MATCHED_BACKGROUND_PAINT_OK','CLIP_PRESERVED_OK','REPARENT_RETIRES_OK','REENTRANT_REPARENT_OK','EMPTY_CAPTION_OK','DISABLED_CLIP_OK','DISABLED_TRANSITION_OK','DISABLED_EMPTY_OK','DISABLED_REENTRANT_STATE_REJECTED','WRONG_PARENT_OWNER_OK','GROUP_SCOPE_FIXTURE_OK')),
        (Case 'vcl-group-frame-32' $run @('--wait',(Join-Path $x86 'core-vcl-group-frame.exe'),(Join-Path $x86 'vcl-group-frame-fixture.dll')) @('NO_BACKGROUND_AND_OUTSIDE_PAINT_OK','MATCHED_BACKGROUND_PAINT_OK','CLIP_PRESERVED_OK','REPARENT_RETIRES_OK','REENTRANT_REPARENT_OK','EMPTY_CAPTION_OK','DISABLED_CLIP_OK','DISABLED_TRANSITION_OK','DISABLED_EMPTY_OK','DISABLED_REENTRANT_STATE_REJECTED','WRONG_PARENT_OWNER_OK','GROUP_SCOPE_FIXTURE_OK')),
        (Case 'vcl-group-frame-64' $run @('--wait',(Join-Path $x64 'core-vcl-group-frame.exe'),(Join-Path $x64 'vcl-group-frame-fixture.dll')) @('NO_BACKGROUND_AND_OUTSIDE_PAINT_OK','MATCHED_BACKGROUND_PAINT_OK','CLIP_PRESERVED_OK','REPARENT_RETIRES_OK','REENTRANT_REPARENT_OK','EMPTY_CAPTION_OK','DISABLED_CLIP_OK','DISABLED_TRANSITION_OK','DISABLED_EMPTY_OK','DISABLED_REENTRANT_STATE_REJECTED','WRONG_PARENT_OWNER_OK','GROUP_SCOPE_FIXTURE_OK')),
        (Case 'vcl-group-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-group-module.exe'),(Join-Path $x86 'vcl-group-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=32 fake=1 native=1 background=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
        (Case 'vcl-group-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-group-module.exe'),(Join-Path $x64 'vcl-group-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=64 fake=1 native=1 background=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
        (Case 'vcl-push-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-push-module.exe'),(Join-Path $x86 'vcl-push-module-fixture.dll')) @('VCL_PUSH_MODULE_OK bits=32 fake=1 foreign=1 wrong_parent=1 bitmap=1 native=1 click=1 keyboard=1 reparent=1 role=1 font_restore=1 cleanup=1')),
        (Case 'vcl-push-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-push-module.exe'),(Join-Path $x64 'vcl-push-module-fixture.dll')) @('VCL_PUSH_MODULE_OK bits=64 fake=1 foreign=1 wrong_parent=1 bitmap=1 native=1 click=1 keyboard=1 reparent=1 role=1 font_restore=1 cleanup=1')),
        (Case 'dbcombo-ansi-32' $run @('--wait',(Join-Path $x86 'core-dbcombo-ansi.exe'),(Join-Path $x86 'dbcombo-ansi-fixture.dll')) @('DBCOMBO_ANSI_MODEL_OK bits=32 fake=1 foreign=1 wrong_parent=1 select=1 font=1 open_remove=1 reparent=1 role=1 query_reparent=1 unsupported=1 unicode_witness_rejected=1 cleanup=1')),
        (Case 'dbcombo-ansi-64' $run @('--wait',(Join-Path $x64 'core-dbcombo-ansi.exe'),(Join-Path $x64 'dbcombo-ansi-fixture.dll')) @('DBCOMBO_ANSI_MODEL_OK bits=64 fake=1 foreign=1 wrong_parent=1 select=1 font=1 open_remove=1 reparent=1 role=1 query_reparent=1 unsupported=1 unicode_witness_rejected=1 cleanup=1')),
        (Case 'vcl-combo-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-combo-module.exe'),(Join-Path $x86 'vcl-combo-module-fixture.dll')) @('VCL_COMBO_MODULE_OK bits=32 fake=1 foreign=1 wrong_parent=1 select=1 font=1 open_remove=1 reparent=1 role=1 query_reparent=1 cleanup=1')),
        (Case 'vcl-combo-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-combo-module.exe'),(Join-Path $x64 'vcl-combo-module-fixture.dll')) @('VCL_COMBO_MODULE_OK bits=64 fake=1 foreign=1 wrong_parent=1 select=1 font=1 open_remove=1 reparent=1 role=1 query_reparent=1 cleanup=1')),
        (Case 'vcl-status-simple-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-simple.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('SIMPLE_FONT_SWITCH_OK','SIMPLE_NONASCII_FALLBACK_OK','SIMPLE_LONG_FALLBACK_OK','SIMPLE_REENTRANT_FALLBACK_OK mode=1','SIMPLE_REENTRANT_FALLBACK_OK mode=2','SIMPLE_LIFECYCLE','SIMPLE_MODEL_OK')),
        (Case 'vcl-status-simple-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-simple.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('SIMPLE_FONT_SWITCH_OK','SIMPLE_NONASCII_FALLBACK_OK','SIMPLE_LONG_FALLBACK_OK','SIMPLE_REENTRANT_FALLBACK_OK mode=1','SIMPLE_REENTRANT_FALLBACK_OK mode=2','SIMPLE_LIFECYCLE','SIMPLE_MODEL_OK')),
        (Case 'status-font-stock-live-32' $run @('--wait',(Join-Path $x86 'core-status-font-stock-live.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('STOCK_LIVE_OK')),
        (Case 'status-font-stock-live-64' $run @('--wait',(Join-Path $x64 'core-status-font-stock-live.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('STOCK_LIVE_OK')),
        (Case 'status-font-stock-32' (Join-Path $x86 'core-status-font-stock.exe') @() @('STOCK_VARIANT_OWNERSHIP_OK')),
        (Case 'status-font-stock-64' (Join-Path $x64 'core-status-font-stock.exe') @() @('STOCK_VARIANT_OWNERSHIP_OK')),
        (Case 'status-font-isolation-32' (Join-Path $x86 'core-status-font-isolation.exe') @() @('STATUS_FONT_ISOLATION_OK shared=1 independent=1 retain=1 cleanup=1')),
        (Case 'status-font-isolation-64' (Join-Path $x64 'core-status-font-isolation.exe') @() @('STATUS_FONT_ISOLATION_OK shared=1 independent=1 retain=1 cleanup=1')),
        (Case 'vcl-status-font-alias-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-font-alias.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('ALIAS_REUSED','ALIAS_OK')),
        (Case 'vcl-status-font-alias-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-font-alias.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('ALIAS_REUSED','ALIAS_OK')),
        (Case 'vcl-combo-simple-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-combo-simple-module.exe'),(Join-Path $x86 'vcl-combo-module-fixture.dll')) @('VCL_COMBO_SIMPLE_MODULE_OK bits=32 fake=1 foreign=1 wrong_parent=1 select=1 font=1 list_remove=1 reparent=1 role=1 mode_change=1 query_reparent=1 cleanup=1')),
        (Case 'vcl-combo-simple-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-combo-simple-module.exe'),(Join-Path $x64 'vcl-combo-module-fixture.dll')) @('VCL_COMBO_SIMPLE_MODULE_OK bits=64 fake=1 foreign=1 wrong_parent=1 select=1 font=1 list_remove=1 reparent=1 role=1 mode_change=1 query_reparent=1 cleanup=1')),
        (Case 'vcl-combo-list-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-combo-list-module.exe'),(Join-Path $x86 'vcl-combo-module-fixture.dll')) @('VCL_COMBO_LIST_MODULE_OK bits=32 fake=1 foreign=1 wrong_parent=1 select=1 font=1 open_remove=1 reparent=1 role=1 mode_change=1 query_reparent=1 cleanup=1')),
        (Case 'vcl-combo-list-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-combo-list-module.exe'),(Join-Path $x64 'vcl-combo-module-fixture.dll')) @('VCL_COMBO_LIST_MODULE_OK bits=64 fake=1 foreign=1 wrong_parent=1 select=1 font=1 open_remove=1 reparent=1 role=1 mode_change=1 query_reparent=1 cleanup=1')),
        (Case 'vcl-check-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-check-module.exe'),(Join-Path $x86 'vcl-check-module-fixture.dll')) @('VCL_CHECK_MODULE_OK bits=32 fake=1 foreign=1 wrong_parent=1 bitmap=1 native=1 click=1 keyboard=1 states=3 background=1 background_reparent=1 reparent=1 role=1 font_restore=1 cleanup=1')),
        (Case 'vcl-check-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-check-module.exe'),(Join-Path $x64 'vcl-check-module-fixture.dll')) @('VCL_CHECK_MODULE_OK bits=64 fake=1 foreign=1 wrong_parent=1 bitmap=1 native=1 click=1 keyboard=1 states=3 background=1 background_reparent=1 reparent=1 role=1 font_restore=1 cleanup=1')),
        (Case 'vcl-bitbtn-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-bitbtn-module.exe'),(Join-Path $x86 'vcl-bitbtn-module-fixture.dll')) @('VCL_BITBTN_MODULE_OK bits=32 fake=1 foreign=1 unwind=1 nested=1 click=1 pixels=1 glyph=1 clip=1 retire=1 cleanup=1')),
        (Case 'vcl-bitbtn-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-bitbtn-module.exe'),(Join-Path $x64 'vcl-bitbtn-module-fixture.dll')) @('VCL_BITBTN_MODULE_OK bits=64 fake=1 foreign=1 unwind=1 nested=1 click=1 pixels=1 glyph=1 clip=1 retire=1 cleanup=1')),
        (Case 'vcl-date-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-date-module.exe'),(Join-Path $x86 'vcl-date-module-fixture.dll')) @('VCL_DATE_MODULE_OK bits=32 fake=1 checked_rejected=1 date=1 keyboard=1 popup=1 pixels=1 clip=1 dc_state=1 unwind=1 exact_region=1 leak_negative=1 repeated_gdi=1 retire=1 destroy=1 cleanup=1')),
        (Case 'vcl-date-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-date-module.exe'),(Join-Path $x64 'vcl-date-module-fixture.dll')) @('VCL_DATE_MODULE_OK bits=64 fake=1 checked_rejected=1 date=1 keyboard=1 popup=1 pixels=1 clip=1 dc_state=1 unwind=1 exact_region=1 leak_negative=1 repeated_gdi=1 retire=1 destroy=1 cleanup=1')),
        (Case 'spin-32' $run @('--wait',(Join-Path $x86 'core-spin-controls.exe')) @('SPIN_CONTROL_NESTED_OK','SPIN_CONTROL_OK bits=32','outside=0','SPIN_CONTROL_CLIP_OK','SPIN_CONTROL_REENTRANT_OK','SPIN_CONTROL_BOUNDARY_OK')),
        (Case 'spin-64' $run @('--wait',(Join-Path $x64 'core-spin-controls.exe')) @('SPIN_CONTROL_NESTED_OK','SPIN_CONTROL_OK bits=64','outside=0','SPIN_CONTROL_CLIP_OK','SPIN_CONTROL_REENTRANT_OK','SPIN_CONTROL_BOUNDARY_OK')),
        (Case 'spin-dpi-32' $run @('--wait',(Join-Path $x86 'core-spin-controls.exe'),'aware') @('SPIN_CONTROL_NESTED_OK','SPIN_CONTROL_OK bits=32','outside=0','SPIN_CONTROL_CLIP_OK','SPIN_CONTROL_REENTRANT_OK','SPIN_CONTROL_BOUNDARY_OK')),
        (Case 'spin-dpi-64' $run @('--wait',(Join-Path $x64 'core-spin-controls.exe'),'aware') @('SPIN_CONTROL_NESTED_OK','SPIN_CONTROL_OK bits=64','outside=0','SPIN_CONTROL_CLIP_OK','SPIN_CONTROL_REENTRANT_OK','SPIN_CONTROL_BOUNDARY_OK')),
        (Case 'spin-passive-32' (Join-Path $x86 'core-spin-controls.exe') @('passive',$d32) @('SPIN_CONTROL_PASSIVE_OK bits=32 unattached=1')),
        (Case 'spin-passive-64' (Join-Path $x64 'core-spin-controls.exe') @('passive',$d64) @('SPIN_CONTROL_PASSIVE_OK bits=64 unattached=1')),
        (Case 'status-v5-32' $run @('--wait',(Join-Path $x86 'core-status-controls.exe')) @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-v5-64' $run @('--wait',(Join-Path $x64 'core-status-controls.exe')) @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-v6-32' $run @('--wait',(Join-Path $x86 'core-status-controls-v6.exe')) @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-v6-64' $run @('--wait',(Join-Path $x64 'core-status-controls-v6.exe')) @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-v5-dpi-32' $run @('--wait',(Join-Path $x86 'core-status-controls.exe'),'aware') @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','aware=1','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-v5-dpi-64' $run @('--wait',(Join-Path $x64 'core-status-controls.exe'),'aware') @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','aware=1','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-v6-dpi-32' $run @('--wait',(Join-Path $x86 'core-status-controls-v6.exe'),'aware') @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','aware=1','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-v6-dpi-64' $run @('--wait',(Join-Path $x64 'core-status-controls-v6.exe'),'aware') @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','aware=1','STATUS_CONTROL_CLEANUP_OK')),
        (Case 'status-passive-v5-32' (Join-Path $x86 'core-status-controls.exe') @('passive',$d32) @('STATUS_PASSIVE_OK bits=32 unattached=1')),
        (Case 'status-passive-v5-64' (Join-Path $x64 'core-status-controls.exe') @('passive',$d64) @('STATUS_PASSIVE_OK bits=64 unattached=1')),
        (Case 'status-passive-v6-32' (Join-Path $x86 'core-status-controls-v6.exe') @('passive',$d32) @('STATUS_PASSIVE_OK bits=32 unattached=1')),
        (Case 'status-passive-v6-64' (Join-Path $x64 'core-status-controls-v6.exe') @('passive',$d64) @('STATUS_PASSIVE_OK bits=64 unattached=1'))
    )
    if($FullIntegration){
        $cases+=@(
            (Case 'font-stress-32' (Join-Path $x86 'core-font-layout.exe') @($d32,'--stress') @('FONT WARMUP STRESS PASS context=0 rounds=8','FONT WARMUP STRESS PASS context=1 rounds=8','FONT LAYOUT PASS 2 contexts 12 font variants') 180),
            (Case 'font-stress-64' (Join-Path $x64 'core-font-layout.exe') @($d64,'--stress') @('FONT WARMUP STRESS PASS context=0 rounds=8','FONT WARMUP STRESS PASS context=1 rounds=8','FONT LAYOUT PASS 2 contexts 12 font variants') 180),
            (Case 'creation-contract' (Join-Path $x64 'creation-contract-driver.exe') @($run,$d32,$d64,(Join-Path $x86 'creation-contract.exe'),(Join-Path $x64 'creation-contract.exe')) @('CREATION CONTRACT PASS 64') 520),
            (Case 'rollback' (Join-Path $x64 'rollback-contract-driver.exe') @($run,$d32,$d64,(Join-Path $x86 'rollback-contract.exe'),(Join-Path $x64 'rollback-contract.exe')) @('ROLLBACK PASS parent=32','ROLLBACK PASS parent=64') 320),
            (Case 'dialogs' (Join-Path $x64 'launch-automatic.exe') ($pair+@('dialogs')) @('LAUNCH MATRIX PASS') 520),
            (Case 'messages' (Join-Path $x64 'launch-automatic.exe') ($pair+@('messages')) @('LAUNCH MATRIX PASS') 520)
        )
    }
    # Dialog tests that do not compare actual screen pixels run on a private
    # desktop, so they cannot consume the user's physical keyboard input.
    foreach($case in $cases){
        if($case.command -eq $dialogDriver -and $case.name -notmatch '^scroll-' -and $case.name -notmatch '^dialog-folder-x(86|64)-controls$'){
            $case.arguments=@('--private')+@($case.arguments)
        }
    }
    $plan=if($focusSpecified){Select-PackageCases -Cases $cases -Focus $Focus}else{Select-PackageCases -Cases $cases}
    if($FullIntegration){$plan.coverage='full-integration'}
    $cases=@($plan.cases)
    $records=[System.Collections.Generic.List[object]]::new()
    $progressConflicts=0
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
                $cost=if($case.name -match '^(vcl-status-font-alias-|scroll-(frame|list|edit)-)' ){$Jobs}elseif($case.name -match '^(font-layout|menu-cancel|dialogs|messages|tree-v[56]-)'){[Math]::Min(2,$Jobs)}else{1}
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
                $progressJson=@($running | ForEach-Object {@{name=$_.case.name;pid=$_.process.Id;seconds=$_.timer.Elapsed.TotalSeconds;budget=$_.case.seconds}})|ConvertTo-Json
                if (!(Set-PackageProgress -Path (Join-Path $scratch 'progress.json') -Json $progressJson)) { ++$progressConflicts }
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
                # A private USER desktop is an isolation prerequisite, not a
                # product assertion.  Some non-interactive execution hosts
                # deny only CreateDesktop (5). Other skips require an exact, case-specific
                # environmental marker and are reported separately.
                $caseSkipped=Test-PackageCaseSkip -Name $entry.case.name -ExitCode $entry.process.ExitCode -Output ($out+$err)
                $record=@{name=$entry.case.name;exit_code=$entry.process.ExitCode;skipped=[bool]$caseSkipped;command=$entry.case.command;arguments=$entry.case.arguments;seconds=$entry.timer.Elapsed.TotalSeconds};$records.Add($record)
                $records|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'cases.json')
                if(!$caseSkipped){
                    if($entry.process.ExitCode){throw "Package test failed: $($entry.case.name), exit $($entry.process.ExitCode)"}
                    foreach($marker in $entry.case.markers){if(!($out+$err).Contains($marker)){throw "Missing marker $marker in $($entry.case.name)"}}
                }
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
    $skipped=@($records|Where-Object skipped|ForEach-Object name)
    $status=if($plan.coverage -eq 'focused'){if($skipped.Count){'passed-focused-with-skips'}else{'passed-focused'}}elseif($skipped.Count){'passed-with-skips'}else{'passed'}
    @{status=$status;coverage=$plan.coverage;requested_cases=@($plan.requested);selected_cases=@($cases | ForEach-Object name);omitted_cases=@($plan.omitted);seconds=$totalTimer.Elapsed.TotalSeconds;jobs=$Jobs;cases=$records.Count;skipped=$skipped;progress_sharing_conflicts=$progressConflicts;full_integration=[bool]$FullIntegration;package=$Package;sha256=$after;physical_input='not-tested';core_matrix='separate Build-Core matrix required'}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
}catch{
    @{status='failed-or-inconclusive';error=$_.Exception.Message}|ConvertTo-Json|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    exit 1
}
