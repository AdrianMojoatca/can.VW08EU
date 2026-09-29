param(
    [switch]$UpdateBaseline,
    [string]$RepoRoot,
    [string]$BaselinePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not $RepoRoot -or $RepoRoot.Trim().Length -eq 0) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

if (-not $BaselinePath -or $BaselinePath.Trim().Length -eq 0) {
    $BaselinePath = Join-Path $PSScriptRoot "legacy_gpio_noncore_allowlist.txt"
}

$scanRoots = @(
    "external/can.sdk.db3/APP",
    "external/can.sdk.db3/HAL",
    "external/can.sdk.db3/ENTRY"
)

$excludePathRegexes = @(
    [regex]"\\UNUSED\\",
    [regex]"\\APP\\framework\\lib\.core\\"
)

$fileExtensions = @("*.c", "*.h")

$legacyRegexes = @(
    [regex]"\bOUT_[A-Z0-9_]+\s*\(",
    [regex]"\bIN_NEGATIVE_[0-9]+\b",
    [regex]"\bIN_POSITIVE_[0-9]+\b",
    [regex]"\bPUSH_IN\b",
    [regex]"\bGPIO_PL_[A-Z0-9_]+\b"
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
    if ($normalizedPath.StartsWith($normalizedRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $normalizedPath.Substring($normalizedRoot.Length).Replace('\\', '/')
    }
    return $normalizedPath.Replace('\\', '/')
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
        foreach ($rx in $legacyRegexes) {
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
    Write-Output "Baseline updated: $BaselinePath"
    Write-Output "Tracked files: $(@($current).Count)"
    exit 0
}

$baseline = @()
if (Test-Path $BaselinePath) {
    $baseline = Get-Content $BaselinePath | ForEach-Object { $_.Trim() } | Where-Object { $_ -ne "" } | Sort-Object -Unique
}

$baselineSet = New-Object System.Collections.Generic.HashSet[string]([System.StringComparer]::OrdinalIgnoreCase)
foreach ($item in $baseline) {
    [void]$baselineSet.Add($item)
}

$newViolations = @()
foreach ($item in $current) {
    if (-not $baselineSet.Contains($item)) {
        $newViolations += $item
    }
}

if (@($newViolations).Count -gt 0) {
    Write-Error ("New non-core legacy GPIO usages detected:\n - " + ($newViolations -join "\n - "))
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

Write-Output "OK: no new non-core legacy GPIO usages."
Write-Output "Tracked files: $(@($current).Count)"
if (@($resolved).Count -gt 0) {
    Write-Output "Resolved since baseline: $(@($resolved).Count)"
}
