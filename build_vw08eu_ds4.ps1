<#
.SYNOPSIS
  Construieste tripleta VW08EU pe DS4 (CORE + FW pe acelasi contract) si scoate .enc-ul de
  flash-uit pe modul DS4.

.DESCRIPTION
  Un singur punct de intrare pentru ciclul "am schimbat ceva in surse -> vreau sa testez":

    1. build CORE      (UV4)
    2. build FW        (UV4)
    3. verifica perechea, citind direct din hex-uri ce publica CORE si ce asteapta FW
    4. combine CORE+FW -> un singur .enc de bring-up
    5. rezumat cu fisierul de flash-uit

  Numele livrabilului se DERIVA din build_identity.h, deci urmeaza automat cind bumpezi
  BUILD_FW_VERSION:  vw08eu_1_00_00.enc  ->  vw08eu_1_00_01.enc

.NOTES
  - Inchide Keil inainte de a rula: UV4 in linie de comanda si IDE-ul deschis pe acelasi
    proiect se bat pe .uvproj/.uvopt.
  - Foloseste UV4 din MDK **v4**. Cu v5 comanda intoarce 0 si NU construieste nimic,
    fiindca proiectele sint in format .uvproj (v5 vrea .uvprojx). De aceea scriptul
    verifica timestamp-ul artefactului, nu doar codul de iesire.
  - .enc-ul combinat e pentru BANC. Productia ramine in doua etape, cu
    core_ds4_release.core.enc si fw_ds4_release.fw.enc separate.

.EXAMPLE
  .\build_vw08eu.ps1
  .\build_vw08eu.ps1 -SkipCore          # am schimbat doar in FW
  .\build_vw08eu.ps1 -NoCombine         # doar build + verificare, fara .enc
#>
param(
    [switch]$SkipCore,
    [switch]$SkipFw,
    [switch]$NoCombine,
    [string]$Uv4 = 'C:\Keil\UV4\UV4.exe'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ROOT     = $PSScriptRoot
$CORE_DIR = Join-Path $ROOT 'core_vw08eu_ds4'
$FW_DIR   = Join-Path $ROOT 'fw_vw08eu_ds4'
$CT_DIR   = Join-Path $ROOT 'vw08eu_ds4_contract'
$OUT933   = Join-Path $ROOT 'out\933'

$CORE_PRJ = Join-Path $CORE_DIR 'CORE_VW08EU_DS4.uvproj'
$FW_PRJ   = Join-Path $FW_DIR   'FW_VW08EU_DS4.uvproj'
$CORE_HEX = Join-Path $CORE_DIR 'build\core_ds4_build\core_ds4_release.hex'
$FW_HEX   = Join-Path $FW_DIR   'build\fw_ds4_build\fw_ds4_release.hex'
$CORE_AXF = Join-Path $CORE_DIR 'build\core_ds4_build\core_ds4_release.axf'
$FW_AXF   = Join-Path $FW_DIR   'build\fw_ds4_build\fw_ds4_release.axf'
$LOGDIR   = Join-Path $ROOT 'build_logs'

$t0 = Get-Date
$fail = @()

function Say([string]$s){ Write-Host $s }
function Head([string]$s){ Write-Host ''; Write-Host ('=== ' + $s + ' ' + ('=' * [Math]::Max(0, 62 - $s.Length))) }
function Die([string]$s){ Write-Host ''; Write-Host ("OPRIT: $s") -ForegroundColor Red; exit 2 }

# ---------------------------------------------------------------- preliminarii
Head 'preliminarii'
foreach($p in @($CORE_PRJ,$FW_PRJ)){ if(-not (Test-Path $p)){ Die "proiect lipsa: $p" } }
if(-not (Test-Path $Uv4)){ Die "UV4 negasit: $Uv4  (MDK v4; v5 nu construieste .uvproj)" }
New-Item -ItemType Directory -Force -Path $LOGDIR | Out-Null

# contractul trebuie sa fie ACELASI pentru ambele proiecte, altfel totul e degeaba
$idFile = Join-Path $CT_DIR 'build_identity.h'
if(-not (Test-Path $idFile)){ Die "build_identity.h lipsa in contract: $idFile" }
foreach($d in @($CORE_DIR,$FW_DIR)){
    $j = Join-Path $d 'external\Core_Contract_SDK_DS4\core_contract_ds4_api_table.h'
    if(-not (Test-Path $j)){ Die "contractul nu e vizibil din $(Split-Path $d -Leaf)" }
    $h1 = (Get-FileHash (Join-Path $CT_DIR 'core_contract_ds4_api_table.h')).Hash
    $h2 = (Get-FileHash $j).Hash
    if($h1 -ne $h2){ Die "contractul din $(Split-Path $d -Leaf) DIFERA de sursa unica - junction rupt?" }
}
Say '  contract: aceeasi sursa pentru CORE si FW  (junction OK)'

# identitatea -> numele livrabilului
$idTxt = [System.IO.File]::ReadAllText($idFile)
$mName = [regex]::Match($idTxt,'#if\s+defined\(CORE_DS4_BUILD\)[\s\S]*?BUILD_FW_NAME\s+"([^"]+)"')
$mVer  = [regex]::Match($idTxt,'#if\s+defined\(CORE_DS4_BUILD\)[\s\S]*?BUILD_FW_VERSION\s+"([^"]+)"')
if(-not ($mName.Success -and $mVer.Success)){ Die 'nu pot citi BUILD_FW_NAME / BUILD_FW_VERSION din build_identity.h' }
$fwName = $mName.Groups[1].Value
$fwVer  = $mVer.Groups[1].Value
$encVersioned = ('933.{0}.{1}.enc' -f $fwName.ToLower(), ($fwVer -replace '\.','_'))   # livrabilul de CORE
$encCombined  = ('{0}_{1}.enc'     -f $fwName.ToLower(), ($fwVer -replace '\.','_'))   # imaginea de banc
Say ('  identitate: {0} {1}' -f $fwName, $fwVer)
Say ('  .enc de banc va fi: {0}' -f $encCombined)

# ---------------------------------------------------------------- build
function Build-Target([string]$tag,[string]$prj,[string]$target,[string]$axf){
    $before = if(Test-Path $axf){ (Get-Item $axf).LastWriteTimeUtc } else { [datetime]::MinValue }
    $log = Join-Path $LOGDIR ("{0}_{1}.log" -f $tag, (Get-Date -Format 'yyyyMMdd_HHmmss'))
    Say ("  build {0} ..." -f $tag)
    $p = Start-Process -FilePath $Uv4 -ArgumentList @('-b',$prj,'-t',$target,'-j0','-o',$log) -Wait -PassThru -WindowStyle Hidden
    $after = if(Test-Path $axf){ (Get-Item $axf).LastWriteTimeUtc } else { [datetime]::MinValue }

    $errLine = ''
    if(Test-Path $log){
        $m = Select-String -Path $log -Pattern '(\d+) Error\(s\), (\d+) Warning\(s\)' | Select-Object -Last 1
        if($m){ $errLine = $m.Matches[0].Value }
    }
    Say ("    exit={0}  {1}" -f $p.ExitCode, $errLine)
    Say ("    log: {0}" -f $log)

    if($p.ExitCode -ge 2){ return "build $tag a intors exit $($p.ExitCode)" }
    if($after -le $before){ return "build $tag nu a regenerat .axf (UV4 v5 pe .uvproj? verifica $Uv4)" }
    if($errLine -match '^(\d+) Error' -and [int]$Matches[1] -gt 0){ return "build ${tag}: $errLine" }
    return $null
}

Head 'build'
if($SkipCore){ Say '  CORE: sarit (-SkipCore)' }
else { $e = Build-Target 'core' $CORE_PRJ 'CORE_VW08EU_DS4' $CORE_AXF; if($e){ Die $e } }
if($SkipFw){ Say '  FW: sarit (-SkipFw)' }
else { $e = Build-Target 'fw' $FW_PRJ 'FW_VW08EU_DS4' $FW_AXF; if($e){ Die $e } }

foreach($h in @($CORE_HEX,$FW_HEX)){ if(-not (Test-Path $h)){ Die "hex lipsa dupa build: $h" } }

# ------------------------------------------------- verificarea perechii din hex
Head 'verificare pereche CORE <-> FW (citit din hex)'

$mem = @{}
foreach($pair in @(@('CORE',$CORE_HEX), @('FW',$FW_HEX))){
    $m = @{}; $upper = 0
    foreach($line in [System.IO.File]::ReadAllLines($pair[1])){
        if($line.Length -lt 11 -or $line[0] -ne ':'){ continue }
        $len  = [Convert]::ToInt32($line.Substring(1,2),16)
        $addr = [Convert]::ToInt32($line.Substring(3,4),16)
        $type = [Convert]::ToInt32($line.Substring(7,2),16)
        if($type -eq 4){ $upper = [Convert]::ToInt32($line.Substring(9,4),16); continue }
        if($type -ne 0){ continue }
        $base = ($upper -shl 16) + $addr
        for($i=0; $i -lt $len; $i++){ $m[[int]($base+$i)] = [int][Convert]::ToByte($line.Substring(9+$i*2,2),16) }
    }
    $mem[$pair[0]] = $m
}

# adresele si valorile asteptate se citesc din contract, nu sint hardcodate
$layout = [System.IO.File]::ReadAllText((Join-Path $CT_DIR 'core_contract_ds4_image_layout.h'))
$abi    = [System.IO.File]::ReadAllText((Join-Path $CT_DIR 'core_contract_ds4_abi.h'))
function HexDef([string]$txt,[string]$name){
    $m=[regex]::Match($txt, '#define\s+' + [regex]::Escape($name) + '\s+\(?(0x[0-9A-Fa-f]+)')
    if(-not $m.Success){ Die "nu gasesc $name in contract" }
    return [int]$m.Groups[1].Value
}
function NumDef([string]$txt,[string]$name){
    $m=[regex]::Match($txt, '#define\s+' + [regex]::Escape($name) + '\s+\(?(\d+)')
    if(-not $m.Success){ Die "nu gasesc $name in contract" }
    return [int]$m.Groups[1].Value
}
$aAbi   = HexDef $layout 'CORE_DS4_SHARED_ABI_INFO_ADDR'
$aTab   = HexDef $layout 'CORE_DS4_SHARED_API_TABLE_ADDR'
$aFw    = HexDef $layout 'CORE_DS4_FLASH_FW_START_ADDR'
$vMajor = NumDef $abi    'CORE_DS4_ABI_VERSION_MAJOR'
$vMinor = NumDef $abi    'CORE_DS4_ABI_VERSION_MINOR'
$vTable = NumDef $abi    'CORE_DS4_API_TABLE_VERSION'

function U32($m,[int]$a){
    if(-not ($m.ContainsKey($a) -and $m.ContainsKey($a+1) -and $m.ContainsKey($a+2) -and $m.ContainsKey($a+3))){ return $null }
    # atentie: -shl pe [byte] da 0 in PowerShell; de aceea inmultire, nu deplasare
    return [uint32](([int]$m[$a]) -bor (([int]$m[$a+1])*256) -bor (([int]$m[$a+2])*65536) -bor (([int]$m[$a+3])*16777216))
}
function Chk([string]$label,$got,$want){
    if($null -eq $got){ Say ("  [--] {0,-30} ABSENT" -f $label); return $false }
    $ok = ([uint32]$got -eq [uint32]$want)
    Say ("  [{0}] {1,-30} 0x{2:X8}{3}" -f $(if($ok){'OK'}else{'!!'}), $label, $got, $(if($ok){''}else{(' (asteptat 0x{0:X8})' -f [uint32]$want)}))
    return $ok
}

$c = $mem['CORE']; $f = $mem['FW']
$okPair = $true
$okPair = (Chk 'ABI info magic'     (U32 $c $aAbi)        0x43445334) -and $okPair
$okPair = (Chk 'API table magic'    (U32 $c $aTab)        0x41505334) -and $okPair
$okPair = (Chk 'abi major|minor'    (U32 $c ($aTab+8))    ($vMajor -bor ($vMinor*65536))) -and $okPair
$okPair = (Chk 'api_table_version'  (U32 $c ($aTab+12))   $vTable)    -and $okPair
$okPair = (Chk "FW desc magic FWD4" (U32 $f $aFw)         0x34445746) -and $okPair
$okPair = (Chk 'FW desc abi'        (U32 $f ($aFw+4))     (($vMajor*65536) -bor $vMinor)) -and $okPair

$cMax = @($c.Keys | Sort-Object)[-1]
$noOverlap = ($cMax -lt $aFw)
Say ("  [{0}] {1,-30} CORE se opreste la 0x{2:X}, FW incepe la 0x{3:X}" -f $(if($noOverlap){'OK'}else{'!!'}), 'fara suprapunere', $cMax, $aFw)
$okPair = $noOverlap -and $okPair

$tSize = U32 $c ($aTab+4)
Say ("       tabela de API: {0} B la 0x{1:X}, fereastra {2} B, liber {3} B" -f $tSize, $aTab, ($aFw-$aTab), (($aFw-$aTab)-$tSize))

if(-not $okPair){ Die 'CORE si FW nu sint pe acelasi contract. Reconstruieste AMBELE (fara -SkipCore/-SkipFw).' }
Say '  perechea e consistenta.'

# ---------------------------------------------------------------- combine
if($NoCombine){
    Head 'gata (fara combine)'
    Say ('  durata: {0:N1} s' -f ((Get-Date)-$t0).TotalSeconds)
    exit 0
}

Head 'combine CORE + FW -> .enc de banc'

# LIVRABIL PENTRU SITE: 933.<fw>.<major>_<minor>_<sub>.enc trebuie sa fie
# imaginea COMBINATA (CORE + FW) - un singur fisier care aduce modulul in stare
# de functionare de la bootloader. Site-ul de unde flash-uiesc clientii cunoaste
# doar acest format de nume (403 = DB3, 933 = DS4) si nu are cimp care sa
# distinga CORE de FW, deci numele TREBUIE sa denumeasca un singur artefact.
#
# out.bat compune numele .enc din blocul ABOUT, identic in imaginea combinata si
# in cea de CORE, deci combine scrie peste el - si asta e chiar ce vrem.
# Varianta doar-CORE o punem deoparte sub un nume explicit, pentru flash-ul de
# productie in doua etape.
$encVerPath = Join-Path $OUT933 $encVersioned
$encCoreOnly = ($encCombined -replace '\.enc$', '.CORE.enc')
# Sursa pentru varianta doar-CORE trebuie sa fie core_ds4_release.core.enc, pe care
# il scrie build-ul de CORE si pe care combine NU il atinge. NU $encVerPath: cu
# -SkipCore acolo se afla deja imaginea combinata din rularea precedenta, si am
# ajunge sa etichetam combinatul drept CORE-only.
$coreOnlySrc = Join-Path $OUT933 'core_ds4_release.core.enc'
if(Test-Path $coreOnlySrc){
    Copy-Item $coreOnlySrc (Join-Path $OUT933 $encCoreOnly) -Force
    Say ("  CORE-only pus deoparte ca: {0}  ({1:N0} B)" -f $encCoreOnly, (Get-Item $coreOnlySrc).Length)
} else {
    Say '  ATENTIE: core_ds4_release.core.enc lipseste - nu pot produce varianta doar-CORE'
}

$combineLog = Join-Path $LOGDIR ("combine_{0}.log" -f (Get-Date -Format 'yyyyMMdd_HHmmss'))
# FARA 2>&1: in PowerShell 5.1 redirectarea stderr-ului unui executabil nativ ambaleaza
# fiecare linie in ErrorRecord (NativeCommandError) si, cu ErrorActionPreference=Stop,
# avertismentul inofensiv de la cygwin ("find_fast_cwd") omoara scriptul.
$prevEap = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
# UMPLEREA REGIUNII DE CORE - OBLIGATORIE PE DS4, nu exista pe DB3.
# ABI info (0x37000) si tabela de API (0x37040) cad in acelasi sector de 32 KB
# (20: 0x30000-0x37FFF) ca sfirsitul codului de CORE, dar separate de o gaura de
# ~8 KB. O a doua rafala necontigua intr-un sector deja scris face flasher-ul din
# bootloader sa abandoneze pe la ~90% - modulul ramine cu imagine incompleta, fara
# niciun mesaj. Umplerea cu 0xFF face sectorul 20 o singura scriere contigua.
# Sursa: DS4_CORE_SPLIT\core_ds4\build\scripts\make_ds4_enc.ps1 (comentariul lung).
# FW-ul (>= 0x38000) e alt sector (21) si NU se umple.
$SREC      = 'C:\cygwin\home\T\bin\srec_cat.exe'
$FILL_FROM = '0x8100'
$FILL_TO   = '0x38000'
$CORE_FILL = [System.IO.Path]::ChangeExtension($CORE_HEX, '.filled.hex')
if(-not (Test-Path $SREC)){ Die "srec_cat negasit: $SREC" }
Remove-Item $CORE_FILL -ErrorAction SilentlyContinue
& $SREC ($CORE_HEX -replace '\\','/') -intel -fill 0xFF $FILL_FROM $FILL_TO -o ($CORE_FILL -replace '\\','/') -intel
if($LASTEXITCODE -ne 0 -or -not (Test-Path $CORE_FILL)){ Die "srec_cat fill(CORE) a esuat ($LASTEXITCODE)" }
Say ('  CORE umplut 0x8100..0x38000 -> {0}' -f (Split-Path $CORE_FILL -Leaf))

Push-Location $CORE_DIR
try {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\build\scripts\combine_core_fw_hex.ps1' `
        -CoreHex $CORE_FILL -FwHex $FW_HEX -RunEnc -Platform NGMM `
        -StableEncName $encCombined | Tee-Object -FilePath $combineLog | ForEach-Object {
            if($_ -match 'Result|Combined|PASS|FAIL|Enc\s+:|Hex\s+:|Artifacts'){ Say ('  ' + ($_ -replace '\s+$','')) }
        }
} finally { Pop-Location; $ErrorActionPreference = $prevEap }

$combined = Join-Path $OUT933 $encCombined
if(-not (Test-Path $combined)){ Die "combine nu a produs $encCombined - vezi $combineLog" }

# Verificare: numele de livrat si numele stabil trebuie sa fie ACELEASI octeti.
# Garda ieftina impotriva confuziei CORE-only / combinat.
if(Test-Path $encVerPath){
    $h1 = (Get-FileHash $encVerPath   -Algorithm MD5).Hash
    $h2 = (Get-FileHash $combined     -Algorithm MD5).Hash
    if($h1 -ne $h2){ Die ("{0} nu e imaginea combinata (md5 {1} vs {2})" -f $encVersioned, $h1, $h2) }
    Say ("  livrabil verificat: {0} = imaginea combinata, {1:N0} B" -f $encVersioned, (Get-Item $encVerPath).Length)
} else {
    Die ("combine nu a produs livrabilul {0}" -f $encVersioned)
}

# ---------------------------------------------------------------- rezumat
Head 'REZUMAT'
$fwSizeM = [regex]::Match((Get-Content $combineLog -Raw), 'Combined\s*:\s*(0x[0-9A-Fa-f]+\.\.0x[0-9A-Fa-f]+\s+\d+ bytes[^\r\n]*)')
if($fwSizeM.Success){ Say ('  imagine combinata : ' + $fwSizeM.Groups[1].Value) }
Say ''
Say '  LIVRABIL (site + banc) - o singura imagine, CORE + FW:'
Say ("    {0}" -f $encVerPath)
Say ("    {0}   (acelasi continut, nume stabil)" -f $combined)
Say ''
Say '  Productie, doua etape (NU imaginea combinata):'
foreach($n in @($encCoreOnly,'core_ds4_release.core.enc','fw_ds4_release.fw.enc')){
    $p = Join-Path $OUT933 $n
    if(Test-Path $p){ Say ('    {0,-38} {1,9:N0} B' -f $n, (Get-Item $p).Length) }
}
Say ''
Say ('  loguri: {0}' -f $LOGDIR)
Say ('  durata: {0:N1} s' -f ((Get-Date)-$t0).TotalSeconds)
exit 0
