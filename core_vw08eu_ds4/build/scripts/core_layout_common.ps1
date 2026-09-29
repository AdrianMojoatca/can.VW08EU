# Shared helpers for the DS4 CORE/FW split tooling.
# Dot-source this file (it defines functions only, no top-level actions):
#   . (Join-Path $PSScriptRoot 'core_layout_common.ps1')

function Get-HexDefineValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Content,
        [Parameter(Mandatory = $true)]
        [string]$DefineName
    )

    $pattern = '(?m)^\s*#define\s+' + [regex]::Escape($DefineName) + '\s*\(0x([0-9A-Fa-f]+)UL\)'
    $match = [regex]::Match($Content, $pattern)
    if (-not $match.Success) {
        throw "Missing or invalid layout define: $DefineName"
    }

    return [UInt32]::Parse($match.Groups[1].Value, [System.Globalization.NumberStyles]::HexNumber)
}

function Get-DefineUIntLiteral {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Content,
        [Parameter(Mandatory = $true)]
        [string]$DefineName
    )

    # Tolerant reader for a plain UInt literal define: (0x..UL) or (<decimal>UL).
    # Does not evaluate expressions (use computed fields for those).
    $pattern = '(?m)^\s*#define\s+' + [regex]::Escape($DefineName) + '\s*\(\s*(0x)?([0-9A-Fa-f]+)\s*UL\s*\)'
    $match = [regex]::Match($Content, $pattern)
    if (-not $match.Success) {
        throw "Missing or non-literal define: $DefineName"
    }

    if ($match.Groups[1].Success) {
        return [UInt32]::Parse($match.Groups[2].Value, [System.Globalization.NumberStyles]::HexNumber)
    }

    return [UInt32]::Parse($match.Groups[2].Value, [System.Globalization.NumberStyles]::Integer)
}

function Get-CoreDs4Layout {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RepoRootPath
    )

    # Load the DS4 flash layout from core_contract_ds4_image_layout.h (single source
    # of truth). Address primitives are read as hex defines; the GAP fields are
    # computed with the same formulas the header uses (they are expressions there).
    $layoutHeaderPath = Join-Path $RepoRootPath 'external\Core_Contract_SDK_DS4\core_contract_ds4_image_layout.h'
    if (-not (Test-Path $layoutHeaderPath)) {
        throw "Layout header not found: $layoutHeaderPath"
    }

    $text = Get-Content $layoutHeaderPath -Raw

    $coreStart = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_CORE_START_ADDR'
    $coreEnd = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_CORE_END_ADDR'
    $fwStart = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_FW_START_ADDR'
    $fwEnd = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_FW_END_ADDR'
    $appStart = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_APP_START_ADDR'
    $appEnd = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_APP_END_ADDR'
    $abiAddr = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_SHARED_ABI_INFO_ADDR'
    $apiAddr = Get-HexDefineValue -Content $text -DefineName 'CORE_DS4_SHARED_API_TABLE_ADDR'
    $minGap = Get-DefineUIntLiteral -Content $text -DefineName 'CORE_DS4_FLASH_MIN_GAP_BYTES'

    return [pscustomobject]@{
        AppStart      = $appStart
        AppEnd        = $appEnd
        CoreStart     = $coreStart
        CoreEnd       = $coreEnd
        FwStart       = $fwStart
        FwEnd         = $fwEnd
        GapStart      = [UInt32]($coreEnd + 1)
        GapEnd        = [UInt32]($fwStart - 1)
        GapSize       = [UInt32]($fwStart - $coreEnd - 1)
        MinGap        = $minGap
        SharedAbiAddr = $abiAddr
        SharedApiAddr = $apiAddr
    }
}

function Get-AboutWindowFromScatter {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RepoRootPath,
        [string]$ScatterRelPath = 'external\can.sdk.ds4\BSP\bootloader\lib.app.scatter.1766\app.sct'
    )

    # The ABOUT identity block is not in the layout header; its single source of
    # truth is the scatter file: `LR_ABOUT 0x<addr> 0x<size>`.
    $scatterPath = Join-Path $RepoRootPath $ScatterRelPath
    if (-not (Test-Path $scatterPath)) {
        throw "Scatter file not found: $scatterPath"
    }

    $text = Get-Content $scatterPath -Raw
    $match = [regex]::Match($text, '(?m)^\s*LR_ABOUT\s+0x([0-9A-Fa-f]+)\s+0x([0-9A-Fa-f]+)')
    if (-not $match.Success) {
        throw "ABOUT load region (LR_ABOUT) not found in scatter: $scatterPath"
    }

    $start = [UInt32]::Parse($match.Groups[1].Value, [System.Globalization.NumberStyles]::HexNumber)
    $size = [UInt32]::Parse($match.Groups[2].Value, [System.Globalization.NumberStyles]::HexNumber)

    return [pscustomobject]@{
        Start = $start
        End   = [UInt32]($start + $size - 1)
    }
}

function Get-Lpc176xSectorInfo {
    param(
        [Parameter(Mandatory = $true)]
        [UInt32]$Address
    )

    # LPC176x on-chip flash sector map (256 KB), dual granularity:
    #   sectors 0..15  = 4 KB  covering 0x00000000..0x0000FFFF
    #   sectors 16..21 = 32 KB covering 0x00010000..0x0003FFFF
    if ($Address -le 0x0000FFFF) {
        $index = [UInt32][Math]::Floor($Address / 0x1000)
        $start = [UInt32]($index * 0x1000)
        $size = [UInt32]0x1000
    }
    elseif ($Address -le 0x0003FFFF) {
        $index = [UInt32](16 + [Math]::Floor(($Address - 0x00010000) / 0x8000))
        $start = [UInt32](0x00010000 + (($index - 16) * 0x8000))
        $size = [UInt32]0x8000
    }
    else {
        throw "Address 0x$('{0:X8}' -f $Address) is outside LPC176x on-chip flash (0..0x3FFFF)."
    }

    return [pscustomobject]@{
        Index = $index
        Start = $start
        End   = [UInt32]($start + $size - 1)
        Size  = $size
    }
}

function Test-Lpc176xSectorStart {
    param(
        [Parameter(Mandatory = $true)]
        [UInt32]$Address
    )

    # True if $Address is the first byte of a flash sector.
    return (Get-Lpc176xSectorInfo -Address $Address).Start -eq $Address
}

function Get-IntelHexMap {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    # Parse an Intel HEX file into an address->byte hashtable.
    # Handles record types 00 (data), 01 (EOF), 04 (extended linear address).
    $bytes = @{}
    $upperAddress = [UInt32]0

    foreach ($line in [System.IO.File]::ReadAllLines($Path)) {
        if ([string]::IsNullOrWhiteSpace($line)) {
            continue
        }

        if (-not $line.StartsWith(':')) {
            throw "Invalid Intel HEX line (missing ':'): $line"
        }

        $byteCount = [Convert]::ToInt32($line.Substring(1, 2), 16)
        $offset = [Convert]::ToUInt16($line.Substring(3, 4), 16)
        $recordType = [Convert]::ToInt32($line.Substring(7, 2), 16)

        switch ($recordType) {
            0 {
                for ($index = 0; $index -lt $byteCount; $index++) {
                    $absAddress = [UInt32]($upperAddress + $offset + $index)
                    $bytes[$absAddress] = [Convert]::ToByte($line.Substring(9 + ($index * 2), 2), 16)
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
                # Other record types are not relevant to the split/guard use-case.
            }
        }
    }

    return $bytes
}

function Get-HexExtentStats {
    param(
        [Parameter(Mandatory = $true)]
        [hashtable]$BytesMap
    )

    # Compute min/max address and byte count over the map keys.
    if ($BytesMap.Count -eq 0) {
        return [pscustomobject]@{ Min = $null; Max = $null; Count = 0 }
    }

    $min = [UInt32]::MaxValue
    $max = [UInt32]0
    foreach ($address in $BytesMap.Keys) {
        if ($address -lt $min) { $min = $address }
        if ($address -gt $max) { $max = $address }
    }

    return [pscustomobject]@{ Min = $min; Max = $max; Count = $BytesMap.Count }
}

function Get-ExpectedEncName {
    param(
        [Parameter(Mandatory = $true)]
        [hashtable]$BytesMap,
        [Parameter(Mandatory = $true)]
        [UInt32]$AboutStart,
        [Parameter(Mandatory = $true)]
        [UInt32]$AboutEnd
    )

    # Replicate enc_rename_933.pl: the ABOUT block holds NUL-separated ASCII fields;
    # the ENC name is lc("933." + field0 + "." + field1 + ".enc") with spaces -> _ in
    # field0 and dots -> _ in field1. Present bytes are walked in address order and
    # split into fields on each 0x00 byte. An image without an ABOUT block yields the
    # degenerate "933...enc" (surfaced by the caller, not silently accepted).
    $fields = @('', '', '')
    $index = 0
    for ($address = $AboutStart; $address -le $AboutEnd; $address++) {
        $key = [UInt32]$address
        if (-not $BytesMap.ContainsKey($key)) {
            continue
        }

        $value = [byte]$BytesMap[$key]
        if ($value -ne 0) {
            if ($index -le 2) {
                $fields[$index] += [char]$value
            }
        }
        else {
            $index++
        }
    }

    $field0 = $fields[0] -replace '\s+', '_'
    $field1 = $fields[1] -replace '\.+', '_'

    return ("933.$field0.$field1.enc").ToLower([System.Globalization.CultureInfo]::InvariantCulture)
}
