function Select-PackageCases {
    param([Parameter(Mandatory)][object[]]$Cases,[AllowEmptyString()][string]$Focus)
    $names=@($Cases | ForEach-Object { $_.name })
    if (@($names | Group-Object | Where-Object Count -gt 1).Count) { throw 'Duplicate registered package case' }
    if (!$PSBoundParameters.ContainsKey('Focus')) {
        return @{coverage='standard';cases=@($Cases);requested=@();omitted=@()}
    }
    if ($Focus -notmatch '^[a-z0-9-]+(,[a-z0-9-]+)*$') { throw 'Invalid focused case list' }
    $requested=@($Focus -split ',')
    if (@($requested | Group-Object | Where-Object Count -gt 1).Count) { throw 'Duplicate focused case' }
    foreach ($name in $requested) { if ($name -cnotin $names) { throw "Unknown focused case: $name" } }
    $required=@('progress-sharing','plan-selection','abi-x86','abi-x64','demo-boundary','demo-pair','automatic','propagation')
    foreach ($name in $required) { if ($name -cnotin $names) { throw "Missing mandatory case: $name" } }
    if (!@($requested | Where-Object { $_ -cnotin $required }).Count) { throw 'Select at least one additional affected case' }
    $selected=@($Cases | Where-Object { $_.name -cin $required -or $_.name -cin $requested })
    $omitted=@($names | Where-Object { $_ -cnotin @($selected | ForEach-Object name) })
    return @{coverage='focused';cases=$selected;requested=$requested;omitted=$omitted}
}

function Assert-PackageCoverage {
    param([Parameter(Mandatory)]$Result)
    $focused=$Result.status -like 'passed-focused*'
    if (!$focused) {
        if ($Result.coverage -eq 'focused') { throw 'Focused coverage requires a focused status' }
        return
    }
    if ($Result.coverage -ne 'focused' -or $Result.full_integration) { throw 'Invalid focused coverage declaration' }
    $selected=@($Result.selected_cases);$requested=@($Result.requested_cases);$omitted=@($Result.omitted_cases)
    $required=@('progress-sharing','plan-selection','abi-x86','abi-x64','demo-boundary','demo-pair','automatic','propagation')
    if (($Result.status -eq 'passed-focused-with-skips') -ne (@($Result.skipped).Count -gt 0)) { throw 'Focused skip status mismatch' }
    if ($selected.Count -ne $Result.cases -or @($selected | Group-Object | Where-Object Count -gt 1).Count) { throw 'Invalid focused case count' }
    foreach ($name in $required) { if ($name -cnotin $selected) { throw "Missing mandatory case: $name" } }
    if (!@($requested | Where-Object { $_ -cnotin $required }).Count) { throw 'Missing affected case' }
    foreach ($name in $requested) { if ($name -cnotin $selected) { throw "Requested case not selected: $name" } }
    foreach ($name in $omitted) { if ($name -cin $selected) { throw "Case both selected and omitted: $name" } }
    foreach ($name in @($Result.skipped)) { if ($name -cnotin $selected) { throw "Skipped case not selected: $name" } }
}

function Test-PackageCaseSkip {
    param([string]$Name,[int]$ExitCode,[string]$Output)
    if ($ExitCode -eq 2 -and $Output.Contains('PRIVATE_DESKTOP_CREATE_FAIL error=5')) { return $true }
    if ($Name -cmatch '^vcl-status-font-alias-(32|64)$' -and $ExitCode -eq 2 -and $Output -cmatch '(?m)^ALIAS_SKIP no_handle_reuse attempts=1200\r?$') { return $true }
    if ($Name -cmatch '^vb(5|6)-checklist-dpi144-(32|64)$' -and $ExitCode -eq 77 -and
        $Output -cmatch '(?m)^VB_CHECKLIST_144_SKIP dpi=([1-9][0-9]*)\r?$') {
        return [uint64]$Matches[1] -ne 144
    }
    return $false
}
