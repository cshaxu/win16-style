[CmdletBinding()]
param(
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='core',
    [ValidatePattern('^t[0-9]+-[a-z-]+$')][string]$TaskRoot='t6-package',
    [ValidatePattern('^[a-zA-Z0-9_/^$.*|()-]+$')][string]$TestPattern='^core/abi$',
    [string]$BuildRoot='build/.ninja',
    [string]$Gcc32=$env:WIN16STYLE_GCC32,
    [string]$Gcc64=$env:WIN16STYLE_GCC64,
    [switch]$Worker,
    [ValidateRange(20,600)][int]$TimeoutSeconds=180
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$scratch=Join-Path $repo "build/$TaskRoot/$RunName"
if(!$Worker){
    if(Test-Path -LiteralPath $scratch){throw 'Select a fresh run name; inspect previous logs before reuse'}
    New-Item -ItemType Directory -Force -Path $scratch | Out-Null
    $arguments=@('-NoProfile','-File',('"'+$PSCommandPath+'"'),'-Worker','-RunName',$RunName,'-TaskRoot',$TaskRoot,'-TestPattern',('"'+$TestPattern+'"'),'-BuildRoot',('"'+$BuildRoot+'"'),'-TimeoutSeconds',$TimeoutSeconds)
    if($Gcc32){$arguments+=@('-Gcc32',('"'+$Gcc32+'"'))}
    if($Gcc64){$arguments+=@('-Gcc64',('"'+$Gcc64+'"'))}
    $job=Start-Process (Join-Path $PSHOME 'pwsh.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $scratch 'worker.log') -RedirectStandardError (Join-Path $scratch 'worker.err')
    @{pid=$job.Id;result=(Join-Path $scratch 'result.json');log=(Join-Path $scratch 'run.log');timeout_seconds_per_architecture=$TimeoutSeconds;maximum_total_seconds=(2*$TimeoutSeconds)}|ConvertTo-Json
    return
}
function Resolve-Gcc([string]$Explicit,[string]$Machine,[string]$Cache){
    $candidates=[System.Collections.Generic.List[string]]::new()
    if($Explicit){$candidates.Add($Explicit)}else{
        foreach($command in @(Get-Command gcc.exe -All -ErrorAction SilentlyContinue)){$candidates.Add($command.Source)}
        $candidates.Add((Join-Path (Split-Path $Cache) 'gcc.exe'))
        foreach($prefix in @('C:/msys64/mingw32','C:/msys64/mingw64','C:/msys64/ucrt64')){$candidates.Add((Join-Path $prefix 'bin/gcc.exe'))}
    }
    foreach($candidate in @($candidates | Select-Object -Unique)){
        if(!(Test-Path -LiteralPath $candidate -PathType Leaf)){continue}
        $target=(& $candidate -dumpmachine | Out-String).Trim()
        if(!$LASTEXITCODE -and $target -eq $Machine){return (Resolve-Path -LiteralPath $candidate).Path.Replace('\','/')}
    }
    throw "Native GCC $Machine unavailable; set WIN16STYLE_GCC32/64 or -Gcc32/-Gcc64"
}
function CMake-Value([string]$Value){
    if($Value.Contains(']==]') -or $Value -match '[\r\n]'){throw 'Unsupported CMake value'}
    return '[==['+$Value+']==]'
}
try{
    # Build-Package invokes -Worker synchronously in a new copied tree.
    # The asynchronous public entry creates this directory before dispatch.
    New-Item -ItemType Directory -Force -Path $scratch | Out-Null
    $cmake=(Get-Command cmake.exe -ErrorAction Stop).Source.Replace('\','/')
    $ninja=(Get-Command ninja.exe -ErrorAction Stop).Source.Replace('\','/')
    $cache=(Get-Command ccache.exe -ErrorAction Stop).Source.Replace('\','/')
    $ctest=(Join-Path (Split-Path $cmake) 'ctest.exe').Replace('\','/')
    $gcc=@{x86=(Resolve-Gcc $Gcc32 'i686-w64-mingw32' $cache);x64=(Resolve-Gcc $Gcc64 'x86_64-w64-mingw32' $cache)}
    $build=[IO.Path]::GetFullPath($(if([IO.Path]::IsPathRooted($BuildRoot)){$BuildRoot}else{Join-Path $repo $BuildRoot}))
    $allowed=[IO.Path]::GetFullPath((Join-Path $repo 'build'))+[IO.Path]::DirectorySeparatorChar
    if(!$build.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw 'BuildRoot must remain under this repository build/'}
    for($path=$build;$path -and $path -ne $repo;$path=Split-Path $path -Parent){
        if((Test-Path -LiteralPath $path) -and ((Get-Item -LiteralPath $path -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)){
            throw 'Build root traverses a reparse point'
        }
    }
    [IO.File]::WriteAllText((Join-Path $scratch 'run.log'),'')
    [IO.File]::WriteAllText((Join-Path $scratch 'run.err'),'')
    $toolchains=foreach($arch in @('x86','x64')){
        $commands=[System.Collections.Generic.List[string]]::new();$commands.Add('@echo off')
        $commands.Add('set "CCACHE_DIR='+(Join-Path $repo 'build/.ccache')+'"')
        $compiler=$gcc[$arch];$bin=Split-Path $compiler
        $windres=(Join-Path $bin 'windres.exe').Replace('\','/')
        if(!(Test-Path -LiteralPath $windres)){throw "Matching windres missing: $arch"}
        $tree=Join-Path $build $arch;$evidence=Join-Path $scratch $arch
        New-Item -ItemType Directory -Force -Path $evidence | Out-Null
        $toolchain=Join-Path $evidence 'toolchain.cmake'
        $rcFlags='--preprocessor="'+$compiler+'" --preprocessor-arg=-E --preprocessor-arg=-xc --preprocessor-arg=-DRC_INVOKED'
        $settings=@(
            ('set(CMAKE_C_COMPILER '+(CMake-Value $compiler)+')'),
            ('set(CMAKE_RC_COMPILER '+(CMake-Value $windres)+')'),
            ('set(CMAKE_C_COMPILER_LAUNCHER '+(CMake-Value $cache)+')'),
            ('set(CMAKE_RC_FLAGS '+(CMake-Value $rcFlags)+')')
        )
        [IO.File]::WriteAllLines($toolchain,$settings,[Text.UTF8Encoding]::new($false))
        foreach($value in @($repo,$scratch,$tree,$compiler,$toolchain,$cmake,$ninja,$ctest)){
            if($value -match '["%\r\n]'){throw 'Unsupported batch metacharacter in path'}
        }
        $fresh=''
        $cacheFile=Join-Path $tree 'CMakeCache.txt'
        if(Test-Path -LiteralPath $cacheFile){
            $old=Select-String -LiteralPath $cacheFile -Pattern '^CMAKE_C_COMPILER:[^=]+=(.+)$'
            if(!$old -or $old.Matches[0].Groups[1].Value.Replace('\','/') -ne $compiler){$fresh=' --fresh'}
        }
        $commands.Add('"'+$cmake+'"'+$fresh+' -S "'+$repo+'" -B "'+$tree+'" -G Ninja -DCMAKE_TOOLCHAIN_FILE="'+$toolchain.Replace('\','/')+'" -DCMAKE_MAKE_PROGRAM="'+$ninja+'" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DWIN16STYLE_ARCH='+$arch+' -DWIN16STYLE_X86_BUILD="'+(Join-Path $build 'x86').Replace('\','/')+'"')
        $commands.Add('if not "%errorlevel%"=="0" exit /b 2')
        $commands.Add('"'+$cmake+'" --build "'+$tree+'" --parallel 4')
        $commands.Add('if not "%errorlevel%"=="0" exit /b 3')
        $commands.Add('set "WIN16STYLE_CAPTURE_DIR='+$evidence+'"')
        $commands.Add('"'+$ctest+'" --test-dir "'+$tree+'" -R "'+$TestPattern+'" --output-on-failure --verbose --no-tests=error --output-junit "'+(Join-Path $evidence 'results.xml')+'"')
        $commands.Add('if not "%errorlevel%"=="0" exit /b 4')
        $commands.Add('python "'+(Join-Path $repo 'tools/check_core_binary.py')+'" --arch '+$arch+' --build-root "'+$build+'"')
        $commands.Add('if not "%errorlevel%"=="0" exit /b 5')
        $commands.Add('exit /b 0');$batch=Join-Path $evidence 'build.cmd'
        [IO.File]::WriteAllLines($batch,$commands,[Text.UTF8Encoding]::new($false))
        $child=Start-Process $env:ComSpec -ArgumentList @('/d','/c',('"'+$batch+'"')) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $evidence 'run.log') -RedirectStandardError (Join-Path $evidence 'run.err')
        @{architecture=$arch;pid=$child.Id;timeout_seconds=$TimeoutSeconds}|ConvertTo-Json|Set-Content (Join-Path $evidence 'process.json')
        try{
            if(!$child.WaitForExit($TimeoutSeconds*1000)){$child.Kill($true);$child.WaitForExit();throw "Owned $arch build/test budget exceeded; inconclusive"}
            if($child.ExitCode -ne 0){throw "$arch build/test exit $($child.ExitCode); inspect architecture run.log/run.err"}
        }finally{
            [IO.File]::AppendAllText((Join-Path $scratch 'run.log'),[IO.File]::ReadAllText((Join-Path $evidence 'run.log')))
            [IO.File]::AppendAllText((Join-Path $scratch 'run.err'),[IO.File]::ReadAllText((Join-Path $evidence 'run.err')))
        }
        @{architecture=$arch;compiler=$compiler;target=(& $compiler -dumpmachine);version=(& $compiler -dumpfullversion);compiler_sha256=(Get-FileHash -LiteralPath $compiler -Algorithm SHA256).Hash.ToLowerInvariant()}
    }
    @{toolchains=@($toolchains);ninja=$ninja;ccache=$cache;ccache_version=(& $cache --version | Select-Object -First 1)}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'toolchains.json') -Encoding utf8
    $skippedCount=0
    $summaries=foreach($arch in @('x86','x64')){
        [xml]$report=Get-Content -Raw (Join-Path $scratch "$arch/results.xml")
        $cases=@($report.SelectNodes('//testcase'));$skipped=@($report.SelectNodes('//testcase[skipped]'))
        if(!$cases.Count -or $report.SelectNodes('//testcase[failure or error]').Count){throw 'Invalid or failed CTest report'}
        $skippedCount+=$skipped.Count
        @{architecture=$arch;tests=$cases.Count;passed=($cases.Count-$skipped.Count);skipped=@($skipped | ForEach-Object { $_.name })}
    }
    $status=if($skippedCount){'passed-with-skips'}else{'passed'}
    @{status=$status;exit_code=0;architectures=@('x86','x64');selection=$TestPattern;tests=$summaries;build_root=$build}|ConvertTo-Json -Depth 5|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
}catch{
    @{status='failed-or-inconclusive';error=$_.Exception.Message}|ConvertTo-Json|Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    exit 1
}
