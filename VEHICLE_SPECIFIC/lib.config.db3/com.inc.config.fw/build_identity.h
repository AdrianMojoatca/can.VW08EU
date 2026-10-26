#ifndef __BUILD_IDENTITY_H__
#define __BUILD_IDENTITY_H__

/* RELEASE 1.00: 29.09.2026. RELEASE 1.01: 26.10.2026 (usa sofer / alte usi pe D2D).
   Versiunea de livrare e "1.01", fara sufix de etapa.
   Sufixele 1.00.01 ... 1.00.03 au fost build-uri de inginerie, pentru banc; ele
   NU ajung pe site.
   Numele .enc se deriva de aici, cu punctele inlocuite de underscore:
       1.00  ->  933.vw08eu.1_00.enc   si   403.vw08eu.1_00.enc
   aceeasi forma ca a ghidurilor de instalare din InstallGuides/.
   Pentru urmatoarea campanie de teste se pune iar un sufix (1.00.04), iar la
   livrare se revine la 1.00 sau se trece la 1.01.

   VW08EU e clona lui VW07EU 1.00 (commit 1fd4ab3 din can.vw07eu), facuta pe
   28.09.2026. Singura diferenta de fond fata de VW07EU: in Remote Start, usa
   deschisa NU opreste motorul (TSK_RS_SKIP_DOOR_SHUTDOWN, a patra optiune din
   DB2) - vezi comentariul din fw_init.c, fw_options_set().
   Sursa de inspiratie in DB2 ramine tot VW07EU: nu exista un VW08 in DB2. */

/* ATENTIE: acest fisier are un DUBLU in contract
   (vw08eu_db3_contract\build_identity.h). Ordinea de include decide care ciștiga,
   iar inainte de 25.08.2026 cele doua aveau valori DIFERITE (FRD37EU_T1 / 1.00_91
   aici, FRD37EU / 1.00.01 in contract) - livrabilul .enc lua versiunea din
   contract. Le tinem identice, ca rezultatul sa nu depinda de ordinea de include.
   De consolidat intr-un singur fisier la o iteratie viitoare. */

#if defined(CORE_DB3_BUILD)
#define BUILD_FW_NAME       "VW08EU"
#define BUILD_FW_VERSION    "1.01"      
#define BUILD_FW_INFO       "CORE split image"
#else
#define BUILD_FW_NAME       "VW08EU"
#define BUILD_FW_VERSION    "1.01"
#define BUILD_FW_INFO       "FW split image"
#endif

#endif
