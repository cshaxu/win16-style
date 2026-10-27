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
