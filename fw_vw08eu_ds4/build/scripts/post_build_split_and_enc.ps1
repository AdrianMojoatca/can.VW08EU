param(
    [Parameter(Mandatory = $true)]
    [string]$InputHex,

    [Parameter(Mandatory = $true)]
    [string]$OutputDir,

    [Parameter(Mandatory = $true)]
    [string]$StableEncName,

    [Parameter(Mandatory = $true)]
    [ValidateSet('core','fw')]
    [string]$EncSource,

    [string]$Platform = "NGMM"
)

$ErrorActionPreference = 'Stop'

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildRoot = Split-Path -Parent $scriptRoot
$projectRoot = Split-Path -Parent $buildRoot
$splitScript = Join-Path $scriptRoot 'split_ds4_hex_images.ps1'
$outBat = Join-Path $buildRoot 'out\out.bat'

. (Join-Path $scriptRoot 'core_layout_common.ps1')

$resolvedInputHex = [System.IO.Path]::GetFullPath((Join-Path $projectRoot $InputHex))
$resolvedOutputDir = [System.IO.Path]::GetFullPath((Join-Path $projectRoot $OutputDir))

if (!(Test-Path $resolvedInputHex)) {
    throw "Input HEX not found: $resolvedInputHex"
}

if (!(Test-Path $resolvedOutputDir)) {
    throw "Output directory not found: $resolvedOutputDir"
}

if (!(Test-Path $splitScript)) {
    throw "Split script not found: $splitScript"
}

if (!(Test-Path $outBat)) {
    throw "out.bat not found: $outBat"
}

& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $splitScript -InputHex $resolvedInputHex -OutputDir $resolvedOutputDir
if ($LASTEXITCODE -ne 0) {
    throw "split_ds4_hex_images.ps1 failed with code $LASTEXITCODE"
}

$baseName = [System.IO.Path]::GetFileNameWithoutExtension($resolvedInputHex)
if ($EncSource -eq 'core') {
    $encInputHex = Join-Path $resolvedOutputDir ($baseName + '.core.hex')
}
else {
    $encInputHex = Join-Path $resolvedOutputDir ($baseName + '.fw.hex')
}

if (!(Test-Path $encInputHex)) {
    throw "ENC input HEX not found: $encInputHex"
}

# Gap/layout guard on the freshly split CORE image, before the ENC step, so a bad image
# fails fast and is never encrypted. CORE only (FW has no ABOUT/core contract blocks).
# Invoked as a separate process so the guard's exit code does not terminate this script.
if ($EncSource -eq 'core') {
    $guardScript = Join-Path $scriptRoot 'verify_core_image_gap.ps1'
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $guardScript -CoreHex $encInputHex -RepoRoot $projectRoot
    if ($LASTEXITCODE -ne 0) {
        throw "CORE image gap guard failed for $encInputHex; aborting before ENC."
    }
}


# UMPLERE OBLIGATORIE PE DS4, pentru imaginea de CORE (si in fluxul de productie
# in doua etape, nu doar la cea combinata de banc). ABI info 0x37000 si tabela de
# API 0x37040 cad in acelasi sector de 32 KB (20: 0x30000-0x37FFF) ca sfirsitul
# codului de CORE, separate de o gaura de ~8 KB. Fara umplere, flasher-ul din
# bootloader abandoneaza pe la ~90% si modulul ramine cu imagine incompleta.
# FW-ul (>= 0x38000) e sector 21 si NU se umple.
if ($EncSource -eq 'core') {
    $srec = 'C:\cygwin\home\T\bin\srec_cat.exe'
    if (!(Test-Path $srec)) { throw "srec_cat not found: $srec" }
    $filledHex = [System.IO.Path]::ChangeExtension($encInputHex, '.filled.hex')
    Remove-Item $filledHex -ErrorAction SilentlyContinue
    & $srec $encInputHex.Replace([char]92,[char]47) -intel -fill 0xFF 0x8100 0x38000 -o $filledHex.Replace([char]92,[char]47) -intel
    if ($LASTEXITCODE -ne 0 -or !(Test-Path $filledHex)) { throw "srec_cat fill(CORE) failed ($LASTEXITCODE)" }
    Write-Host "CORE filled 0x8100..0x38000 -> $(Split-Path $filledHex -Leaf)"
    $encInputHex = $filledHex
}
# Deterministic ENC filename expected from the image identity (ABOUT block), matching
# enc_rename_933.pl. Used to select the exact produced ENC (no time-window guessing).
$aboutWindow = Get-AboutWindowFromScatter -RepoRootPath $projectRoot
$encMap = Get-IntelHexMap -Path $encInputHex
$expectedEnc = Get-ExpectedEncName -BytesMap $encMap -AboutStart $aboutWindow.Start -AboutEnd $aboutWindow.End

Push-Location $projectRoot
try {
    & $outBat "$encInputHex,$Platform"
    if ($LASTEXITCODE -ne 0) {
        throw "out.bat failed with code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}

$workspaceRoot = Split-Path -Parent $projectRoot
$out933 = Join-Path $workspaceRoot 'out\933'
$outWeb = Join-Path $workspaceRoot 'out\web'

$copied = $false
foreach ($dirPath in @($out933, $outWeb)) {
    if (!(Test-Path $dirPath)) {
        continue
    }

    $producedEnc = Join-Path $dirPath $expectedEnc
    if (Test-Path $producedEnc) {
        $stablePath = Join-Path $dirPath $StableEncName
        Copy-Item -Path $producedEnc -Destination $stablePath -Force
        Write-Host "Stable ENC: $stablePath (from $expectedEnc)"
        $copied = $true
    }
}

if (-not $copied) {
    $present = @()
    foreach ($dirPath in @($out933, $outWeb)) {
        if (Test-Path $dirPath) {
            $present += (Get-ChildItem -Path $dirPath -Filter '*.enc' -File | ForEach-Object { $_.Name })
        }
    }
    $presentText = if ($present.Count -gt 0) { ($present | Sort-Object -Unique) -join ', ' } else { '(none)' }
    throw "Expected ENC '$expectedEnc' not found in out\933 or out\web. Present: $presentText"
}
