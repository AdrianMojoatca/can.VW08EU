# Handoff după reset laptop (2026-07-03)

## Repo / branch
- Repo: `can.fw.frd37eu` (FRD37EU_SPLIT)
- Branch de lucru: `split-core-fw-separation-2026-07-02`

## Ce s-a finalizat
1. Corecție split HEX/ENC pentru separare reală CORE/FW.
2. Corecție contract adresare shared (evitare overlap vector/startup).
3. Publicare explicită ABI/API shared în CORE.
4. Fix scatter pentru a elimina eroarea linker `L6788E`.
5. Build-uri validate:
   - CORE build: OK
   - FW build: OK

## Fișiere cheie modificate
- `CORE_DB3.uvproj`
- `FW_DB3.uvproj`
- `external/Core_Contract_SDK_DB3/core_contract_db3_boot_handshake.h`
- `external/Core_Contract_SDK_DB3/core_contract_db3_gpio_iface_shim.c`
- `external/Core_Contract_SDK_DB3/core_contract_db3_image_layout.h`
- `external/Core_Contract_SDK_DB3/core_contract_db3_fw_handoff.h`
- `external/can.sdk.db3/BSP/bootloader/lib.app.scatter.1766/app.sct`
- `external/can.sdk.db3/BSP/bootloader/lib.app.scatter.1766/app_fw_db3_split.sct`
- `external/can.sdk.db3/ENTRY/lib.task_main/task_main.c`

## Artefacte ENC finale (split)
- `out/403/core_db3_release.core.enc`
- `out/403/fw_db3_release.fw.enc`
- `out/web/core_db3_release.core.enc`
- `out/web/fw_db3_release.fw.enc`

## Comenzi utile după restart
```powershell
# 1) Repo + branch
Push-Location d:\ADI\DB3\Based_on_latest_core\FRD37EU_SPLIT\can.fw.frd37eu
git fetch origin
git checkout split-core-fw-separation-2026-07-02
git pull --ff-only

# 2) Rebuild rapid
& C:\Keil_v5\UV4\UV4.exe -b CORE_DB3.uvproj -j0
& C:\Keil_v5\UV4\UV4.exe -b FW_DB3.uvproj -j0

# 3) Regenerare ENC split
& .\build\scripts\post_build_split_and_enc.ps1 -InputHex .\build\core_db3_build\core_db3_release.hex -OutputDir .\build\core_db3_build -StableEncName core_db3_release.core.enc -EncSource core -Platform DBALL3
& .\build\scripts\post_build_split_and_enc.ps1 -InputHex .\build\fw_db3_build\fw_db3_release.hex -OutputDir .\build\fw_db3_build -StableEncName fw_db3_release.fw.enc -EncSource fw -Platform DBALL3
Pop-Location
```

## Flash bench (ordine)
1. Flash `core_db3_release.core.enc`
2. Flash `fw_db3_release.fw.enc`

