# SS4 — Thin-FW Migration Progress

Goal: turn the DB3 FW into a **thin image** that compiles only vehicle-specific
code and calls CORE's platform through the API table at `0x27E40`, in a
**single-boot** model (CORE boots the RTOS once and stays live, then invokes the
FW via a descriptor at `0x28000` — no FW reboot/BX handoff).

## Architecture (proven)

- **Descriptor** `core_db3_fw_descriptor_t` at FW flash base `0x28000` (magic
  `FWD3`). CORE validates it and calls `fw_ram_init -> fw_identity_init ->
  core_init -> fw_init`. If absent/invalid, CORE falls back to the legacy
  reboot handoff (full backward compatibility).
- **RAM**: CORE scatter reserves `RW_FW` at `0x10004500` size `0xA00`; the thin
  FW's RW/ZI live there, initialized by `fw_ram_init` (manual copy/zero, no
  `__scatterload`; link with `--datacompressor off`).
- **API table** (`core_contract_db3_api_table.h`): append-only, versioned,
  drawing from `reserved_slots`. Hard ceiling `0x27E40 + sizeof(table) <=
  0x28000` (448 B / 112 words).
- **Config single-instance**: CORE owns the storage; the table publishes DATA
  pointers; under `FW_DB3_BUILD` each `g_*_config` is redefined to
  `(*core_db3_cfg_*())` so FW reads/writes CORE's one instance.

## Slice roadmap & status

| # | Slice | Status |
|---|-------|--------|
| 0 | API table restructure (append-only framework) | ✅ done |
| 1 | Single-boot flip (descriptor, `fw_ram_init`, thin build, RW_FW) | ✅ bench-proven |
| 2 | Config single-instance (5 DATA pointers, accessors) | ✅ bench-proven |
| 3 | Diag: `trace_print` routed (variadic via table + FW TRACE macro). Rest of group (led_*, d2d_tx, reset_register, wdog_v_force_reset, cmd_lut_insert_table1) deferred to when their vehicle callers return (7-8) | ✅ bench-proven |
| 4 | RTOS primitives routed: `os_evt_create/clr/signal_v/wait_v`, `os_tsk_wait` (Evt opaque as void*; FW os.h macros redirect to CORE's live RTX) | ✅ bench-proven |
| 5 | Timeout one-shot routed: `timeout_f_create`, `timeout_start`, `timeout_stop` (FW callback fired by CORE timer). `timeout_q_*` deferred (7-8) | ✅ bench-proven |
| 6 | NVFS routed: `nvfs_rd/wr/wr_create/delete`, `nvram_usr_commit` (all return NVFS_OK from CORE via table). `nvfs_fw_data` DATA deferred to Slice 8 (non-const init). Roundtrip value test used a synthetic entry (didn't persist) - real entries validate at Slice 8 | ✅ routing proven |
| 7a | CAN routed: `can_per_init`, `can_usr_init`, `can_usr_rx_reg_fast/slow`, `can_usr_tx`, `can_sync_init/add/remove`. Routing proven (CORE printed "CAN USR INIT DEV 0 OK" via routed call). No bus traffic yet - full physical bring-up (baud/pins/transceiver/filters) done by detect at 8b | ✅ routing proven |
| 7b | process/task/hazard/lf routed (8 fns). Bench: CORE printed TSK_RS/HAZARD INIT (routed calls ran in CORE) + process exec callback fired FW<-CORE after #F5 | ✅ bench-proven |
| 8 (routing) | Vehicle sense/detect/func RE-INCLUDED + all ~47 platform symbols routed (sense_rs/status_system setters+getters/led_detect/d2d/gpio_pl/reset/wdog_v/cmd_lut/timeout_q/nvfs_fw_data). FW compiles + LINKS all vehicle code through the table. | ✅ links |
| 8d (activate) | Real fw_init active (detect_car_search/func_init). prog_set_exit_callback routed (last symbol). **DONE + vehicle-validated**: thin FW = Code 10924 + RO 1104 + RW 124 + ZI 44 = ~12 KB, RW/ZI 168B << 0xA00 (no rebalance). CAN messages OK, ignition, OEM lock = parity with legacy. | ✅ COMPLETE |

## Results so far

- Thin FW at Slice 1: **Code=324, RO=64 (descriptor), RW=0, ZI=0** — 388 bytes at
  `0x28000` (vs ~73 KB legacy).
- Bench: CORE stays live, calls FW, no reboot; at Slice 2 `core_init` prints the
  FW-supplied name `FRD37EU_T1 1.00_91` (config from the single instance).

## Key lessons / gotchas

- A thin FW must **exclude** `crt0.s`, `isr.c`, all platform sources, and unused
  vehicle sources from the build. `--remove` alone is not enough: `crt0`'s
  `B.W __main` roots the entire app+RTX graph. (135 files `IncludeInBuild=0`.)
- **Never call `diag_sync_puts()` before `core_init`** — it blocks on the UART3
  THRE bit, and UART3 is only configured inside `core_init` (`sio_uart_create`,
  before `#S0`). This caused several dead boots that looked like other bugs.
- Markers before `core_init` are invisible anyway (pre-UART); use the GREEN LED
  as a pre-UART progress signal when needed.
- `boot_api_table_get()` in FW is a pure `(table*)0x27E40` cast -> valid before
  `core_init`, so config accessors work pre-`core_init`.

## Flash boundary & growth policy (frozen 2026-07-15, layout v2)

- **NVFS is safe by construction.** NVRAM/NVFS flash "Block #0" lives at `0x6000..0x7FFF`
  (8 KB, bootloader-owned via `BootService()` traps; see `boot_1700.sct`), *below* the app.
  Neither CORE nor FW hold NVFS sector addresses. FW grows **upward** (away from NVFS), so
  FW growth cannot corrupt NVFS. No gap/margin needed on the upper boundary.
- **CORE↔FW is hard-capped by the linker.** CORE code region ends at `0x27E00` (= ABI addr;
  `app.sct` Max `0x1FD00`); FW region ends at `0x3FEFF`. Exceeding either is a **link error**,
  not a silent overlap. GAP stays **0** (adjacent) — the ABI+table fill `0x27E00..0x27FFF`.
- **Soft ceiling (policy, not linker-enforced):** keep CORE code below `0x26E00`
  (`CORE_DB3_FLASH_CORE_SOFT_CEILING_ADDR`, 4 KB reserve under the hard cap). ArmLink can't
  "warn and continue", so check the map after each CORE build:
  `ER_IROM_APP1` Base(0x8100) + Size **< 0x26E00**. If it creeps past, reduce CORE **before**
  0x27E00 — never move the frozen FW base `0x28000`.
- Current headroom: CORE ~67 KB used / ~60 KB free to the cap; FW ~12 KB used / ~86 KB free.

## Commits / tags

- `db3_contract`: Slice 0 `6a2180a`, SS3 descriptor `6d0be3b`, Slice 2 `d70c899`, Slice 3 `2c24c7f`, Slice 4 `6282f07`, Slice 5 `c9a5b15`, Slice 6 `76d4d05`, Slice 7a `27f28ff`, Slice 7b `b336dcb`.
- `can.core_db3` / `can.fw_frd37eu`: tags `thin-fw-slice1`, `thin-fw-slice2`.
- Baseline before the flip: tag `boot-flip-baseline` (all repos).
