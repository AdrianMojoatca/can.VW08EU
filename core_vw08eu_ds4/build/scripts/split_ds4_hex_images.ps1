param(
    [Parameter(Mandatory = $false)]
    [string]$InputHex = ".\build\ds4_build\main_ds4_release.hex",

    [Parameter(Mandatory = $false)]
    [string]$OutputDir = ".\build\ds4_build",

    [Parameter(Mandatory = $false)]
    [string]$RepoRoot,                        # defaults to ..\.. from this script

    [Parameter(Mandatory = $false)]
    [Nullable[UInt32]]$CoreStart = $null,     # override only; else from image_layout.h

    [Parameter(Mandatory = $false)]
    [Nullable[UInt32]]$CoreEnd = $null,

    [Parameter(Mandatory = $false)]
    [Nullable[UInt32]]$FwStart = $null,

    [Parameter(Mandatory = $false)]
    [Nullable[UInt32]]$FwEnd = $null,

    [Parameter(Mandatory = $false)]
    [Nullable[UInt32]]$AboutStart = $null,    # override only; else from app.sct LR_ABOUT

    [Parameter(Mandatory = $false)]
    [Nullable[UInt32]]$AboutEnd = $null,

    [Parameter(Mandatory = $false)]
    [bool]$IncludeAboutInCore = $true         # include ABOUT in CORE image so hex2enc can stamp identity
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

. (Join-Path $PSScriptRoot 'core_layout_common.ps1')


function Convert-HexByte {
    param([string]$Hex)
    return [Convert]::ToByte($Hex, 16)
}


function Parse-IntelHex {
    param([string]$Path)

    $bytes = @{}
    $upperAddress = 0

    foreach ($line in [System.IO.File]::ReadAllLines($Path)) {
        if ([string]::IsNullOrWhiteSpace($line)) {
            continue
        }

        if (-not $line.StartsWith(':')) {
            throw "Invalid Intel HEX line (missing ':'): $line"
        }

        $byteCount = Convert-HexByte $line.Substring(1, 2)
        $offset = [Convert]::ToUInt16($line.Substring(3, 4), 16)
        $recordType = Convert-HexByte $line.Substring(7, 2)

        switch ($recordType) {
            0 {
                for ($index = 0; $index -lt $byteCount; $index++) {
                    $dataHex = $line.Substring(9 + ($index * 2), 2)
                    $absAddress = [UInt32]($upperAddress + $offset + $index)
                    $bytes[$absAddress] = Convert-HexByte $dataHex
                }
            }
            1 {
                break
            }
            4 {
                $upper = [Convert]::ToUInt32($line.Substring(9, 4), 16)
                $upperAddress = [UInt32]($upper -shl 16)
            }
            default {
                # Unsupported records are ignored for split use-case.
            }
        }
    }

    return $bytes
}


function Get-IntelHexLine {
    param(
        [byte[]]$Payload,
        [UInt16]$Offset,
        [byte]$RecordType
    )

    $count = [byte]$Payload.Length
    $sum = [int]$count + (($Offset -shr 8) -band 0xFF) + ($Offset -band 0xFF) + $RecordType

    $dataText = ""
    foreach ($item in $Payload) {
        $sum += $item
        $dataText += $item.ToString('X2')
    }

    $checksum = [byte]((0x100 - ($sum -band 0xFF)) -band 0xFF)
    return ":{0}{1}{2}{3}{4}" -f $count.ToString('X2'), $Offset.ToString('X4'), $RecordType.ToString('X2'), $dataText, $checksum.ToString('X2')
}


function Write-IntelHex {
    param(
        [hashtable]$BytesMap,
        [string]$Path
    )

    $lines = New-Object System.Collections.Generic.List[string]

    if ($BytesMap.Count -eq 0) {
        $lines.Add(':00000001FF')
        [System.IO.File]::WriteAllLines($Path, $lines)
        return
    }

    $addresses = $BytesMap.Keys | Sort-Object
    $currentUpper = [UInt32]4294967295
    $chunk = New-Object System.Collections.Generic.List[byte]
    $chunkStart = [UInt32]0

    function Flush-Chunk {
        param(
            [System.Collections.Generic.List[byte]]$Chunk,
            [UInt32]$StartAddress,
            [System.Collections.Generic.List[string]]$OutLines
        )

        if ($Chunk.Count -eq 0) {
            return
        }

        $payload = $Chunk.ToArray()
        $offset = [UInt16]($StartAddress -band 0xFFFF)
        $OutLines.Add((Get-IntelHexLine -Payload $payload -Offset $offset -RecordType 0))
        $Chunk.Clear()
    }

    foreach ($address in $addresses) {
        $upper = [UInt32]($address -band 0xFFFF0000)

        if ($upper -ne $currentUpper) {
            Flush-Chunk -Chunk $chunk -StartAddress $chunkStart -OutLines $lines
            $currentUpper = $upper
            $upperValue = [UInt16]($upper -shr 16)
            $upperBytes = [byte[]]@([byte](($upperValue -shr 8) -band 0xFF), [byte]($upperValue -band 0xFF))
            $lines.Add((Get-IntelHexLine -Payload $upperBytes -Offset 0 -RecordType 4))
            $chunkStart = $address
        }

        if ($chunk.Count -eq 0) {
            $chunkStart = $address
        }

        $expectedNext = $chunkStart + [UInt32]$chunk.Count
        if (($address -ne $expectedNext) -or ($chunk.Count -ge 16)) {
            Flush-Chunk -Chunk $chunk -StartAddress $chunkStart -OutLines $lines
            $chunkStart = $address
        }

        $chunk.Add([byte]$BytesMap[$address])
    }

    Flush-Chunk -Chunk $chunk -StartAddress $chunkStart -OutLines $lines
    $lines.Add(':00000001FF')
    [System.IO.File]::WriteAllLines($Path, $lines)
}


function Add-CoreIdentityRegion {
    param(
        [hashtable]$AllBytes,
        [hashtable]$CoreBytes,
        [UInt32]$AboutStart,
        [UInt32]$AboutEnd
    )

    # Copy the ABOUT identity block (per app.sct LR_ABOUT) from the full image into
    # the CORE byte map so hex2enc_933 / enc_rename_933.pl can read name+version from
    # the 0x8000 block and emit a deterministic CORE .enc filename (fixes the FW/core
    # degenerate-name collision). Addresses already present are left untouched.
    $copied = 0
    foreach ($address in $AllBytes.Keys) {
        if (($address -ge $AboutStart) -and ($address -le $AboutEnd)) {
            if (-not $CoreBytes.ContainsKey($address)) {
                $CoreBytes[$address] = $AllBytes[$address]
                $copied++
            }
        }
    }

    return $copied
}


function Assert-AboutRegionContract {
    param(
        [UInt32]$AboutStart, [UInt32]$AboutEnd,
        [UInt32]$CoreStart,  [UInt32]$FwStart, [UInt32]$FwEnd
    )

    # Fail-fast on a malformed or unsafe ABOUT window.
    if ($AboutStart -gt $AboutEnd) {
        throw "Invalid ABOUT range: start (0x$('{0:X8}' -f $AboutStart)) > end (0x$('{0:X8}' -f $AboutEnd))"
    }

    if ($AboutEnd -ge $CoreStart) {
        throw "ABOUT window (0x$('{0:X8}' -f $AboutStart)-0x$('{0:X8}' -f $AboutEnd)) must sit below CoreStart (0x$('{0:X8}' -f $CoreStart))"
    }

    if (($AboutStart -le $FwEnd) -and ($FwStart -le $AboutEnd)) {
        throw "ABOUT window (0x$('{0:X8}' -f $AboutStart)-0x$('{0:X8}' -f $AboutEnd)) overlaps the FW window (0x$('{0:X8}' -f $FwStart)-0x$('{0:X8}' -f $FwEnd))"
    }
}


# Resolve the split window from the single sources of truth (image_layout.h for
# CORE/FW, app.sct for ABOUT); explicit params override for testing only.
if (-not $RepoRoot -or $RepoRoot.Trim().Length -eq 0) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

$layout = Get-CoreDs4Layout -RepoRootPath $RepoRoot
if ($null -eq $CoreStart) { $CoreStart = $layout.CoreStart }
if ($null -eq $CoreEnd)   { $CoreEnd = $layout.CoreEnd }
if ($null -eq $FwStart)   { $FwStart = $layout.FwStart }
if ($null -eq $FwEnd)     { $FwEnd = $layout.FwEnd }

if ($IncludeAboutInCore -and (($null -eq $AboutStart) -or ($null -eq $AboutEnd))) {
    $aboutWindow = Get-AboutWindowFromScatter -RepoRootPath $RepoRoot
    if ($null -eq $AboutStart) { $AboutStart = $aboutWindow.Start }
    if ($null -eq $AboutEnd)   { $AboutEnd = $aboutWindow.End }
}

if ($CoreStart -gt $CoreEnd) {
    throw "Invalid core range: start ($CoreStart) > end ($CoreEnd)"
}

if ($FwStart -gt $FwEnd) {
    throw "Invalid fw range: start ($FwStart) > end ($FwEnd)"
}

if (($CoreStart -le $FwEnd) -and ($FwStart -le $CoreEnd)) {
    throw "Core and FW ranges overlap."
}

$inputPath = [System.IO.Path]::GetFullPath($InputHex)
if (-not (Test-Path $inputPath)) {
    throw "Input HEX not found: $inputPath"
}

$outputPath = [System.IO.Path]::GetFullPath($OutputDir)
if (-not (Test-Path $outputPath)) {
    New-Item -ItemType Directory -Path $outputPath | Out-Null
}

$allBytes = Parse-IntelHex -Path $inputPath
$coreBytes = @{}
$fwBytes = @{}
$aboutCopied = 0

foreach ($address in $allBytes.Keys) {
    if (($address -ge $CoreStart) -and ($address -le $CoreEnd)) {
        $coreBytes[$address] = $allBytes[$address]
    }
    elseif (($address -ge $FwStart) -and ($address -le $FwEnd)) {
        $fwBytes[$address] = $allBytes[$address]
    }
}

if ($IncludeAboutInCore) {
    Assert-AboutRegionContract -AboutStart $AboutStart -AboutEnd $AboutEnd `
        -CoreStart $CoreStart -FwStart $FwStart -FwEnd $FwEnd

    $aboutCopied = Add-CoreIdentityRegion -AllBytes $allBytes -CoreBytes $coreBytes `
        -AboutStart $AboutStart -AboutEnd $AboutEnd

    if ($aboutCopied -eq 0) {
        Write-Warning "ABOUT identity block (0x$('{0:X8}' -f $AboutStart)-0x$('{0:X8}' -f $AboutEnd)) not found in input; CORE ENC identity may be degenerate."
    }
}

$baseName = [System.IO.Path]::GetFileNameWithoutExtension($inputPath)
$coreHex = Join-Path $outputPath ($baseName + '.core.hex')
$fwHex = Join-Path $outputPath ($baseName + '.fw.hex')

Write-IntelHex -BytesMap $coreBytes -Path $coreHex
Write-IntelHex -BytesMap $fwBytes -Path $fwHex

Write-Host "Split complete:"
Write-Host "  Input : $inputPath"
Write-Host "  Core  : $coreHex ($($coreBytes.Count - $aboutCopied) bytes in range 0x$('{0:X8}' -f $CoreStart)-0x$('{0:X8}' -f $CoreEnd))"
if ($IncludeAboutInCore) {
    Write-Host "  About : included 0x$('{0:X8}' -f $AboutStart)-0x$('{0:X8}' -f $AboutEnd) in CORE image ($aboutCopied bytes)"
}
Write-Host "  FW    : $fwHex ($($fwBytes.Count) bytes in range 0x$('{0:X8}' -f $FwStart)-0x$('{0:X8}' -f $FwEnd))"
