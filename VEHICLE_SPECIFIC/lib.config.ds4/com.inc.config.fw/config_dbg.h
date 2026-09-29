/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: config_dbg.h 33426 2015-08-28 11:08:28Z martin.bouchard $
/*==========================================================================*/

//             <<< Use Configuration Wizard in Context Menu >>>             //

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

/* v19 VW08EU (23.09.2026): PGM ACTIVAT pe DS4, la cererea explicita a lui Adrian,
   pentru prima iesire pe masina. Valoarea 0 e cea din DB2.

   Comutatorul asta guverneaza DOUA lucruri, de cind cu v19:
     - detect_pgm() chiar scaneaza, in loc sa intoarca TRUE (detect_pgm.c)
     - blocul VIN + PGM din detect_car_search.c e compilat
   Fisierul detect_car_search.c e comun DB3/DS4, dar fiecare platforma isi are
   propriul config_dbg.h, deci DB3 ramine neatins cit timp are 1.

   DE STIUT INAINTE DE TEST, altfel PGM-ul pare "stricat":
   scanarea merge pe CAN_DEV_0, NU pe canalul vehiculului. vw06_scan_NEW_pgm2()
   isi ignora parametrul si forteaza pgm_data->dev_id = CAN_DEV_0
   (vw06_scan_pgm2_NEW.c:104, "use FT for scan", identic cu DB2, unde a mers pe
   masina). Pe DS4 CAN_DEV_0 iese pe 8-pin/3 si 8-pin/4, Orange/Brown si
   Orange/Green, care in techsheet sint marcate No Connection.
   Deci perechea portocalie trebuie legata la OBD II (pinii 6 si 14) ca sa existe
   cu cine vorbi. La banc nu se poate testa: tool-ul sta pe CAN_DEV_1.

   Daca PGM-ul esueaza, in trace apare [ DETECT : PGM_ERROR] si pe D2D "PGMERR";
   nu opreste detectia, doar aprinde LED-ul de eroare 10 secunde. */
#ifndef DEBUG_SKIP_PGM
#define DEBUG_SKIP_PGM               0
#endif
/* ======================================================================== */




#if DBG_JTAG == 1
  #warning "DEBUG is enable Please keep this #WARNING"
#endif

//--------------------------------------------------------------------------//
// <e0>  DEBUG_TEXT Enable
#define DEBUG_TEXT              1          // 

#define USE_TRACE             DEBUG_TEXT  //

/* TEMPORAR - banc, DS4. Pe DS4, UART-ul de debug e disponibil numai daca portul
   UNDERDASH are rolul 0 in intrarea NVFS `config_port`. Pe un modul nou intrarea
   nu exista, iar debug_port_avail() o creeaza cu underdash = 2, deci
   dbg_uart_avail ramine 0 si TRACE nu iese niciodata pe fir - fara niciun semn.
   Comutatorul de mai jos forteaza portul disponibil pentru bring-up.
   De pus pe 0 inainte de productie, ca DEBUG_SKIP_PGM. */
#ifndef DEBUG_FORCE_DBG_UART
#define DEBUG_FORCE_DBG_UART         1
#endif
//--------------------------------------------------------------------------//
// <e1> PRINT MEM/STATUS DEBUG
// </e>
#if DEBUG_TEXT == 1
    #define DBG_PRINT_MEM       0   //
#endif

//--------------------------------------------------------------------------//
// <h> TEMPLATE DEBUG
// <q1> lib.detect
// <q2> lib.sense
// <q3> lib.function
// </h>
#if DEBUG_TEXT == 1
    #define DBG_DETECT          1   //
    #define DBG_SENSE           1   //
    #define DBG_FUNCTION        1   //
#endif
//--------------------------------------------------------------------------//
// <h> GENERIC DEBUG
// <q1> lib.cmd
// <q2> lib.ctl_dl
// <q3> lib.d2d
// <q4> lib.eips
// <q5> lib.obd_can
// <q6> lib.prog
// <q7> lib.reset
// <q8> lib.rsr_feature
// <q9> lib.rsr
// <q10> lib.rxt
// <q11> lib.secure_lock
// <q12> lib.status_req
// <q13> lib.vin
// <q14> lib.w2w
// <q15> lib.wdog_v
// <q16> lib.wake
// <q17> lib.gpio/gpio_wake
// </h>
#if DEBUG_TEXT == 1
    #define DBG_CMD             0   //
    #define DBG_CTL_DL          0   //
    #define DBG_D2D             0   //
    #define DBG_EIPS            0   //
    #define DBG_OBD_CAN         0   //
    #define DBG_PROG            0   //
    #define DBG_RESET           0   //
    #define DBG_RSR_FEAT        0   //
    #define DBG_RSR             0   //
    #define DBG_RXT             0   //
    #define DBG_SECURE_LOCK     0   //
    #define DBG_STS_R           0   //
    #define DBG_VIN             0   //
    #define DBG_W2W             0   //
    #define DBG_WDOG_V          0   //
    #define DBG_WAKE            0
    #define DBG_GPIO            0
    #define DBG_PTS_CTL         0

#endif
//--------------------------------------------------------------------------//
// </e>


#endif


