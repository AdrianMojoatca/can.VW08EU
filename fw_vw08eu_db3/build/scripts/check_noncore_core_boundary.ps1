param(
    [switch]$UpdateBaseline,
    [string]$RepoRoot,
    [string]$BaselinePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Get-HexDefineValue lives in the shared helper (single source of truth).
. (Join-Path $PSScriptRoot 'core_layout_common.ps1')

function Assert-LayoutContract {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RepoRootPath
    )

    $layoutHeaderPath = Join-Path $RepoRootPath 'external\Core_Contract_SDK_DB3\core_contract_db3_image_layout.h'
    if (-not (Test-Path $layoutHeaderPath)) {
        throw "Layout header not found: $layoutHeaderPath"
    }

    $layoutText = Get-Content $layoutHeaderPath -Raw

    $appStart = Get-HexDefineValue -Content $layoutText -DefineName 'CORE_DB3_FLASH_APP_START_ADDR'
    $appEnd = Get-HexDefineValue -Content $layoutText -DefineName 'CORE_DB3_FLASH_APP_END_ADDR'
    $coreStart = Get-HexDefineValue -Content $layoutText -DefineName 'CORE_DB3_FLASH_CORE_START_ADDR'
    $coreEnd = Get-HexDefineValue -Content $layoutText -DefineName 'CORE_DB3_FLASH_CORE_END_ADDR'
    $fwStart = Get-HexDefineValue -Content $layoutText -DefineName 'CORE_DB3_FLASH_FW_START_ADDR'
    $fwEnd = Get-HexDefineValue -Content $layoutText -DefineName 'CORE_DB3_FLASH_FW_END_ADDR'

    if ($appStart -ne $coreStart) {
        throw "Layout contract failed: APP start (0x$('{0:X8}' -f $appStart)) must equal CORE start (0x$('{0:X8}' -f $coreStart))."
    }

    if ($appEnd -ne $fwEnd) {
        throw "Layout contract failed: APP end (0x$('{0:X8}' -f $appEnd)) must equal FW end (0x$('{0:X8}' -f $fwEnd))."
    }

    if ($coreEnd -ge $fwStart) {
        throw "Layout contract failed: CORE end (0x$('{0:X8}' -f $coreEnd)) must be below FW start (0x$('{0:X8}' -f $fwStart))."
    }

    $gapSize = [UInt32]($fwStart - $coreEnd - 1)
    Write-Output ("Layout contract OK: CORE[0x{0:X8}-0x{1:X8}] GAP={2} bytes FW[0x{3:X8}-0x{4:X8}]" -f $coreStart, $coreEnd, $gapSize, $fwStart, $fwEnd)
}

if (-not $RepoRoot -or $RepoRoot.Trim().Length -eq 0) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

Assert-LayoutContract -RepoRootPath $RepoRoot

if (-not $BaselinePath -or $BaselinePath.Trim().Length -eq 0) {
    $BaselinePath = Join-Path $PSScriptRoot "noncore_core_boundary_allowlist.txt"
}

$scanRoots = @(
    "VEHICLE_SPECIFIC",
    "external/can.sdk.db3/APP",
    "external/can.sdk.db3/HAL",
    "external/can.sdk.db3/ENTRY"
)

$excludePathRegexes = @(
    [regex]"\\UNUSED\\",
    [regex]"\\external\\can\.sdk\.db3\\APP\\framework\\lib\.core\\",
    [regex]"\\external\\Core_Contract_SDK_DB3\\"
)

$fileExtensions = @("*.c", "*.h")

$boundaryRegexes = @(
    [regex]'(?m)^\s*#include\s+"core\.h"',
    [regex]'(?m)^\s*#include\s+".*APP/framework/lib\.core/.*"',
    [regex]'(?m)^\s*#include\s+".*lib\.core.*"'
)

function Is-ExcludedFile([string]$fullPath) {
    foreach ($ex in $excludePathRegexes) {
        if ($ex.IsMatch($fullPath)) {
            return $true
        }
    }
    return $false
}

function To-RepoRelativePath([string]$fullPath) {
    $normalizedRoot = [System.IO.Path]::GetFullPath($RepoRoot).TrimEnd('\\') + "\\"
    $normalizedPath = [System.IO.Path]::GetFullPath($fullPath)
    $relativeIndex = $normalizedPath.IndexOf($normalizedRoot, [System.StringComparison]::OrdinalIgnoreCase)
    if ($relativeIndex -ge 0) {
        return $normalizedPath.Substring($relativeIndex + $normalizedRoot.Length).Replace('\\', '/')
    }
    return $normalizedPath.Replace('\\', '/')
}

function Normalize-BaselinePath([string]$pathValue) {
    if (-not $pathValue -or $pathValue.Trim().Length -eq 0) {
        return ""
    }

    $trimmed = $pathValue.Trim()
    if ([System.IO.Path]::IsPathRooted($trimmed)) {
        return To-RepoRelativePath $trimmed
    }

    return $trimmed.Replace('\\', '/')
}

$violatingFiles = New-Object System.Collections.Generic.HashSet[string]([System.StringComparer]::OrdinalIgnoreCase)

foreach ($relRoot in $scanRoots) {
    $absRoot = Join-Path $RepoRoot $relRoot
    if (-not (Test-Path $absRoot)) {
        continue
    }

    $files = Get-ChildItem -Path $absRoot -Recurse -File -Include $fileExtensions
    foreach ($file in $files) {
        if (Is-ExcludedFile $file.FullName) {
            continue
        }

        $content = Get-Content $file.FullName -Raw
        $isViolation = $false
        foreach ($rx in $boundaryRegexes) {
            if ($rx.IsMatch($content)) {
                $isViolation = $true
                break
            }
        }

        if ($isViolation) {
            [void]$violatingFiles.Add((To-RepoRelativePath $file.FullName))
        }
    }
}

$current = @($violatingFiles) | Sort-Object

if ($UpdateBaseline) {
    $dir = Split-Path -Parent $BaselinePath
    if (-not (Test-Path $dir)) {
        New-Item -Path $dir -ItemType Directory -Force | Out-Null
    }
    Set-Content -Path $BaselinePath -Value $current -Encoding UTF8
    Write-Output "Boundary baseline updated: $BaselinePath"
    Write-Output "Tracked files: $(@($current).Count)"
    exit 0
}

$baseline = @()
if (Test-Path $BaselinePath) {
    $baseline = Get-Content $BaselinePath |
        ForEach-Object { Normalize-BaselinePath $_ } |
        Where-Object { $_ -ne "" } |
        Sort-Object -Unique
}

$baselineSet = New-Object System.Collections.Generic.HashSet[string]([System.StringComparer]::OrdinalIgnoreCase)
foreach ($item in $baseline) {
    [void]$baselineSet.Add($item)

    if (-not [System.IO.Path]::IsPathRooted($item)) {
        $absoluteFromRelative = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $item)).Replace('\\', '/')
        [void]$baselineSet.Add($absoluteFromRelative)
    }
    else {
        $normalizedRelative = To-RepoRelativePath $item
        [void]$baselineSet.Add($normalizedRelative)
    }
}

$newViolations = @()
foreach ($item in $current) {
    $isKnown = $baselineSet.Contains($item)

    if (-not $isKnown -and [System.IO.Path]::IsPathRooted($item)) {
        $relativeCandidate = To-RepoRelativePath $item
        $isKnown = $baselineSet.Contains($relativeCandidate)
    }

    if (-not $isKnown -and -not [System.IO.Path]::IsPathRooted($item)) {
        $absoluteCandidate = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $item)).Replace('\\', '/')
        $isKnown = $baselineSet.Contains($absoluteCandidate)
    }

    if (-not $isKnown) {
        $newViolations += $item
    }
}

if (@($newViolations).Count -gt 0) {
    Write-Error ("New non-core core-boundary violations detected:\n - " + ($newViolations -join "\n - "))
    exit 1
}

$resolved = @()
$currentSet = New-Object System.Collections.Generic.HashSet[string]([System.StringComparer]::OrdinalIgnoreCase)
foreach ($item in $current) {
    [void]$currentSet.Add($item)
}
foreach ($item in $baseline) {
    if (-not $currentSet.Contains($item)) {
        $resolved += $item
    }
}

Write-Output "OK: no new non-core core-boundary violations."
Write-Output "Tracked files: $(@($current).Count)"
if (@($resolved).Count -gt 0) {
    Write-Output "Resolved since baseline: $(@($resolved).Count)"
}
