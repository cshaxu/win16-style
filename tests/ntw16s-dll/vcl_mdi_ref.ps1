param([Parameter(Mandatory)][string]$Launcher,[Parameter(Mandatory)][string]$Dll,[Parameter(Mandatory)][string]$Probe,[Parameter(Mandatory)][string]$Fixture)
$ErrorActionPreference='Stop'
$line=@(& nm $Dll | Select-String ' [tT] _?attachment_proc(@24)?$')
if($LASTEXITCODE -ne 0 -or $line.Count -ne 1){throw 'exact private attachment symbol unavailable'}
$base=@(& objdump -p $Dll | Select-String '^ImageBase\s+([0-9a-fA-F]+)')
if($LASTEXITCODE -ne 0 -or $base.Count -ne 1){throw 'PE image base unavailable'}
$symbol=[Convert]::ToUInt64((($line[0].ToString().Trim() -split '\s+')[0]),16)
$imageBase=[Convert]::ToUInt64((($base[0].ToString().Trim() -split '\s+')[1]),16)
if($symbol -lt $imageBase){throw 'invalid attachment symbol address'}
$rva='{0:x}' -f ($symbol-$imageBase)
& $Launcher --wait $Probe $Fixture $rva
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
