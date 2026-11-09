param([Parameter(Mandatory=$true)][string]$BuildDirectory,[string]$PackageDirectory,[string]$CapturePath,[switch]$InspectSession,[switch]$CheckRemove,[switch]$CheckCheckbox,[switch]$CheckRadio,[switch]$ExistingRoot,[switch]$Preexisting,[switch]$RemoveBridge,[switch]$KeepFixture)
$CheckCheckbox=$CheckCheckbox -or $CheckRadio
$ErrorActionPreference='Stop'
$build=(Resolve-Path -LiteralPath $BuildDirectory).Path
$package=if($PackageDirectory){(Resolve-Path -LiteralPath $PackageDirectory).Path}else{$build}
$doc=Join-Path $build ("xaml-owned-"+[Guid]::NewGuid().ToString('N')+".txt")
[IO.File]::WriteAllText($doc,'')
$target=$null; $parent=$null; $launch=$null
try {
    $launch=Start-Process (Join-Path $package 'run16s.exe') -ArgumentList @('notepad.exe',"`"$doc`"") -WindowStyle Hidden -PassThru
    if (-not $launch.WaitForExit(10000)) { throw 'launcher timed out' }
    if ($launch.ExitCode -ne 0) { throw "launcher exit $($launch.ExitCode)" }
    $deadline=[DateTime]::UtcNow.AddSeconds(10)
    while ([DateTime]::UtcNow -lt $deadline -and -not $target) {
        $target=Get-CimInstance Win32_Process -Filter "Name='Notepad.exe'" | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($doc) } | Select-Object -First 1
        if (-not $target) { Start-Sleep -Milliseconds 100 }
    }
    if (-not $target) { Write-Output 'SKIP target_not_observed'; exit 77 }
    $parent=$target
    $deadline=[DateTime]::UtcNow.AddSeconds(8)
    while ([DateTime]::UtcNow -lt $deadline) {
        $child=Get-CimInstance Win32_Process -Filter "ParentProcessId=$($parent.ProcessId) AND Name='Notepad.exe'" | Select-Object -First 1
        if ($child) { $target=$child; break }
        Start-Sleep -Milliseconds 100
    }
    if ($target.ProcessId -eq $parent.ProcessId) { Write-Output 'SKIP child_not_observed'; exit 77 }
    $process=Get-Process -Id $target.ProcessId
    $deadline=[DateTime]::UtcNow.AddSeconds(8); $found=$false
    while ([DateTime]::UtcNow -lt $deadline -and -not $process.HasExited -and -not $found) {
        $process.Refresh()
        $found=@($process.Modules | Where-Object { $_.FileName -ieq (Join-Path $package 'ntw16s64.dll') }).Count -gt 0
        if (-not $found) { Start-Sleep -Milliseconds 100 }
    }
    Write-Output "target=$($target.ProcessId) dll_loaded=$found alive=$(-not $process.HasExited)"
    if (-not $found -or $process.HasExited) { exit 1 }
    if ($InspectSession) {
        Write-Output "runtime_modules=$((@($process.Modules | Where-Object { $_.ModuleName -match 'Xaml|FrameworkUdk' } | ForEach-Object ModuleName)) -join ',')"
        Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class XamlMemory {
    [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr OpenProcess(uint rights,bool inherit,int pid);
    [DllImport("kernel32.dll", SetLastError=true)] public static extern bool ReadProcessMemory(IntPtr process,IntPtr address,byte[] bytes,UIntPtr size,out UIntPtr read);
    [DllImport("kernel32.dll")] public static extern bool CloseHandle(IntPtr handle);
}
'@
        $dll=Join-Path $package 'ntw16s64.dll'
        $compiler=((Select-String -LiteralPath (Join-Path $build 'CMakeCache.txt') -Pattern '^CMAKE_C_COMPILER:(FILEPATH|STRING)=').Line -split '=',2)[1]
        $nm=Join-Path (Split-Path $compiler) 'nm.exe'
        $symbols=& $nm $dll
        $line=$symbols | Where-Object { $_ -match '^([0-9a-fA-F]+)\s+[bB]\s+active_site$' } | Select-Object -First 1
        if (-not $line -or $line -notmatch '^([0-9a-fA-F]+)') { throw 'active_site symbol missing' }
        $symbol=[Convert]::ToUInt64($Matches[1],16)
        $reader=[IO.BinaryReader]::new([IO.File]::OpenRead($dll))
        try {
            $reader.BaseStream.Position=60; $pe=$reader.ReadUInt32()
            $reader.BaseStream.Position=$pe+24+24; $preferred=$reader.ReadUInt64()
        } finally { $reader.Dispose() }
        if ($symbol -lt $preferred) { throw 'invalid symbol address' }
        $module=$process.Modules | Where-Object { $_.FileName -ieq $dll } | Select-Object -First 1
        $address=[UInt64]$module.BaseAddress.ToInt64()+($symbol-$preferred)
        $handle=[XamlMemory]::OpenProcess(0x410,$false,$process.Id)
        if ($handle -eq [IntPtr]::Zero) { throw "OpenProcess failed $([Runtime.InteropServices.Marshal]::GetLastWin32Error())" }
        try {
            $value=0; $deadline=[DateTime]::UtcNow.AddSeconds(6)
            do {
                $bytes=[byte[]]::new(8); $read=[UIntPtr]::Zero
                if (-not [XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]$address),$bytes,[UIntPtr]::new(8),[ref]$read) -or $read.ToUInt64() -ne 8) { throw "ReadProcessMemory failed $([Runtime.InteropServices.Marshal]::GetLastWin32Error())" }
                $value=[BitConverter]::ToUInt64($bytes,0)
                if (-not $value) { Start-Sleep -Milliseconds 100 }
            } while (-not $value -and [DateTime]::UtcNow -lt $deadline)
            Write-Output "session_connected=$($value -ne 0)"
            $process.Refresh()
            Write-Output "final_runtime_modules=$((@($process.Modules | Where-Object { $_.ModuleName -match 'Xaml|FrameworkUdk' } | ForEach-Object ModuleName)) -join ',')"
            if (-not $value) { exit 2 }
            $line=$symbols | Where-Object { $_ -match '^([0-9a-fA-F]+)\s+[bB]\s+adapter$' } | Select-Object -First 1
            if (-not $line -or $line -notmatch '^([0-9a-fA-F]+)') { throw 'adapter symbol missing' }
            $adapter=[UInt64]$module.BaseAddress.ToInt64()+([Convert]::ToUInt64($Matches[1],16)-$preferred)
            $layout=(& (Join-Path $build 'xaml-adapter-layout.exe')) -split ' '
            if ($layout.Count -ne 6) { throw 'adapter layout unavailable' }
            $firstOffset=[UInt64]$layout[0]; $busyOffset=[UInt64]$layout[1]; $styleOffset=[UInt64]$layout[2]
            $styled=$false; $deadline=[DateTime]::UtcNow.AddSeconds(5)
            do {
                $bytes=[byte[]]::new(8); $read=[UIntPtr]::Zero
                if ([XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]($adapter+$firstOffset)),$bytes,[UIntPtr]::new(8),[ref]$read) -and $read.ToUInt64() -eq 8) {
                    $entry=[BitConverter]::ToUInt64($bytes,0)
                    if ($entry) {
                        $busy=[byte[]]::new(4); $style=[byte[]]::new(8)
                        $ok=[XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]($entry+$busyOffset)),$busy,[UIntPtr]::new(4),[ref]$read)
                        $ok=$ok -and [XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]($entry+$styleOffset)),$style,[UIntPtr]::new(8),[ref]$read)
                        $styled=$ok -and [BitConverter]::ToInt32($busy,0) -eq 0 -and [BitConverter]::ToUInt64($style,0) -ne 0
                    }
                }
                if (-not $styled) { Start-Sleep -Milliseconds 100 }
            } while (-not $styled -and [DateTime]::UtcNow -lt $deadline)
            Write-Output "button_style_recorded=$styled"
            $errorLine=$symbols | Where-Object { $_ -match '^([0-9a-fA-F]+)\s+[bB]\s+last_xaml_error$' } | Select-Object -First 1
            if ($errorLine -and $errorLine -match '^([0-9a-fA-F]+)') {
                $errorAddress=[UInt64]$module.BaseAddress.ToInt64()+([Convert]::ToUInt64($Matches[1],16)-$preferred)
                $errorBytes=[byte[]]::new(4); $read=[UIntPtr]::Zero
                if ([XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]$errorAddress),$errorBytes,[UIntPtr]::new(4),[ref]$read)) {
                    Write-Output ('last_xaml_error=0x{0:X8}' -f [BitConverter]::ToUInt32($errorBytes,0))
                }
            }
            $ownerBytes=[byte[]]::new(4); $read=[UIntPtr]::Zero
            if ([XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]($adapter+[UInt64]$layout[3])),$ownerBytes,[UIntPtr]::new(4),[ref]$read)) {
                Write-Output "button_thread=$([BitConverter]::ToUInt32($ownerBytes,0))"
            }
            if (-not $styled -and -not $CheckRemove -and -not $CheckCheckbox) { exit 3 }
            if ($CheckRemove -or $CheckCheckbox) {
                if ($CheckCheckbox) { $mode=if($CheckRadio){'--radio'}elseif($KeepFixture){'--keep-fixture'}elseif($RemoveBridge){'--no-bridge'}elseif($Preexisting){'--preexisting'}elseif($ExistingRoot){'--existing'}else{'--checkbox'}; & (Join-Path $build 'xaml-remove-window.exe') $process.Id $dll (Join-Path $build 'libxaml-property-probe.dll') $adapter $mode }
                elseif ($styled) { & (Join-Path $build 'xaml-remove-window.exe') $process.Id $dll (Join-Path $build 'libxaml-property-probe.dll') $adapter }
                else { & (Join-Path $build 'xaml-remove-window.exe') $process.Id $dll }
                if ($LASTEXITCODE -ne 0) {
                    Write-Output "remove_probe_exit=$LASTEXITCODE"; exit $LASTEXITCODE
                }
                $stopped=$false; $deadline=[DateTime]::UtcNow.AddSeconds(5)
                do {
                    $siteBytes=[byte[]]::new(8); $firstBytes=[byte[]]::new(8); $read=[UIntPtr]::Zero
                    $ok=[XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]$address),$siteBytes,[UIntPtr]::new(8),[ref]$read)
                    $ok=$ok -and [XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]($adapter+$firstOffset)),$firstBytes,[UIntPtr]::new(8),[ref]$read)
                    $stopped=$ok -and [BitConverter]::ToUInt64($siteBytes,0) -eq 0 -and [BitConverter]::ToUInt64($firstBytes,0) -eq 0
                    if (-not $stopped) { Start-Sleep -Milliseconds 100 }
                } while (-not $stopped -and [DateTime]::UtcNow -lt $deadline)
                Write-Output "session_and_records_cleared=$stopped"
                if (-not $stopped -and $ok) {
                    Write-Output ('session_pointer=0x{0:X} first_record=0x{1:X}' -f [BitConverter]::ToUInt64($siteBytes,0),[BitConverter]::ToUInt64($firstBytes,0))
                    foreach($field in @(@('thread',$layout[3]),@('connected',$layout[4]),@('stopping',$layout[5]))) {
                        $raw=[byte[]]::new(4)
                        if([XamlMemory]::ReadProcessMemory($handle,[IntPtr]::new([long]($adapter+[UInt64]$field[1])),$raw,[UIntPtr]::new(4),[ref]$read)){
                            Write-Output "$($field[0])=$([BitConverter]::ToUInt32($raw,0))"
                        }
                    }
                }
                if (-not $stopped) { exit 4 }
                if (-not $styled -and -not $CheckCheckbox) { Write-Output 'SKIP no_styled_button_to_restore'; exit 77 }
            }
        } finally { $null=[XamlMemory]::CloseHandle($handle) }
    }
    if ($CapturePath) {
        Add-Type -AssemblyName System.Drawing
        Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class XamlCapture {
    [StructLayout(LayoutKind.Sequential)] public struct Rect { public int Left,Top,Right,Bottom; }
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr window);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr window,out Rect rect);
}
'@
        $deadline=[DateTime]::UtcNow.AddSeconds(5); $window=[IntPtr]::Zero
        while ([DateTime]::UtcNow -lt $deadline -and $window -eq [IntPtr]::Zero) {
            $process.Refresh(); $window=$process.MainWindowHandle
            if ($window -eq [IntPtr]::Zero) { Start-Sleep -Milliseconds 100 }
        }
        if ($window -eq [IntPtr]::Zero -or -not [XamlCapture]::SetForegroundWindow($window)) { Write-Output 'capture=foreground_unavailable'; exit 77 }
        Start-Sleep -Milliseconds 1500
        if ([XamlCapture]::GetForegroundWindow() -ne $window) { Write-Output 'capture=foreground_lost'; exit 77 }
        $rect=[XamlCapture+Rect]::new()
        if (-not [XamlCapture]::GetWindowRect($window,[ref]$rect) -or $rect.Right -le $rect.Left -or $rect.Bottom -le $rect.Top) { throw 'invalid window rectangle' }
        $bitmap=[Drawing.Bitmap]::new($rect.Right-$rect.Left,$rect.Bottom-$rect.Top)
        try {
            $graphics=[Drawing.Graphics]::FromImage($bitmap)
            try { $graphics.CopyFromScreen($rect.Left,$rect.Top,0,0,$bitmap.Size) } finally { $graphics.Dispose() }
            $bitmap.Save($CapturePath,[Drawing.Imaging.ImageFormat]::Png)
        } finally { $bitmap.Dispose() }
        Write-Output "capture=$CapturePath"
    }
} finally {
    if ($target) {
        $current=Get-CimInstance Win32_Process -Filter "ProcessId=$($target.ProcessId)" -ErrorAction SilentlyContinue
        if ($current -and $current.CreationDate -eq $target.CreationDate -and $parent -and $current.ParentProcessId -eq $parent.ProcessId -and $current.Name -eq 'Notepad.exe') {
            $process=Get-Process -Id $target.ProcessId -ErrorAction SilentlyContinue
            if ($process) { $null=$process.CloseMainWindow(); if (-not $process.WaitForExit(2000)) { Stop-Process -Id $process.Id -Force } }
        }
    }
    if ($parent) {
        $current=Get-CimInstance Win32_Process -Filter "ProcessId=$($parent.ProcessId)" -ErrorAction SilentlyContinue
        if ($current -and $current.CreationDate -eq $parent.CreationDate -and $current.CommandLine -and $current.CommandLine.Contains($doc)) {
            $process=Get-Process -Id $parent.ProcessId -ErrorAction SilentlyContinue
            if ($process) { $null=$process.CloseMainWindow(); if (-not $process.WaitForExit(2000)) { Stop-Process -Id $process.Id -Force } }
        }
    }
    if ($launch -and -not $launch.HasExited) { Stop-Process -Id $launch.Id -Force }
    Remove-Item -LiteralPath $doc -Force -ErrorAction SilentlyContinue
}
