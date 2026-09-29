/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_private.h 4229 2012-10-18 18:45:04Z florin.olariu $
/*==========================================================================*/
#ifndef __FUNC_PRIVATE_H__
#define __FUNC_PRIVATE_H__
/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "func.h"
#include "sense.h"
#include "clutch.h"
#include "trace.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

//#define UNLOCK_METHOD1  0
#define UNLOCK_METHOD1  1 // ADI - 13.12.2016

/*--------------------------------------------------------------------------*/

enum
{
    FUNC_CAN_WAKE                  ,

    FUNC_CAN_MSG_COUNT
};

/*--------------------------------------------------------------------------*/

typedef enum
{
    FUNC_ARM                 , 
    FUNC_DISARM              ,
    FUNC_LOCK                ,
    FUNC_UNLOCK              ,
    FUNC_TRUNK_RELEASE       ,
    FUNC_RELOCK_TRUNK_RELEASE,
    FUNC_OEM_LOCK            ,
    FUNC_OEM_UNLOCK          ,
    FUNC_OEM_TRUNK           ,
    //FUNC_WRU                 ,
    //FUNC_WRD_FRONT           ,
    //FUNC_WRD_REAR            ,
    //FUNC_GAS_CAP             ,
    //FUNC_LIGHT               ,
    //FUNC_LIGHT_DISABLE       ,
    FUNC_RAP                 ,
    
    FUNC_PRC_COUNT

}Func_Prc ; 

/*--------------------------------------------------------------------------*/

#define FUNC_CAN_PER    ((func_cfg->can_dev_id == CAN_DEV_0) ? CAN_PER_DEV0 : CAN_PER_DEV1)
#define FUNC_CAN_DEV    (func_cfg->can_dev_id)

/*--------------------------------------------------------------------------*/

#define     FUNC_RS_TAKEOVER_TIMEOUT           90000
#define     FUNC_TIMEOUT_HAZARD_OUT            15    //150 ms
#define     FUNC_GWR_OFF_RELOCK_TIMEOUT		7*100 //7 seconds

/*--------------------------------------------------------------------------*/
#if DBG_FUNCTION == 1
#define     TRACE_FUNC      TRACE
#else
#define     TRACE_FUNC(...)   ;
#endif

/*--------------------------------------------------------------------------*/

#define FUNC_DL_UNKNOWN         -1
#define FUNC_DL_UNLOCKED        0
#define FUNC_DL_UNSAFE_LOCKED   2
#define FUNC_DL_SAFE_LOCKED     6

/*--------------------------------------------------------------------------*/
     
/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

void                func_evt_callback     (DL_EVT evt);
void                func_out_callback     (DL_OUT out);

/*--------------------------------------------------------------------------*/

void                func_cmd_init        ( void );
void                func_prc_init        ( void );

/*--------------------------------------------------------------------------*/

void                func_cmd_lock         ( void );
void                func_cmd_unlock       ( void );
void                func_cmd_trunk        ( void );
void                func_cmd_oem_lock     ( void );
void                func_cmd_oem_unlock   ( void );
void                func_cmd_oem_trunk    ( void );
void                func_cmd_light_on     ( void *arg );
void                func_cmd_light_off    ( void *arg );
void                func_cmd_dome_on      ( void );
void                func_cmd_dome_off     ( void );
void                func_cmd_aux1         ( void );
void                func_cmd_aux2         ( void );
void                func_cmd_aux3         ( void );
void                func_cmd_gwr_on       ( void );
void                func_cmd_ign_off      ( void );

/*--------------------------------------------------------------------------*/

void                func_oem_lock_start   ( void );
Boolean             func_oem_lock_exec    ( void );
void                func_oem_lock_stop    ( void );

/*--------------------------------------------------------------------------*/

void                func_oem_unlock_start ( void );
Boolean             func_oem_unlock_exec  ( void );
void                func_oem_unlock_stop  ( void );

/*--------------------------------------------------------------------------*/

void                func_oem_trunk_start  ( void );
Boolean             func_oem_trunk_exec   ( void );
void                func_oem_trunk_stop   ( void );

/*--------------------------------------------------------------------------*/

void                func_arm_start       ( void );
void                func_arm_stop        ( void );
Boolean             func_arm_exec        ( void );

/*--------------------------------------------------------------------------*/

void                func_disarm_start    ( void );
void                func_disarm_stop     ( void );
Boolean             func_disarm_exec     ( void );

/*--------------------------------------------------------------------------*/

void                func_rap_start       ( void );
void                func_rap_stop        ( void );
Boolean             func_rap_exec        ( void );

/*--------------------------------------------------------------------------*/

void                func_lock_start      ( void );
void                func_lock_stop       ( void );
Boolean             func_lock_exec       ( void );

/*--------------------------------------------------------------------------*/

void                func_unlock_all_start   ( void );
void                func_unlock_all_stop    ( void );
Boolean             func_unlock_all_exec    ( void );

/*--------------------------------------------------------------------------*/

void                func_trunk_release_start( void );
void                func_trunk_release_stop ( void );
Boolean             func_trunk_release_exec ( void );

/*--------------------------------------------------------------------------*/

void                func_gas_start          ( void );
void                func_gas_stop           ( void );
Boolean             func_gas_exec           ( void );

/*--------------------------------------------------------------------------*/

void                func_wru_start       ( void );
void                func_wru_stop        ( void );
Boolean             func_wru_exec        ( void );

/*--------------------------------------------------------------------------*/

void                func_wrd_front_start ( void );
void                func_wrd_front_stop  ( void );
Boolean             func_wrd_front_exec  ( void );

/*--------------------------------------------------------------------------*/

void                func_wrd_rear_start  ( void );
void                func_wrd_rear_stop   ( void );
Boolean             func_wrd_rear_exec   ( void );

/*--------------------------------------------------------------------------*/

void                func_light_start     ( void );
Boolean             func_light_exec      ( void );
void                func_light_stop      ( void );

/*--------------------------------------------------------------------------*/

void                func_hazard_init     ( void );

/*--------------------------------------------------------------------------*/

void 				func_rs_init		  ( void );
void                func_rs_gwr_on       ( void );
void                func_rs_gwr_off      ( void );
void                func_rs_ign_on       ( void );
void                func_rs_ign_off      ( Boolean do_shutdown);
void                func_rs_str_on       ( void );
void                func_rs_str_off      ( void );
void                func_rs_pts          ( void );

/*--------------------------------------------------------------------------*/

void                func_eips_init       ( void );
void                func_eips_shutdown   ( void );
void                func_eips_fail       ( void *arg );
Timeout_Q_Status    func_eips_alert      ( void *arg );

/*--------------------------------------------------------------------------*/

void                func_can_wake_enter  ( void );
void                func_can_wake_leave  ( void );

/*--------------------------------------------------------------------------*/

void 			 func_cmd_gwr_off   	  ( void );
Timeout_Q_Status func_gwr_off_relock     ( void );

void wnd_roll_tmr_func ( void *arg);
/*--------------------------------------------------------------------------*/


/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
extern const Can_Msg       func_can_table[FUNC_CAN_MSG_COUNT] ; 
extern const Process_Cfg   func_prc_cfg  [FUNC_PRC_COUNT];
extern       Process_Node* func_prc      [FUNC_PRC_COUNT];

extern       Boolean       func_light_done; 
extern       Boolean       func_dome_done ;
extern       SInt16        func_dl_type        ;
//extern       Boolean       func_smart_key_power;
extern       Boolean       func_do_unlock_all;

extern       Func_Cfg*     func_cfg;

extern Timeout wnd_rol_tmr;            
/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/


/* Declaratiile familiei gpio_iface_*, definite de shim-ul de contract
   (core_contract_db3_gpio_iface_shim.c, in build). Fara ele compilatorul le
   declara implicit (#223-D) si le apeleaza ca int f() - merge din intimplare pe
   ARM, dar e nedefinit. Headerul e cel din SDK, copiat 1:1 din partea de CORE. */
#include "gpio_iface.h"
/* ==== portare VW08EU: punti catre facilitatile template-ului ==============
   DB2 avea un canal de trace separat (trace_can2). SDK-ul nostru are un singur
   TRACE, rutat prin tabela de API sub FW_DB3_BUILD. Pastram apelurile intacte. */
#ifndef TRACE_CAN2
#define TRACE_CAN2      TRACE_FUNC
#endif

/* Iesirile de functie veneau in DB2 din config_gpio.h. Pe thin-FW GPIO-ul e
   detinut de CORE si seedat prin contract, deci config_gpio.h nu e accesibil din
   FW. Le rutam prin clasele contractului - contractul e sursa de adevar.

   VW08EU e pe DBALL, deci blocul activ in config_gpio.h e cel de sub
   #ifndef PLATFORM_CM800:

     macro DB2                 DB2 (DBALL)              clasa de contract
     FUNC_HAZARD_OUT           OUT_1                    LIGHTS
     FUNC_PTS_OUT              SPEED_PROT_NORMAL(OUT_2) PTS_1 + acelasi gard
     FUNC_BRAKE_OUT            OUT_RELAY1               BRAKE
     FUNC_RF_LOOP_RSR          RSR_OUT_4                RF_LOOP
     FUNC_TRUNK_OUT            ;   (neutralizat in DB2) - pastrat no-op
     FUNC_RS_IGNITION2_OUT     ;   (neutralizat in DB2) - pastrat no-op
     FUNC_EIPS_ALARM_OUT       nedefinit (EIPS_ENABLE != 1) - pastrat no-op

   ATENTIE: pinii din contract sint deocamdata cei seedati de la FRD37EU
   (LIGHTS->NEG_OUT2, PTS_1->NEG_OUT5, BRAKE->POS_OUT1, RF_LOOP->RF_LOOP_POWER),
   iar VW08EU cere HAZARD->OUT_1, PTS->OUT_2, BRAKE->RELAY1. Renumerotarea
   contractului e o decizie separata, deliberata - vezi STARE_PORTARE.md. */
#ifndef FUNC_HAZARD_OUT
#define FUNC_HAZARD_OUT(x)          { (x) ? gpio_iface_lights_on() : gpio_iface_lights_off(); }
#endif

/* SPEED_PROT_NORMAL din DB2, reprodus pe clasa de contract: iesirea nu e
   comandata cit timp starea de RS e invalida (viteza > 0). */
#ifndef FUNC_PTS_OUT
#define FUNC_PTS_OUT(x)             { (tsk_rs_invalid_state() == FALSE) ? gpio_iface_pts_1(x) : gpio_iface_pts_1(FALSE); }
#endif

/* v7 VW08EU (15.09.2026): acelasi pin, dar FARA garda de viteza.
   Se foloseste EXCLUSIV pe ramura de shutdown din func_rs_ign_off().

   De ce: tsk_rs_invalid_state() se zavoraste la prima frama cu viteza != 0
   (tsk_rs_cmd.c:131) si ramine zavorit pina la urmatorul GWR ON - se sterge
   doar in tsk_rs_invalid_state_reset(), chemat din tsk_rs_cmd_gwr_on(). Deci
   de la aparitia vitezei FUNC_PTS_OUT(TRUE) devine gpio_iface_pts_1(FALSE).

   Pe o masina cu buton Start, motorul se opreste APASIND butonul, iar remote
   starter-ul nu da acest puls - el doar scoate contactul fizic. Garda de viteza
   (SPEED_PROT_NORMAL in DB2) bloca exact actiunea ceruta la oprirea pe viteza,
   deci pe shutdown o ocolim deliberat. In rest - func_rs_ign_on,
   func_rs_str_on/off, func_trunk_release - garda ramine neatinsa.

   Simptom raportat de tester pe banc, 15.09.2026: la viteza > 0 pleca secventa
   de frina, dar nu se dadea pulsul de PTS. Identic in DB2
   (VW03_FUNC_PTS_OUT -> SPEED_PROT_NORMAL(OUT_2)), deci nu era o regresie de
   portare, ci o limitare mostenita.

   ATENTIE (v8, 15.09.2026): fiind fara garda, apelul e pazit in cod de
   func_rs_start_is_ours - apasam PTS la shutdown numai daca tot noi am urcat
   contactul. Altfel, o masina pornita normal de sofer si aflata in mers se
   oprea la apasarea butonului de Remote Start. Nu folosi macroul asta in alta
   parte fara aceeasi paza. */
#ifndef FUNC_PTS_SHUTDOWN_OUT
#define FUNC_PTS_SHUTDOWN_OUT(x)    gpio_iface_pts_1(x)
#endif

#ifndef FUNC_BRAKE_OUT
#define FUNC_BRAKE_OUT(x)           gpio_iface_brake(x)
#endif

#ifndef FUNC_RF_LOOP_RSR
#define FUNC_RF_LOOP_RSR(x)         gpio_iface_rf_loop(x)
#endif

/* Neutralizate deja in DB2 - le pastram no-op ca sa nu schimbam comportamentul. */
#ifndef FUNC_TRUNK_OUT
#define FUNC_TRUNK_OUT(x)           ;
#endif
#ifndef FUNC_RS_IGNITION2_OUT
#define FUNC_RS_IGNITION2_OUT(x)    ;
#endif
#ifndef FUNC_EIPS_ALARM_OUT
#define FUNC_EIPS_ALARM_OUT(x)      ;
#endif
#endif

