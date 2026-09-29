# Contract VW08EU — SURSA UNICA

Acest folder este **singura sursa** a contractului pentru VW08EU. Ambele proiecte Keil il
vad prin **junction NTFS**:

- `core_vw08eu_db3\external\Core_Contract_SDK_DB3`  -> junction catre acest folder
- `fw_vw08eu_db3\external\Core_Contract_SDK_DB3`    -> junction catre acest folder

Deci se editeaza **doar aici**. Nu mai exista doua copii care pot diverge.

## Provenienta

Continutul e revizia **dovedita la build** — cea din `core_db3\external\Core_Contract_SDK_DB3`
care a produs CORE cu 0 erori si FW cu 0 erori pe 24.08.2026. NU e revizia din repo-ul
separat `FW_CORE_SPLIT\db3_contract`, care difera in 6-7 fisiere, inclusiv functionale
(`core_contract_db3_config_ptrs_fw.c`, `core_contract_db3_fw_descriptor.h`).

Contractul original din `FW_CORE_SPLIT` **nu se atinge**. FRD37EU ramane exact cum era.

## Reguli

- Numele de fisiere si simboluri rimin `core_contract_db3_*`. Forkul nu coexista niciodata in
  acelasi build cu originalul; redenumirea a 50 de fisiere ar sparge fiecare `#include` fara
  cistig. Identificarea se face dupa numele proiectului (VW08EU).
- `build_identity.h` are deja `#if defined(CORE_DB3_BUILD)` cu ramura CORE si ramura FW in
  acelasi fisier — deci o singura copie e corecta, nu trebuie despicata.
- Ce se **adauga** aici pentru VW08EU: selectori pentru ambreiaj — `RELAY3` si o clasa PHAL —
  eventual `NEG_OUT6+`. Semantica ramane cea de la FRD37EU: contract = `NEG_OUTn` / `POS_OUTn`
  / `RELAYn`, macrouri = `OUT_n` / `OUT_n_PLUS` / `OUT_RELAYn`.
- `core_contract_db3_dst_gpio.h` e inca prezent. VW08EU nu are DST, dar headerul e referit din
  `dst_bus_private.h`, deci se scoate in faza D, dupa testele pe banc.

## ATENTIE la mutarea folderului

Junction-urile NTFS folosesc **cai absolute**. Daca muti sau copiezi `Portate` in alta parte,
junction-urile fie se rup, fie continua sa pointeze la locatia veche. Dupa orice mutare,
recreeaza-le:

```
rmdir "<proiect>\external\Core_Contract_SDK_DB3"
mklink /J "<proiect>\external\Core_Contract_SDK_DB3" "<cale noua>\vw08eu_db3_contract"
```
