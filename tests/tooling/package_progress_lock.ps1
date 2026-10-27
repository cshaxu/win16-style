$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '../../tools/Set-PackageProgress.ps1')
$directory=$env:WIN16STYLE_CAPTURE_DIR
if (!$directory -or !(Test-Path -LiteralPath $directory -PathType Container)) { throw 'Missing owned capture directory' }
$path=Join-Path $directory 'progress-lock.json'
[IO.File]::WriteAllText($path,'old')
$reader=[IO.File]::Open($path,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::Read)
try {
    $oldFailed=$false
    try { Set-Content -LiteralPath $path -Value 'new' -ErrorAction Stop } catch [System.IO.IOException] { $oldFailed=$true }
    if (!$oldFailed) { throw 'Old progress writer did not reproduce the sharing violation' }
    if (Set-PackageProgress -Path $path -Json '{"value":1}') { throw 'Locked snapshot unexpectedly written' }
} finally { $reader.Dispose() }
if (!(Set-PackageProgress -Path $path -Json '{"value":2}')) { throw 'Snapshot did not recover after reader closed' }
if ((Get-Content -Raw -LiteralPath $path | ConvertFrom-Json).value -ne 2) { throw 'Recovered snapshot mismatch' }
$otherFailed=$false
try { Set-PackageProgress -Path (Join-Path $directory 'absent/progress.json') -Json '{}' | Out-Null } catch { $otherFailed=$true }
if (!$otherFailed) { throw 'Non-sharing I/O error was hidden' }
Write-Output 'PACKAGE_PROGRESS_LOCK PASS old-fails locked-skip recovered nonsharing-fails'
