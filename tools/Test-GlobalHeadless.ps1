[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [string]$BuildRoot='build/.ninja'
)
$ErrorActionPreference='Stop'
$identity=[Security.Principal.WindowsIdentity]::GetCurrent()
$principal=[Security.Principal.WindowsPrincipal]::new($identity)
if(!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)){
    Write-Output 'GLOBAL HEADLESS SKIP: administrator token required'
    exit 77
}
$packagePath=(Resolve-Path -LiteralPath $Package).Path
$tree=(Resolve-Path -LiteralPath $BuildRoot).Path
$driver=Join-Path $tree 'x64/dialog-test-driver.exe'
$broker=Join-Path $tree 'x64/global-cross-origin.exe'
$launcher=Join-Path $packagePath 'run16s.exe'
foreach($arch in @('x86','x64')){
    $target=Join-Path $tree "$arch/global-propagation-probe.exe"
    $lines=@(& $driver --private --direct $broker $launcher $target headless 2>&1)
    $exit=$LASTEXITCODE
    $lines | ForEach-Object { Write-Output $_ }
    $bits=if($arch -eq 'x86'){32}else{64}
    $output=$lines -join "`n"
    foreach($marker in @("HEADLESS ROOT bits=$bits dll=1 child=1 child_exit=0",
                         "HEADLESS CHILD bits=$bits dll=1",'CROSS BROKER loaded=0 target_exit=0',
                         'DIALOG_DRIVER exit=0')){
        if(!$output.Contains($marker)){throw "Missing $arch marker: $marker"}
    }
    if($exit){throw "Headless $arch test exited $exit"}
}
Write-Output 'GLOBAL HEADLESS PASS widths=2'
