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
        (Case 'demo-pair' (Join-Path $x64 'demo-observe.exe') @($run,$d64,$d32,$demo) @('DEMO OBSERVE PASS styled=0','DEMO OBSERVE PASS styled=1','DEMO PAIR PASS')),
        (Case 'wait-contract' $python @('-B',(Join-Path $SourceRoot 'tests/run16s-exe/wait.py'),$run,$d32,$d64,(Join-Path $x86 'wait-target.exe'),(Join-Path $x64 'wait-target.exe'),(Join-Path $x64 'wait-gui-target.exe')) @('WAIT CONTRACT PASS 6 cases') 190),
        (Case 'font-layout-x86' (Join-Path $x86 'core-font-layout.exe') @($d32) @('FONT WARMUP SELFTEST PASS transient=1 rising=1 real_gdi_leak=1 cleanup=1','FONT LAYOUT PASS 2 contexts 12 font variants') 180),
        (Case 'font-layout-x64' (Join-Path $x64 'core-font-layout.exe') @($d64) @('FONT WARMUP SELFTEST PASS transient=1 rising=1 real_gdi_leak=1 cleanup=1','FONT LAYOUT PASS 2 contexts 12 font variants') 180),
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
        (Case 'vb-checklist-32' (Join-Path $x86 'core-vb-checklist.exe') @($run,(Join-Path $x86 'vb-checklist-fixture/MSVBVM60.dll')) @('VB_CHECKLIST_OK bits=32 styled=1 selected=1 restored=1 wrong_geometry=1 wrong_source=1 wrong_caller=1 disabled_states=1 mismatched_state_native=1 reentrant_disable_native=1 reentrant_style_native=1 reentrant_reparent_lifecycle=1 system_text=1 unicode_fallback=1 source_font_preserved=1 role_retirement=1 mixed_fallback=1')),
        (Case 'vb-user-edge-32' (Join-Path $x86 'core-vb-user-edge.exe') @($run,(Join-Path $x86 'vb-container-fixture/MSVBVM60.dll')) @('VB_USER_EDGE_OK bits=32 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=6 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=32 label_pixels=','native_preserved=1')),
        (Case 'vb-user-edge-64' (Join-Path $x64 'core-vb-user-edge.exe') @($run,(Join-Path $x64 'vb-container-fixture/MSVBVM60.dll')) @('VB_USER_EDGE_OK bits=64 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=6 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=64 label_pixels=','native_preserved=1')),
        (Case 'vb5-user-edge-32' (Join-Path $x86 'core-vb-user-edge.exe') @($run,(Join-Path $x86 'vb-container-fixture/MSVBVM50.dll')) @('VB_USER_EDGE_OK bits=32 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=5 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=32 label_pixels=','native_preserved=1')),
        (Case 'vb5-user-edge-64' (Join-Path $x64 'core-vb-user-edge.exe') @($run,(Join-Path $x64 'vb-container-fixture/MSVBVM50.dll')) @('VB_USER_EDGE_OK bits=64 styled=1 edge=1 pixels=1 input=1 role_retire=1 wrong_owner=1 unsupported=1 version=5 restored=1 dormant_reparent=1 initial_unedged=1 reentrant_native=1','VB_USER_LABEL_OK bits=64 label_pixels=','native_preserved=1')),
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
        (Case 'tab-controls-32' (Join-Path $x86 'core-tab-controls.exe') @($d32) @('TAB_CONTROL_OK bits=32','styled=6 preserved=15')),
        (Case 'tab-controls-64' (Join-Path $x64 'core-tab-controls.exe') @($d64) @('TAB_CONTROL_OK bits=64','styled=6 preserved=15')),
        (Case 'tab-controls-dpi-32' (Join-Path $x86 'core-tab-controls.exe') @($d32,'aware') @('TAB_CONTROL_OK bits=32','aware=1 styled=6 preserved=15')),
        (Case 'tab-controls-dpi-64' (Join-Path $x64 'core-tab-controls.exe') @($d64,'aware') @('TAB_CONTROL_OK bits=64','aware=1 styled=6 preserved=15')),
        (Case 'tab-controls-v5-32' (Join-Path $x86 'core-tab-controls-v5.exe') @($d32) @('TAB_CONTROL_OK bits=32','styled=6 preserved=15')),
        (Case 'tab-controls-v5-64' (Join-Path $x64 'core-tab-controls-v5.exe') @($d64) @('TAB_CONTROL_OK bits=64','styled=6 preserved=15')),
        (Case 'tab-controls-v5-dpi-32' (Join-Path $x86 'core-tab-controls-v5.exe') @($d32,'aware') @('TAB_CONTROL_OK bits=32','aware=1 styled=6 preserved=15')),
        (Case 'tab-controls-v5-dpi-64' (Join-Path $x64 'core-tab-controls-v5.exe') @($d64,'aware') @('TAB_CONTROL_OK bits=64','aware=1 styled=6 preserved=15')),
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
        (Case 'startup-hooks-32' (Join-Path $x86 'launch-startup-hooks.exe') @() @('STARTUP_CAPACITY_OK slots=40 overflow41=1 abort_restored=41','C STARTUP HOOK PASS width=32')),
        (Case 'updown-v5-64' (Join-Path $x64 'core-updown-v5.exe') @($run) @('UPDOWN_CONTROL_OK bits=64 styled=1','UPDOWN_SHARED_THEME_OK','UPDOWN_PASSIVE_OK','UPDOWN_RETIRE_OK','UPDOWN_RESOURCE_OBSERVED','UPDOWN_CLEANUP_OK')),
        (Case 'updown-v6-64' (Join-Path $x64 'core-updown-v6.exe') @($run) @('UPDOWN_CONTROL_OK bits=64 styled=1','UPDOWN_SHARED_THEME_OK','UPDOWN_PASSIVE_OK','UPDOWN_RETIRE_OK','UPDOWN_RESOURCE_OBSERVED','UPDOWN_CLEANUP_OK')),
        (Case 'updown-owner-64' (Join-Path $x64 'core-updown-owner.exe') @() @('UPDOWN_OWNER_OK bits=64','owned_dc_handles_released=165','UPDOWN_DC_CAPACITY_OK')),
        (Case 'startup-hooks-64' (Join-Path $x64 'launch-startup-hooks.exe') @() @('STARTUP_CAPACITY_OK slots=40 overflow41=1 abort_restored=41','C STARTUP HOOK PASS width=64')),
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
        (Case 'superclass-32' (Join-Path $x86 'core-superclass.exe') @($run) @('SUPER_PROTOCOL_OK bits=32 styled=1','SUPER_VCL_CHOICE_OK native_superclass=3 state_click=1 styled=1 detach_pixels=1','SUPER_VCL_EDIT_OK native_superclass=2 multiline_undo_readonly_scroll=1 styled=1 detach_pixels=1','SUPER_VCL_LIST_OK item_data_selection=1 multiselect=1 ownerdraw_rejected=1 detach_pixels=1 styled=1','SUPER_RENDER_OK','SUPER_ACTUAL_FILL_OK','SUPER_REJECT_OK','SUPER_CLEANUP_OK')),
        (Case 'superclass-64' (Join-Path $x64 'core-superclass.exe') @($run) @('SUPER_PROTOCOL_OK bits=64 styled=1','SUPER_VCL_CHOICE_OK native_superclass=3 state_click=1 styled=1 detach_pixels=1','SUPER_VCL_EDIT_OK native_superclass=2 multiline_undo_readonly_scroll=1 styled=1 detach_pixels=1','SUPER_VCL_LIST_OK item_data_selection=1 multiselect=1 ownerdraw_rejected=1 detach_pixels=1 styled=1','SUPER_RENDER_OK','SUPER_ACTUAL_FILL_OK','SUPER_REJECT_OK','SUPER_CLEANUP_OK')),
        (Case 'checklist-32' (Join-Path $x86 'core-checklist.exe') @($run) @('CHECKLIST_MODEL_OK bits=32 styled=0','CHECKLIST_MODEL_OK bits=32 styled=1 checked_mismatch=0','CHECKLIST_PUSHED_OK bits=32','CHECKLIST_CLEANUP_OK')),
        (Case 'checklist-64' (Join-Path $x64 'core-checklist.exe') @($run) @('CHECKLIST_MODEL_OK bits=64 styled=0','CHECKLIST_MODEL_OK bits=64 styled=1 checked_mismatch=0','CHECKLIST_PUSHED_OK bits=64','CHECKLIST_CLEANUP_OK'))
    )
    $cases+=@(
        (Case 'vcl-static-32' $run @('--wait',(Join-Path $x86 'core-vcl-static.exe')) @('VCL_STATIC_OK bits=32 font_preserved=1 fake_rejected=1 nested_native=1')),
        (Case 'vcl-static-64' $run @('--wait',(Join-Path $x64 'core-vcl-static.exe')) @('VCL_STATIC_OK bits=64 font_preserved=1 fake_rejected=1 nested_native=1')),
        (Case 'vcl-static-bare-32' (Join-Path $x86 'core-vcl-static.exe') @('bare') @('VCL_STATIC_BARE_OK bits=32 font_preserved=1')),
        (Case 'vcl-static-bare-64' (Join-Path $x64 'core-vcl-static.exe') @('bare') @('VCL_STATIC_BARE_OK bits=64 font_preserved=1')),
        (Case 'vcl-memo-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-edit-module.exe'),(Join-Path $x86 'vcl-memo-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=32 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-memo-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-edit-module.exe'),(Join-Path $x64 'vcl-memo-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=64 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-edit-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-edit-module.exe'),(Join-Path $x86 'vcl-edit-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=32 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-edit-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-edit-module.exe'),(Join-Path $x64 'vcl-edit-module-fixture.dll')) @('VCL_EDIT_MODULE_OK bits=64 fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 cleanup=1')),
        (Case 'vcl-trackbar-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-trackbar-module.exe'),(Join-Path $x86 'vcl-trackbar-module-fixture.dll')) @('VCL_TRACK_MODULE_OK bits=32 fake=1 selection=1 nested=1 callbacks=1 apppost=1 appskip=1 pixels=1 clip=1 disabled=1 reentrant_remove=1 destroy=1 gdi_sample=1 cleanup=1')),
        (Case 'vcl-trackbar-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-trackbar-module.exe'),(Join-Path $x64 'vcl-trackbar-module-fixture.dll')) @('VCL_TRACK_MODULE_OK bits=64 fake=1 selection=1 nested=1 callbacks=1 apppost=1 appskip=1 pixels=1 clip=1 disabled=1 reentrant_remove=1 destroy=1 gdi_sample=1 cleanup=1')),
        (Case 'vcl-color-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-color-module.exe'),(Join-Path $x86 'vcl-color-module-fixture.dll')) @('VCL_COLOR_MODULE_OK bits=32 fake=1 selection=1 keyboard=1 popup=1 ownerdraw=1 nested=1 pixels=1 clip=1 dc_state=1 print_resources=1 unwind=1 retire=1 destroy=1 cleanup=1')),
        (Case 'vcl-color-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-color-module.exe'),(Join-Path $x64 'vcl-color-module-fixture.dll')) @('VCL_COLOR_MODULE_OK bits=64 fake=1 selection=1 keyboard=1 popup=1 ownerdraw=1 nested=1 pixels=1 clip=1 dc_state=1 print_resources=1 unwind=1 retire=1 destroy=1 cleanup=1')),
        (Case 'vcl-status-pane-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-pane.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('PANE_CLIP changed=','PANE mode=0 changed=','PANE mode=1 changed=','PANE mode=2 changed=','PANE mode=3 changed=0','VCL_STATUS_PANE_OK')),
        (Case 'vcl-status-pane-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-pane.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('PANE_CLIP changed=','PANE mode=0 changed=','PANE mode=1 changed=','PANE mode=2 changed=','PANE mode=3 changed=0','VCL_STATUS_PANE_OK')),
        (Case 'vcl-status-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-module.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll')) @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'vcl-status-module-dpi-32' $run @('--wait',(Join-Path $x86 'core-vcl-status-module.exe'),(Join-Path $x86 'vcl-status-module-fixture.dll'),'aware') @('STATUS_LIVE_OK bits=32','STATUS_CONTROL_OK bits=32','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'vcl-status-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-module.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll')) @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'vcl-status-module-dpi-64' $run @('--wait',(Join-Path $x64 'core-vcl-status-module.exe'),(Join-Path $x64 'vcl-status-module-fixture.dll'),'aware') @('STATUS_LIVE_OK bits=64','STATUS_CONTROL_OK bits=64','STATUS_CLIP_OK','STATUS_BOUNDARY_OK','VCL_STATUS_MODULE_OK')),
        (Case 'vcl-tab-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-tab-module.exe'),(Join-Path $x86 'vcl-tab-module-fixture.dll')) @('VCL_TAB_MODULE_OK bits=32 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-tab-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-tab-module.exe'),(Join-Path $x64 'vcl-tab-module-fixture.dll')) @('VCL_TAB_MODULE_OK bits=64 fake=1 selection=1 ownerdraw=1 reparent=1 cleanup=1')),
        (Case 'vcl-toolbar-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-toolbar-module.exe'),(Join-Path $x86 'vcl-toolbar-module-fixture.dll')) @('VCL_TOOLBAR_MODULE_OK bits=32 fake=1 zero=1 duplicate=1 image=1 click=1 callback=1 app_skip=1 reparent=1 clipped=1 cap64=1 cap65=1 removal=1 cleanup=1')),
        (Case 'vcl-toolbar-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-toolbar-module.exe'),(Join-Path $x64 'vcl-toolbar-module-fixture.dll')) @('VCL_TOOLBAR_MODULE_OK bits=64 fake=1 zero=1 duplicate=1 image=1 click=1 callback=1 app_skip=1 reparent=1 clipped=1 cap64=1 cap65=1 removal=1 cleanup=1')),
        (Case 'vcl-grid-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-grid-module.exe'),(Join-Path $x86 'vcl-grid-module-fixture.dll')) @('VCL_GRID_MODULE_OK bits=32 invalid=1 both=1 scroll=1 vertical=1 client=1 removal=1 reparent=1 cleanup=1')),
        (Case 'vcl-grid-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-grid-module.exe'),(Join-Path $x64 'vcl-grid-module-fixture.dll')) @('VCL_GRID_MODULE_OK bits=64 invalid=1 both=1 scroll=1 vertical=1 client=1 removal=1 reparent=1 cleanup=1')),
        (Case 'vcl-radio-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-group-module.exe'),(Join-Path $x86 'vcl-radio-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=32 fake=1 native=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
        (Case 'vcl-radio-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-group-module.exe'),(Join-Path $x64 'vcl-radio-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=64 fake=1 native=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
        (Case 'vcl-group-module-32' $run @('--wait',(Join-Path $x86 'core-vcl-group-module.exe'),(Join-Path $x86 'vcl-group-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=32 fake=1 native=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
        (Case 'vcl-group-module-64' $run @('--wait',(Join-Path $x64 'core-vcl-group-module.exe'),(Join-Path $x64 'vcl-group-module-fixture.dll')) @('VCL_GROUP_MODULE_OK bits=64 fake=1 native=1 selection=1 keyboard=1 font_restore=1 reparent=1 role=1 cleanup=1')),
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
                $cost=if($case.name -match '^(font-layout|menu-cancel|dialogs|messages|tree-v[56]-)'){[Math]::Min(2,$Jobs)}else{1}
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
                # deny only CreateDesktop (5).  Preserve every other exit 2
                # as a failure: the exact marker is the sole skip contract.
                $privateDesktopDenied=$entry.process.ExitCode -eq 2 -and
                    ($out+$err).Contains('PRIVATE_DESKTOP_CREATE_FAIL error=5')
                $record=@{name=$entry.case.name;exit_code=$entry.process.ExitCode;skipped=[bool]$privateDesktopDenied;command=$entry.case.command;arguments=$entry.case.arguments;seconds=$entry.timer.Elapsed.TotalSeconds};$records.Add($record)
                $records|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'cases.json')
                if(!$privateDesktopDenied){
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
