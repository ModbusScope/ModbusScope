param(
    [Parameter(Mandatory = $true)][string]$DeployDir,
    [Parameter(Mandatory = $true)][string]$Executables,
    [int]$TimeoutSeconds = 5
)

$ErrorActionPreference = 'Stop'
$VerbosePreference = 'Continue'

# NTSTATUS values Windows uses when a process image fails to load. Bare hex
# literals above 0x7FFFFFFF parse as (negative) Int32, not UInt32, so they'd
# never match $code below (also UInt32) unless reinterpreted the same way.
function ConvertTo-UInt32Bits([int]$value) {
    return [uint32]([int64]$value -band 0xFFFFFFFFL)
}
$LoaderFailureCodes = @{
    (ConvertTo-UInt32Bits 0xC0000135) = 'STATUS_DLL_NOT_FOUND - a required DLL could not be located'
    (ConvertTo-UInt32Bits 0xC0000139) = 'STATUS_ENTRYPOINT_NOT_FOUND - a DLL is present but missing an expected export'
    (ConvertTo-UInt32Bits 0xC000007B) = 'STATUS_INVALID_IMAGE_FORMAT - architecture/bitness mismatch'
    (ConvertTo-UInt32Bits 0xC0000005) = 'STATUS_ACCESS_VIOLATION - crashed on startup'
}

# $Executables is a single comma-separated string, not [string[]]: PowerShell's
# -File argument binder does not slurp multiple bareword/CLI tokens into an
# array parameter, so a batch caller can't produce a real array literal.
$executableList = $Executables -split ','

# Launch everything first so one shared timeout covers all executables.
$launched = foreach ($exeName in $executableList) {
    $exePath = Join-Path $DeployDir $exeName
    if (-not (Test-Path $exePath)) {
        Write-Verbose "[FAIL] $exeName - not found in $DeployDir"
        [PSCustomObject]@{ Name = $exeName; Process = $null }
        continue
    }
    Write-Verbose "Starting $exeName ..."
    # Use Process.Start directly rather than the Start-Process cmdlet: with
    # -PassThru, Start-Process's returned object unreliably reports ExitCode
    # (observed blank/0 even for a process that has genuinely exited).
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $exePath
    $psi.WorkingDirectory = $DeployDir
    $psi.UseShellExecute = $false
    # Give the child its own stdin pipe rather than inheriting ours: on a
    # non-interactive CI runner, inherited stdin can already be at EOF, which
    # makes a stdio-server adapter read no request and exit(0) immediately
    # instead of idling - a false failure unrelated to missing DLLs. An
    # explicit pipe we never close/write to just blocks forever, in every
    # environment.
    $psi.RedirectStandardInput = $true
    $proc = [System.Diagnostics.Process]::Start($psi)
    [PSCustomObject]@{ Name = $exeName; Process = $proc }
}

Start-Sleep -Seconds $TimeoutSeconds

$failures = @()
foreach ($item in $launched) {
    if (-not $item.Process) { $failures += $item.Name; continue }
    $proc = $item.Process
    if ($proc.HasExited) {
        # ExitCode is a signed Int32; loader failure NTSTATUS codes (e.g.
        # 0xC0000135) are >= 0x80000000, so reinterpret the bits rather than
        # casting directly (a direct [uint32] cast overflows on negative values).
        $code = ConvertTo-UInt32Bits $proc.ExitCode
        $hex = '0x{0:X8}' -f $code
        if ($LoaderFailureCodes.ContainsKey($code)) {
            Write-Output "[FAIL] $($item.Name) exited after ${TimeoutSeconds}s with $hex ($($LoaderFailureCodes[$code]))"
        } else {
            Write-Output "[FAIL] $($item.Name) exited early after ${TimeoutSeconds}s with exit code $($proc.ExitCode) ($hex)"
        }
        $failures += $item.Name
    } else {
        Write-Output "[PASS] $($item.Name) still running after ${TimeoutSeconds}s"
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    }
}

if ($failures.Count -gt 0) {
    Write-Output ""
    Write-Output "Deployment smoke test FAILED for: $($failures -join ', ')"
    exit 1
}
Write-Output ""
Write-Output "Deployment smoke test passed for all executables."
exit 0
