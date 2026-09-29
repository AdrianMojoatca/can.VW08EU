<#
.SYNOPSIS
  TEST-ONLY: union the authoritative CORE + FW DS4 images into one bring-up .hex
  (and optionally one .enc), with full audit artifacts.

.DESCRIPTION
  This produces a single combined image used ONLY to validate, on the bench, that
  the split-compiled code boots and hands off correctly (CORE -> FW at address C),
  WITHOUT the confound of the two-stage flashing flow. It is NOT the production
  deployment path: production stays two-image / two-stage (separate CORE.enc + FW.enc).

  Standing assumptions (also written to summary.txt):
    - The bootloader at 0x00000000..0x00007FFF is already flashed on the device;
      this image covers only 0x00008000 and above.
    - A PASS proves the split-compiled code + CORE->FW handoff boot; it does NOT
      validate the two-stage flashing protocol (that is a separate, device-side concern).

  Host-side build tooling (PowerShell). It operates on Keil build outputs (Intel-HEX)
  only; it is not firmware and does not run on the device.
#>
param(
    [string]$CoreHex   = "build\core_ds4_build\core_ds4_release.hex",
    [string]$FwHex     = "build\fw_ds4_build\fw_ds4_release.hex",
    [string]$OutputHex = "build\ds4_build\combined\combined_core_fw_ds4.hex",   # dedicated dir: out.bat globs %ABS_PATH%\*.hex
    [switch]$RunEnc,
    [string]$StableEncName = "combined_TEST_core_fw_ds4.enc",                    # unmistakable TEST-only name
    [string]$Platform  = "NGMM",
    [string]$RepoRoot,
    [string]$LayoutHeader,
    [string]$ArtifactsRoot = "artifacts",
    [string]$RunId,
    [UInt32]$AboutStart = 0x00008000,   # ABOUT identity block base (app.sct LR_ABOUT)
    [UInt32]$AboutEnd   = 0x000080FF    # ABOUT identity block end  (LR_ABOUT size 0x100)
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ===========================
# INTEL HEX I/O  (self-contained; logic mirrors split_ds4_hex_images.ps1)
# ===========================

# ------------------------------------------------------------
function Convert-HexByte {
    param([string]$Hex)
    return [Convert]::ToByte($Hex, 16)
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Parse-IntelHex {
    param([string]$Path)

    # Read an Intel-HEX file into a hashtable {[UInt32]addr = [byte]value}.
    #   00 data, 04 extended-linear-address -> handled.
    #   01 EOF -> stop consuming.
    #   03/05 start-address records carry no flash data -> ignored (Keil emits a 05).
    #   02 extended-segment-address would remap addresses and is unsupported -> throw.
    #   anything else -> throw (no silent drop that could corrupt the map).
    $bytes = @{}
    $upperAddress = [UInt32]0

    foreach ($rawLine in [System.IO.File]::ReadAllLines($Path)) {
        if ([string]::IsNullOrWhiteSpace($rawLine)) { continue }
        $line = $rawLine.Trim()
        if (-not $line.StartsWith(':')) {
            throw "Invalid Intel HEX line (missing ':') in ${Path}: $line"
        }

        $byteCount = Convert-HexByte $line.Substring(1, 2)
        $offset = [Convert]::ToUInt16($line.Substring(3, 4), 16)
        $recordType = Convert-HexByte $line.Substring(7, 2)
        $isEof = $false

        switch ($recordType) {
            0 {
                for ($index = 0; $index -lt $byteCount; $index++) {
                    $dataHex = $line.Substring(9 + ($index * 2), 2)
                    $absAddress = [UInt32]($upperAddress + $offset + $index)
                    $bytes[$absAddress] = Convert-HexByte $dataHex
                }
            }
            1 { $isEof = $true }
            3 { }   # start-segment-address: no flash data
            5 { }   # start-linear-address (entry point): no flash data
            4 {
                $upper = [Convert]::ToUInt16($line.Substring(9, 4), 16)
                $upperAddress = [UInt32]([UInt32]$upper -shl 16)
            }
            2 {
                throw ("Unsupported Intel HEX record type 02 (extended segment address) in ${Path}: $line")
            }
            default {
                throw ("Unsupported Intel HEX record type 0x{0:X2} in ${Path}: {1}" -f $recordType, $line)
            }
        }

        if ($isEof) { break }
    }

    return $bytes
}
# ------------------------------------------------------------

# ------------------------------------------------------------
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
# ------------------------------------------------------------

# ------------------------------------------------------------
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
# ------------------------------------------------------------

# ===========================
# LAYOUT CONTRACT
# ===========================

# ------------------------------------------------------------
function Get-LayoutDefineValue {
    param(
        [string]$Content,
        [string]$DefineName
    )

    # Read a LITERAL numeric define from the layout header. Tolerates hex/decimal
    # literals with optional U/L suffixes and surrounding spaces, e.g.:
    #   (0x00027FFFUL)  (0x27FFF UL)  (0UL)  (0 UL)  (16U)
    # Expression-valued defines (GAP_*, *_SIZE) are NOT literals -> compute in PS.
    $pattern = '(?m)^\s*#define\s+' + [regex]::Escape($DefineName) + '\s*\(\s*(0[xX][0-9A-Fa-f]+|[0-9]+)\s*[Uu]?[Ll]?[Ll]?\s*\)'
    $match = [regex]::Match($Content, $pattern)
    if (-not $match.Success) {
        throw "Missing or non-literal layout define: $DefineName"
    }

    $token = $match.Groups[1].Value
    if ($token.StartsWith('0x') -or $token.StartsWith('0X')) {
        return [UInt32]::Parse($token.Substring(2), [System.Globalization.NumberStyles]::HexNumber)
    }
    return [UInt32]::Parse($token, [System.Globalization.NumberStyles]::Integer)
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Read-LayoutContract {
    param([string]$LayoutHeaderPath)

    if (-not (Test-Path $LayoutHeaderPath)) {
        throw "Layout header not found: $LayoutHeaderPath"
    }

    $text = Get-Content $LayoutHeaderPath -Raw

    $appStart  = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_APP_START_ADDR'
    $appEnd    = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_APP_END_ADDR'
    $coreStart = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_CORE_START_ADDR'
    $coreEnd   = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_CORE_END_ADDR'
    $fwStart   = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_FW_START_ADDR'
    $fwEnd     = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_FW_END_ADDR'
    $minGap    = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_FLASH_MIN_GAP_BYTES'
    $abiAddr   = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_SHARED_ABI_INFO_ADDR'
    $apiAddr   = Get-LayoutDefineValue -Content $text -DefineName 'CORE_DS4_SHARED_API_TABLE_ADDR'

    # Header self-consistency (mirror of the C compile-time asserts).
    if ($appStart -ne $coreStart) { throw ("Layout: APP start 0x{0:X8} != CORE start 0x{1:X8}" -f $appStart, $coreStart) }
    if ($appEnd   -ne $fwEnd)     { throw ("Layout: APP end 0x{0:X8} != FW end 0x{1:X8}"     -f $appEnd, $fwEnd) }
    if ($coreEnd  -ge $fwStart)   { throw ("Layout: CORE end 0x{0:X8} must be below FW start 0x{1:X8}" -f $coreEnd, $fwStart) }

    $gapSize = [UInt32]($fwStart - $coreEnd - 1)

    return [pscustomobject]@{
        AppStart    = $appStart
        AppEnd      = $appEnd
        CoreStart   = $coreStart
        CoreEnd     = $coreEnd
        FwStart     = $fwStart
        FwEnd       = $fwEnd
        MinGap      = $minGap
        GapSize     = $gapSize
        AbiInfoAddr = $abiAddr
        ApiTableAddr = $apiAddr
    }
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Test-Lpc176xSectorBoundary {
    param([UInt32]$Address)

    # LPC176x dual-granularity flash sector map:
    #   sectors 0..15  : 4 KB each,  0x00000000..0x0000FFFF
    #   sectors 16..   : 32 KB each, 0x00010000 and above
    if ($Address -lt 0x00010000) {
        return (($Address % 0x1000) -eq 0)
    }
    return ((($Address - 0x00010000) % 0x8000) -eq 0)
}
# ------------------------------------------------------------

# ===========================
# AUDIT HELPERS
# ===========================

# ------------------------------------------------------------
function New-RunId {
    # Timestamp + short GUID so two runs in the same second never collide.
    $stamp = (Get-Date).ToString('yyyyMMdd_HHmmss')
    $suffix = ([guid]::NewGuid().ToString('N')).Substring(0, 8)
    return "combine_${stamp}_${suffix}"
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Get-Sha256Hex {
    param([string]$Path)
    return (Get-FileHash -Algorithm SHA256 -Path $Path).Hash.ToLower()
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Add-Event {
    param(
        [System.Collections.Generic.List[string]]$Events,
        [string]$Level,
        [string]$Component,
        [string]$EventType,
        [string]$Message,
        $Context
    )

    $record = [ordered]@{
        ts         = [DateTime]::UtcNow.ToString('o')
        level      = $Level
        component  = $Component
        event_type = $EventType
        message    = $Message
        context    = $Context
    }
    $Events.Add(($record | ConvertTo-Json -Compress -Depth 8))
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Get-AboutIdentity {
    param(
        [hashtable]$CoreBytes,
        [UInt32]$AboutStart,
        [UInt32]$AboutEnd
    )

    # Decode the ABOUT block for traceability: the .enc filename reflects CORE
    # identity only, so we record the raw ABOUT bytes/ascii/hash here as well.
    $collected = New-Object System.Collections.Generic.List[byte]
    for ($addr = $AboutStart; $addr -le $AboutEnd; $addr++) {
        $key = [UInt32]$addr
        if ($CoreBytes.ContainsKey($key)) {
            $collected.Add([byte]$CoreBytes[$key])
        }
    }

    $arr = $collected.ToArray()
    $asciiChars = foreach ($b in $arr) { if ($b -ge 32 -and $b -le 126) { [char]$b } else { '.' } }
    $ascii = (-join $asciiChars).Trim('.', ' ')
    $hex = (($arr | ForEach-Object { $_.ToString('x2') }) -join '')

    $sha256 = ''
    if ($arr.Length -gt 0) {
        $sha = [System.Security.Cryptography.SHA256]::Create()
        try {
            $sha256 = (($sha.ComputeHash($arr) | ForEach-Object { $_.ToString('x2') }) -join '')
        }
        finally {
            $sha.Dispose()
        }
    }

    return [pscustomobject]@{
        ByteCount = $arr.Length
        Ascii     = $ascii
        Hex       = $hex
        Sha256    = $sha256
    }
}
# ------------------------------------------------------------

# ===========================
# VALIDATION
# ===========================

# ------------------------------------------------------------
function Add-Check {
    param(
        [System.Collections.Generic.List[object]]$Checks,
        [string]$Name,
        [bool]$Ok,
        [string]$Detail
    )
    $Checks.Add([pscustomobject]@{ name = $Name; ok = $Ok; detail = $Detail })
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Get-MapExtents {
    param([hashtable]$Map)
    $mm = $Map.Keys | Measure-Object -Minimum -Maximum
    return [pscustomobject]@{
        Min   = [UInt32]$mm.Minimum
        Max   = [UInt32]$mm.Maximum
        Count = $Map.Count
    }
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Test-Disjoint {
    param(
        [hashtable]$CoreBytes,
        [hashtable]$FwBytes,
        [System.Collections.Generic.List[object]]$Checks,
        [System.Collections.Generic.List[string]]$Events
    )

    # Fail on ANY overlapping address (even if the byte value matches): a shared
    # address between the two independently-linked images is a contract smell.
    $overlap = 0
    $sample = New-Object System.Collections.Generic.List[string]
    foreach ($key in $CoreBytes.Keys) {
        if ($FwBytes.ContainsKey($key)) {
            $overlap++
            if ($sample.Count -lt 8) { $sample.Add(("0x{0:X8}" -f $key)) }
        }
    }

    $ok = ($overlap -eq 0)
    $detail = if ($ok) { "no overlapping addresses" } else { "overlap=$overlap (e.g. $($sample -join ', '))" }
    Add-Check -Checks $Checks -Name 'disjoint' -Ok $ok -Detail $detail
    $lvl = if ($ok) { 'info' } else { 'error' }
    Add-Event -Events $Events -Level $lvl -Component 'validate' -EventType 'disjoint' -Message $detail -Context @{ overlap = $overlap }
    return $overlap
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Test-CombineSanity {
    param(
        [pscustomobject]$CoreExtents,
        [pscustomobject]$FwExtents,
        [int]$CombinedCount,
        [hashtable]$CoreBytes,
        [pscustomobject]$Layout,
        [UInt32]$AboutStart,
        [UInt32]$AboutEnd,
        [System.Collections.Generic.List[object]]$Checks,
        [System.Collections.Generic.List[string]]$Events
    )

    $combinedMin = [Math]::Min([uint64]$CoreExtents.Min, [uint64]$FwExtents.Min)
    $combinedMax = [Math]::Max([uint64]$CoreExtents.Max, [uint64]$FwExtents.Max)

    # CORE must end strictly below FW start.
    $c1 = ($CoreExtents.Max -lt $Layout.FwStart)
    Add-Check -Checks $Checks -Name 'core_below_fw' -Ok $c1 -Detail ("CORE max 0x{0:X8} vs FW start 0x{1:X8}" -f $CoreExtents.Max, $Layout.FwStart)

    # FW must begin exactly at the contract FW start.
    $c2 = ($FwExtents.Min -eq $Layout.FwStart)
    Add-Check -Checks $Checks -Name 'fw_starts_at_contract' -Ok $c2 -Detail ("FW min 0x{0:X8} vs FW start 0x{1:X8}" -f $FwExtents.Min, $Layout.FwStart)

    # FW start must be a real LPC176x flash-sector boundary and inside device flash.
    $c3 = (Test-Lpc176xSectorBoundary -Address $Layout.FwStart) -and ($Layout.FwStart -lt 0x00040000)
    Add-Check -Checks $Checks -Name 'fw_sector_aligned' -Ok $c3 -Detail ("FW start 0x{0:X8} sector-aligned + within 256KB flash" -f $Layout.FwStart)

    # Every combined address must sit within [ABOUT_START .. APP_END]
    # (ABOUT at 0x8000 is legitimately below APP_START 0x8100).
    $c4 = ($combinedMin -ge $AboutStart) -and ($combinedMax -le $Layout.AppEnd)
    Add-Check -Checks $Checks -Name 'addresses_in_window' -Ok $c4 -Detail ("combined 0x{0:X8}..0x{1:X8} vs [0x{2:X8}..0x{3:X8}]" -f $combinedMin, $combinedMax, $AboutStart, $Layout.AppEnd)

    # Shared contract blocks must be present in CORE (range check over the upper
    # CORE window, so we do not false-fail if no byte lands exactly on 0x27E00).
    $contractCount = 0
    foreach ($key in $CoreBytes.Keys) {
        if (($key -ge $Layout.AbiInfoAddr) -and ($key -le $Layout.CoreEnd)) { $contractCount++ }
    }
    $c5 = ($contractCount -ge 1) -and ($CoreExtents.Max -ge $Layout.ApiTableAddr)
    Add-Check -Checks $Checks -Name 'contract_blocks_present' -Ok $c5 -Detail ("$contractCount byte(s) in [0x{0:X8}..0x{1:X8}], CORE max 0x{2:X8} >= API 0x{3:X8}" -f $Layout.AbiInfoAddr, $Layout.CoreEnd, $CoreExtents.Max, $Layout.ApiTableAddr)

    # Mis-parse guard: with disjoint images the union size must equal the sum.
    $c6 = ($CombinedCount -eq ($CoreExtents.Count + $FwExtents.Count))
    Add-Check -Checks $Checks -Name 'combined_count' -Ok $c6 -Detail ("combined $CombinedCount == CORE $($CoreExtents.Count) + FW $($FwExtents.Count)")

    foreach ($chk in $Checks) {
        $lvl = if ($chk.ok) { 'info' } else { 'error' }
        Add-Event -Events $Events -Level $lvl -Component 'validate' -EventType $chk.name -Message $chk.detail -Context @{ ok = $chk.ok }
    }

    return [pscustomobject]@{
        CombinedMin   = [UInt32]$combinedMin
        CombinedMax   = [UInt32]$combinedMax
        ContractCount = $contractCount
    }
}
# ------------------------------------------------------------

# ===========================
# ENC  (only with -RunEnc)
# ===========================

# ------------------------------------------------------------
function Get-EncSnapshot {
    param([string]$Dir)
    # Map of enc-name -> LastWriteTimeUtc ticks; empty if dir absent.
    $snap = @{}
    if (Test-Path $Dir) {
        Get-ChildItem -Path $Dir -Filter '*.enc' -File | ForEach-Object { $snap[$_.Name] = $_.LastWriteTimeUtc.Ticks }
    }
    return $snap
}
# ------------------------------------------------------------

# ------------------------------------------------------------
function Invoke-Enc {
    param(
        [string]$CombinedHexAbs,
        [string]$ProjectRoot,
        [string]$Platform,
        [string]$StableEncName,
        [System.Collections.Generic.List[string]]$Events
    )

    $outBat = Join-Path $ProjectRoot 'build\out\out.bat'
    if (-not (Test-Path $outBat)) { throw "out.bat not found: $outBat" }

    # out.bat does `cd ..` from the CWD then writes into <CWD-parent>\out\{933,web}.
    # So CWD must be the project root; the .enc lands under the workspace root.
    $workspaceRoot = Split-Path -Parent $ProjectRoot
    $out933 = Join-Path $workspaceRoot 'out\933'
    $outWeb = Join-Path $workspaceRoot 'out\web'

    # Deterministic capture: snapshot BEFORE, diff AFTER. A file counts as produced
    # if it is new OR its mtime advanced (robust to re-runs overwriting the name).
    $before933 = Get-EncSnapshot -Dir $out933
    $beforeWeb = Get-EncSnapshot -Dir $outWeb

    # out.bat does not propagate hex2enc_933's exit status; its final errorlevel
    # reflects a trailing copy, not the ENC result. So a non-zero exit is only a
    # warning here -- the produced-.enc set (below) is the authoritative signal.
    # Pipe out.bat's stdout to Out-Null: an external command's console output would
    # otherwise be captured as part of this function's return value (polluting it into
    # an array). $LASTEXITCODE is still set through the pipe.
    $outExit = 0
    Push-Location $ProjectRoot
    try {
        & $outBat "$CombinedHexAbs,$Platform" | Out-Null
        $outExit = $LASTEXITCODE
    }
    finally {
        Pop-Location
    }
    if ($outExit -ne 0) {
        Add-Event -Events $Events -Level 'warning' -Component 'enc' -EventType 'outbat_exit' -Message "out.bat returned exit code $outExit (hex2enc status not propagated; using produced .enc set as source of truth)" -Context @{ exit = $outExit }
    }

    $produced = New-Object System.Collections.Generic.List[string]
    $stablePaths = New-Object System.Collections.Generic.List[string]

    foreach ($pair in @(@{ Dir = $out933; Before = $before933 }, @{ Dir = $outWeb; Before = $beforeWeb })) {
        $dir = $pair.Dir
        $before = $pair.Before
        if (-not (Test-Path $dir)) { continue }

        $new = Get-ChildItem -Path $dir -Filter '*.enc' -File | Where-Object {
            (-not $before.ContainsKey($_.Name)) -or ($_.LastWriteTimeUtc.Ticks -gt $before[$_.Name])
        }

        if (@($new).Count -gt 1) {
            Add-Event -Events $Events -Level 'warning' -Component 'enc' -EventType 'multi_enc' -Message "$(@($new).Count) new .enc in $dir; StableEncName keeps only the last" -Context @{ dir = $dir; count = @($new).Count }
        }

        foreach ($encFile in $new) {
            $produced.Add($encFile.FullName)
            $stablePath = Join-Path $dir $StableEncName
            Copy-Item -Path $encFile.FullName -Destination $stablePath -Force
            $stablePaths.Add($stablePath)
        }
    }

    Add-Event -Events $Events -Level 'info' -Component 'enc' -EventType 'produced' -Message "ENC produced: $($produced.Count)" -Context @{ produced = @($produced); stable = @($stablePaths) }

    if ($produced.Count -eq 0) {
        throw "ENC step ran but produced no new .enc in $out933 / $outWeb"
    }

    return [pscustomobject]@{
        Produced = @($produced)
        Stable   = @($stablePaths)
    }
}
# ------------------------------------------------------------

# ===========================
# MAIN
# ===========================

$events = New-Object System.Collections.Generic.List[string]
$checks = New-Object System.Collections.Generic.List[object]
$artifactsDir = $null
$passed = $false

try {
    if (-not $RepoRoot -or $RepoRoot.Trim().Length -eq 0) {
        $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
    }
    if (-not $LayoutHeader -or $LayoutHeader.Trim().Length -eq 0) {
        $LayoutHeader = Join-Path $RepoRoot 'external\Core_Contract_SDK_DS4\core_contract_ds4_image_layout.h'
    }
    if (-not $RunId -or $RunId.Trim().Length -eq 0) {
        $RunId = New-RunId
    }

    function Resolve-UnderRepo {
        param([string]$PathValue)
        if ([System.IO.Path]::IsPathRooted($PathValue)) { return $PathValue }
        return (Join-Path $RepoRoot $PathValue)
    }

    $coreHexPath = Resolve-UnderRepo $CoreHex
    $fwHexPath   = Resolve-UnderRepo $FwHex
    $outHexPath  = Resolve-UnderRepo $OutputHex
    $artifactsDir = Join-Path (Resolve-UnderRepo $ArtifactsRoot) $RunId

    $startTs = (Get-Date)
    Add-Event -Events $events -Level 'info' -Component 'main' -EventType 'start' -Message "combine run $RunId" -Context @{ core = $coreHexPath; fw = $fwHexPath; output = $outHexPath }

    if (-not (Test-Path $coreHexPath)) { throw "CORE hex not found: $coreHexPath" }
    if (-not (Test-Path $fwHexPath))   { throw "FW hex not found: $fwHexPath" }

    $layout = Read-LayoutContract -LayoutHeaderPath $LayoutHeader
    Add-Event -Events $events -Level 'info' -Component 'layout' -EventType 'contract' -Message "layout contract read" -Context $layout

    $coreBytes = Parse-IntelHex -Path $coreHexPath
    $fwBytes   = Parse-IntelHex -Path $fwHexPath
    if ($coreBytes.Count -eq 0) { throw "CORE hex parsed to 0 bytes: $coreHexPath" }
    if ($fwBytes.Count -eq 0)   { throw "FW hex parsed to 0 bytes: $fwHexPath" }

    $coreExtents = Get-MapExtents -Map $coreBytes
    $fwExtents   = Get-MapExtents -Map $fwBytes

    # Gate 1: images must be disjoint before we union them.
    $overlap = Test-Disjoint -CoreBytes $coreBytes -FwBytes $fwBytes -Checks $checks -Events $events

    # Union (safe only if disjoint).
    $combined = @{}
    if ($overlap -eq 0) {
        foreach ($key in $coreBytes.Keys) { $combined[$key] = $coreBytes[$key] }
        foreach ($key in $fwBytes.Keys)   { $combined[$key] = $fwBytes[$key] }
    }

    # Gate 2: structural / contract sanity on the union.
    $sanity = Test-CombineSanity -CoreExtents $coreExtents -FwExtents $fwExtents -CombinedCount $combined.Count `
        -CoreBytes $coreBytes -Layout $layout -AboutStart $AboutStart -AboutEnd $AboutEnd -Checks $checks -Events $events

    $about = Get-AboutIdentity -CoreBytes $coreBytes -AboutStart $AboutStart -AboutEnd $AboutEnd

    $passed = ($overlap -eq 0) -and (-not ($checks | Where-Object { -not $_.ok }))

    $encResult = $null
    if ($passed) {
        $outDir = Split-Path -Parent $outHexPath
        if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir -Force | Out-Null }
        Write-IntelHex -BytesMap $combined -Path $outHexPath
        Add-Event -Events $events -Level 'info' -Component 'combine' -EventType 'written' -Message "combined hex written" -Context @{ path = $outHexPath; bytes = $combined.Count }

        if ($RunEnc) {
            $encResult = Invoke-Enc -CombinedHexAbs $outHexPath -ProjectRoot $RepoRoot -Platform $Platform -StableEncName $StableEncName -Events $events
        }
    }
    else {
        Add-Event -Events $events -Level 'error' -Component 'combine' -EventType 'skipped' -Message "validation failed; combined hex NOT written"
    }

    $endTs = (Get-Date)

    # --- assemble metadata / stats ---
    $gitCommit = 'unknown'
    try { $gitCommit = (& git -C $RepoRoot rev-parse HEAD 2>$null); if (-not $gitCommit) { $gitCommit = 'unknown' } } catch { $gitCommit = 'unknown' }

    $coreSha = Get-Sha256Hex -Path $coreHexPath
    $fwSha   = Get-Sha256Hex -Path $fwHexPath
    $combinedSha = if ($passed -and (Test-Path $outHexPath)) { Get-Sha256Hex -Path $outHexPath } else { '' }
    $resultStr = if ($passed) { 'PASS' } else { 'FAIL' }

    $metadata = [ordered]@{
        run_id     = $RunId
        tool       = 'combine_core_fw_hex.ps1'
        result     = $resultStr
        started_at = $startTs.ToUniversalTime().ToString('o')
        ended_at   = $endTs.ToUniversalTime().ToString('o')
        duration_s = [Math]::Round(($endTs - $startTs).TotalSeconds, 3)
        runtime    = @{ powershell = $PSVersionTable.PSVersion.ToString(); os = [System.Environment]::OSVersion.VersionString }
        commit     = "$gitCommit".Trim()
        config     = [ordered]@{
            repo_root   = $RepoRoot
            core_hex    = $coreHexPath
            fw_hex      = $fwHexPath
            output_hex  = $outHexPath
            layout_hdr  = $LayoutHeader
            platform    = $Platform
            run_enc     = [bool]$RunEnc
            stable_enc  = $StableEncName
            about_start = ("0x{0:X8}" -f $AboutStart)
            about_end   = ("0x{0:X8}" -f $AboutEnd)
        }
        layout     = [ordered]@{
            app  = ("0x{0:X8}..0x{1:X8}" -f $layout.AppStart, $layout.AppEnd)
            core = ("0x{0:X8}..0x{1:X8}" -f $layout.CoreStart, $layout.CoreEnd)
            fw   = ("0x{0:X8}..0x{1:X8}" -f $layout.FwStart, $layout.FwEnd)
            gap_bytes = $layout.GapSize
            min_gap   = $layout.MinGap
            abi_info  = ("0x{0:X8}" -f $layout.AbiInfoAddr)
            api_table = ("0x{0:X8}" -f $layout.ApiTableAddr)
        }
        inputs     = @(
            [ordered]@{ name = 'core_hex'; path = $coreHexPath; sha256 = $coreSha },
            [ordered]@{ name = 'fw_hex';   path = $fwHexPath;   sha256 = $fwSha }
        )
    }

    $stats = [ordered]@{
        passed        = $passed
        overlap       = $overlap
        core          = [ordered]@{ min = ("0x{0:X8}" -f $coreExtents.Min); max = ("0x{0:X8}" -f $coreExtents.Max); bytes = $coreExtents.Count; sha256 = $coreSha }
        fw            = [ordered]@{ min = ("0x{0:X8}" -f $fwExtents.Min);   max = ("0x{0:X8}" -f $fwExtents.Max);   bytes = $fwExtents.Count;   sha256 = $fwSha }
        combined      = [ordered]@{ min = ("0x{0:X8}" -f $sanity.CombinedMin); max = ("0x{0:X8}" -f $sanity.CombinedMax); bytes = $combined.Count; sha256 = $combinedSha }
        gap_bytes     = $layout.GapSize
        fw_sector_ok  = (Test-Lpc176xSectorBoundary -Address $layout.FwStart)
        contract_bytes = $sanity.ContractCount
        about         = [ordered]@{ ascii = $about.Ascii; sha256 = $about.Sha256; bytes = $about.ByteCount }
        checks        = @($checks | ForEach-Object { [ordered]@{ name = $_.name; ok = $_.ok; detail = $_.detail } })
    }

    # --- build summary.txt ---
    $encLine = if ($null -ne $encResult) { ($encResult.Stable -join '; ') } elseif ($RunEnc -and -not $passed) { '(skipped: validation failed)' } elseif ($RunEnc) { '(RunEnc set but none produced)' } else { '(not produced; -RunEnc not set)' }
    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine('============================================================')
    [void]$sb.AppendLine(' COMBINE CORE+FW  -  TEST-ONLY BRING-UP ARTIFACT')
    [void]$sb.AppendLine(' *** NOT FOR PRODUCTION *** two-stage (separate CORE.enc + FW.enc) remains the deployment path.')
    [void]$sb.AppendLine('============================================================')
    [void]$sb.AppendLine("Run ID    : $RunId")
    [void]$sb.AppendLine("Result    : $resultStr")
    [void]$sb.AppendLine("Duration  : $([Math]::Round(($endTs - $startTs).TotalSeconds, 3)) s")
    [void]$sb.AppendLine('')
    [void]$sb.AppendLine('Assumptions:')
    [void]$sb.AppendLine('  - Bootloader 0x00000000..0x00007FFF is already flashed on the device.')
    [void]$sb.AppendLine('  - A PASS proves the split-compiled code + CORE->FW handoff; it does NOT')
    [void]$sb.AppendLine('    validate the two-stage flashing protocol (device-side, separate concern).')
    [void]$sb.AppendLine('')
    [void]$sb.AppendLine("Inputs:")
    [void]$sb.AppendLine(("  CORE : {0}  (0x{1:X8}..0x{2:X8}, {3} bytes)" -f $coreHexPath, $coreExtents.Min, $coreExtents.Max, $coreExtents.Count))
    [void]$sb.AppendLine(("  FW   : {0}  (0x{1:X8}..0x{2:X8}, {3} bytes)" -f $fwHexPath, $fwExtents.Min, $fwExtents.Max, $fwExtents.Count))
    [void]$sb.AppendLine('')
    [void]$sb.AppendLine("Layout ($LayoutHeader):")
    [void]$sb.AppendLine(("  APP 0x{0:X8}..0x{1:X8}  CORE 0x{2:X8}..0x{3:X8}  FW 0x{4:X8}..0x{5:X8}  GAP {6} (min {7})" -f $layout.AppStart, $layout.AppEnd, $layout.CoreStart, $layout.CoreEnd, $layout.FwStart, $layout.FwEnd, $layout.GapSize, $layout.MinGap))
    [void]$sb.AppendLine('')
    [void]$sb.AppendLine('Checks:')
    foreach ($chk in $checks) {
        [void]$sb.AppendLine(("  [{0}] {1} - {2}" -f $(if ($chk.ok) { 'OK  ' } else { 'FAIL' }), $chk.name, $chk.detail))
    }
    [void]$sb.AppendLine('')
    [void]$sb.AppendLine('Output:')
    [void]$sb.AppendLine(("  combined hex : {0}" -f $(if ($combinedSha) { $outHexPath } else { '(not written)' })))
    [void]$sb.AppendLine(("  combined enc : {0}" -f $encLine))
    [void]$sb.AppendLine('')
    [void]$sb.AppendLine(("ABOUT identity (CORE): `"{0}`"  sha256 {1}" -f $about.Ascii, $about.Sha256))
    [void]$sb.AppendLine(("FW image sha256      : {0}  (NOT reflected in .enc name)" -f $fwSha))
    $summaryText = $sb.ToString()

    # --- write artifacts ---
    $outputsDir = Join-Path $artifactsDir 'outputs'
    New-Item -ItemType Directory -Path $outputsDir -Force | Out-Null

    $utf8 = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText((Join-Path $artifactsDir 'run_metadata.json'), (($metadata | ConvertTo-Json -Depth 12)), $utf8)
    [System.IO.File]::WriteAllText((Join-Path $artifactsDir 'stats.json'), (($stats | ConvertTo-Json -Depth 12)), $utf8)
    [System.IO.File]::WriteAllText((Join-Path $artifactsDir 'summary.txt'), $summaryText, $utf8)
    [System.IO.File]::WriteAllLines((Join-Path $artifactsDir 'events.jsonl'), [string[]]$events, $utf8)

    $outputFiles = New-Object System.Collections.Generic.List[string]
    if ($combinedSha) { $outputFiles.Add($outHexPath) }
    if ($null -ne $encResult) { foreach ($p in $encResult.Stable) { if (Test-Path $p) { $outputFiles.Add($p) } } }
    foreach ($f in $outputFiles) {
        $leaf = Split-Path $f -Leaf
        $destPath = Join-Path $outputsDir $leaf
        if (Test-Path $destPath) {
            # disambiguate a name collision (e.g. the 933 and web stable .enc share a leaf)
            $parent = Split-Path (Split-Path $f -Parent) -Leaf
            $destPath = Join-Path $outputsDir ("${parent}_${leaf}")
        }
        Copy-Item -Path $f -Destination $destPath -Force
    }

    # checksums.txt: inputs + every artifact file (except checksums.txt itself).
    $checkLines = New-Object System.Collections.Generic.List[string]
    $checkLines.Add(("{0}  {1}" -f $coreSha, $coreHexPath))
    $checkLines.Add(("{0}  {1}" -f $fwSha, $fwHexPath))
    Get-ChildItem -Path $artifactsDir -Recurse -File | Where-Object { $_.Name -ne 'checksums.txt' } | ForEach-Object {
        $rel = $_.FullName.Substring($artifactsDir.Length).TrimStart('\', '/')
        $checkLines.Add(("{0}  {1}" -f (Get-Sha256Hex -Path $_.FullName), $rel))
    }
    [System.IO.File]::WriteAllLines((Join-Path $artifactsDir 'checksums.txt'), [string[]]$checkLines, $utf8)

    # --- console: artifacts path + short summary only ---
    Write-Host ("Artifacts: {0}" -f $artifactsDir)
    Write-Host ("Result   : {0}" -f $resultStr)
    Write-Host ("Combined : 0x{0:X8}..0x{1:X8}  {2} bytes  (CORE {3} + FW {4})" -f $sanity.CombinedMin, $sanity.CombinedMax, $combined.Count, $coreExtents.Count, $fwExtents.Count)
    foreach ($chk in ($checks | Where-Object { -not $_.ok })) { Write-Host ("  FAIL {0}: {1}" -f $chk.name, $chk.detail) }
    if ($combinedSha) { Write-Host ("Hex      : {0}" -f $outHexPath) }
    Write-Host ("Enc      : {0}" -f $encLine)

    if ($passed) { exit 0 } else { exit 1 }
}
catch {
    $err = $_.Exception.Message
    Add-Event -Events $events -Level 'error' -Component 'main' -EventType 'exception' -Message $err

    if ($artifactsDir) {
        try {
            New-Item -ItemType Directory -Path $artifactsDir -Force | Out-Null
            $utf8f = New-Object System.Text.UTF8Encoding($false)
            [System.IO.File]::WriteAllLines((Join-Path $artifactsDir 'events.jsonl'), [string[]]$events, $utf8f)
            [System.IO.File]::WriteAllText((Join-Path $artifactsDir 'summary.txt'), "Result: FAIL`r`nError : $err`r`n", $utf8f)
        }
        catch { }
        Write-Host ("Artifacts: {0}" -f $artifactsDir)
    }
    Write-Error "combine_core_fw_hex failed: $err"
    exit 2
}
