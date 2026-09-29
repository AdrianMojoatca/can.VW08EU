# vw08eu_ds4_contract

Sursa unica de adevar pentru ABI-ul si contractul CORE<->FW ale lui **VW08EU pe
platforma DS4** (933).

**Nu e submodul.** E o copie de lucru independenta, pornita din `ds4_contract`
(contractul comun al DS4, folosit de FRD37EU). Sursa comuna **`can.ds4_contract`
nu se atinge**: orice schimbare ceruta de VW08EU se face numai aici, ca sa nu se
strice FRD37EU, care e validat pe masina.

Va fi consumat de `core_vw08eu_ds4` si `fw_vw08eu_ds4` — proiectele Keil apar la
Faza D. Amindoua il vor vedea prin junction NTFS, ca perechea de pe DB3, ca sa
existe fizic un singur exemplar al contractului.

## Regula de aur

Initializatorul din `core_contract_ds4_gpio_iface_shim.c` e **pozitional**: ordinea
lui trebuie sa corespunda exact ordinii cimpurilor din `core_ds4_api_table_t`. O
nepotrivire nu da eroare de compilare — FW-ul cheama pur si simplu alta functie
decit crede, si se vede abia la runtime.

Dupa orice modificare a tabelei, ruleaza:

```
perl check_api_table_alignment.pl core_contract_ds4_api_table.h core_contract_ds4_gpio_iface_shim.c
```

Adaugarile se fac **append-only, la coada**, inainte de `reserved_slots`, si se
bumpeaza `CORE_DS4_API_TABLE_VERSION` (plus `CORE_DS4_ABI_VERSION_MINOR` la
adaugari aditive).

## Documente

- `RELEASE_NOTES.md` — istoricul versiunilor si ce contine fiecare
- `check_api_table_alignment.pl` — verificarea alinierii tabelei
- `check_core_ceiling.ps1` — verifica sa nu creasca CORE-ul peste baza FW-ului
