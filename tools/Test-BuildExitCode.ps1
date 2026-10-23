$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$root=Join-Path $repo 'build/t2-core/exit-code'
New-Item -ItemType Directory -Force -Path $root|Out-Null
foreach($code in @(1,-1)){
    $path=Join-Path $root ('exit-'+$code+'.cmd')
    $lines=@('@echo off',('cmd /d /c exit /b '+$code),'if not "%errorlevel%"=="0" exit /b 7','exit /b 0')
    [IO.File]::WriteAllLines($path,$lines,[Text.UTF8Encoding]::new($false))
    & $env:ComSpec /d /c ('"'+$path+'"')
    if($LASTEXITCODE -ne 7){throw "Failed to stop for native exit code $code"}
}
foreach($file in @('Build-Core.ps1','Probe-Toolchain.ps1')){
    if([IO.File]::ReadAllText((Join-Path $PSScriptRoot $file)) -match 'if errorlevel 1'){
        throw "Signed failure check remains: $file"
    }
}
Write-Output 'Build exit guard PASS: positive and negative native failures stop'
