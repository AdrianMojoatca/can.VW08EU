# vw08eu_ds4_contract — note de versiune

Contractul CORE<->FW pentru **VW08EU pe platforma DS4** (933).

Copie de lucru, independenta, pornita din `ds4_contract` (contractul comun al
DS4, folosit de FRD37EU). Sursa comuna **`can.ds4_contract` nu se atinge** —
orice schimbare ceruta de VW08EU se face numai aici.

---

## v6 — paritate de IO cu DB3 pentru func_rs.c (2026-09-14)

**Regula stabilita de Adrian:** `func_rs.c` e fisier comun DB3 + DS4, deci
**IO-urile pe care le atinge trebuie sa aiba aceeasi functie pe amindoua
modulele**. Aplicata ca filtru, a scos la iveala trei nepotriviri, nu una.

| clasa | DB3 | DS4 inainte | DS4 acum |
|---|---|---|---|
| `PTS_1` | NEG_OUT2 | NEG_OUT5 | **NEG_OUT2** |
| `CLUTCH_SENSOR_B` | NEG_OUT5 | **NONE** | **NEG_OUT5** |
| `RF_LOOP` | RF_LOOP_POWER | NEG_OUT13 | neschimbat, vezi mai jos |

`PTS_1` pe NEG_OUT5 era mostenit din seed-ul FRD37EU si nu fusese comparat
niciodata cu referinta: DB2 are `VW03_FUNC_PTS_OUT(x) = SPEED_PROT_NORMAL(OUT_2, x)`
(`config_gpio.h:40`), iar DB3 respecta.

`CLUTCH_SENSOR_B` **lipsea de tot** pe DS4. In DB2 semnalul de senzor merge
simultan pe doua iesiri - `VW03_CLUTCH_SENSOR(x) = OUT_RELAY2(x);OUT_5(x)` - deci
pe DS4 se comanda doar RELAY2 si **simularea apasarii ambreiajului era pe
jumatate**. Senzorul e necesar ca modulul sa "spuna" masinii ca ambreiajul a fost
apasat, desi fizic nu a fost.

**Ciocnirea si cum s-a rezolvat.** Cererea initiala era Ground When Arm pe OUT_5,
dar acolo trebuia sa ajunga senzorul B. Nu pot coexista: modulul e armat exact in
timpul pornirii de la distanta, deci GWA ar tine iesirea la masa fix cind
simularea de ambreiaj vrea s-o comute. Decizia lui Adrian: **ambreiajul primeaza**
- el intra sub regula de mai sus, pe cind Ground When Arm e functie programabila,
deci mutabila fara sa atingi contractul.

Ground When Arm a trecut de pe **Output 2** pe **Output 6** (`FEAT_DEI_SYS41` ->
`FEAT_DEI_SYS45`). Output 6 e prima iesire libera pe **amindoua** modulele - pe
DB3, 1..5 sint lights, PTS, doors, trunk si clutch sensor B - deci alegerea nu
recreeaza ciocnirea mai tirziu.

Vizibilitatea pe web ramine coerenta: iesirile cu functie fixa de contract
(1, 2, 5) au `ENABLE 0x00`, cele libere (3, 4) ramin programabile, iar 6 e fixa.

**ATENTIE la banc:** `FEAT_DEI_*` sint **default-uri de NVFS**, se scriu doar cind
NVFS e virgin. Mutarea GWA de pe out2 pe out6 **nu se vede** fara stergere de
NVFS. Reperul din trace la pornire: `[PRGMAP] out2 id=0` si `[PRGMAP] out6 id=2`.
Schimbarile de contract (PTS, senzor B) sint compile-time si se aplica imediat.

**RF_LOOP: amanat deliberat.** Pe DS4 ar trebui sa fie optiunea DEI 32,
"ALT RF Loop for FOB", nu `NEG_OUT13` fix. Dar optiunea e comentata in listele de
web si nu i-am gasit handler; singurul cod care atinge RF loop-ul e
`dei_out_ground_when_running_status.c`, prin macroul fix `OUT_RF_LOOP`. Pus pe
`NONE` acum, riscul e sa ramina fara RF loop. De verificat separat unde ajunge de
fapt optiunea 32.

## v5 — statusul de frina pe DAC_RELAY, separat de actuatie (2026-09-11)

Oglinda fixului de pe DB3 - vezi `vw08eu_db3_contract/RELEASE_NOTES.md` pentru
analiza completa. Pe DS4 situatia era chiar mai incilcita: statusul de frina nu
statea pe RELAY1 ca pe DB3, ci pe **`POS_OUT1`** - a treia varianta, diferita si
de DB2, si de DB3.

In plus, SDK-ul DS4 e alta generatie: foloseste direct macrourile
`BRAKE_SENSE_SET` / `BRAKE_SENSE_CLR`, nu `gpio_iface_brake`. Au fost rutate prin
aceeasi intrare noua de contract, `BRAKE_SENSE_OUTPUT -> DAC_POWER`
(= `GPIO_PL_DAC_PWR_CTL` = **P1.4**, acelasi pin ca `_401_OUT_DAC_RELAY` din DB2).

Consecinta specifica DS4: `run_safe_shutdown.c` foloseste tot `BRAKE_SENSE_*`,
deci **si frina lui de oprire a trecut pe DAC_RELAY**. Pe DB3 acel fisier
foloseste `gpio_iface_brake` si a ramas pe RELAY1. Semantic DS4 pare corect -
functia semnaleaza "frina apasata" in paralel cu `INPUT_BRAKE_STATUS_PRESS`, deci
e linie de status, nu releu fizic - dar **cele doua platforme difera acum la
`run_safe`**. De decis daca se aliniaza si DB3.

## v3.0 — suprafata de rutare pentru VW08EU (2026-09-01)

**ABI 1.2 / tabela de API v3.** Adaugare pur aditiva: **34 de sloturi noi**, toate
la coada tabelei. Niciun offset anterior nu se schimba.

Cu asta contractul acopera tot ce cheama codul de vehicul al lui VW08EU. Este
prima bucata substantiala din portarea pe DS4 (Faza B din cinci).

### De ce 34 si nu 30

Estimarea din faza de analiza spunea 30. Numarase doar cimpurile care se termina
in `_fn`; lipseau **4 iesiri gpio de ambreiaj**, care au alt tipar de nume:

```
gpio_clutch_set  gpio_clutch_pwm_set  gpio_clutch_sensor_a_set  gpio_clutch_sensor_b_set
```

In sens invers, `can_per_add` si `can_per_remove` erau **deja** rutate pe DS4 in
grupul S5, deci nu s-au repetat. Net: 30 de functii + 4 gpio.

### Ce s-a adaugat

| grup | sloturi | pentru cine |
|---|---|---|
| sense / status_system | 7 | `sense_rs_can_set_dl_status`, `get_ss_trigger`, `sense_rs_oem_autolock_can`, `get_hood_status`, `sense_rs_neutral_can`, `get_rpm`, `get_door_status` |
| sursa de hood | 1 | `set_hood_source` — restul surselor de sense erau deja rutate |
| sense OEM alarm | 3 | `sense_rs_can_oem_alarm_arm` / `_disarm` / `_trigger` |
| per_exec | 3 | `com.lib.scan` |
| os | 4 | `com.lib.scan_new` — `os_irq_create` / `_set` / `_wait_v`, `os_mem_request` |
| led_detect | 2 | `led_detect_err_can1`, `led_detect_err_invalid_bcm` — restul familiei era rutat |
| func / lib.fw | 4 | `dl_cfg`, `can_usr_rx_filter_disable`, `tsk_rs_invalid_state`, `lf_autopulse_enable` |
| ambreiaj: timer + FIQ | 6 | `fiq_register`, `tmr_create` / `_ena` / `_dis` / `_update` / `_int_sta` |
| gpio ambreiaj | 4 | iesirile comandate de `com.lib.clutch` |

Trei typedef-uri noi: `core_ds4_fn_ptr_u32` (`os_mem_request`),
`core_ds4_fn_per_exec_reg_t` si `core_ds4_fn_os_irq_wait_v_t` (amindoua variadice).
Restul refolosesc typedef-uri existente.

### Ambreiajul

`com.lib.clutch` isi conduce PWM-ul dintr-un handler de FIQ propriu, la 2 ms.
Vectorul de intrerupere ramine al CORE-ului; prin tabela trec doar inregistrarea
handler-ului si comanda timerului, iar handler-ul insusi sta in flash-ul de FW.
Aceeasi schema ca pe DB3, unde a fost validata la banc (PWM 500 Hz pe ISO2_TX,
factor de umplere 25% / 74%, verificat pe LED-uri la 5 Hz si 0,5 Hz).

`CLUTCH_SENSOR` are **doua** clase (A si B), pentru ca in DB2 macroul comanda doi
pini.

Iesirile de ambreiaj merg **prin contract**, nu prin `OUT_PHAL2_TX` din SDK. Acela
are un bug — pe ramura FALSE stinge ISO**1**, nu ISO2 (`ds4_gpio.h`). Bug-ul
exista pe amindoua platformele, dar nu e atins de nimeni fiindca ruta trece prin
contract.

### Maparea de iesiri

DS4 nu lucreaza cu un Remote Starter extern: face el pornirea. Nu mai exista fire
catre un starter, deci **iesirile de raportare a starii dispar** si devin
configurabile de pe web prin `config_ngmm_feature.h`.

Regula ceruta de Adrian: **blocat = doar ce foloseste `func_rs.c`**, plus
ambreiajul (chemat din el) si singurul IO din `func_hazard_ctrl.c`.

```
BLOCAT (7)                        CONFIGURABIL PE WEB (12, pe NONE)
  BRAKE             RELAY1          HANDBRAKE   IGNITION   DOORS    TRUNK
  CLUTCH            RELAY1          HOOD        KEY_WRAP   PTS_2    RAP
  CLUTCH_PWM        ISO2_TX         RSR_LOCK    RSR_UNLOCK RSR_TRUNK
  CLUTCH_SENSOR_A   RELAY2          CLUTCH_SENSOR_B (vezi mai jos)
  LIGHTS            NEG_OUT1
  PTS_1             NEG_OUT5
  RF_LOOP           NEG_OUT13
```

`BRAKE` si `CLUTCH` **impart deliberat RELAY1**. Vine din DB2, unde
`VW03_CLUTCH_PWM_CUT(x)` era `OUT_RELAY1(x)`. Validat la banc pe DB3.

Pentru `PTS_1` si `RF_LOOP` s-au pastrat valorile DS4, nu cele de pe DB3 —
`RF_LOOP` e pe `NEG_OUT13` pentru ca acolo e firul pe aceasta platforma.

Selectoarele `ISO1_TX` si `ISO2_TX` au fost adaugate in enum (DS4 nu le avea).
Sint pe aceiasi pini ca pe DB3: P0.10 si P0.11.

### De stiut: `CLUTCH_SENSOR_B` e pe `NONE`

Pe DB2/DB3 e `OUT_5`, dar pe DS4 `NEG_OUT5` e ocupat de `PTS_1`. Slotul de tabela
si accesorul exista, dar clasa e `GPIO_IFACE_OUTPUT_NONE`, deci **apelul e no-op**
pina se alege un pin.

Libere azi: `NEG_OUT3`, `NEG_OUT4`, `POS_OUT1`, `POS_OUT2`, `RF_LOOP_POWER`,
`ISO1_TX`. BSP-ul DS4 mai are `NEG_OUT8..NEG_OUT12` — iesirile in plus fata de
DB3 — dar nu sint inca in enum-ul contractului si **polaritatea lor nu e
validata**, nici pe schema, nici la banc.

### Compatibilitate

`CORE_DS4_API_TABLE_VERSION` 2 -> 3, `CORE_DS4_ABI_VERSION_MINOR` 1 -> 2.
**MAJOR ramine 1**: adaugarea e strict la coada, deci un FW mai vechi citeste
corect tabela.

Ruptura de MAJOR nu era necesara aici, spre deosebire de DB3 v2.0, unde tabela a
trebuit mutata la `0x27C40` fiindca nu mai incapea. Pe DS4 fereastra
`0x37040..0x38000` are **4032 B**, iar tabela ajunge la ~630 B.

`local_api_table` din `core_contract_ds4_boot_handshake.h` (ramura de rezerva,
activa cind nu e definit nici `CORE_DS4_BUILD`, nici `FW_DS4_BUILD`) **nu a fost
atinsa**: se termina cu `{0}`, iar cimpurile noi fiind la coada raman zero.

### Fisiere atinse

| fisier | ce |
|---|---|
| `core_contract_ds4_api_table.h` | 3 typedef-uri + 34 cimpuri |
| `core_contract_ds4_gpio_iface_shim.c` | 4 include-uri, 3 `extern`, 34 intrari pozitionale, 4 functii `gpio_iface_clutch*` pe partea de FW |
| `core_contract_ds4_gpio_output.h` | 4 setteri CORE |
| `core_contract_ds4_boot_handshake.h` | 4 wrappere `api_gpio_clutch*` |
| `core_contract_ds4_fw_api.h` | 4 redirectari FW |
| `core_contract_ds4_gpio_config.h` | selectoare ISO1/ISO2, clase de ambreiaj, accesori, EXPECT, garzi |
| `core_contract_ds4_abi.h` | versiunile |

Cele trei functii `sense_rs_can_oem_alarm_*` nu sint declarate in
`sense_rs_can.h`, ci in headere private. Au legatura externa
(`sense_rs_can_oem_alarm.c`), deci sint rutabile — shim-ul le declara `extern`.

### Verificari facute

**Simbolurile exista.** Toate cele 30 de functii sint in `can.sdk.ds4`. Semnaturile
celor cu risc au fost comparate una cite una cu DB3 si sint identice:
`tmr_update`, `os_irq_wait_v`, `per_exec_register`, `dl_cfg`, `fiq_register`,
`tmr_create`.

**Alinierea struct <-> initializator: 142 = 142.** Verificata pozitional cu
`check_api_table_alignment.pl` (vezi mai jos). Singurele nepotriviri de nume sint
cele cunoscute si intentionate:

| pozitie | cimp | initializator | de ce |
|---|---|---|---|
| 0..6 | antet | constante | `table_magic`, `sizeof`, versiuni, masca |
| 25 | `trace_print` | `dbg_print` | DS4 isi numeste altfel functia de trace |
| 102 | `prog_set_exit_callback_fn` | `0` | DS4 nu are functia; slot lasat NULL, dinainte |

**Nu e compilat inca.** Proiectele Keil pentru DS4 apar la Faza D; abia acolo se
vede daca lipseste vreun include.

### Unealta noua: `check_api_table_alignment.pl`

Initializatorul din shim e **pozitional**. Daca ordinea lui nu corespunde exact
ordinii cimpurilor din structura, FW-ul cheama alta functie decit crede si se vede
abia la runtime, ca un comportament aiurea, nu ca o eroare de compilare.

Scriptul compara cele doua liste pozitie cu pozitie. Se ruleaza din folderul
contractului:

```
perl check_api_table_alignment.pl core_contract_ds4_api_table.h core_contract_ds4_gpio_iface_shim.c
```

**Sterge comentariile inainte de comparatie**, si asta nu e un detaliu: fara strip,
text din comentarii prinde regexul (`instances;` dintr-o fraza engleza, cuvinte
romanesti terminate in `-ului`) si produce o decalare aparenta care te trimite sa
cauti bug-uri inexistente. S-a intamplat de doua ori inainte sa ajunga scriptul in
forma asta.

---

## Faze

| faza | ce | stare |
|---|---|---|
| A | analiza delta DS4 vs DB3 | incheiata |
| B | contract | **incheiata — v3.0** |
| C | `lib.config.ds4` + `lib.fw.ds4` in `VEHICLE_SPECIFIC` | urmeaza |
| D | proiectele Keil `core_vw08eu_ds4` / `fw_vw08eu_ds4` | |
| E | build + livrabil `933.vw08eu.1_00_00.enc` | |

`VEHICLE_SPECIFIC` e **unul singur**, comun DB3 si DS4. De aceea cele trei foldere
DS4 stau flat linga cele de DB3, nu grupate intr-un subfolder: altfel `..\VEHICLE_SPECIFIC`
ar diverge intre platforme si s-ar rupe toate caile relative din `.uvproj`.
