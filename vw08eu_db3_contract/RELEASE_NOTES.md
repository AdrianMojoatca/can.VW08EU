# Thin-FW v1.0 — DB3 / FRD37EU (2026-07-15)

**Milestone: the DB3 firmware now builds as a THIN, vehicle-only image that runs
on top of a shared CORE platform, validated end-to-end on a real vehicle.**

## Patch log

### v5 — statusul de frina pe DAC_RELAY, separat de actuatie (2026-09-11)

**Simptom:** la frina primita pe CAN se auzea si se vedea **releul 1**, ceea ce nu
trebuia sa se intimple. Raportat de Adrian la banc.

**Cauza:** in DB2 existau **doua** iesiri de frina, distincte:

```c
VW03_FUNC_BRAKE_OUT(x) = OUT_RELAY1(x)            // actuatia, la demaraj
BRAKE_SENSE_SET/CLR    = _401_OUT_DAC_RELAY(...)  // STATUSUL   (core_gpio.h:122-123)
```

La portare cele doua au fost contopite intr-o singura intrare de contract
(`BRAKE_OUTPUT = RELAY1`), deci statusul a ajuns sa comande releul mecanic.

`DAC_RELAY` nu e un releu obisnuit - comanda un convertor digital-analogic
(CPC1008N, vezi `lib.dac/dac_set_relay_ctl.c` din DB2). Optiunea exista deja in
contract, dar se numeste `DAC_POWER`, de aceea nu a fost observata: SDK-ul
aliaza explicit `OUT_DAC_RELAY(x) -> GPIO_PL_DAC_PWR_CTL` pe amindoua
platformele (`db3_gpio.h:19`, `933_gpio.h:456`). **Pinul difera: P0.18 pe DB3,
P1.4 pe DS4** - P1.4 fiind exact pinul din DB2. Confuzia de semantica vine din
faptul ca toate trei se numesc "RELAY".

**Reparat:** intrare noua de contract `BRAKE_SENSE_OUTPUT -> DAC_POWER`, cu
`EXPECT_` si `MAP_ASSERT`; `core_contract_db3_gpio_brake_sense_set()`;
`gpio_iface_brake_sense()` in shim, **inauntrul** lui `CORE_DB3_BUILD`.
Rutati pe ea: `sense_rs_can_brake.c` si `sense_notify_brake.c`.
Raman pe RELAY1: `FUNC_BRAKE_OUT` (func_rs.c) si `run_safe_shutdown.c`.
Exact impartirea din DB2.

Tabela de API **nu** a fost extinsa: toti apelantii de status sint in CORE, deci
functia nu are nevoie de slot si nu are ce cauta in FW.

**De retinut la retestare:** `sense_rs_can_brake_force_on()` trece tot prin
`sense_brake()`, deci si pulsul de frina de la oprirea la viteza (10.09.2026) se
muta pe DAC_RELAY. Corect - in DB2 tot acolo mergea - dar observabilul se
schimba: releul 1 nu mai clanteaza la testul de viteza.

**DS4: inca nefacut.** Aceeasi operatie, in contractul DS4 si in copia lui de CORE.

### v1.1.1 — lights/hazard pulse fix (2026-07-24)

The parking-light **hazard pulse** (one output pulse per `PARKING ON` / `0xC8` on
d2d) fired only **once** — the first `PARKING ON` pulsed, every later one was
ignored — while the rest of the module worked normally. Bench-confirmed, then fixed.

**Root cause — a latent one-line bug in `func_hazard_ctrl.c`** (not introduced by
the split; the same line exists in the monolith source, just never exercised for a
repeated pulse). `hazard_button_release()` set `button_state = TRUE` (a copy-paste
from `hazard_button_press()`), but `hazard_button_press()` only acts when
`if(!button_state)` and `button_state` is cleared only at init. So after the first
pulse the flag stuck `TRUE` forever and blocked every subsequent press.

**The fix is one line** — `hazard_button_release()` now sets `button_state = FALSE`,
re-arming for the next `PARKING ON`. **FW-only, no contract change** (ABI major 1 /
minor 10 / table v12 and the freeze asserts untouched — no CORE or contract
re-release). Bench-validated: every `PARKING ON` now pulses. The identical bug was
found + fixed first on the DS4 split.

### Repo hygiene — thin-FW disk cleanup (2026-07-16 … 2026-07-20)

Made each image carry only what it actually builds, verified against the Keil
`.dep` files (the authoritative compiled+included set) — never by guesswork. All
changes are file removals/relocations with **no ABI or behavior change**; every
image rebuilds identically on the bench, and everything is git-recoverable.

**CORE (`core_db3`)** — the platform image:
- Removed the entire `VEHICLE_SPECIFIC` tree (detect/func/sense/fw + unused
  config) — CORE compiles no vehicle code. Its one used file, `config_d2d.h`, was
  relocated into the SDK (`external/can.sdk.db3/INC/dball/inc.config.d2d/`) and the
  project IncludePath updated; the `VEHICLE_SPECIFIC` folder is gone.
- Tier B: removed 46 **superseded** dead files under `external/can.sdk.db3`
  (`APP/vehicle/diagnostics/com.ext.lib.obd_can` → superseded by falcon
  `lib.flc.obd_can`; the in-SDK `CONTRACT_SDK_DB3` → superseded by the
  `Core_Contract_SDK_DB3` submodule; a stale `inc.config.template` `config_d2d.h`).
  **Kept** all config-dependent platform capabilities (immobilizer/DST, CAN-v HAL,
  HOBP, EIPS, RXT, software CAN-LUT, gpio_iface): CORE is the shared platform and a
  different vehicle/product config could enable them.

**FW (`fw_frd37eu`)** — the thin vehicle image:
- Removed all 1107 platform `.c` files from `external/can.sdk.db3` (the thin FW
  compiles no platform sources — it reaches CORE via the API table).
- Removed 161 unused platform headers (of 213; kept the 47 actually included plus
  5 config-dependent ones). Whole unused subtrees gone (rf, security/dst, status,
  d2di, io, diagnostics, …). The FW now carries only its own vehicle code, the
  contract, and the ~52 platform headers it includes.

### v1.1 — FW-owned runtime debug config (2026-07-16)

**First deliberate post-freeze ABI evolution — a backward-compatible MINOR bump.**
The runtime debug switch is now owned by the thin FW, so "one CORE, many FWs" each
choose their own debug verbosity without rebuilding CORE. The UART / physical trace
output stays entirely in CORE.

**Mechanism (clones the proven config-single-instance pattern).** A `dbg_config`
pointer is appended to the API table **at the tail** (consuming a reserved slot, so
every v1.0 field offset is preserved — an old FW still reads the table correctly).
CORE owns `g_dbg_config` storage and publishes `&g_dbg_config`; under `FW_DB3_BUILD`
`g_dbg_config` becomes `(*core_db3_cfg_dbg())`; the FW seeds the flags in
`fw_identity_init()` (before `core_init`). CORE honors them: `trace_print()` now gates
on `get_use_trace()`, and `core_init` stops self-initialising the dbg config when a
v1.1 FW is present.

**Boot-line policy:** CORE always emits exactly one identity banner
(`Debug Enable for <fw> <ver>`) regardless of the FW's switch, so module/version
provenance is visible even on a FW shipped with debug OFF; everything else honors the
FW.

**Versions:** ABI minor 9→10, API table version 11→12, reserved slots 7→6,
`sizeof(table)` unchanged at 436 B. Freeze asserts + `FROZEN.md` updated.

**Backward compatibility:** a pre-v1.1 (minor 9) FW runs unchanged — CORE reads the
FW's ABI minor from the descriptor and self-inits its own dbg defaults. A v1.1 FW on
a pre-v1.1 CORE is guarded (`api_table_version >= 12`) so it never dereferences a
missing slot. This also resolves the former dual-`config_dbg.h` smell (runtime
authority is now singular). Compile-time note: both images must still be built with
`USE_TRACE/DEBUG_TEXT=1` for trace code to exist; the FW controls only the runtime
enable.

### v1.0.1 — wire-to-wire (w2w) digital inputs restored (2026-07-16)

The digital **wire-to-wire (w2w)** inputs — the direct-on-the-wire fallback the
module uses when a signal (Ignition, Start, Lock/Unlock, Trunk, Aux1-3, Parking,
Push, GWR) has **no explicit CAN message** — had gone silent in the split build
while remaining functional in the legacy monolith. Bench-confirmed, then fixed.

**Root cause — a single split bring-up regression.** `w2w_init()` was left
commented out in CORE's `core_init()` (`core.c`), so the w2w subsystem never
created its input edge detectors nor registered its periodic pin scanner with
`per_exec`. The physical pins were simply never read. Everything downstream was
already intact.

**Diagnosis (architect-reviewed).** The entire consumer chain survives the split
unchanged: `w2w_execute()` reads the physical pins through the CORE-real
`core_contract_db3_gpio_input_read()`, edge-detects, and enqueues `INPUT_*`
commands via `cmd_q_insert()`; CORE's cmd task dispatches them through
`cmd_exec_table`; and the thin FW's vehicle handlers register into that **same**
table via the already-routed `cmd_lut_insert_table1` — the identical path the
CAN-sourced commands take today. A wire input therefore yields exactly the same
effect as its CAN equivalent, and CAN detection correctly **overrides the source**
when a message is present.

**The fix is one line, CORE-only** (re-enable `w2w_init()`), restoring parity with
the monolith. **No contract change**: the consumer path rides a pre-existing,
already-populated frozen API-table field, so ABI major 1 / minor 9 / table v11 and
the freeze asserts are untouched — **no thin-FW re-release required**. Bench-
validated on real hardware: every input behaves as its CAN counterpart. The
power-up arming window and the CAN-overrides-source behavior are by design (the
installer chooses whether to wire w2w, per the install guide).

## What this is

The DB3 application was split into two flash images that share one RAM/RTOS
address space in a single-boot model:

- **CORE** (`0x00008100–0x00027FFF`) — the platform: boots the RTOS once, owns all
  drivers/peripherals (CAN, GPIO, NVFS, timers, RTX), and stays live.
- **Thin FW** (`0x00028000–0x0003FEFF`) — only the vehicle-specific code
  (detect / func / sense + FW glue). It compiles **no platform code**; every
  platform service is reached through a versioned API table.

CORE finds the FW via a descriptor at `0x28000` (magic `FWD3`) and calls its
entry points — no reboot, no second scatter-load. The FW's RW/ZI lives in a
reserved window (`0x10004500`, `0xA00`) initialized by `fw_ram_init`.

## Headline results

| Metric | Before (monolithic) | Thin FW v1.0 |
|---|---|---|
| FW image size | ~73 KB | **~12 KB** (Code 10924 + RO 1104 + RW 124 + ZI 44) |
| FW RW/ZI RAM | — | 168 B (well under the 0xA00 window) |
| Platform code in FW | all | **none** (routed via API table) |

**Bench-validated on a real vehicle:** boot, CAN rx/tx messages, ignition on/off,
OEM lock/unlock — parity with the legacy monolithic build.

## How it works (contract)

- **API table** at `0x27E40` (`core_contract_db3_api_table.h`), append-only,
  versioned; a compile-time assert keeps it under the `0x28000` ceiling.
- **Function routing:** FW-side headers, under `FW_DB3_BUILD`, redirect each
  platform call to `core_contract_db3_boot_api_table_get()->*_fn(...)`. Handles
  are carried opaquely (`void*`), enums/ints as `uint32_t`.
- **Config single-instance:** CORE owns the `g_*_config` storage; the table
  exposes pointers; FW's `g_*_config` macros dereference them, so FW and CORE
  read/write the SAME instance (fixes the split-config trap).
- **CORE→FW callbacks:** FW registers callbacks (timeout, CAN rx, process exec,
  tsk_rs, hazard) that CORE calls from its own context — the mechanism vehicle
  tasks are built on.

## The path (SS4 slices, all bench-proven)

0. API-table framework · 1. single-boot flip (descriptor, `fw_ram_init`) ·
2. config single-instance · 3. `trace_print` · 4. RTOS (`os_evt_*`, `os_tsk_wait`) ·
5. timeout · 6. NVFS · 7a CAN · 7b process/task/hazard/lf ·
8. re-include vehicle (sense/detect/func) + route the full falcon surface ·
8d activate real `fw_init` + vehicle test.

## Build & combine

- Build CORE_DB3 → CORE hex; build FW_DB3 → thin FW hex (`~12 KB` at `0x28000`).
- Combine (`combine_core_fw_hex.ps1 -RunEnc`) → single bring-up ENC; flash.
- FW project (`FW_DB3.uvproj`) now lists only the 5 thin groups
  (lib.fw.db3, com.lib.detect/func/sense, com.Core_Contract_SDK_DB3); platform
  headers are resolved via IncludePath, not compiled.

## Production follow-ups — DONE (2026-07-15/16)

- [x] Retire the `#if 0` synthetic slice self-tests + test callbacks in `fw_init.c`.
      *(done: fw_frd37eu `caa0607`)*
- [x] Strip the temporary sync-UART `#S`/`#T`/`#F` diagnostic markers + the
      `[FW][PROG]` info trace. *(done: core_db3 `dd4694d`, fw_frd37eu)*
- [x] Restore the hardware watchdog to production. *(done: core_db3 `dd4694d`,
      `wdog_v_init.c` — bench-validated, no spurious reset. `DEBUG_TEXT` kept `1`;
      `DBG_WDOG_V` kept `1` for WDT reset diagnostics — see FROZEN.md smell note.)*
- [x] Freeze the contract: addresses (`0x27E00`/`0x27E40`/`0x28000`), table layout,
      and the pass-through struct ABIs (Can_Msg, Process_Cfg, Sense_Rs_Can_Cfg,
      Cmd_Array1) — pinned identically in both images. **See `FROZEN.md`** and the
      build-time freeze asserts (abi.h / api_table.h). *(ABI major 1 / minor 9 /
      table v11 frozen; flash layout v2.)*

## Flash boundary & growth policy (frozen 2026-07-15, layout v2)

The CORE↔FW flash boundary and the NVFS reservation are now documented and frozen
in `core_contract_db3_image_layout.h` (version bumped `1 → 2`):

- **NVFS is safe by construction.** NVRAM/NVFS flash "Block #0" lives at
  `0x00006000–0x00007FFF` (8 KB, 4k-aligned, declared in the bootloader
  `boot_1700.sct`), *below* the application and reached only through bootloader
  `BootService()` traps — neither CORE nor FW hold its sector addresses. The FW
  grows **upward** (away from NVFS), so FW growth cannot corrupt NVFS. No upper
  margin needed.
- **CORE↔FW is hard-capped by the linker.** CORE code ends at `0x27E00` (= ABI
  addr; `app.sct` `ER_IROM_APP1` Max `0x1FD00`); the FW region ends at `0x3FEFF`.
  Exceeding either is a **link error** (region overflow), never a silent overlap.
  The gap stays **0** (adjacent) — ABI + API table fill `0x27E00–0x27FFF`.
- **CORE soft ceiling** `0x26E00` (`CORE_DB3_FLASH_CORE_SOFT_CEILING_ADDR`, 4 KB
  reserve under the hard cap). ArmLink cannot "warn and continue", so it is checked
  after each CORE build with `check_core_ceiling.ps1` (map: `LR_IROM_APP1`
  Base 0x8100 + Size < `0x26E00`). Current CORE end `0x18E38` → ~56 KB free.
- **FW base `0x28000` is FROZEN** — every shipped thin FW links against it; if CORE
  needs more room, reduce CORE, never move the FW base.
- New compile-time asserts pin these relationships (`core_code_cap_at_abi`,
  `core_code_cap_below_table`, `soft_ceiling_below_cap`, `soft_ceiling_above_start`).

## Repo state at v1.0

- `can.db3_contract` (submodule) — the CORE↔FW ABI + this progress log.
- `can.core_db3` — CORE image (descriptor-aware boot, API-table publisher).
- `can.fw_frd37eu` — the thin FRD37EU FW.
- Tag `thin-fw-v1.0` on all three; see `SS4_THIN_FW_PROGRESS.md` for the slice log.
