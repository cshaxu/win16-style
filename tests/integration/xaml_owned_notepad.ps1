param([Parameter(Mandatory=$true)][string]$BuildDirectory,[switch]$Session,[switch]$Parse,[switch]$CheckboxParse,[switch]$RadioParse,[switch]$TextboxParse,[switch]$ComboParse,[switch]$ScrollParse,[switch]$GenericSite,[switch]$Visible,[switch]$WaitForWindow,[switch]$HoldController,[switch]$WaitChain,[switch]$DeferSubscribe,[switch]$WorkerSubscribe,[switch]$AsyncSite,[switch]$DisconnectSite,[switch]$Factory,[switch]$Adapter)
$kinds=@('FOUND','TREE','APPLIED','RESTORED','FAILED','PARSE_OK','CHECKBOX_OK','RADIO_OK','TEXTBOX_OK','COMBO_OK','SCROLL_OK','SITE_CONNECTED','SITE_READY','SITE_FAILED','SITE_DISCONNECTING','SITE_DISCONNECTED','SUBSCRIBE_SCHEDULED','SUBSCRIBE_TIMER','WORKER_SUBSCRIBING','WORKER_SERVICE_MARSHALLED','WORKER_MARSHAL_FAILED','WORKER_UNMARSHAL_FAILED','STATES','CONTENT_TEXT_MATCH','BITMAP_OK','BITMAP_FAILED','BITMAP_NOTIMPL','BITMAP_DENIED','BITMAP_INVALID','BITMAP_EFAIL','BITMAP_OTHER','LOCAL_BINDING','LOCAL_HANDLE','LOCAL_EVALUATED','LOCAL_NULL','SESSION_APPLIED','SESSION_STOPPED','SESSION_NOCHANGE','SESSION_ERROR','SESSION_WRONG_THREAD','SESSION_OTHER_ERROR','SESSION_NOINTERFACE','SESSION_INVALID','SESSION_NOTFOUND','SESSION_DEPTH')
$events=@{};$nonce=[Guid]::NewGuid().ToString('N');$starter=$null;$child=$null
foreach($k in $kinds){$n="Local\w16-$k-$nonce";[Environment]::SetEnvironmentVariable("WIN16_XAML_${k}_EVENT",$n,'Process');$events[$k]=[Threading.EventWaitHandle]::new($false,[Threading.EventResetMode]::ManualReset,$n)}
if($CheckboxParse){$Parse=$true;$env:WIN16_XAML_CHECKBOX_TEST='1'}
if($RadioParse){$Parse=$true;$env:WIN16_XAML_RADIO_TEST='1'}
if($TextboxParse){$Parse=$true;$env:WIN16_XAML_TEXTBOX_TEST='1'}
if($ComboParse){$Parse=$true;$env:WIN16_XAML_COMBO_TEST='1'}
if($ScrollParse){$Parse=$true;$env:WIN16_XAML_SCROLL_TEST='1'}
if($Session){$env:WIN16_XAML_SESSION_TEST='1'}elseif($Parse){$env:WIN16_XAML_PARSE_TEST='1'}else{$env:WIN16_XAML_APPLY_TEST='1'}
if($GenericSite){$env:WIN16_XAML_GENERIC_SITE_TEST='1'}
if($AsyncSite){$env:WIN16_XAML_GENERIC_SITE_TEST='1';$env:WIN16_XAML_ASYNC_SITE_TEST='1'}
if($Factory){$env:WIN16_XAML_FACTORY_TEST='1'}
if($Adapter){$env:WIN16_XAML_ADAPTER_TEST='1'}
if($DisconnectSite){$env:WIN16_XAML_DISCONNECT_TEST='1'}
if($DeferSubscribe){$env:WIN16_XAML_DEFER_SUBSCRIBE='1'}
if($WorkerSubscribe){$env:WIN16_XAML_DEFER_SUBSCRIBE='1';$env:WIN16_XAML_WORKER_SUBSCRIBE='1'}
if($WaitChain){$env:WIN16_XAML_THREAD_TAG="Local\w16-site-$nonce";$env:WIN16_XAML_CALLBACK_TAG="Local\w16-callback-$nonce"}
$done=$null
if($HoldController){$doneName="Local\w16-controller-done-$nonce";$done=[Threading.EventWaitHandle]::new($false,[Threading.EventResetMode]::ManualReset,$doneName);$env:WIN16_XAML_CONTROLLER_DONE_EVENT=$doneName}
$doc=Join-Path ([IO.Path]::GetTempPath()) "w16-owned-$nonce.txt";[IO.File]::WriteAllText($doc,'')
try {
 $windowStyle=if($Visible){'Normal'}else{'Hidden'}
 $starter=Start-Process "$env:WINDIR\System32\notepad.exe" -ArgumentList @("`"$doc`"") -WindowStyle $windowStyle -PassThru
 $until=[DateTime]::UtcNow.AddSeconds(8);$udk=$null;$xaml=$null
 while([DateTime]::UtcNow -lt $until -and (-not $udk -or -not $xaml)){
  $child=Get-CimInstance Win32_Process -Filter "ParentProcessId=$($starter.Id) AND Name='Notepad.exe'" | Select-Object -First 1
  if($child){$p=Get-Process -Id $child.ProcessId -ErrorAction SilentlyContinue;if($p){$mods=@($p.Modules);$udk=$mods|Where-Object ModuleName -eq 'Microsoft.Internal.FrameworkUdk.dll'|Select-Object -First 1;$xaml=$mods|Where-Object ModuleName -eq 'Microsoft.UI.Xaml.dll'|Select-Object -First 1}}
  if(-not $udk -or -not $xaml){Start-Sleep -Milliseconds 150}
 }
 if(-not $udk -or -not $xaml){Write-Output 'SKIP target_not_ready';exit 77}
 if($WaitForWindow){
  $deadline=[DateTime]::UtcNow.AddSeconds(5);$window=[IntPtr]::Zero
  while([DateTime]::UtcNow -lt $deadline -and $window -eq [IntPtr]::Zero){
   $target=Get-Process -Id $child.ProcessId -ErrorAction SilentlyContinue
   if($target){$target.Refresh();$window=$target.MainWindowHandle}
   if($window -eq [IntPtr]::Zero){Start-Sleep -Milliseconds 100}
  }
  if($window -eq [IntPtr]::Zero){Write-Output 'SKIP owned_window_not_ready';exit 77}
  Write-Output 'owned_window_ready=True'
 }
 $build=(Resolve-Path -LiteralPath $BuildDirectory).Path;$tap=Join-Path $build 'libxaml-diagnostics-tap.dll';$argList=@('--winui',"$($child.ProcessId)","`"$tap`"")
 $ctrl=Start-Process (Join-Path $build 'xaml-diagnostics-controller.exe') -ArgumentList $argList -WindowStyle Hidden -PassThru
 if(-not $HoldController -and -not $ctrl.WaitForExit(6000)){Stop-Process -Id $ctrl.Id -Force;throw 'controller timeout'}
 $success=if($DisconnectSite){'SITE_DISCONNECTED'}elseif($Session){'SESSION_STOPPED'}elseif($CheckboxParse){'CHECKBOX_OK'}elseif($RadioParse){'RADIO_OK'}elseif($TextboxParse){'TEXTBOX_OK'}elseif($ComboParse){'COMBO_OK'}elseif($ScrollParse){'SCROLL_OK'}elseif($Parse){'PARSE_OK'}else{'RESTORED'}
 $terminal=[Threading.WaitHandle[]]@($events[$success],$events['FAILED'],$events['SITE_FAILED']);$index=[Threading.WaitHandle]::WaitAny($terminal,4500)
 if($WaitChain){
  $report=Join-Path $build "wct-$nonce.txt"
  try {
   $probe=Start-Process (Join-Path $build 'xaml-wait-chain.exe') -ArgumentList @("$($child.ProcessId)") -WindowStyle Hidden -RedirectStandardOutput $report -PassThru
   if(-not $probe.WaitForExit(4000)){Stop-Process -Id $probe.Id -Force;Write-Output 'wait_chain=timeout'}
   else {
    Write-Output "wait_chain_exit=$($probe.ExitCode)";$lines=@(Get-Content -LiteralPath $report -ErrorAction SilentlyContinue);$lines
    $tag=$lines | Where-Object {$_ -match '^site_thread=\d+$'} | Select-Object -First 1
    if($tag){
     $threadId=$tag.Substring(12);$stackReport=Join-Path $build "stack-$nonce.txt"
     try {
      $stack=Start-Process (Join-Path $build 'xaml-thread-stack.exe') -ArgumentList @("$($child.ProcessId)",$threadId) -WindowStyle Hidden -RedirectStandardOutput $stackReport -PassThru
      if(-not $stack.WaitForExit(4000)){Stop-Process -Id $stack.Id -Force;Write-Output 'thread_stack=timeout'}
      else {Write-Output "thread_stack_exit=$($stack.ExitCode)";Get-Content -LiteralPath $stackReport -ErrorAction SilentlyContinue}
     } finally {Remove-Item -LiteralPath $stackReport -Force -ErrorAction SilentlyContinue}
    }
   }
  } finally {Remove-Item -LiteralPath $report -Force -ErrorAction SilentlyContinue}
 }
 if($done){$null=$done.Set();if(-not $ctrl.WaitForExit(2000)){Stop-Process -Id $ctrl.Id -Force;throw 'controller timeout after stop'}}
 Write-Output "controller=$($ctrl.ExitCode) terminal=$index"
 foreach($k in $kinds){if($events[$k].WaitOne(0)){Write-Output "$k=True"}}
 if($ctrl.ExitCode -ne 0 -or $index -eq 1 -or $index -eq 2){exit 1};if($index -eq 258){exit 77}
} finally {
 if($child -and $starter){$v=Get-CimInstance Win32_Process -Filter "ProcessId=$($child.ProcessId)";if($v -and $v.ParentProcessId -eq $starter.Id -and $v.CreationDate -eq $child.CreationDate){$p=Get-Process -Id $v.ProcessId -ErrorAction SilentlyContinue;if($p){$null=$p.CloseMainWindow();if(-not $p.WaitForExit(2000)){Stop-Process -Id $p.Id -Force}}}}
 if($starter -and -not $starter.HasExited){$null=$starter.CloseMainWindow();if(-not $starter.WaitForExit(2000)){Stop-Process -Id $starter.Id -Force -ErrorAction SilentlyContinue}}
 foreach($k in $kinds){$events[$k].Dispose();[Environment]::SetEnvironmentVariable("WIN16_XAML_${k}_EVENT",$null,'Process')}
 if($done){$null=$done.Set();$done.Dispose()}
 Remove-Item Env:WIN16_XAML_APPLY_TEST,Env:WIN16_XAML_SESSION_TEST,Env:WIN16_XAML_PARSE_TEST,Env:WIN16_XAML_CHECKBOX_TEST,Env:WIN16_XAML_RADIO_TEST,Env:WIN16_XAML_TEXTBOX_TEST,Env:WIN16_XAML_COMBO_TEST,Env:WIN16_XAML_SCROLL_TEST,Env:WIN16_XAML_GENERIC_SITE_TEST,Env:WIN16_XAML_ASYNC_SITE_TEST,Env:WIN16_XAML_FACTORY_TEST,Env:WIN16_XAML_ADAPTER_TEST,Env:WIN16_XAML_DISCONNECT_TEST,Env:WIN16_XAML_CONTROLLER_DONE_EVENT,Env:WIN16_XAML_THREAD_TAG,Env:WIN16_XAML_CALLBACK_TAG,Env:WIN16_XAML_DEFER_SUBSCRIBE,Env:WIN16_XAML_WORKER_SUBSCRIBE -ErrorAction SilentlyContinue;Remove-Item -LiteralPath $doc -Force -ErrorAction SilentlyContinue
}
