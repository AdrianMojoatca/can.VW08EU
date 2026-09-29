# Release Notes - 2026-07-07

## Scope
- Stabilizare fază split CORE/FW pentru DB3 în repo-ul FRD37EU_SPLIT.
- Întărire contracte de layout și interfață (`gpio_iface`) cu guardrails de build.
- Instrumentare de diagnostic handoff CORE -> FW pentru bench validation.

## Architectural contract (confirmed)
- CORE window: `A..B` (`0x00008100..0x00027FFF`).
- GAP window: `B+1..C-1` (în prezent `0 bytes`, pregătit pentru extindere ulterioară).
- FW window: `C..end` (`0x00028000..0x0003FEFF`).
- Handoff din CORE se face la entry-ul imaginii FW de la adresa `C` (nu direct la `fw_init`).

## Changes included

### 1) FW target linker config fix
- `FW_DB3.uvproj`
  - `LDads/useFile` activat (`1`) pentru a folosi explicit scatter-ul split:
    - `external/can.sdk.db3/BSP/bootloader/lib.app.scatter.1766/app_fw_db3_split.sct`

### 2) Layout guardrails (A/B/C + GAP)
- `external/Core_Contract_SDK_DB3/core_contract_db3_image_layout.h`
  - Adăugate definiții explicite pentru GAP:
    - `CORE_DB3_FLASH_GAP_START_ADDR`
    - `CORE_DB3_FLASH_GAP_END_ADDR`
    - `CORE_DB3_FLASH_GAP_SIZE_BYTES`
    - `CORE_DB3_FLASH_MIN_GAP_BYTES`
  - Adăugate assert-uri compile-time pentru relațiile de layout și zonele shared ABI/API.

- `build/scripts/check_noncore_core_boundary.ps1`
  - Adăugat `Assert-LayoutContract` cu fail-fast pentru contractul de layout.
  - Scriptul validează acum explicit relațiile APP/CORE/FW și calculează GAP-ul la rulare.

### 3) GPIO_IFACE freeze guardrail (contract stability)
- `external/Core_Contract_SDK_DB3/core_contract_db3_gpio_config.h`
  - Ridicat `CORE_DB3_GPIO_CONFIG_VERSION` la `3`.
  - Adăugat `CORE_DB3_GPIO_CONFIG_FREEZE_DATE`.
  - Adăugate expected mappings + assert-uri compile-time pentru maparea IO contractuală fixă.

### 4) CORE->FW handoff hardening + diagnostics
- `external/Core_Contract_SDK_DB3/core_contract_db3_fw_handoff.h`
  - Validare mai strictă pentru entry FW la adresa contractuală `C`.
  - Menținere branch no-return pentru jump.

- `external/can.sdk.db3/ENTRY/lib.task_main/task_main.c`
  - Diagnostic extins pentru handoff (`valid`, `w0..w3`, `decide`, `jumping`).

## Validation summary (bench)
- CORE-only: debug prezent, handoff raportat invalid când FW nu este vizibil la `0x28000`.
- CORE+FW (split ENC în două etape): comportament inconsistent la runtime (în unele scenarii dispare complet debug), indicând o problemă de deployment/programming flow, nu de mapare HEX.
- FW HEX validat cu segment `0x0002xxxx` și date la `0x00028000`.

## Known limitation at this checkpoint
- Tool-ul de flash (fără readback memorie) pare să nu garanteze update incremental sigur cu două ENC-uri split.
- Arhitectura split CORE/FW rămâne ținta principală; workaround-uri monolitice sunt doar pentru test, nu pentru design final.
