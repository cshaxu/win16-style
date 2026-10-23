[CmdletBinding()]
param(
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='core',
    [ValidatePattern('^t[0-9]+-[a-z-]+$')][string]$TaskRoot='t2-core',
    [ValidatePattern('^[a-zA-Z0-9_/^$.*|()-]+$')][string]$TestPattern='^core/abi$',
    [switch]$Worker,
    [ValidateRange(20,600)][int]$TimeoutSeconds=180
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$scratch=Join-Path $repo "build/$TaskRoot/$RunName"
if(!$Worker){
    if(Test-Path -LiteralPath $scratch){throw 'Select a fresh run name; previous logs must be inspected before reuse'}
    New-Item -ItemType Directory -Force -Path $scratch | Out-Null
    $arguments=@('-NoProfile','-File',('"'+$PSCommandPath+'"'),'-Worker','-RunName',$RunName,'-TaskRoot',$TaskRoot,'-TestPattern',('"'+$TestPattern+'"'),'-TimeoutSeconds',$TimeoutSeconds)
    $job=Start-Process (Join-Path $PSHOME 'pwsh.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $scratch 'worker.log') -RedirectStandardError (Join-Path $scratch 'worker.err')
    @{pid=$job.Id;result=(Join-Path $scratch 'result.json');log=(Join-Path $scratch 'run.log');timeout_seconds=$TimeoutSeconds}|ConvertTo-Json
    return
}
try{
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    $vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if(!$vs -or $LASTEXITCODE){throw 'MSVC x86/x64 Build Tools missing'}
    $vcvars=Join-Path $vs 'VC/Auxiliary/Build/vcvarsall.bat'
    $cmake=(Get-Command cmake.exe -ErrorAction Stop).Source
    $ninja=(Get-Command ninja.exe -ErrorAction Stop).Source
    $ctest=Join-Path (Split-Path $cmake) 'ctest.exe'
    foreach($value in @($repo,$scratch,$vcvars,$cmake,$ninja)){
        if($value -match '["%\r\n]'){throw 'Unsupported batch metacharacter in path'}
    }
    $commands=[System.Collections.Generic.List[string]]::new()
    $commands.Add('@echo off')
    foreach($arch in @('x86','x64')){
        $tree=Join-Path $repo "build/.ninja/$arch"
        $evidence=Join-Path $scratch $arch
        New-Item -ItemType Directory -Force -Path $evidence | Out-Null
        $commands.Add('call "'+$vcvars+'" '+$arch)
        $commands.Add('if not "%errorlevel%"=="0" exit /b 1')
        $commands.Add('"'+$cmake+'" -S "'+$repo+'" -B "'+$tree+'" -G Ninja -DCMAKE_MAKE_PROGRAM="'+$ninja+'" -DCMAKE_BUILD_TYPE=Release -DWIN16STYLE_ARCH='+$arch)
        $commands.Add('if not "%errorlevel%"=="0" exit /b 2')
        $commands.Add('"'+$cmake+'" --build "'+$tree+'" --verbose')
        $commands.Add('if not "%errorlevel%"=="0" exit /b 3')
        $commands.Add('set "WIN16STYLE_CAPTURE_DIR='+$evidence+'"')
        $commands.Add('"'+$ctest+'" --test-dir "'+$tree+'" -R "'+$TestPattern+'" --output-on-failure --verbose --no-tests=error')
        $commands.Add('if not "%errorlevel%"=="0" exit /b 4')
        $commands.Add('python "'+(Join-Path $repo 'tools/check_core_binary.py')+'" --arch '+$arch)
        $commands.Add('if not "%errorlevel%"=="0" exit /b 5')
    }
    $commands.Add('exit /b 0')
    $batch=Join-Path $scratch 'build.cmd'
    [IO.File]::WriteAllLines($batch,$commands,[Text.UTF8Encoding]::new($false))
    $child=Start-Process $env:ComSpec -ArgumentList @('/d','/c',('"'+$batch+'"')) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $scratch 'run.log') -RedirectStandardError (Join-Path $scratch 'run.err')
    if(!$child.WaitForExit($TimeoutSeconds*1000)){
        $child.Kill($true);$child.WaitForExit();throw 'Owned build/test budget exceeded; inconclusive'
    }
    if($child.ExitCode -ne 0){throw "Build/test exit $($child.ExitCode); inspect run.log/run.err"}
    @{status='passed';exit_code=0;architectures=@('x86','x64');selection=$TestPattern}|ConvertTo-Json|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
}catch{
    @{status='failed-or-inconclusive';error=$_.Exception.Message}|ConvertTo-Json|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    exit 1
}
