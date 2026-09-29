#ifndef __CONFIG_DBG_H__
#define __CONFIG_DBG_H__

/* ==== MACROURI DE DEPANARE SPECIFICE VW08EU ==============================
   ATENTIE: exista DOUA fisiere config_dbg.h in build-ul de FW, cu ACEEASI
   garda __CONFIG_DBG_H__:
       VEHICLE_SPECIFIC/lib.config.db3/com.inc.config.fw/config_dbg.h
       vw08eu_db3_contract/config_dbg.h  (= external/Core_Contract_SDK_DB3)
   Cistiga cel care intra primul in fiecare unitate de compilare, iar asta
   difera de la fisier la fisier:
       detect_*.c -> detect_private.h -> detect.h -> core_framework.h ->
       can_usr.h -> core_contract_db3_boot_handshake.h -> ..._api_table.h ->
       ..._dbg_config.h -> "config_dbg.h" rezolvat in PROPRIUL director,
       adica cel din contract.
   Platit pe 27.08.2026: macrourile puse doar in fisierul din VEHICLE_SPECIFIC
   nu ajungeau in detect_pgm.c si detect_can_search.c, deci #if-urile luau
   ramura de 0 in tacere.

   De aceea sint definite in AMBELE fisiere, fiecare cu #ifndef, ca sa nu
   conteze ordinea. CELE DOUA BLOCURI TREBUIE TINUTE IDENTICE.
   Valorile vin din DBALL_egal_DB2/code/common/inc.com.config/config_dbg.h. */

#ifndef DBG_JTAG
#define DBG_JTAG                     0
#endif

#ifndef DEBUG_CAN_ENABLE
#define DEBUG_CAN_ENABLE             0
#endif

/* 1 = fara wake OBD la detectie, ca in DB2 vers. 1_03. Cu 0, detect_can_search
   executa wakeup_obd_can() de 5 ori: ~5 s in plus si bus-off pe CAN_DEV_0. */
#ifndef DEBUG_SKIP_OBD_CAN_INIT
#define DEBUG_SKIP_OBD_CAN_INIT      1
#endif

/* v6 VW08EU (23.09.2026): PGM ACTIVAT pe DB3, oglinda lui DS4 v19. Valoarea 0
   e cea din DB2, unde PGM-ul a mers pe masina.

   Comutatorul guverneaza DOUA lucruri, de cind cu v19 pe DS4:
     - detect_pgm() chiar scaneaza, in loc sa intoarca TRUE (detect_pgm.c)
     - blocul VIN + PGM din detect_car_search.c e compilat
   detect_car_search.c e comun DB3/DS4, dar fiecare platforma isi vede propriul
   config_dbg.h, deci cele doua platforme pot avea valori diferite.

   ATENTIE, SPECIFIC DB3: cele doua config_dbg.h descrise mai sus sint AMINDOUA
   in build-ul de FW si au aceeasi garda, deci intra doar unul per unitate de
   compilare. Verificat in .dep (23.09.2026): pentru detect_car_search.c,
   detect_pgm.c si fw_init.c intra PRIMUL cel din contract. De aceea valoarea se
   schimba in AMINDOUA fisierele, niciodata intr-unul singur.
   Pe DS4 problema nu exista: acolo e o singura copie, cea din VEHICLE_SPECIFIC.

   DE STIUT INAINTE DE TEST, altfel PGM-ul pare "stricat":
   scanarea merge pe CAN_DEV_0, NU pe canalul vehiculului. vw06_scan_NEW_pgm2()
   isi ignora parametrul si forteaza pgm_data->dev_id = CAN_DEV_0
   (vw06_scan_pgm2_NEW.c:104, "use FT for scan", identic cu DB2).
   Pe DB3 CAN_DEV_0 iese pe conectorul DATA 14-pin albastru (H1):
       H1/5  Orange/Green  = HS CAN High  ->  OBD II pin 6
       H1/6  Orange/Brown  = HS CAN Low   ->  OBD II pin 14
   Amindoua sint marcate No Connection in techsheet, deci perechea portocalie
   trebuie legata explicit la priza de diagnoza. Dovada: ghidul de instalare DB2
   (dball2-vw07_en_ig_ac20220627.pdf) deseneaza exact perechea portocalie catre
   "Diagnostic connector OBD II", separata de perechea Tan / Tan-Black care merge
   la BCM. La banc nu se poate testa: tool-ul sta pe CAN_DEV_1.

   Daca PGM-ul esueaza, in trace apare [ DETECT : PGM_ERROR] si pe D2D "PGMERR";
   nu opreste detectia, doar aprinde LED-ul de eroare 10 secunde. */
#ifndef DEBUG_SKIP_PGM
#define DEBUG_SKIP_PGM               0
#endif
/* ======================================================================== */


#define DEBUG_TEXT              1
#define USE_TRACE               DEBUG_TEXT

#if DEBUG_TEXT == 1
    #define DBG_PRINT_MEM       0
#endif

#if DEBUG_TEXT == 1
    #define DBG_DETECT          1
    #define DBG_SENSE           1
    #define DBG_FUNCTION        1
#endif

#if DEBUG_TEXT == 1
    #define DBG_CMD             0
    #define DBG_CTL_DL          0
    #define DBG_D2D             0
    #define DBG_EIPS            0
    #define DBG_OBD_CAN         0
    #define DBG_PROG            0
    #define DBG_RESET           0
    #define DBG_RSR_FEAT        0
    #define DBG_RSR             0
    #define DBG_RXT             0
    #define DBG_SECURE_LOCK     0
    #define DBG_STS_R           0
    #define DBG_VIN             0
    #define DBG_W2W             0
    #define DBG_WDOG_V          1
    #define DBG_WAKE            0
    #define DBG_GPIO            0
    #define DBG_PTS_CTL         0
#endif

#endif
