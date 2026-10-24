[CmdletBinding()]
param(
    [ValidatePattern('^[a-z0-9-]+$')][string]$RunName='candidate',
    [ValidateRange(1,65535)][int]$Task=8,
    [switch]$Worker
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$scratch=Join-Path $repo "build/t$Task-package/$RunName"
if(!$Worker){
    if(Test-Path -LiteralPath $scratch){throw 'Select a fresh run name; inspect the previous result first'}
    New-Item -ItemType Directory -Path $scratch | Out-Null
    $arguments=@('-NoProfile','-File',('"'+$PSCommandPath+'"'),'-Worker','-RunName',$RunName,'-Task',$Task)
    $job=Start-Process (Join-Path $PSHOME 'pwsh.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $scratch 'worker.log') -RedirectStandardError (Join-Path $scratch 'worker.err')
    @{pid=$job.Id;result=(Join-Path $scratch 'result.json');candidate=(Join-Path $scratch 'candidate');timeout_seconds_per_build_architecture=600;maximum_build_seconds=1200}|ConvertTo-Json
    return
}
try{
    $clean=Join-Path $scratch 'clean-source'
    $candidate=Join-Path $scratch 'candidate'
    if((Test-Path -LiteralPath $clean) -or (Test-Path -LiteralPath $candidate)){throw 'Worker destination exists'}
    New-Item -ItemType Directory -Path $clean,$candidate | Out-Null
    $files=[System.Collections.Generic.List[object]]::new()
    $roots=@('src','tests','tools','docs')
    foreach($directory in $roots){
        $source=Join-Path $repo $directory
        $items=@(Get-Item -LiteralPath $source)+@(Get-ChildItem -LiteralPath $source -Recurse -Force)
        if($items | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }){throw 'Source reparse point'}
        foreach($file in $items | Where-Object { !$_.PSIsContainer }){$files.Add($file)}
    }
    if($files | Where-Object { $_.Extension -in '.cpp','.cxx','.cc' }){throw 'Pure C package cannot contain C++ sources'}
    foreach($name in @('CMakeLists.txt','LICENSE','README.md','AGENTS.md','CONTRIBUTING.md','.gitignore','.gitattributes','assets/README.md')){
        $file=Get-Item -LiteralPath (Join-Path $repo $name)
        if($file.Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Source reparse file'}
        $files.Add($file)
    }
    $snapshot=foreach($file in $files){
        $relative=[IO.Path]::GetRelativePath($repo,$file.FullName)
        $destination=Join-Path $clean $relative
        New-Item -ItemType Directory -Force -Path (Split-Path $destination -Parent) | Out-Null
        Copy-Item -LiteralPath $file.FullName -Destination $destination
        $hash=(Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()
        if($hash -ne (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()){throw 'Snapshot copy changed'}
        @{path=$relative;sha256=$hash}
    }
    $snapshot | ConvertTo-Json -Depth 3 | Set-Content (Join-Path $scratch 'snapshot.json') -Encoding utf8
    Push-Location $clean
    try{
        & python (Join-Path $clean 'tools/verify_source_manifest.py') --self-test
        if($LASTEXITCODE){throw 'Snapshot source gate failed'}
        & python (Join-Path $clean 'tools/verify_resource_identity.py') --self-test
        if($LASTEXITCODE){throw 'Snapshot glyph gate failed'}
        & (Join-Path $clean 'tools/Verify-Governance.ps1') -SelfTest
        & (Join-Path $clean 'tools/Build-Core.ps1') -Worker -TaskRoot "t$Task-package" -RunName clean-build -TestPattern '^core/abi$' -TimeoutSeconds 600
        $build=Get-Content -Raw (Join-Path $clean "build/t$Task-package/clean-build/result.json") | ConvertFrom-Json
        if($build.status -ne 'passed' -or $build.exit_code -ne 0){throw 'Clean build failed'}
        $tree=Join-Path $clean 'build/.ninja'
        $toolchains=Get-Content -Raw (Join-Path $clean "build/t$Task-package/clean-build/toolchains.json") | ConvertFrom-Json
        $compilerRoots=@($toolchains.toolchains | ForEach-Object {
            [IO.Path]::GetFullPath((Split-Path (Split-Path $_.compiler -Parent) -Parent))+[IO.Path]::DirectorySeparatorChar
        })
        foreach($pair in @(@('x64','run16s.exe'),@('x86','ntw16s32.dll'),@('x64','ntw16s64.dll'),@('x64','demo.exe'))){
            Copy-Item -LiteralPath (Join-Path $tree "$($pair[0])/$($pair[1])") -Destination (Join-Path $candidate $pair[1])
        }
        foreach($arch in @('x86','x64')){
            $dependencies=& ninja -C (Join-Path $tree $arch) -t deps
            if($LASTEXITCODE){throw 'Dependency trace failed'}
            $dependencies | Set-Content (Join-Path $scratch "$arch-deps.txt") -Encoding utf8
            foreach($line in $dependencies){
                if(!$line.StartsWith('    ')){continue}
                $inputPath=[IO.Path]::GetFullPath($line.Trim(),(Join-Path $tree $arch))
                if($inputPath.StartsWith($clean+'\',[StringComparison]::OrdinalIgnoreCase)){continue}
                $allowed=@($compilerRoots | Where-Object { $inputPath.StartsWith($_,[StringComparison]::OrdinalIgnoreCase) })
                if(!$allowed.Count){throw "Foreign build input: $inputPath"}
            }
        }
        $verification=& python (Join-Path $clean 'tools/check_package.py') $candidate --source-root $clean --built-root $tree --task $Task
        if($LASTEXITCODE){throw 'Candidate package gate failed'}
        $verification | Set-Content (Join-Path $scratch 'package.json') -Encoding utf8
    }finally{Pop-Location}
    @{status='passed';exit_code=0;candidate=$candidate;clean_source=$clean;physical_acceptance='pending';full_matrix='pending'} | ConvertTo-Json | Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
}catch{
    @{status='failed-or-inconclusive';error=$_.Exception.Message} | ConvertTo-Json | Set-Content (Join-Path $scratch 'result.json') -Encoding utf8
    exit 1
}
