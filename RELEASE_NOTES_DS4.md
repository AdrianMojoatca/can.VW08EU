# VW08EU pe DS4 — note de versiune

VW08EU e o **clona a lui VW07EU 1.00** (commit `1fd4ab3` din `can.vw07eu`),
facuta pe 28.09.2026. Codul e identic, cu o singura diferenta de comportament,
descrisa mai jos.

> **Ce urmeaza dupa sectiunea VW08EU e istoricul mostenit de la VW07EU.** E
> pastrat fiindca explica de ce arata codul asa, dar acele teste, build-uri si
> iesiri pe masina s-au facut pe **VW07EU**, nu pe VW08EU. Numele de livrabile de
> acolo sint ale lui VW07EU si au ramas ca atare. Nimic din VW08EU nu a fost inca
> testat pe masina.

---

## v1.00 — VW08EU, primul release (2026-09-29)

**Livrabile: `933.vw08eu.1_00.enc` (591.763 B) si `403.vw08eu.1_00.enc`
(243.058 B).**

**Singura diferenta fata de VW07EU 1.00: in Remote Start, deschiderea usii nu mai
opreste motorul.**

In `fw_init.c`, pe amindoua platformele (fisiere separate - aici nu e cod comun):

```c
tsk_rs_update_cfg(TSK_RS_SKIP_DOOR_SHUTDOWN);
```

adica **a patra** optiune din DB2 (`PROG_TAKEOVER_SKIP_DOOR_SHUTDOWN` = 4).
Blocul inlocuit citea optiunea din `g_prog_config`, populat de un hook `__weak`
care nu se cheama niciodata peste granita CORE / FW - acelasi defect ca la
ambreiaj - deci pe VW07EU rezulta `TAKEOVER_DISABLE` si orice usa deschisa oprea
motorul.

Atentie la semantica: **nu e un takeover cu fereastra mai lunga.**
`SKIP_DOOR_SHUTDOWN` intra negat si in `ALLOWED_BY_OEM()` si in
`ALLOWED_BY_AFTERMARKET()` (`tsk_rs_private.h:70..73`), deci cu el setat toate
cele trei macrouri devin FALSE: nu se mai armeaza nicio fereastra de 45 s, nu mai
e nevoie de unlock in prealabil, iar lock-ul nu mai are ce inchide. Oprirea
motorului la deschiderea usii pur si simplu nu mai exista.

Build-uri de inginerie, inainte de release:

| versiune | ce era |
|---|---|
| 1.00.01 | clona curata, identica functional cu VW07EU |
| 1.00.02 | `TAKEOVER_OEM \| AFTERMARKET` - a treia optiune, fereastra de 45 s |
| 1.00.03 | corectie ceruta de Adrian: a patra optiune, `SKIP_DOOR_SHUTDOWN` |

**Cum s-a verificat:** in dezasamblarea lui `fw_options_set` (`fromelf -c` pe
`.axf`), imediat inainte de apelul indirect prin tabela de API, apare
`MOVS r0,#8` pe amindoua platformele. Dimensiunea functiei din `.map` nu ar fi
spus nimic - se schimba doar o constanta - iar `tsk_rs_update_cfg` nu apare in
`.map`-ul de FW, fiindca sta in CORE. Felia de FW a ramas identica intre 1.00.03
si 1.00 (17396 B pe DS4, 17468 B pe DB3), ceea ce confirma ca la scoaterea
sufixului s-a schimbat doar sirul de versiune.

---

# Istoric mostenit de la VW07EU

Tot ce urmeaza s-a intimplat pe **VW07EU**, intre 04.09 si 25.09.2026.

Portarea firmware-ului **VW07EU** (VW Golf 7) de pe DB2/DBALL pe platforma
**DS4 (933)**, in arhitectura thin-FW cu CORE si FW separate.

Sursa de plecare: VW07EU pe DB3, deja validat la banc. Modelul de platforma:
FRD37EU pe DS4, produs validat pe masina. `DBALL_egal_DB2` a fost folosit numai
ca sursa de inspiratie, niciodata copiat.

---

## v1.00.00 — portare incheiata, validata la banc (2026-09-04)

**Livrabil: `933.vw07eu.1_00_00.enc`** — imagine unica, CORE + FW combinate.
Se produce cu `build_vw08eu_ds4.ps1` de la radacina, intr-un singur pas.

### Ce merge, verificat pe banc

| | stare |
|---|---|
| detectie masina | trece, se aseaza in NVFS, `[CAR_FOUND]` la boot-ul urmator |
| CAN | receptie si transmisie pe canalul vehiculului, 500 kbit/s |
| sense | contact ON/OFF, usi, portbagaj, capota, frina, frina de mina, viteza, RPM |
| alarma OEM | lock / unlock / arm / disarm, de pe magistrala |
| takeover usi | `INPUT_ALL_DOOR_STATUS_OPEN` -> `tsk_rs_tke_door_shutdown` |
| avarii (hazard) | puls 150 ms pe OUT_1, si pe panica si pe RS |
| iesiri programabile | Ground When Arm pe OUT2; ACC / IGN / Starter pe OUT9 / 10 / 11 |
| LED | rosu la detectie, verde la functionare, tranzitie naturala |
| trace | UART de debug functional pe modul nou |

Ramase netestate la banc, din lipsa de echipament: `detect_pgm()` (cere ECU) si
ambreiajul cu actuator real.

---

## v1.00.17 - candidat de masina (2026-09-22)

**Livrabile: `933.vw07eu.1_00_17.enc` si `403.vw07eu.1_00_04.enc`.**
Amindoua validate la banc de tester. Prima iesire pe masina: 23.09.2026.

> **La flash se sterge NVFS-ul.** Valorile `FEAT_DEI_*` sint *default-uri*: se
> scriu numai cind memoria e virgina. Un reflash peste o NVFS scrisa pastreaza
> maparea veche a iesirilor, iar simptomul arata exact ca un bug de firmware.
> Ne-a costat timp de trei ori: la Lock/Unlock, la GWA/GWR si la GWR pe releu.
> `NGMM_FEATURE_VERSION` e fixat de SDK la 23 si NU poate fi urcat ca sa forteze
> rescrierea - exista `#error` pe el.

### Ce s-a reparat de la 1.00.00 incoace

Sase defecte, si merita spus ce au in comun: **niciunul nu a raportat vreo
eroare.** Nici la compilare, nici la rulare, nici in trace. Lantul
CORE - contract - FW nu semnaleaza esecul nicaieri, asa ca singura metoda care a
functionat a fost sa urmarim fiecare semnal pina la `gpio_pl_set()` si sa
verificam ca fiecare veriga *exista*, nu doar ca pare configurata.

**1.00.01 - PTS-ul la oprire, pazit de cine a pornit motorul**

Dupa scoaterea garzii de viteza de pe pulsul de shutdown a aparut un caz nou:
masina pornita normal de sofer, aflata in mers, iar cineva apasa Remote Start -
secventa ajungea tot pe `func_rs_ign_off()` cu contactul sus si **oprea motorul
in mers**. Discriminatorul e `func_rs_start_is_ours`: pe VW08EU motorul porneste
NUMAI daca modulul apasa PTS, deci "contactul era deja sus la IGN_ON" inseamna
fara ambiguitate ca a pornit-o soferul. Cod comun, se aplica pe amindoua
platformele.

**1.00.02 / 1.00.03 - trunk release-ul OEM pe D2D**

Octetul de trunk release nu pleca pe D2D desi se vedea in trace. Cauza nu era in
codul de vehicul: `bus_status_notify_input_keyless_trunk()` avea `d2d_tx()`
comentat in CORE-ul de DB3. DS4 il avea deja corect. Reparat in CORE pe amindoua.
`SET_KEYLESS_DISARM` ramine comentat deliberat, pina se vede pe masina daca BCM-ul
cere si dezarmarea la deschiderea portbagajului.

**1.00.12 - pulsul de Unlock: `prg_out_hw_o5()` era stub gol**

Lock-ul mergea, Unlock-ul nu. Trace-ul arata comanda emisa corect
(`[DEI_OUT_UNLOCK_ALL]`), dar functia de capat nu facea nimic - ramasese
neutralizata de cind OUT_5 apartinea PTS-ului. Lantul iesirilor DEI are **patru**
etaje - default in header, tabela din NVFS, inregistrarea comenzii, functia
hardware - si niciunul nu raporteaza ca urmatorul lipseste.

**1.00.13 - ambreiajul: un hook `__weak` chemat peste granita de imagini**

`OUT_4` nu se activa la STARTER_ON; trace-ul dadea `[CLUTCH_PRESS enter active=0]`.
`cfg.is_rsr` se calcula din `get_prog_feat_rf_output_value()`, valoare scrisa
numai in `core_contract_ds4_prog_config_apply_vehicle_specific()` - un hook
`__weak` declarat in CORE si definit in FW. **CORE si FW sint imagini separate**,
deci hook-ul nu se cheama niciodata: zero referinte in `.map`. `g_prog_config`
ramine pe zero, `is_rsr` iese TRUE si `clutch_disable()` taie ambreiajul in
tacere. Ocolit punctual cu `cfg.is_rsr = FALSE`.

**Cauza generala ramine deschisa: 89 de apeluri `set_prog_*` se pierd la fel, pe
amindoua platformele.** Reparatia cere o campanie de regresie.

**1.00.14 / 1.00.15 - GWA si GWR pe iesiri negative, si optiunea 28**

Amindoua semnalele trebuie sa traga firul la masa, dar stateau pe `POS_OUT6` si
`POS_OUT7`. Mutate pe Output 14 (22-pin/5) si Output 13 (22-pin/10).

Apoi GWR tot nu iesea: pusesem optiunea **27**, "Ground when in ready mode",
fiindca asa se cheama in lista. Numele minte - optiunea 27 se aprinde dintr-un
singur loc, `dei_rs_mts_enable_event()`, adica procedura MTS de rezervare pentru
cutie manuala, la care o pornire normala la distanta nu ajunge niciodata.
GWR-ul real e optiunea **28, "Status"**:

```
INPUT_GWR_ON / INPUT_GWR_OFF          (dei_out_init.c:64-65)
  -> dei_out_ground_when_running_status_on/off
  -> prg_out_ground_when_running_status()
  -> prg_out_execute( 1 + PRG_OUT_STATUS )     = 28 = 0x1C
```

**Regula de calcul, valabila la orice iesire DEI:** numarul optiunii =
`1 + pozitia in enumul Prg_Out_Type` din `prg_out.h`. Enumul are intrari
**comentate afara**, deci nu se numara pe sarite - se citeste efectiv. Verificat
pe valori stiute bune: Lock 4, Unlock 6, Ground When Arm 2, ACC/IGN/START
16/17/18. Numele din techsheet, "GWR (Status)", spunea de fapt exact asta.

RF Loop-ul a fost mutat de pe `NEG_OUT13` inapoi pe `RF_LOOP_POWER`, ca pe DB3,
ca sa elibereze pinul: maparea veche venea din `ds4_contract`-ul lui FRD37EU si
era artefact de portare - pe 933 SDK-ul comanda deja bucla interna singur, din
`rf_loop_init()` pe `INPUT_GWR_ON/OFF`.

**1.00.16 - ISO2, PWM-ul de ambreiaj, nu iesea pe DS4 desi pe DB3 mergea**

`GPIO_IFACE_OUTPUT_ISO2_TX` si `ISO1_TX` fusesera adaugate **la coada enumului**
din contract, dar cele sase `case`-uri din cele trei switch-uri
(`_on` / `_off` / `_toggle`) nu au fost adaugate niciodata. Scrierea cadea pe
`default: break;` - fara eroare si fara warning, fiindca `case NONE: default:`
acopera orice valoare noua din enum. Pe DB3 cazurile existau din v2.1 / v4.1, de
aici asimetria intre platforme.

De atunci exista o **verificare automata**: extrage numele selectorilor din
`typedef enum` si le compara cu cazurile din **fiecare** dintre cele trei
switch-uri. Se ruleaza la orice atingere a contractului. Stare la 22.09.2026:
DS4 18 selectori, DB3 17, zero lipsuri.

**1.00.17 - alocarea releelor**

Regula stabilita de Adrian, valabila de acum la toate firmware-urile:

| releu | DB3 (403) | DS4 (933) |
|---|---|---|
| RELAY1 | **aceeasi functie pe amindoua** - actuatie de frina / ambreiaj | identic |
| RELAY2 | senzor de ambreiaj A | **GWR ON** |
| RELAY3 | nu exista - DB3 are doua relee | preia **ce face RELAY2 pe DB3** |

Ratiunea: pe DB3 releul de GWR il pune remote starter-ul **extern**, deci
firmware-ul de acolo nu are ce sa comande si RELAY2 ramine pe treaba lui. Pe DS4
modulul e si starter, deci GWR-ul trebuie sa iasa din el, iar RELAY3 - care
exista numai pe DS4 - absoarbe functia deplasata. RELAY1 ramine ancora comuna si
nu se rupe nimic la portarile viitoare.

Implementare: GWR pe releu prin DEI (`FEAT_DEI_SYS55 = 0x1C`); senzorul A prin
contract (`GPIO_IFACE_OUTPUT_RELAY3`, adaugat la coada enumului plus cele trei
cazuri). `FEAT_DEI_SYS56_ENABLE` (Relay 3) trecut pe `0x00`: releul e acum
detinut de contract si nu are voie sa fie configurabil si de pe web, altfel
ajunge cu doi stapini pe P1.16 - exact ce s-a intimplat pe Output 13 cu RF Loop.

**Exceptie asumata:** senzorul A de ambreiaj e atins de `clutch_press()`, chemat
din `func_rs_str_on()`, deci cade sub regula de paritate IO pentru `func_rs.c`.
Totusi difera acum intre platforme. E ceruta explicit - a nu se "repara" inapoi
la un audit viitor.

### Iesirile pe DS4, starea finala

| semnal | unde e definit | pin |
|---|---|---|
| Hazard | contract, `NEG_OUT1` | 22-pin/1 Light Green/Black |
| PTS | contract, `NEG_OUT2` | 22-pin/2 Blue/White |
| Lock | DEI `SYS42 = 0x04` | 22-pin/3 Green/Black |
| Ambreiaj, senzor B | contract, `NEG_OUT4` | 22-pin/4 Red/White |
| GWA | DEI `SYS53 = 0x02` | 22-pin/5 Brown/Black |
| Unlock | DEI `SYS44 = 0x06` | 22-pin/9 Blue/Black |
| GWR, fir la masa | DEI `SYS52 = 0x1C` | 22-pin/10 Light Blue/Black |
| GWR, contact uscat | DEI `SYS55 = 0x1C` | 12-pin 7/8, Relay2 |
| Ambreiaj, senzor A | contract, `RELAY3` | 12-pin 4/5 |
| Frina / ambreiaj, actuatie | contract, `RELAY1` | 12-pin 2/3 |
| Ambreiaj, PWM | contract, `ISO2_TX` | 8-pin/7 Orange/Black |
| Accessory / Ignition / Starter | DEI `SYS48/49/50` | 10-pin 8/10/7 |

### Ramas deschis la predarea pe masina

- ~~DB3 are inca defectul de ambreiaj~~ - **reparat 22.09.2026 in 403 1.00.05**,
  oglinda reparatiei de pe DS4. Dovada in `.map` dupa build: `clutch_disable` are
  acum **zero** referinte, ramura fiind eliminata de compilator, iar
  `clutch_enable` si `clutch_press` sint linkate.
- **Cele 89 de apeluri `set_prog_*`** care se pierd pe amindoua platformele.
- **`detect_pgm()`** ramine comentat pina la OK-ul explicit al lui Adrian.
- **Contactele lui RELAY3 pe 933**: foaia `Install 933` numeste pinul 4
  "Relay3 N.C." (White/Brown), dar `Module Connection Reference` asociaza culoarea
  White/Brown cu "NO RELAY3". Aceeasi contradictie e si in techsheet-ul FRD37EU,
  deci vine din sablonul DEI. De confirmat pe schema; **firmware-ul comanda doar
  bobina**, deci raspunsul schimba numai techsheet-ul, nu si codul.
- **Ghidul de instalare**: `Wire Location` inca `_TBD_` - coloana nu exista in
  techsheet, vine din schema de cablare a masinii. Foaia `Features` are inca
  bifele mostenite din sablonul FRD37EU.
- **Comutatoare de banc** de intors inainte de productie: `DEBUG_SKIP_PGM`,
  `DEBUG_FORCE_DBG_UART`, blocurile VIN/PGM, trace-ul de RX si trace-urile de
  diagnostic din `clutch_press`, lasate pornite deliberat pentru testare.

---

## v1.00.18 (933) si v1.00.06 (403) - VALIDATE PE MASINA (2026-09-24)

**Livrabile: `933.vw07eu.1_00_18.enc` (591.787 B) si `403.vw07eu.1_00_06.enc`
(242.988 B).** Testate pe masina de tester pe 23-24.09.2026, amindoua OK.
Acesta e primul release al VW07EU validat altfel decit la banc.

### detect_pgm() activat

Pina aici `detect_pgm()` intorcea `TRUE` fara sa scaneze. Activarea a cerut
**doua** schimbari, nu una - a doua era usor de ratat:

1. `DEBUG_SKIP_PGM` de la `1` la `0`, ceea ce scoate `return TRUE` din
   `detect_pgm.c`;
2. blocul VIN + PGM din `detect_car_search.c`, care era **comentat in bloc**,
   inclusiv apelul `if( !detect_pgm() )`.

`detect_car_search.c` e **comun DB3 si DS4**, deci decomentarea pur si simplu ar
fi legat cele doua platforme intre ele. Blocul a fost pus sub
`#if DEBUG_SKIP_PGM == 0 / #else / #endif`, cu `car.bits.car = GOLF7` mutat pe
ramura de banc. Peste el, o garda:

```c
#if !defined(DEBUG_SKIP_PGM)
   #error "DEBUG_SKIP_PGM nu e vizibil in detect_car_search.c - vezi config_dbg.h"
#endif
```

fiindca un macro **nedefinit** ar face `#if X == 0` sa fie adevarat si ar activa
blocul in tacere pe amindoua platformele - exact clasa de bug care ne-a mai
costat o zi pe 27.08.

### Capcana celor doua `config_dbg.h`, pe DB3

Pe DS4 e o singura copie de `config_dbg.h`. **Pe DB3 sint doua in acelasi build
de FW**, cu aceeasi garda `__CONFIG_DBG_H__`, deci intra doar una per unitate de
compilare. Verificat in `.dep` pe 23.09: pentru `detect_car_search.c`,
`detect_pgm.c` si `fw_init.c` intra **prima cea din contract**, nu cea din
`VEHICLE_SPECIFIC` - un `#include "config_dbg.h"` cu ghilimele cauta intii in
directorul fisierului care include, iar lantul trece prin
`core_contract_db3_dbg_config.h`. Valoarea a fost deci schimbata in **amindoua**.

Daca s-ar fi schimbat doar in `VEHICLE_SPECIFIC`, buildul ar fi trecut curat si
PGM-ul ar fi ramas mort.

### Ce trebuie cablat ca PGM-ul sa aiba cu cine vorbi

Scanarea **nu** merge pe canalul vehiculului. `vw06_scan_NEW_pgm2()` isi ignora
parametrul si forteaza `pgm_data->dev_id = CAN_DEV_0`
(`vw06_scan_pgm2_NEW.c:104`, comentariul "use FT for scan", identic cu DB2).
`CAN_DEV_0` e perechea **portocalie**, si merge la priza de diagnoza:

| platforma | Orange/Green = CAN High | Orange/Brown = CAN Low |
|---|---|---|
| DB3 / 403 | H1/5 -> OBD II pin 6 | H1/6 -> OBD II pin 14 |
| DS4 / 933 | DATA/4 -> OBD II pin 6 | DATA/3 -> OBD II pin 14 |

Dovada nu e deductie: ghidul de instalare DB2
(`dball2-vw07_en_ig_ac20220627.pdf`) deseneaza doua perechi CAN distincte -
Tan / Tan-Black catre priza BCM, Orange/Green si Orange/Brown catre
"Diagnostic connector OBD II".

**In techsheet perechea portocalie e inca `No Connection` pe amindoua
platformele**, deci instalatorul nu o leaga de la sine. De corectat in techsheet.

Esecul de PGM **nu opreste detectia**: aprinde LED-ul de eroare 10 s, scrie
`[ DETECT : PGM_ERROR]` in trace si `PGMERR` pe D2D. Daca se intimpla cu perechea
legata, primul lucru de incercat e `DEBUG_SKIP_OBD_CAN_INIT` pe `0` (adauga
`wakeup_obd_can()` de cinci ori); acum e `1`, ca in DB2 vers. 1_03, unde PGM-ul
mergea pe masina.

### Verificari de build, ca reper pentru data viitoare

| | fara PGM | cu PGM |
|---|---|---|
| felia FW, ambele platforme | 11.840 B | **17.532 B** |
| warning `DETECT_PGM  DISABLED` | prezent | absent |
| `vw06_scan_NEW_pgm2`, `pgm_callback`, `detect_vin_search` in `.map` | absente | prezente |

Feliile FW sint identice ca dimensiune pe cele doua platforme, fiindca sint
acelasi cod din `VEHICLE_SPECIFIC`.

> **`detect_vin_search()` e un stub** (`detect_vin_search.c:32-37`): nu citeste
> VIN-ul, doar pune `car.bits.car = GOLF7` si intoarce `TRUE`. Deci
> `[ DETECT : VIN_FOUND]` in trace nu dovedeste nimic despre VIN.

---
## v1.00 VW07EU — RELEASE (2026-09-25)

**Livrabile: `933.vw07eu.1_00.enc` (591.985 B) si `403.vw07eu.1_00.enc`
(243.162 B).** Primul release de livrare al VW07EU.

Versiunea de livrare e **`1.00`, fara sufix**. Sufixele `1.00.01` ... `1.00.21`
au fost build-uri de inginerie, pentru banc si pentru masina; ele nu ajung pe
site. Numele `.enc` se deriva din `build_identity.h` cu punctele inlocuite de
underscore, deci `1.00` -> `933.vw07eu.1_00.enc`, aceeasi forma ca a ghidurilor
de instalare.

### Telecomanda OEM in Remote Start

Cu motorul pornit de la distanta, apasarea pe Lock sau Unlock de pe cheia masinii
nu misca iesirile modulului. BCM-ul nu raspunde la telecomanda cit timp motorul
merge, deci modulul trebuie sa actioneze in locul lui - si nu o facea.

Cauza, in `dei_out_init.c`: `INPUT_OEM_LOCK` si `INPUT_OEM_UNLOCK` sint
inregistrate **doar** pe `dei_out_ground_when_locked` si `dei_out_gwa_*`, in timp
ce pulsurile de Lock si Unlock atirna de `INPUT_LOCK1_ON` si
`INPUT_UNLOCK_ALL1_ON`. Calea OEM nu emitea niciodata acele comenzi - nici in
`dei_system_oem_lock()` / `dei_system_oem_unlock()`, nici in `dei_ss_*`, care
schimba numai starea. Telecomanda aftermarket le emite, in
`dei_system_rf_lock()` / `dei_system_rf_unlock()`; de acolo am luat modelul.

Reparat prin emiterea comenzii si pe calea OEM, cu acelasi payload, si **numai in
Remote Start** (`get_rs_gwr()`): in rest masina se incuie singura la apasarea
telecomenzii - de altfel exact de acolo aflam noi ca s-a incuiat, din cadrul de
pe magistrala - deci un puls in plus ar fi o a doua actionare.

> **Pista falsa, de notat pentru viitor.** In `dei_out_lock()` exista
> `if( DEI_OUT_PAYLOAD( payload ) & CMD_LOCK_OEM ) return ;`, care pare exact
> explicatia. `CMD_LOCK_OEM` **nu e atribuit nicaieri** in arbore: e definit in
> `cmd.h` si testat doar acolo. E cod mort. Lectia: cind o conditie pare sa
> explice un simptom, verifica intii daca poate fi vreodata adevarata.

### Cadrele CAN parazite, scoase de pe DS4

Cu contactul pornit - adica in Remote Start - `func_lock_start()` transmitea doua
cadre proprii cu `can_usr_tx()`:

```
id 0x000   00 00 00 00 00 00 00 00      trezirea magistralei
id 0x3D0   40 00 00 C0 80 00 00 00      comanda de incuiere
```

iar `func_unlock_all_start()` pornea, prin `func_can_wake_enter()`, un
`can_per_loop` care trimitea **periodic** acelasi `0x000`, si in plus
retransmitea `0x3D0` cu `data[0] |= 0xA0`, de trei ori.

### Regula care a iesit din asta: `VEHICLE_SPECIFIC` e comun

`func_oem.c` e in `VEHICLE_SPECIFIC`, deci **scoaterea apelurilor a intrat automat
si in DB3**. Pe DB3 insa modulul **nu are iesiri de lock/unlock** - are `H3/1
Lock Input` si `H3/2 Unlock Input`, adica primeste comanda de la remote starter-ul
extern si o executa **pe magistrala**. Acolo cadrele CAN nu sint reminiscenta,
sint chiar mecanismul.

Prins comparind dimensiunea feliei de FW: DB3 scazuse de la 17.532 la 17.468, la
fel ca DS4. Scoaterea e acum pazita de `#ifndef PLATFORM_933`, iar DB3 a revenit
la 17.532 - byte cu byte cit imaginea validata pe masina.

**De retinut la fiecare reparatie de acum inainte:** o modificare in
`VEHICLE_SPECIFIC` atinge amindoua platformele. Daca e specifica uneia, trebuie
pazita explicit, si verificata prin dimensiunea feliei de FW a celeilalte.

### Flex Relay in tandem cu Ignition

`Flex Relay Output` (Pink/White, 10-pin/4) = **Output 8**; `Ignition Output`
(Pink, 10-pin/10) = **Output 10**, optiunea DEI 17. Output 8 a fost pus pe
aceeasi optiune:

```c
#define FEAT_DEI_SYS47_ENABLE   0x00     /* ascuns de pe web */
#define FEAT_DEI_SYS47          0x11     /* Ignition, ca Output 10 */
```

Fara cod: `reg_func()` **adauga** in bucketul de id, nu inlocuieste, deci doua
iesiri pot imparti aceeasi functie si se aprind amindoua.

### Cum s-a verificat, fara acces la debug

Adrian nu avea trace disponibil, deci fiecare modificare a fost dovedita din
artefactele de build, cu baseline **masurat**, nu presupus:

| ce | fara | cu |
|---|---|---|
| `i.dei_system_oem_lock` | 0x7E = 126 B | 0xAE = 174 B |
| `i.dei_system_oem_unlock` | 0xC4 = 196 B | 0xF0 = 240 B |
| felia FW DS4, dupa scoaterea celor doua cai CAN | 17.532 B | 17.468 B |
| felia FW DB3, dupa paza `PLATFORM_933` | 17.468 B | **17.532 B** (revenit) |

Schimbarea de **date** nu se vede in dimensiuni, asa ca a fost cautata direct in
imagine: tiparul default-urilor DEI pentru iesiri apare o singura data in
`combined_core_fw_ds4.hex`, la `0x30F95`:

```
00 00 04 00 06 00 00 11 10 11 12 00 1C 02 00 1C 00
                     ^^ SYS47 = 0x11 la 0x30F9C, intre SYS46=0x00 si SYS48=0x10,
                        alaturi de SYS49=0x11 care e Ignition
```

Metoda constanta a proiectului: **nu te increde in ce compileaza, verifica ce
ajunge in imagine.** Aici a si prins o regresie reala (DB3).

---
## Arhitectura

Aceeasi ca pe DB3, cu adresele DS4:

```
CORE   0x08000 .. 0x372B3      ABI info  0x37000      tabela API  0x37040 (4032 B)
FW     0x38000 .. 0x3ADBF      descriptor FW la 0x38000
```

Trei componente, toate in `VW08EU/Portate`, plat, simetric cu DB3:

- **`vw08eu_ds4_contract`** — contractul CORE<->FW. Junction NTFS din ambele
  proiecte, deci o singura sursa. Note proprii in `RELEASE_NOTES.md`.
- **`VEHICLE_SPECIFIC`** — **comun DB3 si DS4**. Codul de vehicul se scrie o
  singura data; ce difera intre platforme sta in `lib.config.*` si `lib.fw.*`.
- **`core_vw08eu_ds4` / `fw_vw08eu_ds4`** — proiectele Keil.

Consecinta care se uita usor: **orice garda de platforma din `VEHICLE_SPECIFIC`
trebuie sa acopere ambele build-uri**, nu doar unul.

---

## Fazele portarii

| faza | continut | commit |
|---|---|---|
| A | delta masurata DS4 vs DB3 | — |
| B | contract v3.0, 34 de sloturi noi, append-only | `dc20038` |
| C | `lib.config.ds4` + `lib.fw.ds4` | `ca7e816` |
| D | proiectele Keil, ambele construiesc | `d28ca89` |
| E | lantul de build si livrabilul | `14774ed` |
| bring-up | CAN, sense, hazard, iesiri | `14774ed`, `ab681cb`, acesta |

Metoda la Faza C merita retinuta: **delta de platforma = FRD37EU/DS4 minus
FRD37EU/DB3**, adica acelasi vehicul pe doua platforme; apoi acea delta aplicata
peste VW07EU/DB3. Asa nu se amesteca "ce tine de platforma" cu "ce tine de
masina". Rezultatul a fost curat: sapte fisiere de config identice, doua care
difera doar prin nume, si un singur fisier complet diferit (`config_gpio`).

---

## Bring-up: ce a costat timp si de ce

Patru clase de probleme, toate cu acelasi tipar — **esec tacut**. Niciuna nu a
dat eroare de compilare, warning sau mesaj pe seriala.

### 1. Trace mut — trei cauze independente, in aceeasi zi

- **Portul de debug inchis din NVFS.** Pe DS4, UART-ul e disponibil numai daca
  portul UNDERDASH are rolul 0 in `config_port`. Pe un modul nou intrarea nu
  exista, iar `debug_port_avail()` o creeaza cu underdash = 2. Comutator nou:
  `DEBUG_FORCE_DBG_UART`.
- **Umplerea sectorului 20.** Imaginea de CORE trebuie umpluta cu `0xFF` de la
  `0x8100` la `0x38000` inainte de criptare. ABI info si tabela de API sint
  plasate cu `__at` in acelasi sector de 32 KB ca sfirsitul codului, dar separate
  de o gaura de ~8 KB; a doua rafala necontigua face flasher-ul din bootloader sa
  abandoneze pe la 90%. Modulul ramine cu imagine incompleta, iar flasher-ul
  poate raporta succes. **Nu exista pe DB3.**
- **Praguri de versiune mostenite.** Contractul DS4 a fost derivat din DB3 si a
  mostenit `api_table_version >= 12` in doua locuri; tabela DS4 publica 3. Cit
  timp amindoua erau gresite, sistemul mergea din intimplare. **Corectarea doar a
  unuia a facut bug-ul accesibil** — dereferentiere de NULL in `fw_identity_init`,
  care ruleaza inaintea lui `dbg_init`, deci moarte fara niciun caracter pe fir.

**Lectie:** un prag corectat pe jumatate e mai rau decit unul gresit peste tot.
Cauta toate aparitiile deodata.

### 2. CAN

- **Viteza CAN1 la 125K** in loc de 500K, din seed-ul FRD37EU — o saptamina
  pierduta, fiindca prima corectie a fost facuta in locul gresit. Vezi sectiunea
  dedicata mai jos.
- **Canalul vehiculului**: ramine `CAN_DEV_1` (tan / tan-black) pe amindoua
  platformele. Comutatorul temporar `VEHICLE_ON_CAN_DEV_0` a fost scos pe
  08.09.2026 — vezi "Inchis 08.09.2026: canalul vehiculului".
- **Canal hardcodat peste parametru, sase locuri.** `sense_init` inregistra toate
  cele 24 de handlere pe `CAN_DEV_1` hardcodat, ocolind parametrul primit — pe
  DB2/DB3 valorile coincideau, deci nu se vedea. Pe DS4: **sense complet mut**,
  desi detectia trecea si promptul `dbg>` aparea.

**Capcana de diagnostic:** pe LPC176x enum-ul `Can_Dev_Id` e **permutat**
(`CAN_DEV_1 == 0`). Un trace care tipareste `dev_id` cu `%d` arata invers si
trimite pe piste false. Tipareste numele, nu numarul.

### 3. Hazard iesea pe OUT_2

Contractul spunea corect `NEG_OUT1`, `CORE_DS4_GPIO_MAP_ASSERT` trecea, tot
lantul era prezent si linkabil — **dar nu era consultat**. `core_framework.h`
include `config_gpio.h`, iar in `func_private.h` macrourile de iesire sint sub
`#ifndef`: cine ajunge primul cistiga. Pe DB3 fisierul se numeste
`config_gpio_1.h` si nimeni nu cere acel nume, deci seed-ul e cod mort. Pe DS4 se
numeste exact `config_gpio.h`. **Diferenta e de nume de fisier, nu de platforma.**

Efect secundar mai grav decit pinul gresit: `FUNC_PTS_OUT` nimerea acelasi pin,
dar pierdea garda de viteza — versiunea de contract e invelita in
`tsk_rs_invalid_state()`, cea din seed nu.

**Lectie:** un `MAP_ASSERT` verifica valoarea din contract, nu daca contractul e
chemat. Cind o iesire merge pe alt pin decit spune contractul, cauta cine mai
defineste macroul.

### 4. Configul de iesiri nu se aplica

`FEAT_DEI_SYSnn` din `config_ngmm_feature.h` **sint default-uri, nu valori**. Se
scriu in NVFS doar cind intrarea `nvfs_feat_dei` lipseste; pe un modul care a mai
pornit, tabela stocata cistiga, iar reflash-ul nu atinge NVFS-ul.

Incrementarea `NGMM_FEATURE_VERSION` pare pirghia potrivita — chiar sterge tabela
— dar e versiunea de **layout** a listei, fixata de SDK cu `#error`.

**Solutia e la banc:** sterge NVFS-ul, din butonul modulului sau din tool-ul de
NVFS. **Reflex:** cind o schimbare de config nu se vede, citeste intii intrarea
din NVFS. Daca difera de header, nu cauta bug-ul in cod.

---

## Diferente DS4 fata de DB3 care conteaza

| | DB3 (403) | DS4 (933) |
|---|---|---|
| Remote Starter | extern, pe D2D | **intern** — modulul porneste el |
| producator `INPUT_PARKING_ON` | `lib.rsr`, `lib.w2w` (sub `features/dball3/`) | alt mecanism — vezi nota |
| `NEG_OUT1` | P1.23 | **P1.22** (FAA OUTPUT) |
| umplere inainte de criptare | nu | **obligatorie** |
| fisier de config gpio | `config_gpio_1.h` (neinclus) | `config_gpio.h` (**inclus**) |
| iesiri programabile pe web | — | 14 iesiri + 3 relee, prin `FEAT_DEI` |

---

## Iesiri, stare curenta

Blocate de firmware (`_ENABLE 0x00`, nereconfigurabile de pe web):

```
OUT_1   hazard (clasa lights)        OUT_5    PTS_1
OUT_13  RF loop                      RELAY1   frina + ambreiaj
RELAY2  senzor ambreiaj A            OUT_2    Ground When Arm
OUT_9   Accessory                    OUT_10   Ignition        OUT_11  Starter
```

`Ground When Arm` a cerut si o modificare in CORE: functia generica citea
`get_ss_arm()` — starea sistemului de securitate DEI — si era inregistrata numai
pe `INPUT_ARM` / `INPUT_DISARM`, comenzi pe care VW08EU nu le emite niciodata.
Armarea vine de la alarma din fabrica, pe CAN, care emite `INPUT_OEM_ARM` si
seteaza `alarm.bit.armed`, dar nu cheama `set_ss_arm()`. Adaugate doua handlere
care comanda iesirea explicit, legate de cele patru comenzi OEM.

---

## TEMPORAR — inca active

| ce | unde |
|---|---|
| `DEBUG_FORCE_DBG_UART = 1` | ambele `config_dbg.h` |
| trace de RX pe fiecare cadru | `can_usr_rx_tsk.c` |
| trace-uri de diagnostic in `clutch_press` | `com.lib.clutch` |

**Inchis 23.09.2026:** `DEBUG_SKIP_PGM` e acum `0` pe amindoua platformele, iar
blocurile VIN si PGM nu mai sint comentate, ci guvernate de acelasi comutator,
cu `car.bits.car = GOLF7` pe ramura `#else`. Pe DB3 valoarea trebuie schimbata
in **amindoua** fisierele `config_dbg.h`. Vezi sectiunea v1.00.18 / v1.00.06.

**Scos 08.09.2026:** sonda `INPUT_PARKING_ON` din `func_cmd_oem_lock`. Se banuia
ca pe DS4 nu exista niciun producator al comenzii, deci ca sonda nu poate fi
scoasa fara inlocuitor. **Banuiala era gresita** — dupa scoatere, hazardul merge
in continuare pe panica si pe RS, verificat la banc. Vezi nota de mai jos.

## Nota: declansatorii de hazard pe DS4

Pe DB3, `INPUT_PARKING_ON` e produs de `lib.rsr/rsr_parking.c` si `lib.w2w`, sub
`APP/features/dball3/` — folder care **nu exista** pe DS4. Cautarea in surse a
gasit pe DS4 un singur producator, `func_set_headlights()`, chemat exclusiv din
tabela de comenzi M2M (`M2M_ID_HEADLIGHTS`).

De aici concluzia — **gresita** — ca sonda de banc nu poate fi scoasa fara un
inlocuitor. La banc, dupa scoaterea ei, hazardul merge in continuare pe panica si
pe RS. Deci exista pe DS4 cel putin un declansator real pe care cautarea in surse
nu l-a prins; ruta lui nu a fost localizata si nu conteaza practic, fiindca
functioneaza.

**Lectia:** absenta unui apelant vizibil intr-un grep nu dovedeste absenta caii,
mai ales cind lantul trece prin tabele de comenzi indexate si prin cod DEI. Testul
la banc bate deductia din surse.

## Decizii deschise

1. **Polaritatea OUT_8..OUT_12** — singura asumptie de mapare nevalidata, si acum
   conteaza: iesirile chiar comanda.
2. **`CLUTCH_SENSOR_B` nu are pin.** Pe DB3 e `NEG_OUT5`, ocupat pe DS4 de
   `PTS_1`. Lasat `NONE`; slotul exista, apelul e no-op.
3. **Output 12 (SYS51)** are in referinta `0x1E` = 30, dar optiunea marcata e
   `<31> AG Starter-kill`. Inconsecventa in referinta insasi, neatinsa.

### Inchis 08.09.2026: canalul vehiculului

Magistrala sta pe **`CAN_DEV_1`, Tan / Tan-Black**, pe amindoua platformele — ca
in DB2 si pe DB3. Comutatorul `VEHICLE_ON_CAN_DEV_0` a fost **scos**.

Intre 02 si 08.09.2026 pe DS4 canalul era mutat pe `CAN_DEV_0`, fiindca pe
`CAN_DEV_1` nu se receptiona nimic. Am pus asta pe seama cablajului placii de
test, apoi a transceiverului. **Amindoua erau gresite** — cauza reala era viteza,
vezi sectiunea urmatoare. Confirmat de hardware: pe 933,
`GPIO_PL_EXT_CAN_HS_RX/TX` sint P0.0 / P0.1, adica CAN1, adica tan. Maparea
culoare-canal era corecta tot timpul.

Rezolva si coliziunea semnalata atunci: `CAN_DEV_OBD` redevine un dev distinct de
canalul vehiculului, deci `wakeup_obd_can` are pe ce lucra pe masina. Iar
`vw06_scan_pgm2_NEW.c`, care forteaza `dev_id = CAN_DEV_0` pentru scan, ajunge
din nou pe canalul FT — identic cu DB3, unde comportamentul e dovedit.

**Ce a ramas din episod, si merita pastrat:** indirectarile `CAN_DEV_VEHICLE` /
`VEHICLE_CAN_DETECTED`. Inainte, sase locuri scriau canalul direct, ocolind
aliasul primit ca parametru. Pe DB2 si DB3 valorile coincideau, deci nu se vedea
nimic — dar orice mutare de canal le lasa in urma, in tacere.

**Lectia:** trei dovezi software consistente pot descrie perfect o cauza gresita.

### Inchis 09.09.2026: de ce nu mergea CAN1 pe DS4

**`CAN_DEV_1_SPEED = 0` (125K) in
`VEHICLE_SPECIFIC/lib.config.ds4/com.inc.config.fw/config_can.h`.**

Valoarea vine din seed-ul FRD37EU, care foloseste `CAN_DEV_0` si deci n-a avut
niciodata motiv s-o corecteze. Golf 7 are magistrala pe CAN1 la 500K.

Corectia din 02.09.2026 fusese facuta in `CORE_CAN_DEV_1_SPEED_DEFAULT`, in
contract — **si e inerta pe DS4**:

| header SDK | pe DB3 include | pe DS4 include |
|---|---|---|
| `can_usr_private.h:25` | `core_contract_db3_can_config.h` | **`config_can.h`** |

Ladder-ul care alege BTR/TSEG/SJW e un `#if CAN_DEV_1_SPEED == n` rezolvat la
compilare in `can_usr_private.h`, iar acel TU se compileaza in CORE si nu vede
niciodata wrapper-ul de contract. De aici si raspunsul la "de ce merge pe DB3 si
nu pe DS4 cu acelasi cod de vehicul": diferenta nu e in platforma, e in ce include
SDK-ul.

Acelasi tipar de umbrire ca la `config_gpio.h` (sectiunea "Hazard iesea pe OUT_2").
**Al doilea incident, deci e o clasa de probleme, nu un accident.** Triaj rapid la
orice portare pe DS4: `find . -name "config_*.h"` — fiecare fisier gasit e un
candidat la umbrire, verifica-i valorile fata de contract.

**Semnatura de diagnostic, de retinut:** `CAN_DEV_0` intra in bus-off la
transmisie, `CAN_DEV_1` nu — desi amindoua transmisesera 10 cadre. Un canal
configurat corect care transmite fara ACK *trebuie* sa ajunga in bus-off. Cind nu
ajunge, nu transmite deloc: la bit-time de patru ori mai lung nu gaseste niciodata
magistrala libera. **Absenta bus-off-ului inseamna viteza gresita, nu canal mort.**

**Ce a costat o saptamina:** viteza fusese "verificata" — dar in fisierul care nu
conteaza pe DS4. Cind un fix nu are efect, verifica intii ca fixul ajunge unde
trebuie, inainte sa pui la indoiala diagnosticul.

## Curatenie amanata

- SDK-ul de FW nu e degresat: 2333 de fisiere, fata de 83 pe DB3. De redus cu
  ajutorul `.dep`-ului dupa ce se stabilizeaza.
- Include path mort mostenit din referinta DS4:
  `external\can.sdk.ds4\INC\cmall\core\inc.config.d2d`.
