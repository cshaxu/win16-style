function Set-PackageProgress {
    param([Parameter(Mandatory)][string]$Path,[Parameter(Mandatory)][string]$Json)
    try {
        Set-Content -LiteralPath $Path -Value $Json -Encoding utf8 -ErrorAction Stop
        return $true
    } catch [System.IO.IOException] {
        # A reader can hold the snapshot briefly. Only sharing/lock violations
        # are advisory; the next scheduled update retries the snapshot.
        if ($_.Exception.HResult -in @(-2147024864,-2147024863)) { return $false }
        throw
    }
}
