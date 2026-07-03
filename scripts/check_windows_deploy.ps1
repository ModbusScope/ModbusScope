param(
    [Parameter(Mandatory = $true)][string]$DeployDir,
    [Parameter(Mandatory = $true)][string[]]$Executables,
    [int]$TimeoutSeconds = 5
)

$ErrorActionPreference = 'Stop'

# NTSTATUS values Windows uses when a process image fails to load.
$LoaderFailureCodes = @{
    0xC0000135 = 'STATUS_DLL_NOT_FOUND - a required DLL could not be located'
    0xC0000139 = 'STATUS_ENTRYPOINT_NOT_FOUND - a DLL is present but missing an expected export'
    0xC000007B = 'STATUS_INVALID_IMAGE_FORMAT - architecture/bitness mismatch'
    0xC0000005 = 'STATUS_ACCESS_VIOLATION - crashed on startup'
}

# Launch everything first so one shared timeout covers all executables.
$launched = foreach ($exeName in $Executables) {
    $exePath = Join-Path $DeployDir $exeName
    if (-not (Test-Path $exePath)) {
        Write-Host "[FAIL] $exeName - not found in $DeployDir"
        [PSCustomObject]@{ Name = $exeName; Process = $null }
        continue
    }
    Write-Host "Starting $exeName ..."
    $proc = Start-Process -FilePath $exePath -WorkingDirectory $DeployDir -NoNewWindow -PassThru
    [PSCustomObject]@{ Name = $exeName; Process = $proc }
}

Start-Sleep -Seconds $TimeoutSeconds

$failures = @()
foreach ($item in $launched) {
    if (-not $item.Process) { $failures += $item.Name; continue }
    $proc = $item.Process
    if ($proc.HasExited) {
        $code = [uint32]$proc.ExitCode
        $hex = '0x{0:X8}' -f $code
        if ($LoaderFailureCodes.ContainsKey($code)) {
            Write-Host "[FAIL] $($item.Name) exited after ${TimeoutSeconds}s with $hex ($($LoaderFailureCodes[$code]))"
        } else {
            Write-Host "[FAIL] $($item.Name) exited early after ${TimeoutSeconds}s with exit code $($proc.ExitCode) ($hex)"
        }
        $failures += $item.Name
    } else {
        Write-Host "[PASS] $($item.Name) still running after ${TimeoutSeconds}s"
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    }
}

if ($failures.Count -gt 0) {
    Write-Host ""
    Write-Host "Deployment smoke test FAILED for: $($failures -join ', ')"
    exit 1
}
Write-Host ""
Write-Host "Deployment smoke test passed for all executables."
exit 0
