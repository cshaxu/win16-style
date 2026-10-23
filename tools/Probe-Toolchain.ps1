[CmdletBinding()]
param([switch]$Worker, [ValidateRange(10,300)][int]$TimeoutSeconds = 120)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$scratch = Join-Path $repo 'build/t1-toolchain'
if (!$Worker) {
    if (Test-Path -LiteralPath $scratch) { throw 'Probe scratch already exists; inspect its owner/result before reuse' }
    New-Item -ItemType Directory -Path $scratch | Out-Null
    $arguments = @('-NoProfile', '-File', ('"' + $PSCommandPath + '"'), '-Worker', '-TimeoutSeconds', $TimeoutSeconds)
    $job = Start-Process (Join-Path $PSHOME 'pwsh.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $scratch 'worker.log') -RedirectStandardError (Join-Path $scratch 'worker.err')
    @{pid=$job.Id; scratch=$scratch; result=(Join-Path $scratch 'result.json'); timeout_seconds=$TimeoutSeconds} | ConvertTo-Json
    return
}
$resultPath = Join-Path $scratch 'result.json'
$child = $null
try {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    $vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs -or $LASTEXITCODE) { throw 'MSVC x86/x64 Build Tools not found' }
    $vcvars = Join-Path $vs 'VC/Auxiliary/Build/vcvarsall.bat'
    $cmake = (Get-Command cmake.exe -ErrorAction Stop).Source
    $ninja = (Get-Command ninja.exe -ErrorAction Stop).Source
    foreach ($value in @($repo,$scratch,$vcvars,$cmake,$ninja)) {
        if ($value -match '["%\r\n]') { throw 'Unsupported batch metacharacter in tool path' }
    }
    $commands = [System.Collections.Generic.List[string]]::new()
    $commands.Add('@echo off')
    foreach ($arch in @('x86','x64')) {
        $commands.Add('call "' + $vcvars + '" ' + $arch)
        $commands.Add('if errorlevel 1 exit /b 1')
        $commands.Add('"' + $cmake + '" -S "' + (Join-Path $repo 'tests/toolchain') + '" -B "' + (Join-Path $scratch $arch) + '" -G Ninja -DCMAKE_MAKE_PROGRAM="' + $ninja + '" -DCMAKE_BUILD_TYPE=Release -DPROBE_ARCH=' + $arch)
        $commands.Add('if errorlevel 1 exit /b 2')
        $commands.Add('"' + $cmake + '" --build "' + (Join-Path $scratch $arch) + '" --verbose')
        $commands.Add('if errorlevel 1 exit /b 3')
        $commands.Add('"' + (Join-Path (Split-Path $cmake) 'ctest.exe') + '" --test-dir "' + (Join-Path $scratch $arch) + '" --output-on-failure --verbose')
        $commands.Add('if errorlevel 1 exit /b 4')
    }
    $commands.Add('exit /b 0')
    $batch = Join-Path $scratch 'probe.cmd'
    [IO.File]::WriteAllLines($batch,$commands,[Text.UTF8Encoding]::new($false))
    $child = Start-Process $env:ComSpec -ArgumentList @('/d','/c',('"' + $batch + '"')) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $scratch 'run.log') -RedirectStandardError (Join-Path $scratch 'run.err')
    if (!$child.WaitForExit($TimeoutSeconds*1000)) {
        $child.Kill($true)
        $child.WaitForExit()
        throw 'Probe exceeded owned execution budget; terminated own child tree; result inconclusive'
    }
    if ($child.ExitCode -ne 0) { throw "Probe failed with exit code $($child.ExitCode); inspect run.log/run.err" }
    @{status='passed'; exit_code=$child.ExitCode; architectures=@('x86','x64'); timeout_seconds=$TimeoutSeconds} | ConvertTo-Json | Set-Content -LiteralPath $resultPath -Encoding utf8
} catch {
    @{status='failed-or-inconclusive'; error=$_.Exception.Message; timeout_seconds=$TimeoutSeconds} | ConvertTo-Json | Set-Content -LiteralPath $resultPath -Encoding utf8
    exit 1
}
