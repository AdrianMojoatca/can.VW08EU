# can.vw08eu

Firmware pentru Audi / Volkswagen / Seat / Skoda, PTS si cheie clasica, piata EU,
pe doua platforme: **DB3 (403)** si **DS4 (933)**, in arhitectura **thin-FW**.

**VW08EU e o clona a lui VW07EU 1.00** (commit `1fd4ab3` din `can.vw07eu`),
facuta pe 28.09.2026. Codul e identic, cu o singura diferenta de comportament:

> **In Remote Start, deschiderea usii nu opreste motorul.** Pe VW07EU o opreste.
> Se face cu `tsk_rs_update_cfg(TSK_RS_SKIP_DOOR_SHUTDOWN)` in `fw_init.c`, adica
> a patra optiune din DB2. Detalii in `RELEASE_NOTES_DS4.md`.

| platforma | livrabil | stare |
|---|---|---|
| DB3 / 403 | `403.vw08eu.1_00.enc` | **RELEASE**, validat pe masina |
| DS4 / 933 | `933.vw08eu.1_00.enc` | **RELEASE**, validat pe masina |

Amindoua au trecut testele pe masina, confirmat pe 29.09.2026. Codul de baza era
oricum validat: VW07EU a trecut pe masina pe 23-24.09.2026, tot pe amindoua
platformele. Diferenta dintre cele doua firmware-uri e o singura linie, deci ce
urmeaza mai jos - portarea, defectele si reparatiile - descrie **istoria lui
VW07EU**, mostenita aici.

Versiunea de livrare e **`1.00`, fara sufix**. Sufixele `1.00.01` ... `1.00.03`
au fost build-uri de inginerie si nu ajung pe site.

Amindoua au `detect_pgm()` activ. **PGM-ul scaneaza pe `CAN_DEV_0`, adica pe
perechea portocalie catre priza OBD II** - nu pe canalul vehiculului - iar in
techsheet perechea aia e inca marcata `No Connection`. Fara ea legata (OBD II
pinii 6 si 14), PGM-ul pare stricat din firmware. Detalii in
`RELEASE_NOTES_DS4.md`.

> **La flash se sterge NVFS-ul.** Valorile `FEAT_DEI_*` sint *default-uri*: se
> scriu numai cind memoria e virgina, deci un reflash peste o NVFS scrisa
> pastreaza maparea veche a iesirilor - si simptomul arata exact ca un bug de
> firmware. Ne-a pacalit de trei ori.

---

## Structura

| director | ce contine |
|---|---|
| `VEHICLE_SPECIFIC/` | **codul de vehicul, unul singur, comun DB3 + DS4.** Cele 8 biblioteci portate din DB2: `com.lib.sense`, `com.lib.func`, `com.lib.clutch`, `com.lib.detect` si familia de scan (`com.lib.scan`, `scan_manager`, `scan_new`, `vw06_scan`), plus `lib.config.*` si `lib.fw.*` pe platforma |
| `vw08eu_db3_contract/` `vw08eu_ds4_contract/` | contractul CORE <-> FW, cite unul pe platforma. Intra in proiectele Keil prin junction NTFS, deci exista o singura copie |
| `core_vw08eu_db3/` `fw_vw08eu_db3/` | proiectele Keil pentru 403 |
| `core_vw08eu_ds4/` `fw_vw08eu_ds4/` | proiectele Keil pentru 933 |
| `TechSheet/` | tech sheet-ul, **sursa unica** pentru ghidurile de instalare |
| `InstallGuides/` | generatoarele techsheet -> markdown -> HTML si desenul de conexiuni |
| `out/403/` `out/933/` | livrabilele `.enc` (ignorat de git) |
| `Release/` | **nu exista inca pe VW08EU.** La VW07EU e folderul care pleaca din firma: `.enc`-ul si fisierele de configurare cu care a fost construit, cite un subfolder pe platforma. Notele de release de acolo sint de uz intern si nu ajung pe site |

## Constructie

```
.\build_vw08eu.ps1          # DB3
.\build_vw08eu_ds4.ps1      # DS4
```

Scriptul face tot: CORE, FW, verificarea perechii citita direct din hex-uri,
umplerea pina la baza FW-ului si `.enc`-ul final. Numele livrabilului se deriva
din `build_identity.h`, deci urmeaza automat versiunea.

Doua conditii, amindoua invatate pe pielea noastra:

- **Keil MDK v4.** `UV4.exe` din v5 intoarce 0 si nu construieste nimic, fiindca
  proiectele sint `.uvproj`, nu `.uvprojx`. De aceea scriptul verifica
  timestamp-ul artefactului, nu codul de iesire.
- **IDE-ul inchis** inainte de build: UV4 in linie de comanda si Keil deschis pe
  acelasi proiect se bat pe `.uvproj` / `.uvopt`.

Harta de memorie: pe DB3 FW-ul incepe la `0x28000` cu tabela de API la `0x27C40`
(fereastra 960 B); pe DS4, `0x38000` si `0x37040` (fereastra 4032 B). Pe DS4
umplerea pina la baza FW-ului e **obligatorie**, altfel flasher-ul abandoneaza la
circa 90%.

## Ce merge, verificat la banc (pe VW07EU)

Lista de mai jos a fost verificata pe VW07EU. Codul fiind identic, se aplica si
lui VW08EU, dar pe VW08EU nu a fost reluata.

Detectie de masina si asezare in NVFS; CAN pe canalul vehiculului la 500 kbit/s;
sense complet (contact, usi, portbagaj, capota, frina, frina de mina, viteza,
RPM); alarma OEM lock / unlock / arm / disarm de pe magistrala; trunk release
OEM raportat pe D2D; avarii cu puls de 150 ms; secventa de Remote Start cu PTS,
frina si ambreiaj; pulsuri de Lock si Unlock; statusurile GWA si GWR; pe DS4
si comanda directa de Accessory / Ignition / Starter.

## Reguli de lucru

Cele care au schimbat efectiv decizii, nu preferinte de stil:

- **`DBALL_egal_DB2` e read-only** — sursa de inspiratie, niciodata copiata.
  La fel si **FRD37EU**, care e modelul de platforma: se citeste, nu se atinge.
- **`VEHICLE_SPECIFIC` e un singur folder**, comun celor doua platforme. Un fisier
  comun modificat inseamna automat versiune noua pe amindoua - si, mai important,
  **comportament schimbat pe amindoua**. O reparatie specifica unei platforme se
  pazeste explicit (`#ifndef PLATFORM_933`) si se verifica prin dimensiunea feliei
  de FW a celeilalte. Pe 25.09.2026 asa a fost prinsa o regresie pe DB3.
- **Contract separat pe platforma**, adus prin junction. `ds4_contract`-ul lui
  FRD37EU nu se atinge.
- **Culorile firelor sint contractul cu instalatorul.** VW08EU merge pe tan si
  tan/negru (`CAN_DEV_1`); FRD37EU pe portocaliu (`CAN_DEV_0`).
- **Apartenenta la build se verifica in `.map`, nu in `.uvproj`** — proiectele
  folosesc amalgamare, deci `.uvproj` minte.
- **Cadrele de test pe CAN traiesc doar la banc.** Sondele care *transmit* se
  scot imediat ce au dat raspunsul; unele BCM-uri le interpreteaza.
- **Analiza inainte de cod.**

## Cum s-a ajuns aici (istoricul lui VW07EU)

Nimic din ce urmeaza nu s-a intimplat pe VW08EU: e portarea lui VW07EU, de la
care VW08EU a fost clonat pe 28.09.2026.

**24.08 - 04.09, portarea propriu-zisa.** Intii DB3: cele 8 biblioteci de vehicul
mutate pe template-ul thin-FW, contractul extins la v2.1 si apoi la v3.0 pentru
DS4, proiecte Keil noi pe ambele platforme. Criteriul pe increment a fost
"compileaza curat, nu neaparat linkeaza" — tabela de API se extinde o singura
data, nu la fiecare pas.

**Bring-up-ul CAN a costat cel mai mult.** Doua cauze, amindoua de acelasi fel:
un fisier `config_*.h` din SDK umbrea in tacere valoarea din contract. Intii
viteza (125K in loc de 500K), apoi canalul. Pe LPC176x `CAN_DEV_1 == 0`, deci
numele pacalesc — se citesc registrii, nu eticheta. Semnatura utila de
diagnostic: un canal care transmite fara ACK **trebuie** sa intre in bus-off;
cind nu intra, nu transmite deloc, si asta inseamna viteza gresita.

**05.09 - 12.09, reparatii de functionalitate.** Statusurile de Remote Start nu
se scriau deloc pe DB3 — biblioteca din CORE care le scria pe DB2 nu are
corespondent — asa ca oprirea de siguranta la viteza nu se putea declansa. Frina
avea doua iesiri in DB2, una de actuatie si una de status, contopite gresit la
portare. Ambele reparate pe amindoua platformele.

**15.09 - 17.09, comportament si iesiri.** PTS-ul la oprire se da acum doar daca
**noi** am pornit motorul, altfel apasarea Remote Start pe o masina in mers il
oprea. Trunk release-ul OEM se trimite pe D2D din CORE, nu din sense. Pe DS4 s-au
fixat iesirile care nu au voie sa fie programabile de pe web — Lock, Unlock,
Accessory, Ignition, Starter, GWA, GWR — si s-a reparat ambreiajul.

**22.09, alocarea releelor si ultimele doua defecte tacute.** ISO2, adica PWM-ul
de ambreiaj, nu iesea pe DS4 desi pe DB3 mergea: selectorul fusese adaugat in
enumul contractului, dar nu si in cele trei `switch`-uri care il traduc in pin,
asa ca scrierea cadea pe `default: break;`. De atunci exista o verificare
automata care compara enumul cu fiecare `switch` si se ruleaza la orice atingere
a contractului. Apoi s-a asezat regula de relee: **RELAY1 identic pe amindoua
platformele, RELAY2 devine GWR ON doar pe DS4, iar RELAY3 - care exista numai pe
DS4 - preia ce face RELAY2 pe DB3.** Se poate face tocmai fiindca pe DB3 releul
de GWR il pune remote starter-ul extern, deci firmware-ul de acolo nu-l comanda.

**Patru defecte care merita retinute**, fiindca niciunul nu raporta eroare:

- `prg_out_hw_o5()` era un **stub gol** ramas de cind OUT_5 apartinea PTS-ului.
  Lantul de iesiri DEI are patru etaje si niciunul nu semnaleaza esecul.
- `cfg.is_rsr` se calcula dintr-o valoare scrisa intr-un hook `__weak` din CORE,
  definit in FW. **CORE si FW sint imagini separate**, deci hook-ul nu se cheama
  niciodata: zero referinte in `.map`. Ambreiajul iesea dezactivat in tacere.
- GWR pus pe optiunea 27, "Ground when in ready mode", nu dadea nimic: optiunea
  aia se aprinde doar din procedura MTS de cutie manuala. GWR-ul real e
  optiunea 28, "Status". Numarul optiunii = 1 + pozitia in enumul `Prg_Out_Type`,
  care are intrari comentate afara — se numara citind, nu ghicind.
- `GPIO_IFACE_OUTPUT_ISO2_TX` fusese adaugat in enumul contractului DS4, dar nu
  si in cele trei `switch`-uri care il traduc in pin, deci scrierea cadea pe
  `default: break;`. Pe DB3 cazurile existau, de aici asimetria. De atunci
  enumul si `switch`-urile se compara automat la orice atingere a contractului.

**25.09, release.** Telecomanda OEM nu misca iesirile in Remote Start: in tabela
de iesiri DEI, evenimentele OEM sint legate doar de `ground_when_locked` si
`gwa_*`, in timp ce pulsurile atirna de `INPUT_LOCK1_ON` / `INPUT_UNLOCK_ALL1_ON`,
pe care calea OEM nu le emitea niciodata. Reparat dupa modelul telecomenzii
aftermarket, numai in Remote Start. Odata cu asta au fost scoase de pe DS4 doua
cadre CAN parazite (`0x000` si `0x3D0`), mostenite din DB2 - **dar numai de pe
DS4**: pe DB3 modulul nu are iesiri de lock/unlock, ci intrari, si executa comanda
chiar pe magistrala, deci acolo cadrele sint mecanismul, nu zgomotul. Regresia a
fost prinsa comparind dimensiunea feliei de FW a lui DB3. In fine, `Flex Relay
Output` a fost pus pe aceeasi optiune DEI cu `Ignition`, fara cod: `reg_func()`
adauga in bucketul de id, deci doua iesiri pot imparti o functie.

**23 - 24.09, prima iesire pe masina si PGM-ul.** Amindoua platformele au trecut
testele. `detect_pgm()` a fost activat pentru ocazie, si a cerut **doua**
schimbari, nu una: comutatorul `DEBUG_SKIP_PGM`, plus blocul VIN + PGM din
`detect_car_search.c`, care era comentat in bloc. Fisierul fiind comun celor
doua platforme, blocul a fost pus sub `#if` in loc sa fie decomentat, cu o garda
`#error` impotriva macroului nedefinit - care altfel ar fi activat blocul in
tacere pe amindoua. Pe DB3 a mai fost o capcana: exista **doua** `config_dbg.h`
in acelasi build, cu aceeasi garda, si cea care ajunge in familia `detect_*.c` e
cea din contract, nu cea din `VEHICLE_SPECIFIC`. Valoarea se schimba in amindoua.

**Tech sheet-ul e sursa unica** a ghidurilor de instalare: `xlsx` -> `md` ->
`html`, plus desenul de conexiuni generat din acelasi fisier, deci nu poate
ajunge sa contrazica tabelele.

## Ce ramane deschis

**Pe VW08EU, in plus fata de lista mostenita de mai jos:**

- **Folderul `Release/` nu exista inca.** Cind se livreaza efectiv, se asambleaza
  ca la VW07EU: `.enc`-ul plus fisierele de configurare cu care a fost construit,
  cite un subfolder pe platforma.
- **Doua unelte de build nu sint versionate**: `D2DiConfigFile.exe` si
  `GetSelectedForBatchBuild.exe` din `build/scripts`, excluse deliberat de
  `core_vw08eu_db3/.gitignore`. Restul uneltelor intra in repo, deci un clone
  curat produce `.enc` pe DS4; pe DB3 cele doua trebuie copiate manual.

**Mostenite de la VW07EU:**

- **Un `0x000` la butonul de portbagaj.** `func_oem_trunk_start()` cheama tot
  `func_unlock_all_start()`, deci apasarea butonului de portbagaj de pe telecomanda
  OEM produce inca un cadru de trezire in Remote Start, pe amindoua platformele.
  E acelasi tipar ca la lock si unlock, dar comportamentul lui pe masina nu a fost
  verificat, deci nu a fost atins.

- **Techsheet-ul nu arata perechea portocalie.** `CAN_DEV_0`, pe care scaneaza
  PGM-ul, apare `No Connection` pe amindoua platformele. De corectat, altfel
  instalatorul nu are de unde sti ca trebuie legata la OBD II.
- **Hook-ul de contract necheamat** (`..._prog_config_apply_vehicle_specific`):
  ambreiajul a fost ocolit punctual pe amindoua platformele, dar **89 de apeluri
  `set_prog_*` se pierd in tacere**. Reparatia generala cere o campanie de
  regresie.
- **Contactele lui RELAY3 pe 933**: foaia `Install 933` numeste pinul 4
  "Relay3 N.C.", dar `Module Connection Reference` asociaza aceeasi culoare cu
  "NO RELAY3". Contradictia vine din sablonul DEI. Firmware-ul comanda doar
  bobina, deci raspunsul schimba numai techsheet-ul.
- **Ghidul de instalare**: `Wire Location` e inca `_TBD_`, iar foaia `Features`
  are bifele mostenite din sablon.
- **Comutatoare de banc de intors** inainte de productie:
  `DEBUG_FORCE_DBG_UART`, trace-ul de RX si trace-urile din `clutch_press`.
  `DEBUG_SKIP_PGM` si blocurile VIN/PGM sint inchise de pe 23.09.2026.
- **Curatenie**: SDK-ul de FW pe DS4 nu e degresat (2336 de fisiere, fata de 85
  pe DB3) si mai sint cai de include moarte in `.uvproj`.

Detaliile de platforma sint in `RELEASE_NOTES_DS4.md` si in
`vw08eu_db3_contract/RELEASE_NOTES.md`.
