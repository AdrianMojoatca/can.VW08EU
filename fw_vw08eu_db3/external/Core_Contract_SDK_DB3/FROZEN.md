# CORE<->FW Contract - FROZEN (thin-FW v2.0, VW08EU)

> **v2.0 (25.08.2026) - RUPERE MAJOR de ABI, valabila NUMAI pentru VW08EU.**
> Tabela de API si blocul de ABI info s-au mutat cu 512 B mai jos, ca sa incapa
> grupul de 25 de servicii cerut de portarea VW08EU. Un FW v1.x pe un CORE v2.0
> (sau invers) citeste tabela la adresa gresita - de aceea MAJOR, nu MINOR.
> In practica nu e o problema: VW08EU are contract propriu, iar CORE-ul si FW-ul
> lui se construiesc mereu in pereche din acelasi junction. FRD37EU nu e atins.
>
> | | v1.x | v2.0 |
> |---|---|---|
> | ABI info | `0x27E00` | **`0x27C00`** |
> | API table | `0x27E40` | **`0x27C40`** |
> | fereastra tabelei | 448 B | **960 B** |
> | `sizeof(tabela)` | 440 B | **588 B** |
> | sloturi rezervate | 4 | **16** |
> | plafon dur cod CORE | `0x27E00` (scatter Max `0x1FD00`) | **`0x27C00`** (Max `0x1FB00`) |
> | plafon soft cod CORE | `0x26E00` | **`0x26C00`** |
> | ABI major.minor | 1.12 | **2.0** |
> | versiune tabela | 14 | **15** |
>
> Codul de CORE se termina la `0x18A08` (din `core_db3_release.map`), deci dupa
> mutare ramin ~61 KB liberi sub plafon. Costul in spatiu e neglijabil.
>
> **Deriva de documentatie corectata cu aceasta ocazie.** Inainte de v2.0 acest
> fisier spunea reserved=6, minor=10, table=12, sizeof=436. Valorile reale din cod
> erau reserved=4, minor=12, table=14, sizeof=440: documentul ramasese in urma fata
> de trei apendari succesive (dbg_config, set_hood_source, gpio_clutch_set).
> **Codul e sursa de adevar; tabelul de mai jos a fost recitit din cod.**

---


**Freeze date: 2026-07-16 (v1.1).** This contract is the stable ABI between the
shared CORE platform image and any thin FW. Addresses, the frozen field offsets,
and the pass-through struct ABIs below **must not change** without a **MAJOR** ABI
bump and a re-release of every thin FW. New capabilities may be added as a
backward-compatible **MINOR** bump only by appending a field at the TAIL (consuming
a `reserved_slots` entry) so all existing offsets are preserved — see "How to
change the ABI".

**v1.1 (istoric, inlocuit de v2.0 - vezi capul fisierului):** runtime debug config is now FW-owned. A `dbg_config` pointer
was appended (tail), ABI minor 9→10, table version 11→12, reserved 7→6, sizeof
unchanged (436 B). Backward-compatible: a v1.0 (minor 9) FW still runs — CORE
detects the older ABI minor from the descriptor and self-inits its own dbg defaults.

## Frozen invariants

| Invariant | Frozen value | Guarded by |
|---|---|---|
| ABI version major | `2` | `core_db3_abi_frozen_major_assert` (abi.h) |
| ABI version minor | `0` | `core_db3_abi_frozen_minor_assert` (abi.h) |
| API table version | `15` | `core_db3_abi_frozen_table_ver_assert` (abi.h) |
| API table reserved slots | `16` | `core_db3_api_table_frozen_reserved_assert` (api_table.h) |
| `sizeof(core_db3_api_table_t)` | `588` B @ `0x27C40` (fereastra 960 B) | ceiling assert + reserved-slots pin |
| Runtime debug config owner | FW (`dbg_config` tail ptr, v1.1) | `api_table_version >= 12` guard (CORE + FW) |
| ABI info address | `0x00027C00` | image_layout.h asserts |
| API table address | `0x00027C40` | image_layout.h asserts |
| FW image base | `0x00028000` | image_layout.h asserts (frozen base) |
| CORE code hard cap | `0x00027C00` | scatter Max `0x1FB00` (link error on overflow) |
| CORE code soft ceiling | `0x00026C00` | `check_core_ceiling.ps1` (post-build map check) |
| NVFS/NVRAM flash | `0x6000–0x7FFF` (bootloader) | out of app window; FW grows away from it |
| Flash layout version | `2` | image_layout.h |

## Pass-through struct ABIs (carried as `void*` across the table)

These structs are **not** re-declared in the contract — they cross the ABI by
pointer and are defined in the shared `can.sdk.db3` SDK. CORE and FW **must** be
built against the same `can.sdk.db3`, so their layout matches:

- `Can_Msg`
- `Process_Cfg`
- `Sense_Rs_Can_Cfg`
- `Cmd_Array1`

## Pinning (must be identical in both consumers)

- `Core_Contract_SDK_DB3` submodule → the same commit in `can.core_db3` **and**
  `can.fw_frd37eu`.
- `can.sdk.db3` → the same base in both images (shared SDK; only the FW-side
  `FW_DB3_BUILD` routing redirects differ, not the struct layouts).

## How to change the ABI (deliberately)

1. Make the change (append-only where possible — consume a `reserved_slots` entry).
2. Bump `CORE_DB3_API_TABLE_VERSION`; bump `CORE_DB3_ABI_VERSION_MINOR` (backward-
   compatible add) or `_MAJOR` (breaking change).
3. Update the frozen-value asserts above to the new values, and this file.
4. Re-release: for a MAJOR bump, rebuild + reflash **every** thin FW.

## Debug config authority (v1.1)

Runtime debug is now a **single shared instance** owned by the FW: CORE holds the
`g_dbg_config` storage, publishes `&g_dbg_config` via the `dbg_config` table pointer,
and the FW writes it in `fw_identity_init()` (before `core_init`). CORE honors it —
`trace_print()` gates on `get_use_trace()`, and `core_init` no longer self-inits the
dbg config when a v1.1 FW is present. This resolves the former dual-`config_dbg.h`
smell: runtime authority is now singular (the FW's values win). The two compile-time
`config_dbg.h` copies now serve distinct, non-conflicting roles:
- FW vehicle copy (`DBG_WDOG_V=0`) → the values the FW pushes at runtime (source of truth).
- Contract copy (`DEBUG_TEXT/USE_TRACE=1`) → CORE compile-time trace *presence* (the
  code must exist to be gated) + the legacy-FW (minor 9) runtime fallback defaults.

**Compile-time vs runtime:** both images must still be built with `USE_TRACE=1` for the
trace *code* to exist; the FW only controls the runtime enable via the shared instance.
Only `use_trace` currently has an observable runtime effect; the per-module `dbg_*`
flags are plumbed for future call-site gating.
