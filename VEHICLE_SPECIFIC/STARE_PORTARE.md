# VEHICLE_SPECIFIC - stare portare VW08EU

## Stare pe lib (actualizat 25.08.2026)

| Lib | Stare | Note |
|---|---|---|
| `com.lib.sense` | **PORTAT** | 19 fisiere; 5 Ford scoase, 4 DB2 adaugate |
| `com.lib.scan_manager` | **PORTAT** | 26 fisiere, 2037 linii; copiere pura |
| `com.lib.scan` | **PORTAT** | 12 fisiere, 611 linii; copiere pura |
| `com.lib.vw06_scan` | **PORTAT** | 4 fisiere; 4 orfane excluse; prefix pastrat pe nume |
| `com.lib.scan_new` | **PORTAT** | 3 fisiere; nume pastrate |
| `com.lib.detect` | **PORTAT** | 9 fisiere; `detect_init.c` (Ford) sters |
| `com.lib.func` | **PORTAT** | 25 fisiere; 2 Ford sterse, `func_rap.c` adaugat |
| `lib.fw.db3` | **PORTAT (partea de vehicul)** | vezi mai jos |
| `com.lib.clutch` | **PORTAT SI IN BUILD** (25.08.2026) | 10 fisiere; vezi mai jos |

Toate cele 8 lib-uri sint portate. `.uvproj` are **115 intrari**, 0 lipsa pe disc,
9 grupuri.

## Regulile de redenumire (com.lib.func)

Fisierele pierd prefixul `vw06_`. In continut, ordinea regulilor conteaza si toate
sint **case-sensitive**:

1. `VW03_FUNC_` -> `FUNC_`, apoi `VW03_` -> `FUNC_`  (macrouri, membri de enum,
   garzile de include: `__FUNC_H__`, `__FUNC_CFG_H__`, `__FUNC_PRIVATE_H__` -
   verificate ca libere in tot proiectul)
2. `Vw03_` -> `Func_`  (`Func_Cfg`, `Func_Aux_Type`, `Func_Confort`, `Func_Prc` -
   exact numele din Ford)
3. `vw03_func_` -> `func_`, apoi `vw03_sense_` -> `sense_`, `vw03_clutch_` ->
   `clutch_`, `vw03_detect_` -> `detect_`, `vw03_car` -> `car`
4. ce a rimas (`vw03_cfg`, `vw03_prc`, `vw03_prc_cfg`, `vw03_dl_type`,
   `vw03_can_table`) -> `func_...`

**Capcana, deja platita o data:** in PowerShell `-replace` e case-INsensitive, deci
regula 1 prinde si `vw03_func_` si transforma toate functiile in `FUNC_xxx`. Se
foloseste **`-creplace`**.

**Capcana de nume:** pasul 3 e obligatoriu inainte de pasul 4. Fara el, un blanket
`vw03_` -> `func_` face `vw03_sense_init` sa devina `func_sense_init`, iar in
`func_init.c` linia `vw03_cfg = cfg;` ar deveni `cfg = cfg;`.

## com.lib.clutch - reintrodus in build (25.08.2026)

Adrian a cerut ambreiajul in FW. Cele trei blocaje de dinainte s-au rezolvat astfel.

**1. Timer + FIQ: 6 simboluri rutate prin tabela (v2.1).**
`fiq_register`, `tmr_create`, `tmr_ena`, `tmr_dis`, `tmr_update`, `tmr_int_sta`. Toate
se potrivesc pe typedef-uri **existente** - niciunul nou. Vectorul de intrerupere
ramine al CORE-ului; prin tabela trece doar inregistrarea handler-ului si comanda
timerului, iar handler-ul (`clutch_fiq`) sta in flash-ul de FW.

De urmarit la banc: PWM-ul de ambreiaj are perioada 2 ms
(`CLUTCH_PWM_PERIOD 72*2000`) cu impulsuri sub milisecunda, deci **fiecare FIQ trece
din vectorul CORE-ului in flash-ul de FW**. E o intrebare de latenta, nu de corectitudine.

**2. Cele trei iesiri, prin clase noi de contract.** Nu se putea comanda pinul direct:
garda `check_noncore_legacy_gpio.ps1` interzice `OUT_*(` si `GPIO_PL_*` in codul de
vehicul - exact ce trebuie.

| macro DB2 (ramura DBALL) | pin | clasa de contract |
|---|---|---|
| `CLUTCH_PWM_OUT(x)` = `OUT_PHAL2_TX(!x)` | ISO2_TX, P0.11, **inversat** | `CLUTCH_PWM` (selector nou `ISO2_TX`) |
| `CLUTCH_PWM_CUT(x)` = `OUT_RELAY1(x)` | RELAY1 | `CLUTCH` - **aceeasi clasa cu BRAKE** |
| `CLUTCH_SENSOR(x)` = `OUT_RELAY2(x);OUT_5(x)` | RELAY2 + NEG_OUT5 | `CLUTCH_SENSOR_A` + `_B` |

Trei decizii confirmate de Adrian:
- **Frana si ambreiajul sint pe acelasi releu** (RELAY1), ca in DB2. `BRAKE_OUTPUT` a
  trecut de la `POS_OUT1` (seed FRD37EU) la `RELAY1`. La demaraj releul e actionat si
  PWM-ul e taiat de aceeasi comanda.
- **PWM-ul si RAP-ul stau pe acelasi pin fizic** (ISO2_TX) si in DB2
  (`RAP_ON_WIRE_SET -> ISO2_TX_SET`). Nu sint niciodata active simultan: RAP e dupa
  shutdown, PWM-ul la demaraj.
- **`CLUTCH_SENSOR` s-a spart in doua clase**, ca sa nu rupem regula "o clasa = un
  selector". `NEG_OUT5` e liber fiindca PTS trece pe `OUT_2` la VW08EU.

Inversarea PWM-ului sta in `clutch_private.h`, la apelant - nu in contract. Exact ca in
DB2, unde `!x` era in `config_gpio.h`, adica in configurarea de vehicul.

**3. Un bug gasit in SDK, ocolit.** `INC/falcon/core/db3_dummy/db3_gpio.h`:
```c
#define OUT_PHAL2_TX(x)  (x ? (gpio_pl_set(GPIO_PL_ISO2_TX)) : (gpio_pl_clr(GPIO_PL_ISO1_TX)))
```
Pune ISO2 dar **sterge ISO1** - copy-paste. Daca am fi rutat PWM-ul prin acest macro,
iesirea nu s-ar fi stins niciodata si ISO1 ar fi fost alterat. Nu il folosim: clasa de
contract merge direct pe selectorul `ISO2_TX`. **Macroul din SDK ramine buguit** - de
raportat separat; mai are si comentariul "PHAL1" pe blocul PHAL2.
## lib.fw.db3 - ce s-a portat si ce NU se atinge

`fw_init.c` **nu** s-a inlocuit cu versiunea DB2. Planul initial spunea asa, si era
gresit: fisierul din DB3 duce infrastructura de thin-FW, care nu are echivalent in DB2:
`fw_identity_init()`, `core_contract_db3_pts_ctl_config_apply_vehicle_specific()`,
`..._status_config_...`, `..._prog_config_...`, `fw_prog_vehicle_specific_enable_apply()`.
S-a inlocuit doar partea de vehicul:

- **`fw_init()`** - fluxul DB2: `detect_car_search()` (void), poarta pe
  `car.bits.can1_detected`, `cfg.can_dev_id = CAN_DEV_FUNC_SENSE`, apoi
  `func_init(&cfg, car.bits.can0_detected)`.
- **`fw_options_set()`** - semantica DB2, exprimata prin accesorii de contract in
  loc de `prog_retrieve_option(FEAT_*)`. Maparea e comentata bloc cu bloc in fisier.
- **`static Func_Cfg cfg`** (era `func_cfg`) - `func_cfg` e globalul lui
  `com.lib.func`, iar `func_init()` il face sa pointeze aici.
- **`fw.h`** - adaugat `#include "clutch.h"`.
- **`fw_init_reset.c`** - deja corect (`detect_reset_init()`), neatins. Nu e in
  `.uvproj`: descriptorul de FW are slotul `fw_init_reset_fn` pe 0 (nefolosit in Slice 1).
- **`fw_dbg.c`** - copiat din DB2, **lasat in afara build-ului**. Nimic nu apeleaza
  `fw_dbg` pe thin-FW (descriptorul nu are slot pentru el), iar singurul lui simbol
  nou, `cmd_q_insert`, ar cere o rutare in tabela de API degeaba.
- **`fw_boot.c`, `fw_descriptor.c` - NEATINSE.** Sint infrastructura thin-FW
  (descriptorul de la `0x28000`, `fw_ram_init`), nu au echivalent in DB2.

Backup-uri: `fw_init.c.bak_prefuncport`, `fw_init.c.bak_predb3flow`.

## Iesirile lui func - rutate prin contract

In DB2 veneau din `config_gpio.h`, inaccesibil pe thin-FW (GPIO-ul e al CORE-ului,
seedat prin contract). Puntile sint in `func_private.h`. VW08EU e pe DBALL, deci
blocul activ in DB2 e cel de sub `#ifndef PLATFORM_CM800`:

| macro | DB2 (DBALL) | clasa de contract |
|---|---|---|
| `FUNC_HAZARD_OUT` | `OUT_1` | `gpio_iface_lights_on/off` |
| `FUNC_PTS_OUT` | `SPEED_PROT_NORMAL(OUT_2)` | `gpio_iface_pts_1` + acelasi gard |
| `FUNC_BRAKE_OUT` | `OUT_RELAY1` | `gpio_iface_brake` |
| `FUNC_RF_LOOP_RSR` | `RSR_OUT_4` | `gpio_iface_rf_loop` |
| `FUNC_TRUNK_OUT` | `;` neutralizat | no-op, ca in DB2 |
| `FUNC_RS_IGNITION2_OUT` | `;` neutralizat | no-op, ca in DB2 |
| `FUNC_EIPS_ALARM_OUT` | nedefinit (`EIPS_ENABLE != 1`) | no-op |

`SPEED_PROT_NORMAL` s-a reprodus exact: `tsk_rs_invalid_state()` exista in SDK.

**DE FACUT IN CONTRACT.** Pinii sint deocamdata cei seedati de la FRD37EU:
`LIGHTS->NEG_OUT2`, `PTS_1->NEG_OUT5`, `BRAKE->POS_OUT1`. VW08EU cere
`HAZARD->OUT_1`, `PTS->OUT_2`, `BRAKE->RELAY1`. Renumerotarea e o decizie separata,
deliberata - nu s-a facut in acest increment.


## gpio_iface.h lipsea din SDK-ul de FW

Primul build a dat `#223-D: function "gpio_iface_lights_on" declared implicitly` in
`func_hazard_ctrl.c` si `func_rs.c` (plus `gpio_iface_brake`, `_pts_1`, `_rf_loop`).
Cauza: familia `gpio_iface_*` e **definita** de `core_contract_db3_gpio_iface_shim.c`
(care e in build, in afara blocului `#if defined(CORE_DB3_BUILD)`), dar headerul care
o **declara**, `APP/services/gpio_iface/gpio_iface.h`, exista numai in copia de SDK a
CORE-ului. Pe partea de FW lipsea, deci compilatorul le declara implicit ca `int f()`.

Pe ARM apelul ar fi mers din intimplare (acelasi ABI pentru un argument int-size),
dar e comportament nedefinit si ascunde erori de semnatura.

**Reparat:** `gpio_iface.h` copiat 1:1 din SDK-ul CORE in cel de FW (numai headerul,
niciun `.c` - SDK-ul de FW ramine headers-only), include path
`./external/can.sdk.db3/APP/services/gpio_iface` adaugat in `.uvproj`, si `#include
"gpio_iface.h"` pus in `func_private.h` si `sense_private.h`. In `sense_private.h` a
inlocuit cele doua declaratii `gpio_iface_handbrake_on/off` scrise de mina la portarea
lui sense.

## Adaptari facute (punti catre facilitatile template-ului)

- `TRACE_VW03_SENSE` -> `TRACE_SENSE`, `TRACE_VW03_FUNC` -> `TRACE_FUNC`;
  `TRACE_CAN2` -> punte catre `TRACE_SENSE` (sense), `TRACE_FUNC` (func) si `TRACE`
  (detect); `#include "trace_can2.h"` -> `"trace.h"` (headerul nu exista la noi)
- `HBRAKE_SENSE_SET/CLR` -> `gpio_iface_handbrake_on()/off()`. Pe thin-FW
  `config_gpio_1.h` NU e inclus nicaieri (configurarea e a CORE-ului, seedata prin
  contract), de aceea `gpio.h` avertizeaza mereu "config_gpio is not up to date".
- `OUT_SENSE_DOOR_TRUNK(x)` -> no-op; in DB2 era deja neutralizat.
- **OBD CAN lasat neactivat** in `func_init()`, deliberat. In DB2 erau
  `obd_can_init(CAN_DEV_OBD, TRUE)` + `obd_can_activate()`. Doua motive:
  `lib.flc.obd_can` exista numai in CORE si nu e rutata (pe FW nu exista nici
  declaratia), iar API-ul s-a scindat intre timp in `obd_can_init(void)` +
  `obd_can_init_can(dev, is_std, do_init_can)` - semantica lui `do_init_can` pentru
  cazul nostru nu e stabilita, deci nu se ghiceste. In CORE apelurile echivalente
  sint comentate azi (`core.c` ~225), deci OBD nu e activat nici acolo.
- `func_eips.c` intra in build dar corpul e sub `#if EIPS_ENABLE == 1`, iar
  `EIPS_ENABLE` nu e definit nicaieri (nici in DB2). Ramurile `#else` dau
  `func_eips_init(){}`, deci linkeaza. Comportament identic cu DB2.

## Datoria tehnica curatata in acest increment

Cele trei blocuri `TEMPORAR` din `com.lib.sense` au fost **sterse**:
`ignore_doors_sense` (vine acum din `func_rs.c`, ca in DB2), plus declaratiile si
definitiile lui `GLOBAL_LOCK_446` / `GLOBAL_UNLOCK_446` / `GLOBAL_331` / `GLOBAL_332`
(erau date Ford, folosite doar de `func_rs.c` Ford si `func_tester_lock_unlock.c`,
ambele inlocuite/sterse). Au disparut si cele trei resturi Ford: `GLOBAL_RKE`,
`sense_331_present`, `sense_332_present`.

## Link: 30 simboluri nerezolvate (masurat, build 25.08.2026)

Compilarea e curata (0 erori, 95 `.c`). La link cad 30 de simboluri, in trei clase:

**1. Ambreiaj (5) - prin proiectare.** `clutch_init`, `clutch_press`, `clutch_release`,
`clutch_enable`, `clutch_disable`. Dispar cind se reinclude `com.lib.clutch`.

**2. De rutat prin tabela de API (24).** Toate verificate ca **exista si sint definite
in CORE**, deci sint rutabile:

| lib | simboluri |
|---|---|
| sense (7) | `sense_rs_can_set_dl_status`, `get_ss_trigger`, `sense_rs_oem_autolock_can`, `get_hood_status`, `sense_rs_neutral_can`, `get_rpm`, `get_door_status` |
| sense OEM alarm (3) | `sense_rs_can_oem_alarm_arm`, `_disarm`, `_trigger` |
| scan (3) | `per_exec_register`, `per_exec_critical_enter`, `per_exec_critical_leave` |
| scan_new (4) | `os_irq_create`, `os_irq_set`, `os_irq_wait_v`, `os_mem_request` |
| detect (2) | `led_detect_err_can1`, `led_detect_err_invalid_bcm` |
| func (3) | `dl_cfg`, `can_per_add`, `can_per_remove` |
| func/RS (1) | `tsk_rs_invalid_state` |
| lib.fw (1) | `lf_autopulse_enable` |

**3. Fara omolog in CORE (1).** `can_sw_filter_disable` (`detect_car_search.c:44-45`).
Nu exista nicaieri in `can.sdk.db3`, nici pe FW nici pe CORE. Inlocuitorul probabil e
familia `can_lut_sw_*` (filtrare software prin LUT), dar semantica trebuie stabilita
inainte de a scrie ceva.

### Corectii la lista de dinainte de build

Documentul dadea 21 de simboluri, dintr-o lista care insuma de fapt 19, si sustinea ca
4 lipsesc din SDK-ul modern. Masuratoarea pe link arata altceva:

- **`sense_rs_can_oem_alarm_arm/_disarm/_trigger` NU sint disparute** - sint definite in
  CORE (`sense_rs_can_oem_alarm.c`), doar lipsesc din copia headers-only de FW. Sint
  rutabile ca oricare altele. Singurul fara omolog ramine `can_sw_filter_disable`.
- **`can_per_add` lipsea din analiza statica** fiindca `func_can_wake.c:35` apeleaza
  `can_per_loop(...)`, un macro care se expandeaza in `can_per_add(...)`. Un scan pe
  text nu vede prin macrouri - de retinut pentru urmatoarele lib-uri.
- **`tsk_rs_invalid_state` lipsea din cauza mea**: l-am folosit in puntea `FUNC_PTS_OUT`
  dupa ce am verificat ca *exista* in SDK, fara sa verific si ca e *rutat*. Existenta
  unei declaratii nu implica o intrare in tabela.

## Contract v2.0 - tabela de API mutata si extinsa (25.08.2026)

La `0x27E40` nu mai incapeau: fereastra era 448 B, tabela ocupa deja 440 B, iar
`RESERVED_SLOTS` era 4. Necesarul era 25. S-a coborit baza cu 512 B.

| | v1.x | v2.0 |
|---|---|---|
| ABI info | `0x27E00` | `0x27C00` |
| API table | `0x27E40` | `0x27C40` |
| fereastra tabelei | 448 B | **960 B** |
| `sizeof(tabela)` | 440 B | **588 B** (131 membri) |
| sloturi rezervate | 4 | **16** |
| liber peste rezerva | 8 B | **372 B** (93 cuvinte) |
| scatter CORE `Max` | `0x1FD00` | `0x1FB00` |
| plafon soft CORE | `0x26E00` | `0x26C00` |
| ABI major.minor | 1.12 | **2.0** |
| versiune tabela | 14 | **15** |

Costul in spatiu e neglijabil: codul de CORE se termina la `0x18A08`
(`core_db3_release.map`, `ER_IROM_APP1` base `0x8100` size `0x10908`), deci sub noul
plafon rimin **~61 KB** liberi. E o rupere **MAJOR** de ABI fiindca un FW v1.x ar citi
tabela la adresa veche - dar VW08EU are contract propriu, iar CORE-ul si FW-ul lui se
construiesc mereu in pereche din acelasi junction. **FRD37EU nu e atins.**

### Fisierele modificate

**In contract** (`Portate\vw08eu_db3_contract`, sursa unica prin junction):
- `core_contract_db3_image_layout.h` - cele doua adrese
- `core_contract_db3_abi.h` - MAJOR 1->2, MINOR 12->0, tabela 14->15, plus cele trei asserturi de freeze
- `core_contract_db3_api_table.h` - 4 typedef-uri noi, 25 de cimpuri noi (append-only, offset-urile v1.x neschimbate), `RESERVED_SLOTS` 4->16
- `core_contract_db3_gpio_iface_shim.c` - 25 de intrari in initializator, `#include "per_exec.h"` + `"dl.h"`, si trei declaratii `extern` pentru alarma OEM (sint declarate numai in `sense_rs_can_private.h`, iar shim-ul nu include headere private)
- `check_core_ceiling.ps1` - plafoane -512 B; **si calea implicita a map-ului**, care arata inca spre arborele FRD37EU
- `FROZEN.md` - invariantii recititi din cod

**In SDK-ul de CORE:** `app.sct` - `Max 0x1FD00` -> `0x1FB00` (ambele copii, desi numai
cea din proiectul de CORE e folosita). Scatter-ul de FW nu s-a atins: el referea doar
`0x28000`, care nu se misca.

**In SDK-ul de FW:** 25 de macrouri de redirectare, in 10 headere. Tiparul e cel din
`os.h`: `#if defined(FW_DB3_BUILD)` + apel prin `core_contract_db3_boot_api_table_get()`.
Doua cazuri variadice au cerut grija - `per_exec_register` si `os_irq_wait_v`:

- **fara `__VA_ARGS__`.** Nu exista precedent in SDK, si tiparul stabilit la `os_evt_*`
  e altul: `#undef` pe macroul invelitoare + redefinire cu **aritate fixa**, apelind
  direct cimpul variadic cu argumentele promovate `(VARG)`.
- `per_exec_register` e declarat variadic, dar unicul apel din VW08EU
  (`scan_init.c:56`) ii da un singur argument -> aritate fixa 1.
- `os_irq_wait_v` nu e apelat direct; se ajunge la el prin `os_irq_wait(irq,pre,to)`.
  S-a facut `#undef os_irq_wait` si redefinit pe 3 argumente.

### Scripturile de build nu au avut nevoie de modificari

`combine_core_fw_hex.ps1` si `core_layout_common.ps1` **citesc adresele din
`image_layout.h`** (`Get-LayoutDefineValue` / `Get-HexDefineValue`), nu le au
hardcodate. Bootloader-ul nu cunoaste cele doua adrese - el incarca aplicatia de la
`0x8100`, iar NVFS-ul lui e la `0x6000-0x7FFF`, sub aplicatie.

## can_sw_filter_disable - rezolvat

Nu era un feature disparut: functia a fost **redenumita si mutata**, 1:1.

| DB2 (`lib.flc.can`) | modern (`lib.flc.can.usr`) |
|---|---|
| `can_sw_filter_add_id(dev,id)` | `can_usr_rx_filter_add(dev,id)` |
| `can_sw_filter_check_id(rx,id)` | `can_usr_rx_filter_check_id(dev,id)` |
| `can_sw_filter_enable(dev)` | `can_usr_rx_filter_enable(dev)` |
| `can_sw_filter_disable(dev)` | `can_usr_rx_filter_disable(dev)` |

Acelasi corp, aceeasi semantica: `filter.enable == FALSE` => trec toate mesajele.

Detaliul care conteaza: in DB2 filtrul era **pornit implicit**
(`can_rx_create.c:40`, `filter.enable = TRUE`), deci cele doua apeluri din
`detect_car_search` erau **esentiale** - fara ele detectia ar fi vazut numai ID-urile
deja inregistrate. In SDK-ul modern implicitul e invers (`can_usr_rx_dev0/1_init.c`
pun `FALSE`), deci apelurile ar fi redundante azi. **Le-am pastrat oricum**: exprima o
cerinta a detectiei (RX nefiltrat, ca sa poata asculta ID-uri necunoscute), si nu e
sanatos sa depinzi de un implicit din alt strat.

`detect_car_search.c` foloseste acum `can_usr_rx_filter_disable`, iar simbolul a intrat
in tabela ca oricare altul. Backup: `detect_car_search.c.bak_precanfilter`.
## Doua consecinte ale rutarii, prinse la build

**1. Un simbol rutat devine macrou, deci nu mai suporta prototip.**
`sense_oem_alarm.c` isi declara local `sense_rs_can_oem_alarm_disarm/_arm/_trigger`
(nu exista in headerul public, doar in cel privat). Dupa rutare acestea sint macrouri
fara parametri, iar `void nume ( void );` devine o invocare de macrou cu un argument:
`error #55: too many arguments in macro invocation`, plus cascada de #18 si #147.

Reparat cu `#if !defined(FW_DB3_BUILD)` in jurul celor trei. `_arming` nu e rutat (nu
e apelat de nicaieri in VW08EU), deci prototipul lui sta neconditionat.

Verificat apoi pe tot build-ul: **nu mai exista alt prototip** al vreunuia din cele 25,
in afara guard-ului. Atentie insa la doua forme care ar da acelasi tip de eroare si care
nu apar azi: luarea adresei functiei (`&nume` sau `nume` fara paranteze, ca argument de
callback) si un apel cu alta aritate decit macroul.

**2. `MINOR == 0` face verificarea de minor vida.**
`core_contract_db3_api_table_is_valid()` compara
`api_table->abi_version_minor < CORE_DB3_ABI_VERSION_MINOR`. Cu MINOR trecut la 0,
comparatia e `unsigned < 0` - mereu falsa (`warning #186-D`) si fara efect. Am pus-o
sub `#if (CORE_DB3_ABI_VERSION_MINOR > 0U)`. La MAJOR 2 / MINOR 0 garda care conteaza e
cea de major, fiindca adresele s-au mutat.
## Maparea de pini VW08EU, aplicata in contract (v4, 25.08.2026)

Contractul are un mecanism propriu de fixare a mapei: `CORE_DB3_GPIO_MAP_ASSERT`
compara fiecare clasa cu un `CORE_DB3_GPIO_IFACE_EXPECT_*`. Cind am mutat frana pe
RELAY1 fara sa actualizez si valoarea asteptata, build-ul a picat in **fiecare** fisier
cu `error #94: the size of an array must be greater than zero`. Garda a facut exact ce
trebuia: a prins o schimbare de pini neconfirmata.

`CORE_DB3_GPIO_CONFIG_VERSION` 3 -> **4**, `FREEZE_DATE` -> `20260825`.

Maparea completa, fiecare valoare cu dovada intr-o linie din `config_gpio.h`
(ramura `#ifndef PLATFORM_CM800`, adica DBALL):

| clasa | selector | dovada in DB2 |
|---|---|---|
| `LIGHTS` (hazard) | `NEG_OUT1` | `FUNC_HAZARD_OUT -> OUT_1` |
| `PTS_1` | `NEG_OUT2` | `FUNC_PTS_OUT -> SPEED_PROT_NORMAL(OUT_2)` |
| `BRAKE` | `RELAY1` | `FUNC_BRAKE_OUT -> OUT_RELAY1` |
| `CLUTCH` (PWM cut) | `RELAY1` | `CLUTCH_PWM_CUT -> OUT_RELAY1` |
| `CLUTCH_PWM` | `ISO2_TX` | `CLUTCH_PWM_OUT -> OUT_PHAL2_TX(!x)` |
| `CLUTCH_SENSOR_A` | `RELAY2` | `CLUTCH_SENSOR -> OUT_RELAY2(x);...` |
| `CLUTCH_SENSOR_B` | `NEG_OUT5` | `CLUTCH_SENSOR -> ...;OUT_5(x)` |
| `RAP` | `ISO2_TX` | `RAP_ON_WIRE_SET -> ISO2_TX_SET` |
| `RF_LOOP` | `RF_LOOP_POWER` | `RF_LOOP_RSR -> RSR_OUT_4` |
| `HANDBRAKE` | `NONE` | `HBRAKE_SENSE` e selectabil de instalator in `core_gpio.h` (OUT_1..5, RELAY1/2, PHAL1/2 sau nimic); nu e o iesire fixa |
| `TRUNK`, `DOORS`, `IGNITION` | `NONE` | neutralizate in DB2 (`;`) |
| `PTS_2` | `NONE` | DB2 are o singura iesire PTS |

### Conflictul care exista si a fost rezolvat

Cind am pus `CLUTCH_SENSOR_B` pe `NEG_OUT5`, am scris ca "PTS trece pe OUT_2 la
VW08EU" - dar **nu facusem mutarea**. Rezultatul: `PTS_1` si `CLUTCH_SENSOR_B` erau
amindoua pe `NEG_OUT5`, si spre deosebire de celelalte suprapuneri **sint active in
acelasi moment**, la demaraj. Mutarea lui `PTS_1` pe `NEG_OUT2` l-a eliminat.

### Suprapunerile rimase, toate legitime

| pin | clase | de ce e in regula |
|---|---|---|
| `RELAY1` | `BRAKE` + `CLUTCH` | deliberat, ca in DB2: la demaraj releul e actionat si PWM-ul e taiat de aceeasi comanda |
| `ISO2_TX` | `CLUTCH_PWM` + `RAP` | ca in DB2; RAP e dupa shutdown, PWM-ul la demaraj |
| `NEG_OUT3` | `RSR_UNLOCK` + `RSR_TRUNK` | din seed-ul FRD37EU; numai in modul RSR |
| `NEG_OUT5` | `CLUTCH_SENSOR_B` + `RSR_LOCK` | idem; VW08EU e pe DBALL non-RSR, deci clasele RSR sint inerte |

Regula de citit tabelul: doua clase pe acelasi pin nu e o eroare in acest model - e o
eroare numai daca pot fi active simultan.
## De verificat la banc

- Echivalenta valorii 1 intre `NO_RF` (0x01, `rf.h` DB2) si
  `get_prog_feat_rf_output_value()` din contract - de asta atirna `cfg.is_rsr`, si
  de `is_rsr` atirna daca ambreiajul e activ.
- Renumerotarea contractului pentru HAZARD / PTS / BRAKE (vezi tabelul mai sus).

---

# Portarea pe DS4 (933)

`VEHICLE_SPECIFIC` e **unul singur**, comun DB3 si DS4. Ce difera intre platforme
sta in perechile `lib.config.*` / `lib.fw.*`; restul celor 8 lib-uri de vehicul
(`com.lib.sense`, `com.lib.func`, `com.lib.clutch`, `com.lib.scan*`,
`com.lib.detect`) sint folosite de amindoua, nemodificate.

## lib.config.ds4 + lib.fw.ds4 (01.09.2026)

Create prin masurarea deltei de platforma pe FRD37EU (acelasi vehicul, doua
platforme) si aplicarea ei peste VW08EU/DB3. Delta reala e mica:

| fisier | DB3 vs DS4 |
|---|---|
| config_can, config_dbg, config_obd_can, config_prog, config_pts_ctl, config_status, config_d2d | identice |
| build_identity, config_firmware | difera doar numele/versiunea masinii |
| **config_gpio** | complet diferit - harta de pini a platformei |

Deci continutul de vehicul vine de la DB3 (redenumit, **fara sufixul `_1`**),
`config_gpio.h` de la platforma DS4, plus directorul nou `inc.config_fw`
(933, 933_gpio, 800_gpio, cm800, ngmm_feature, dei_feature, as_feature).

Garzile de include erau deja `__CONFIG_CAN_H__` etc. pe amindoua platformele -
doar numele de FISIER purtau `_1`. Nicio referinta incrucisata de reparat.

`lib.fw.ds4` are aceleasi 6 fisiere ca `lib.fw.db3` plus `fw_private.h`.
Renumire mecanica `db3->ds4`, `DB3->DS4`, `0x28000->0x38000`,
`app_fw_db3_thin.sct -> app_fw_ds4_thin.sct`.

`fw_option.c` din FRD37EU/DS4 **nu s-a portat**: e orfan acolo (nu e in `.uvproj`,
iar `fw_init.c` are propriul `fw_options_set`).

Enum-ul `Feature_Name` (`FEAT_CUSTOM1`, `FEAT_LIGHT_CONTROL`, `FEAT_RF_OUTPUT`,
`FEAT_UNLOCK_DRIVER`, `FEAT_COMFORT_CLOSURE`) e **identic pozitional** pe cele doua
platforme, deci `prog_retrieve_option()` citeste aceleasi sloturi NVFS.

## Iesirile configurabile pe web

Pe DS4 modulul face el pornirea, fara Remote Starter extern. Iesirile de raportare
a starii dispar si devin configurabile de pe web, prin `config_ngmm_feature.h`,
sectiunea `Programmable Inputs/Outputs`.

Fiecare iesire fizica are o pereche: `FEAT_DEI_SYSnn_ENABLE` (0x01 = Visible on
WEB) si `FEAT_DEI_SYSnn` (optiunea implicita). Maparea codurilor, dedusa din
adnotarile Configuration Wizard (`Output1 (-)` confirma ca "Output N" = `NEG_OUT N`):

```
SYS40..SYS53 = Output 1..14     SYS54..SYS56 = Relay 1..3     SYS57+ = intrari
```

Blocate pentru VW08EU (ENABLE = 0x00):

| DEI | iesire | clasa |
|---|---|---|
| SYS40 | Output 1 | LIGHTS (singurul IO din `func_hazard_ctrl.c`) |
| SYS44 | Output 5 | PTS_1 |
| SYS52 | Output 13 | RF_LOOP |
| SYS54 | Relay 1 | BRAKE + CLUTCH |
| SYS55 | Relay 2 | CLUTCH_SENSOR_A |

Restul iesirilor: ENABLE = 0x01. `CLUTCH_PWM` e pe `ISO2_TX`, care nu e iesire
programabila - nu are intrare DEI.

**DE CONFIRMAT:** optiunile implicite ale iesirilor lasate pe web sint toate pe
`0x00` ("Not Configured"). FRD37EU avea acolo cablajul lui (Out3=Lock,
Out5=Unlock, Out9=Accessory, Out10=Ignition, Out11=Starter), care nu se transfera
la alta masina. Daca se vor prealocari, e o linie per iesire.

## Doua capcane gasite si tratate

1. `build_identity.h` exista **dublu** (vehicul + contract), ca pe DB3, si ordinea
   de include decide care cistiga. Cea din contract avea `403.CORE.DS4` -
   platforma gresita (403 = DB3, DS4 = **933**). Copia de vehicul testa inca
   `CORE_DB3_BUILD` dupa copiere. Ambele rescrise identic.
2. `config_dbg.h` din contractul DS4 nu avea deloc `DBG_JTAG`,
   `DEBUG_CAN_ENABLE`, `DEBUG_SKIP_OBD_CAN_INIT`, `DEBUG_SKIP_PGM`. Nedefinit = 0
   in tacere. Adaugat acelasi bloc cu garzi `#ifndef` ca pe DB3.

## Gasit in FRD37EU/DS4, NU reparat acolo

`fw_init.c` de acolo gateaza configul de debug pe `api_table_version >= 12U`, dar
tabela DS4 publica **2**. Garda e mereu falsa, deci
`dbg_config_set_defaults_from_defines()` si `set_use_trace()` nu ruleaza niciodata
pe DS4. E un copy-paste din DB3, unde tabela e la 15. In VW08EU s-a pus `>= 2U`.
FRD37EU e validat pe masina si nu se atinge.

## Ce urmeaza

Faza D: proiectele Keil `core_vw08eu_ds4` si `fw_vw08eu_ds4`. Acolo se compileaza
prima data tot ce s-a scris la fazele B si C. Faza E: livrabilul
`933.vw08eu.1_00_00.enc`.
