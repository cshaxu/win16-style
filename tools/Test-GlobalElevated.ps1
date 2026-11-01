[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$BuildRoot,
    [Parameter(Mandatory)][uint32]$MediumTokenOwnerPid,
    [switch]$MediumOnly
)
$ErrorActionPreference='Stop'
$identity=[Security.Principal.WindowsIdentity]::GetCurrent()
$principal=[Security.Principal.WindowsPrincipal]::new($identity)
if(!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)){
    Write-Output 'GLOBAL ELEVATED SKIP: administrator token required'
    exit 77
}
$packagePath=(Resolve-Path -LiteralPath $Package).Path
$tree=(Resolve-Path -LiteralPath $BuildRoot).Path
$driver=Join-Path $tree 'x64/dialog-test-driver.exe'
$broker=Join-Path $tree 'x64/global-cross-origin.exe'
$lifecycle=Join-Path $tree 'x64/global-lifecycle.exe'
$launcher=Join-Path $packagePath 'run16s.exe'
$failures=[System.Collections.Generic.List[string]]::new()
function Traces { @(& logman.exe query -ets | Where-Object { $_ -match '^\s*Win16S-ETW-' }) }
function Invoke-Checked([string]$Name,[string]$Executable,[string[]]$Arguments,[string[]]$Markers){
    Write-Output "BEGIN $Name"
    $output=@(& $Executable @Arguments 2>&1);$exit=$LASTEXITCODE
    $output | ForEach-Object { Write-Output $_ }
    $joined=$output -join "`n"
    if($exit){$failures.Add("$Name exit $exit");Write-Output "FAIL $Name exit=$exit";return}
    foreach($marker in $Markers){if(!$joined.Contains($marker)){$failures.Add("$Name missing marker: $marker");Write-Output "FAIL $Name missing=$marker";return}}
    Write-Output "PASS $Name"
}
$before=Traces
foreach($arch in @('x86','x64')){
    $bits=if($arch -eq 'x86'){32}else{64}
    $target=Join-Path $tree "$arch/global-propagation-probe.exe"
    $orphan=Join-Path $tree "$arch/core-global-process-integration.exe"
    if(!$MediumOnly){
        Invoke-Checked "orphan-$arch" $orphan @() @('GLOBAL ORPHAN before=0 start=0 after=4201 stop=0')
        Invoke-Checked "headless-$arch" $driver @('--private','--direct',$broker,$launcher,$target,'headless-event') @("HEADLESS READY bits=$bits ready=1","HEADLESS ROOT bits=$bits dll=1 child=1 child_exit=0",'CROSS BROKER loaded=0 target_exit=0','DIALOG_DRIVER exit=0')
    }
    $traceRoot=Join-Path (Split-Path $PSScriptRoot -Parent) 'build/t21-s13/admin-etw'
    New-Item -ItemType Directory -Force -Path $traceRoot | Out-Null
    $env:WIN16S_TEST_HEADLESS_TRACE=Join-Path $traceRoot ("medium-$arch-"+[guid]::NewGuid().ToString('N')+'.log')
    Invoke-Checked "medium-native-$arch" $driver @('--low-private','--direct',$broker,$launcher,$target,'native-control','--medium',"$MediumTokenOwnerPid") @('MEDIUM CREATE made=1 error=0','CROSS NATIVE loaded=0 target_exit=','DIALOG_DRIVER exit=0')
    Invoke-Checked "medium-$arch" $driver @('--low-private','--direct',$broker,$launcher,$target,'headless-event','--medium',"$MediumTokenOwnerPid") @('MEDIUM CREATE made=1 error=0',"HEADLESS READY bits=$bits ready=1","HEADLESS ROOT bits=$bits dll=1 child=1 child_exit=0",'CROSS BROKER loaded=0 target_exit=0','DIALOG_DRIVER exit=0')
    if(Test-Path -LiteralPath $env:WIN16S_TEST_HEADLESS_TRACE){Get-Content -LiteralPath $env:WIN16S_TEST_HEADLESS_TRACE}else{Write-Output "MEDIUM TRACE $arch unavailable"}
    Remove-Item Env:WIN16S_TEST_HEADLESS_TRACE
    if(!$MediumOnly){
        $gui=Join-Path $tree "$arch/global-e2e-target.exe"
        Invoke-Checked "owner-crash-$arch" $driver @('--private','--direct',$lifecycle,$launcher,$gui) @('GLOBAL LIFECYCLE PASS duplicate=10 crash=77 host_exit=1','DIALOG_DRIVER exit=0')
        Invoke-Checked "host-crash-$arch" $driver @('--private','--direct',$lifecycle,$launcher,$gui,'host-crash') @('GLOBAL HOST FAILURE PASS host=78 controller_exit=10 existing_alive=1','DIALOG_DRIVER exit=0')
    }
}
$after=Traces
$added=@($after | Where-Object { $_ -notin $before })
if($added.Count){$failures.Add("New ETW session remains: $($added -join ', ')")}
if($failures.Count){throw ($failures -join '; ')}
if($MediumOnly){Write-Output 'GLOBAL ELEVATED MEDIUM PASS widths=2 traces_clean=1'}
else{Write-Output 'GLOBAL ELEVATED PASS widths=2 medium=2 crash=4 orphan=2 traces_clean=1'}
