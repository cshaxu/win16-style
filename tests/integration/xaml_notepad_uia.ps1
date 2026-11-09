param([Parameter(Mandatory=$true)][string]$BuildDirectory,[switch]$ListMenus,[switch]$ExpandEdit,[switch]$VerifySettings)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName UIAutomationClient
$build=(Resolve-Path -LiteralPath $BuildDirectory).Path
$doc=Join-Path $build ('uia-owned-'+[guid]::NewGuid().ToString('N')+'.txt')
[IO.File]::WriteAllText($doc,'')
$parent=$null; $child=$null
try {
    $launcher=Start-Process (Join-Path $build 'run16s.exe') -ArgumentList @('notepad.exe',('"'+$doc+'"')) -WindowStyle Hidden -PassThru
    if(-not $launcher.WaitForExit(10000) -or $launcher.ExitCode -ne 0){throw 'launcher failed'}
    $deadline=[DateTime]::UtcNow.AddSeconds(10)
    do {
        $parent=Get-CimInstance Win32_Process -Filter "Name='Notepad.exe'" | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($doc) } | Select-Object -First 1
        if(-not $parent){Start-Sleep -Milliseconds 100}
    } while(-not $parent -and [DateTime]::UtcNow -lt $deadline)
    if(-not $parent){throw 'owned parent not found'}
    $deadline=[DateTime]::UtcNow.AddSeconds(8)
    do {
        $child=Get-CimInstance Win32_Process -Filter "ParentProcessId=$($parent.ProcessId) AND Name='Notepad.exe'" | Select-Object -First 1
        if(-not $child){Start-Sleep -Milliseconds 100}
    } while(-not $child -and [DateTime]::UtcNow -lt $deadline)
    if(-not $child){throw 'owned child not found'}
    $condition=[System.Windows.Automation.PropertyCondition]::new([System.Windows.Automation.AutomationElement]::ProcessIdProperty,[int]$child.ProcessId)
    $deadline=[DateTime]::UtcNow.AddSeconds(8); $window=$null
    do {
        $window=[System.Windows.Automation.AutomationElement]::RootElement.FindFirst([System.Windows.Automation.TreeScope]::Children,$condition)
        if(-not $window){Start-Sleep -Milliseconds 100}
    } while(-not $window -and [DateTime]::UtcNow -lt $deadline)
    if(-not $window){throw 'owned window unavailable'}
    if($ListMenus -or $ExpandEdit){
        $menuCondition=[System.Windows.Automation.PropertyCondition]::new([System.Windows.Automation.AutomationElement]::ControlTypeProperty,[System.Windows.Automation.ControlType]::MenuItem)
        $deadline=[DateTime]::UtcNow.AddSeconds(5); $menus=@()
        do {
            $menus=@($window.FindAll([System.Windows.Automation.TreeScope]::Descendants,$menuCondition))
            if(-not $menus.Count){Start-Sleep -Milliseconds 100}
        } while(-not $menus.Count -and [DateTime]::UtcNow -lt $deadline)
        foreach($item in $menus){Write-Output ('menu name="{0}" id="{1}"' -f $item.Current.Name,$item.Current.AutomationId)}
        if(-not $menus.Count){exit 77}
        if($ExpandEdit){
            $edit=@($menus | Where-Object {$_.Current.AutomationId -eq 'Edit'})
            if($edit.Count -ne 1){throw 'Edit menu not unique'}
            $edit[0].GetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern).Expand()
            Start-Sleep -Milliseconds 300
            $ownedMenu=[System.Windows.Automation.AndCondition]::new($condition,$menuCondition)
            $expanded=@([System.Windows.Automation.AutomationElement]::RootElement.FindAll([System.Windows.Automation.TreeScope]::Descendants,$ownedMenu))
            foreach($item in $expanded){Write-Output ('expanded name="{0}" id="{1}"' -f $item.Current.Name,$item.Current.AutomationId)}
        }
        exit 0
    }
    $buttons=[System.Windows.Automation.PropertyCondition]::new([System.Windows.Automation.AutomationElement]::ControlTypeProperty,[System.Windows.Automation.ControlType]::Button)
    $deadline=[DateTime]::UtcNow.AddSeconds(5); $settings=@()
    do {
        $items=$window.FindAll([System.Windows.Automation.TreeScope]::Descendants,$buttons)
        $settings=@($items | Where-Object { $_.Current.AutomationId -eq 'SettingsButton' })
        if(-not $settings.Count){Start-Sleep -Milliseconds 100}
    } while(-not $settings.Count -and [DateTime]::UtcNow -lt $deadline)
    if($settings.Count -ne 1){throw "settings button count=$($settings.Count)"}
    $invoke=$settings[0].GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern)
    $invoke.Invoke()
    if($VerifySettings){
        $combo=[System.Windows.Automation.PropertyCondition]::new([System.Windows.Automation.AutomationElement]::ControlTypeProperty,[System.Windows.Automation.ControlType]::ComboBox)
        $deadline=[DateTime]::UtcNow.AddSeconds(5); $foundCombo=$null
        do {
            $foundCombo=$window.FindFirst([System.Windows.Automation.TreeScope]::Descendants,$combo)
            if(-not $foundCombo){Start-Sleep -Milliseconds 100}
        } while(-not $foundCombo -and [DateTime]::UtcNow -lt $deadline)
        Write-Output "settings_combobox_found=$($null -ne $foundCombo)"
        if(-not $foundCombo){exit 1}
        exit 0
    }
    $checkboxes=[System.Windows.Automation.PropertyCondition]::new([System.Windows.Automation.AutomationElement]::ControlTypeProperty,[System.Windows.Automation.ControlType]::CheckBox)
    $deadline=[DateTime]::UtcNow.AddSeconds(5); $found=@()
    do {
        $found=@($window.FindAll([System.Windows.Automation.TreeScope]::Descendants,$checkboxes))
        if(-not $found.Count){Start-Sleep -Milliseconds 100}
    } while(-not $found.Count -and [DateTime]::UtcNow -lt $deadline)
    Write-Output "checkbox_count=$($found.Count)"
    foreach($item in $found){Write-Output ('checkbox name="{0}" id="{1}"' -f $item.Current.Name,$item.Current.AutomationId)}
    $all=$window.FindAll([System.Windows.Automation.TreeScope]::Descendants,[System.Windows.Automation.Condition]::TrueCondition)
    $all | Group-Object { $_.Current.ControlType.ProgrammaticName } | Sort-Object Name | ForEach-Object {Write-Output ("type=$($_.Name) count=$($_.Count)")}
    if(-not $found.Count){exit 77}
} finally {
    if($child -and $parent){$actual=Get-CimInstance Win32_Process -Filter "ProcessId=$($child.ProcessId)";if($actual -and $actual.ParentProcessId -eq $parent.ProcessId -and $actual.CreationDate -eq $child.CreationDate){Stop-Process -Id $actual.ProcessId -Force}}
    if($parent){$actual=Get-CimInstance Win32_Process -Filter "ProcessId=$($parent.ProcessId)";if($actual -and $actual.CreationDate -eq $parent.CreationDate){Stop-Process -Id $actual.ProcessId -Force}}
    Remove-Item -LiteralPath $doc -Force -ErrorAction SilentlyContinue
}
