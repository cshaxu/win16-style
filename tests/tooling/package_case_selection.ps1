$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '../../tools/Select-PackageCases.ps1')
$required=@('progress-sharing','plan-selection','abi-x86','abi-x64','demo-boundary','demo-pair','automatic','propagation')
$cases=@(($required+@('target-a','target-b')) | ForEach-Object { @{name=$_} })
$all=Select-PackageCases -Cases $cases
if ($all.coverage -ne 'standard' -or $all.cases.Count -ne 10 -or $all.omitted.Count) { throw 'Default matrix changed' }
$focus=Select-PackageCases -Cases $cases -Focus 'target-a'
if ($focus.coverage -ne 'focused' -or $focus.cases.Count -ne 9 -or @($focus.omitted).Count -ne 1 -or $focus.omitted[0] -ne 'target-b') { throw 'Focused plan mismatch' }
foreach ($bad in @('','target-a,target-a','missing','target-a,','abi-x86','TARGET-A')) {
    $rejected=$false
    try { Select-PackageCases -Cases $cases -Focus $bad | Out-Null } catch { $rejected=$true }
    if (!$rejected) { throw "Invalid plan accepted: $bad" }
}
$rejected=$false
try { Select-PackageCases -Cases @($cases | Where-Object name -ne 'abi-x64') -Focus 'target-a' | Out-Null } catch { $rejected=$true }
if (!$rejected) { throw 'Missing mandatory case accepted' }
$result=@{status='passed-focused';coverage='focused';full_integration=$false;selected_cases=@($focus.cases | ForEach-Object name);requested_cases=@('target-a');omitted_cases=@('target-b');cases=9;skipped=@()}
Assert-PackageCoverage -Result $result
foreach ($mutate in @('missing','count','overlap','coverage','full','skips')) {
    $bad=$result.Clone()
    switch($mutate){
        'missing' {$bad.selected_cases=@($bad.selected_cases | Where-Object { $_ -ne 'abi-x64' });$bad.cases=8}
        'count' {$bad.cases=8}
        'overlap' {$bad.omitted_cases=@('target-a')}
        'coverage' {$bad.coverage='standard'}
        'full' {$bad.full_integration=$true}
        'skips' {$bad.skipped=@('target-a')}
    }
    $rejected=$false;try { Assert-PackageCoverage -Result $bad } catch { $rejected=$true }
    if (!$rejected) { throw "Invalid focused result accepted: $mutate" }
}
$skipCases=@(
    @{name='vb5-checklist-dpi144-32';code=77;output="VB_CHECKLIST_144_SKIP dpi=96`n";expected=$true},
    @{name='vb6-checklist-dpi144-64';code=77;output="VB_CHECKLIST_144_SKIP dpi=144`n";expected=$false},
    @{name='vb6-checklist-dpi144-64';code=1;output="VB_CHECKLIST_144_SKIP dpi=96`n";expected=$false},
    @{name='other';code=77;output="VB_CHECKLIST_144_SKIP dpi=96`n";expected=$false},
    @{name='vb5-checklist-dpi144-32';code=77;output="VB_CHECKLIST_144_SKIP dpi=0`n";expected=$false},
    @{name='vb5-checklist-dpi144-32';code=77;output="VB_CHECKLIST_144_SKIP dpi=96 extra`n";expected=$false},
    @{name='other';code=2;output='PRIVATE_DESKTOP_CREATE_FAIL error=5';expected=$true},
    @{name='other';code=2;output='PRIVATE_DESKTOP_CREATE_FAIL error=6';expected=$false}
)
foreach($case in $skipCases){
    if ((Test-PackageCaseSkip -Name $case.name -ExitCode $case.code -Output $case.output) -ne $case.expected) {throw "Invalid skip decision: $($case.name) $($case.code) $($case.output)"}
}
Write-Output 'PACKAGE_PLAN PASS default focused unknown duplicate empty missing-required rejected'
