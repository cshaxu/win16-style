[CmdletBinding()]
param(
    [string]$Root = (Split-Path $PSScriptRoot -Parent),
    [switch]$SelfTest
)
$ErrorActionPreference = 'Stop'

# Read-only gate. Negative fixtures mutate an in-memory document map only.
function Test-Governance([hashtable]$Files) {
    $issues = [System.Collections.Generic.List[string]]::new()
    $required = @(
        'AGENTS.md', 'README.md', 'CONTRIBUTING.md', 'docs/README.md',
        'docs/design/GOAL.md', 'docs/design/ARCHITECTURE.md', 'docs/design/CODING.md',
        'docs/design/UI.md', 'docs/design/ROADMAP.md', 'docs/rules/EXECUTION.md',
        'docs/rules/ARCHITECTURE.md', 'docs/rules/CODING.md', 'docs/rules/DOCUMENT.md',
        'docs/states/CURRENT.md', 'docs/states/QUEUE.md', 'docs/states/TODO.md',
        'docs/etc/README.md', 'docs/etc/product-contract.json',
        'docs/etc/import/README.md', 'docs/etc/import/source-inventory.tsv',
        'docs/etc/operations/source-policy.md', 'docs/proposals/native-style-import.md',
        'docs/history/M0-Td-S1-bootstrap.md', 'assets/binary/README.md'
    )
    foreach ($path in $required) {
        if (!$Files.ContainsKey($path) -or [string]::IsNullOrWhiteSpace($Files[$path])) {
            $issues.Add("Missing/empty required file: $path")
        }
    }
    foreach ($group in @('design', 'rules', 'states')) {
        $expected = @($required | Where-Object { $_ -like "docs/$group/*" } | Sort-Object)
        $actual = @($Files.Keys | Where-Object { $_ -like "docs/$group/*" } | Sort-Object)
        if (@(Compare-Object $expected $actual).Count) { $issues.Add("Invalid fixed file set: $group") }
    }
    $dirs = @($Files.Keys | Where-Object { $_ -match '^docs/[^/]+/' } |
        ForEach-Object { ($_ -split '/')[1] } | Sort-Object -Unique)
    if (@(Compare-Object @('design','etc','history','proposals','rules','states') $dirs).Count) {
        $issues.Add('Invalid docs directory topology')
    }
    foreach ($path in @($Files.Keys | Where-Object { $_ -like '*.md' })) {
        $body = $Files[$path]
        # Discard fenced examples before interpreting Markdown links.
        $body = [regex]::Replace($body, '(?ms)^```.*?^```[^\r\n]*', '')
        foreach ($link in [regex]::Matches($body, '\[[^\]]*\]\(([^)]+)\)')) {
            $target = $link.Groups[1].Value.Trim()
            if ($target -match '^(https?://|mailto:|#)') { continue }
            $target = ($target -split '#')[0].Trim('<','>')
            $segments = [System.Collections.Generic.List[string]]::new()
            $parent = ($path -split '/')[0..(($path -split '/').Count - 2)]
            if ($path.Contains('/')) { foreach ($part in $parent) { $segments.Add($part) } }
            foreach ($part in ($target -split '/')) {
                if ($part -eq '..') {
                    if (!$segments.Count) { $issues.Add("Escaping link: $path -> $target"); break }
                    $segments.RemoveAt($segments.Count - 1)
                } elseif ($part -and $part -ne '.') { $segments.Add($part) }
            }
            $resolved = $segments -join '/'
            if (!$Files.ContainsKey($resolved)) { $issues.Add("Broken link: $path -> $target") }
        }
        if ($body -match '(?m)^## M[1-9][0-9]* .*Packet\s*$') {
            $issues.Add("Non-M0 packet: $path")
        }
    }
    $current = [string]$Files['docs/states/CURRENT.md']
    $packets = [regex]::Matches($current, '(?m)^## M0 (Td|T[1-9][0-9]*) S([1-9][0-9]*) Packet\s*$')
    if ([regex]::Matches($current, '(?m)^## .*Packet\s*$').Count -ne $packets.Count) {
        $issues.Add('Malformed active packet identifier')
    }
    if ($packets.Count -gt 1) { $issues.Add('More than one active packet') }
    if ($current -notmatch '(?m)^- Milestone: M0 \(fixed\)\s*$') { $issues.Add('Missing fixed M0 state') }
    if ($current -notmatch '(?m)^- Last allocated T: [0-9]+\s*$') { $issues.Add('Missing T allocation state') }
    if ($current -notmatch '(?m)^- Last allocated Td S: [0-9]+\s*$') { $issues.Add('Missing Td allocation state') }
    if (!$packets.Count -and $current -notmatch '无活动任务包') { $issues.Add('Idle state must be explicit') }
    $fields = @('Identifier Mode','Admission And Approval','Objective','Non-goals',
        'Reference Baseline','Candidate Proposal','Files And ABI Surface','Applicable Rules',
        'Verification','Expected Markers','Asset Needs','Reporting Requirements',
        'Stop Conditions','Exit Criteria','Original Owner Request','Similar-Issue Sweep')
    foreach ($packet in $packets) {
        $table = ($current.Substring($packet.Index + $packet.Length) -split '(?m)^## ', 2)[0]
        if ($table -notmatch '\| Field \| Required record \|') { $issues.Add('Packet table header missing') }
        foreach ($field in $fields) {
            $pattern = '(?m)^\| ' + [regex]::Escape($field) + ' \|\s*\S[^\r\n]*\|\s*$'
            if ([regex]::Matches($table, $pattern).Count -ne 1) { $issues.Add("Packet field missing/duplicate: $field") }
        }
        if ($table -notmatch '\| Identifier Mode \| (New|Continuation|Corrective|Owner-Reopen|Governance)[；; ]') {
            $issues.Add('Invalid identifier mode')
        }
        if ($packet.Groups[1].Value -eq 'Td') {
            if ($table -notmatch '\| Identifier Mode \| Governance' -or
                $current -notmatch ('(?m)^- Last allocated Td S: ' + $packet.Groups[2].Value + '\s*$')) {
                $issues.Add('Td packet/allocation mismatch')
            }
        } else {
            $number = $packet.Groups[1].Value.Substring(1)
            if ($current -notmatch ('(?m)^- Last allocated T: ' + $number + '\s*$')) {
                $issues.Add('Numeric T packet/allocation mismatch')
            }
        }
    }
    $queue = [string]$Files['docs/states/QUEUE.md']
    if ($queue -match '\bT[0-9]+\b') { $issues.Add('Queue preallocates numeric T') }
    foreach ($proposal in @($Files.Keys | Where-Object { $_ -like 'docs/proposals/*.md' })) {
        if (!$queue.Contains('../proposals/' + ($proposal -split '/')[-1])) {
            $issues.Add("Orphan candidate: $proposal")
        }
    }
    try {
        $contract = $Files['docs/etc/product-contract.json'] | ConvertFrom-Json
        $expected = @('run16s-exe|run16s.exe|x64','ntw16s-dll|ntw16s32.dll|x86',
            'ntw16s-dll|ntw16s64.dll|x64','demo-exe|demo.exe|x64') | Sort-Object
        $actual = @($contract.components | ForEach-Object {
            $component = $_.id
            foreach ($artifact in $_.artifacts) { "$component|$($artifact.name)|$($artifact.architecture)" }
        } | Sort-Object)
        if ($contract.milestone -ne 0 -or @($contract.components).Count -ne 3 -or
            @(Compare-Object $expected $actual).Count) { $issues.Add('Invalid three-component/four-artifact contract') }
        if ($contract.demo.direct -ne 'native' -or $contract.demo.launched -ne 'win16' -or
            $contract.demo.links_style_dll -ne $false) { $issues.Add('Invalid demo dual-launch contract') }
        if ($contract.import_status -eq 'not-imported' -and @($Files.Keys | Where-Object { $_ -like 'src/*' }).Count) {
            $issues.Add('Source present while import status is not-imported')
        }
    } catch { $issues.Add("Invalid product contract JSON: $_") }
    $inventory = [string]$Files['docs/etc/import/source-inventory.tsv']
    if ($inventory -notmatch '^# source-commit: 5b0b65f66c281050aa4ce1454c6cd31441a6d484') {
        $issues.Add('Source inventory missing pinned commit')
    }
    $rows = @($inventory -split '\r?\n' | Where-Object { $_ -and !($_.StartsWith('#')) })
    if (!$rows.Count) { $issues.Add('Empty source inventory') }
    $seen = @{}
    foreach ($row in $rows) {
        if ($row -notmatch '^100[0-7]{3} blob [0-9a-f]{40}\s+[0-9]+\t(.+)$') {
            $issues.Add("Malformed source row: $row"); continue
        }
        $sourcePath = $Matches[1]
        if ($seen.ContainsKey($sourcePath)) { $issues.Add("Duplicate source path: $sourcePath") }
        $seen[$sourcePath] = $true
    }
    return $issues.ToArray()
}

$resolvedRoot = (Resolve-Path -LiteralPath $Root).Path
$files = @{}
foreach ($file in (Get-ChildItem -LiteralPath $resolvedRoot -File -Recurse -Force |
    Where-Object { $_.FullName -notmatch '[\\/](\.git|build|\.vs)[\\/]' })) {
    $path = [IO.Path]::GetRelativePath($resolvedRoot, $file.FullName).Replace('\','/')
    # Only metadata is needed for binary entries, never decode packaged images as text.
    $files[$path] = if ($file.Extension -in '.exe','.dll','.png','.bmp') { '<binary>' } else {
        [IO.File]::ReadAllText($file.FullName)
    }
}
$issues = @(Test-Governance $files)
if ($issues.Count) { throw ($issues -join "`n") }
Write-Output 'Governance PASS'
if ($SelfTest) {
    $cases = [ordered]@{
        missing_authority = { param($f) $f.Remove('docs/rules/CODING.md') }
        foreign_topology = { param($f) $f['docs/other/file.md'] = 'foreign' }
        broken_link = { param($f) $f['README.md'] += "`n[bad](missing.md)" }
        duplicate_packet = { param($f) $f['docs/states/CURRENT.md'] += "`n## M0 Td S999 Packet`n" }
        malformed_packet = { param($f) $f['docs/states/CURRENT.md'] = $f['docs/states/CURRENT.md'] -replace '(?m)^## M0 (Td|T[1-9][0-9]*) S[1-9][0-9]* Packet', '## M0 T0 S0 Packet' }
        missing_field = { param($f) $f['docs/states/CURRENT.md'] = $f['docs/states/CURRENT.md'] -replace 'Similar-Issue Sweep', 'Missing-Field' }
        nonzero_milestone = { param($f) $f['docs/etc/product-contract.json'] = $f['docs/etc/product-contract.json'] -replace '"milestone": 0','"milestone": 1' }
        fifth_product = { param($f) $f['docs/etc/product-contract.json'] = $f['docs/etc/product-contract.json'] -replace '"run16s.exe"','"helper.exe"' }
        demo_self_style = { param($f) $f['docs/etc/product-contract.json'] = $f['docs/etc/product-contract.json'] -replace '"links_style_dll": false','"links_style_dll": true' }
        premature_source = { param($f) $f['src/demo-exe/main.c'] = 'source' }
        numbered_queue = { param($f) $f['docs/states/QUEUE.md'] += "`nT1 reserved" }
        orphan_candidate = { param($f) $f['docs/proposals/orphan.md'] = 'candidate' }
        wrong_source = { param($f) $f['docs/etc/import/source-inventory.tsv'] = '# source-commit: floating' }
    }
    # Keep packet-specific negative tests meaningful after the repository becomes idle.
    $fixture = $files.Clone()
    if ($fixture['docs/states/CURRENT.md'] -notmatch '(?m)^## M0 .*Packet') {
        $fixture['docs/states/CURRENT.md'] += "`n## M0 Td S1 Packet`n| Field | Required record |`n| --- | --- |`n"
        foreach ($field in @('Identifier Mode','Admission And Approval','Objective','Non-goals',
            'Reference Baseline','Candidate Proposal','Files And ABI Surface','Applicable Rules',
            'Verification','Expected Markers','Asset Needs','Reporting Requirements','Stop Conditions',
            'Exit Criteria','Original Owner Request','Similar-Issue Sweep')) {
            $record = if ($field -eq 'Identifier Mode') { 'Governance; fixture' } else { 'fixture' }
            $fixture['docs/states/CURRENT.md'] += "| $field | $record |`n"
        }
        $fixture['docs/states/CURRENT.md'] = $fixture['docs/states/CURRENT.md'] -replace 'Last allocated Td S: [0-9]+','Last allocated Td S: 1'
    }
    if (@(Test-Governance $fixture).Count) { throw 'SelfTest baseline is invalid' }
    foreach ($case in $cases.GetEnumerator()) {
        $mutated = $fixture.Clone()
        & $case.Value $mutated
        if (!@(Test-Governance $mutated).Count) { throw "SelfTest failed to reject: $($case.Key)" }
        Write-Output "Rejected: $($case.Key)"
    }
    Write-Output "SelfTest PASS ($($cases.Count) rejection cases)"
}
