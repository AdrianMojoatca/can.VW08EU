param(
    [Parameter(Mandatory = $true)]
    [string]$CoreHex,                   # path to the .core.hex to verify

    [string]$RepoRoot,                  # defaults to ..\.. from this script

    [Nullable[UInt32]]$AboutStart = $null,   # override only; else from app.sct LR_ABOUT

    [string]$ArtifactsDir               # optional; defaults to build\out\artifacts\core_gap_guard
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

. (Join-Path $PSScriptRoot 'core_layout_common.ps1')


function Save-Utf8NoBom {
    param(
        [Parameter(Mandatory = $true)] [string]$Path,
        [Parameter(Mandatory = $true)] [string[]]$Lines
    )

    # Write UTF-8 without a BOM (Set-Content -Encoding UTF8 on PS 5.1 adds a BOM).
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllLines($Path, $Lines, $encoding)
}


function Assert-CoreImageGap {
    param(
        [Parameter(Mandatory = $true)] $Layout,
        [Parameter(Mandatory = $true)] [hashtable]$BytesMap,
        [Parameter(Mandatory = $true)] [UInt32]$AboutStart
    )

    # Gap/layout guard on a built CORE .core.hex. Collects all findings (does not
    # stop at the first) so the caller can emit a complete audit even on failure.
    $failures = New-Object System.Collections.Generic.List[string]
    $info = New-Object System.Collections.Generic.List[string]

    $stats = Get-HexExtentStats -BytesMap $BytesMap
    if ($stats.Count -eq 0) {
        $failures.Add("CORE image is empty (no data records).")
        return [pscustomobject]@{ Pass = $false; Failures = $failures; Info = $info; Stats = $stats }
    }

    # 1. nothing below the ABOUT identity block
    if ($stats.Min -lt $AboutStart) {
        $failures.Add("Min address 0x$('{0:X8}' -f $stats.Min) is below ABOUT start 0x$('{0:X8}' -f $AboutStart).")
    }

    # 2. no spill into GAP/FW: max must stay within CORE and below FwStart
    if ($stats.Max -gt $Layout.CoreEnd) {
        $failures.Add("Max address 0x$('{0:X8}' -f $stats.Max) exceeds CoreEnd 0x$('{0:X8}' -f $Layout.CoreEnd).")
    }
    if ($stats.Max -ge $Layout.FwStart) {
        $failures.Add("Max address 0x$('{0:X8}' -f $stats.Max) reaches FW window (FwStart 0x$('{0:X8}' -f $Layout.FwStart)).")
    }

    # 3. no data byte inside the GAP window (CoreEnd, FwStart)
    if ($Layout.GapSize -gt 0) {
        $inGap = 0
        foreach ($address in $BytesMap.Keys) {
            if (($address -ge $Layout.GapStart) -and ($address -le $Layout.GapEnd)) {
                $inGap++
            }
        }
        if ($inGap -gt 0) {
            $failures.Add("$inGap byte(s) fall inside the GAP window 0x$('{0:X8}' -f $Layout.GapStart)-0x$('{0:X8}' -f $Layout.GapEnd).")
        }
    }

    # 4. FwStart must be a flash-sector start
    if (-not (Test-Lpc176xSectorStart -Address $Layout.FwStart)) {
        $failures.Add("FwStart 0x$('{0:X8}' -f $Layout.FwStart) is not on a flash-sector boundary.")
    }

    # 5. CoreEnd and FwStart must live in different sectors (erase-disjoint images)
    $coreEndSector = Get-Lpc176xSectorInfo -Address $Layout.CoreEnd
    $fwStartSector = Get-Lpc176xSectorInfo -Address $Layout.FwStart
    if ($coreEndSector.Index -eq $fwStartSector.Index) {
        $failures.Add("CoreEnd and FwStart share flash sector $($fwStartSector.Index) (images not erase-disjoint).")
    }
    $info.Add("CoreEnd sector = $($coreEndSector.Index), FwStart sector = $($fwStartSector.Index)")

    # 6. gap policy
    if ($Layout.GapSize -lt $Layout.MinGap) {
        $failures.Add("GAP size $($Layout.GapSize) is below MinGap $($Layout.MinGap).")
    }

    # 7. shared contract blocks present and inside CORE
    foreach ($block in @(
            @{ Name = 'SharedAbiAddr'; Addr = $Layout.SharedAbiAddr },
            @{ Name = 'SharedApiAddr'; Addr = $Layout.SharedApiAddr })) {
        $addr = [UInt32]$block.Addr
        if (-not $BytesMap.ContainsKey($addr)) {
            $failures.Add("$($block.Name) block missing at 0x$('{0:X8}' -f $addr).")
        }
        elseif (($addr -lt $Layout.CoreStart) -or ($addr -gt $Layout.CoreEnd)) {
            $failures.Add("$($block.Name) 0x$('{0:X8}' -f $addr) is outside the CORE window.")
        }
    }

    # 8. ABOUT identity present (so hex2enc can stamp a deterministic ENC name)
    if (-not $BytesMap.ContainsKey($AboutStart)) {
        $failures.Add("ABOUT identity block missing at 0x$('{0:X8}' -f $AboutStart); CORE ENC name would be degenerate.")
    }

    return [pscustomobject]@{
        Pass     = ($failures.Count -eq 0)
        Failures = $failures
        Info     = $info
        Stats    = $stats
    }
}


function Write-GapGuardAudit {
    param(
        [Parameter(Mandatory = $true)] [string]$ArtifactsDir,
        [Parameter(Mandatory = $true)] [string]$CoreHexPath,
        [Parameter(Mandatory = $true)] $Layout,
        [Parameter(Mandatory = $true)] $Result
    )

    # Emit audit artifacts with stable names. Console shows only the path + summary.
    if (-not (Test-Path $ArtifactsDir)) {
        New-Item -ItemType Directory -Path $ArtifactsDir -Force | Out-Null
    }

    $stats = $Result.Stats
    $minText = if ($null -eq $stats.Min) { 'n/a' } else { '0x{0:X8}' -f $stats.Min }
    $maxText = if ($null -eq $stats.Max) { 'n/a' } else { '0x{0:X8}' -f $stats.Max }

    $statsObject = [pscustomobject]@{
        core_hex   = $CoreHexPath
        pass       = $Result.Pass
        min_addr   = $minText
        max_addr   = $maxText
        byte_count = $stats.Count
        layout     = [pscustomobject]@{
            core_start = '0x{0:X8}' -f $Layout.CoreStart
            core_end   = '0x{0:X8}' -f $Layout.CoreEnd
            fw_start   = '0x{0:X8}' -f $Layout.FwStart
            fw_end     = '0x{0:X8}' -f $Layout.FwEnd
            gap_size   = $Layout.GapSize
            min_gap    = $Layout.MinGap
        }
        failures   = @($Result.Failures)
        info       = @($Result.Info)
    }

    $statsPath = Join-Path $ArtifactsDir 'core_gap_guard.stats.json'
    $summaryPath = Join-Path $ArtifactsDir 'core_gap_guard.summary.txt'

    Save-Utf8NoBom -Path $statsPath -Lines ($statsObject | ConvertTo-Json -Depth 5)

    $summaryLines = New-Object System.Collections.Generic.List[string]
    $summaryLines.Add("CORE image gap guard")
    $summaryLines.Add("core_hex : $CoreHexPath")
    $summaryLines.Add("result   : $(if ($Result.Pass) { 'PASS' } else { 'FAIL' })")
    $summaryLines.Add("extent   : $minText .. $maxText ($($stats.Count) bytes)")
    $summaryLines.Add("core/fw  : CORE[0x$('{0:X8}' -f $Layout.CoreStart)-0x$('{0:X8}' -f $Layout.CoreEnd)] GAP=$($Layout.GapSize) FW[0x$('{0:X8}' -f $Layout.FwStart)-0x$('{0:X8}' -f $Layout.FwEnd)]")
    foreach ($item in $Result.Info) { $summaryLines.Add("info     : $item") }
    foreach ($item in $Result.Failures) { $summaryLines.Add("FAIL     : $item") }
    Save-Utf8NoBom -Path $summaryPath -Lines $summaryLines.ToArray()

    return [pscustomobject]@{ StatsPath = $statsPath; SummaryPath = $summaryPath }
}


function Write-CrashAudit {
    param(
        [Parameter(Mandatory = $true)] [string]$ArtifactsDir,
        [Parameter(Mandatory = $true)] [string]$CoreHexPath,
        [Parameter(Mandatory = $true)] [string]$Message
    )

    # Ensure an audit artifact exists even when the guard aborts on a hard error
    # (missing header/hex, malformed input) before the normal audit is written.
    if (-not (Test-Path $ArtifactsDir)) {
        New-Item -ItemType Directory -Path $ArtifactsDir -Force | Out-Null
    }

    $statsObject = [pscustomobject]@{ core_hex = $CoreHexPath; pass = $false; error = $Message }
    Save-Utf8NoBom -Path (Join-Path $ArtifactsDir 'core_gap_guard.stats.json') -Lines ($statsObject | ConvertTo-Json -Depth 3)
    Save-Utf8NoBom -Path (Join-Path $ArtifactsDir 'core_gap_guard.summary.txt') -Lines @(
        "CORE image gap guard",
        "core_hex : $CoreHexPath",
        "result   : FAIL (guard error)",
        "error    : $Message")
}


# ---- main ----
if (-not $RepoRoot -or $RepoRoot.Trim().Length -eq 0) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

if (-not $ArtifactsDir -or $ArtifactsDir.Trim().Length -eq 0) {
    $ArtifactsDir = Join-Path $RepoRoot 'build\out\artifacts\core_gap_guard'
}

$resolvedCoreHex = [System.IO.Path]::GetFullPath($CoreHex)

try {
    if (-not (Test-Path $resolvedCoreHex)) {
        throw "CORE hex not found: $resolvedCoreHex"
    }

    # ABOUT window from its single source of truth (the scatter) unless overridden.
    if ($null -eq $AboutStart) {
        $AboutStart = (Get-AboutWindowFromScatter -RepoRootPath $RepoRoot).Start
    }

    $layout = Get-CoreDb3Layout -RepoRootPath $RepoRoot
    $map = Get-IntelHexMap -Path $resolvedCoreHex
    $result = Assert-CoreImageGap -Layout $layout -BytesMap $map -AboutStart $AboutStart
    $audit = Write-GapGuardAudit -ArtifactsDir $ArtifactsDir -CoreHexPath $resolvedCoreHex -Layout $layout -Result $result

    Write-Host "CORE gap guard: $(if ($result.Pass) { 'PASS' } else { 'FAIL' })"
    Write-Host "Artifacts: $ArtifactsDir"
    Write-Host (Get-Content $audit.SummaryPath -Raw)

    if (-not $result.Pass) {
        exit 1
    }

    exit 0
}
catch {
    Write-CrashAudit -ArtifactsDir $ArtifactsDir -CoreHexPath $resolvedCoreHex -Message $_.Exception.Message
    Write-Host "CORE gap guard: FAIL (guard error)"
    Write-Host "Artifacts: $ArtifactsDir"
    Write-Host $_.Exception.Message
    exit 1
}
