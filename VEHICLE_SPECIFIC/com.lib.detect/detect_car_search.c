/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: detect_car_search.c 4270 2012-11-07 06:19:27Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "detect_private.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

void detect_car_search( void )
{
	UInt8 i;
	Boolean can0_ok = TRUE ; 
    Boolean can1_ok = TRUE ;
	

    if( detect_init_nvfs()   ) return ;         
    
    led_detect_started();
    
    can_usr_init  (CAN0_ENABLE | CAN1_ENABLE) ;

    /* PORTARE VW08EU: in DB2 se chema can_sw_filter_disable(dev), din lib.flc.can.
       Filtrul software de ID-uri s-a mutat intre timp la nivelul can_usr
       (lib.flc.can.usr/can_usr_rx_filter.c), cu acelasi corp si aceeasi semantica:
       filter.enable == FALSE => trec toate mesajele. Redenumire 1:1.
       Apelurile ramin explicite chiar daca in SDK-ul modern implicitul e deja FALSE
       (can_usr_rx_dev0/1_init.c) - in DB2 era TRUE (can_rx_create.c:40), deci erau
       esentiale. Detectia are nevoie de RX nefiltrat ca sa asculte ID-uri
       necunoscute; asta e o cerinta, nu un implicit de imprumutat. */
    can_usr_rx_filter_disable(CAN_DEV_0);
    can_usr_rx_filter_disable(CAN_DEV_1);
    
    can_sync_init (CAN_DEV_0 , 5) ;
    can_sync_init (CAN_DEV_1 , 5) ;
    
    detect_evt = os_evt_create();

    detect_can_search();

    if( !VEHICLE_CAN_DETECTED )
    {
        TRACE( "\n\r[ DETECT : VEHICLE_CAN_ERROR %s]\n\r" , CAN_DEV_VEHICLE == CAN_DEV_1 ? "DEV_1" : "DEV_0" ) ;
		TRACE_CAN2( "\r\nC1ERR" );
	
        
        led_detect_err_can1();

        os_tsk_wait( 10000 ) ;

        goto end_detect_phase;
    }

    if( !detect_ign_search() ) 
    {        
        led_detect_err_invalid_ignition();

        os_tsk_wait( 10000 ) ;

        goto end_detect_phase;
    }

   led_detect_in_progress();

   TRACE( "\n\r[ DETECT : IGNITION_FOUND]\n\r") ;

   TRACE( "\n\r[ DETECT : CAN0_%s]\n\r" ,  car.bits.can0_detected ? "FOUND" : "NOT FOUND" ) ;
   TRACE( "\n\r[ DETECT : CAN1_%s]\n\r" ,  car.bits.can1_detected ? "FOUND" : "NOT FOUND" ) ;
	if( !car.bits.can0_detected ) TRACE_CAN2( "\r\nC0ERR" );
	if( !car.bits.can1_detected ) TRACE_CAN2( "\r\nC1ERR" );
   
   /* v19 VW08EU (23.09.2026): blocul VIN + PGM nu mai e comentat, ci guvernat de
      DEBUG_SKIP_PGM, ca sa fie UN SINGUR comutator si ca sa poata diferi pe
      platforme. Fisierul e comun DB3/DS4, dar fiecare platforma isi vede
      propriul config_dbg.h.
        DEBUG_SKIP_PGM == 0  -> detectie completa, cu VIN si PGM
        DEBUG_SKIP_PGM == 1  -> banc: se sare peste, ca pina acum

      CARE config_dbg.h ajunge aici, verificat in .dep pe 23.09.2026:
        DS4 - o singura copie, VEHICLE_SPECIFIC/lib.config.ds4/com.inc.config.fw.
        DB3 - DOUA copii sint in build, cu aceeasi garda __CONFIG_DBG_H__, deci
              intra doar prima: cea din vw08eu_db3_contract (ajunge aici prin
              detect_private.h -> detect.h -> core_framework.h -> can_usr.h ->
              ..._boot_handshake.h -> ..._api_table.h -> ..._dbg_config.h, care
              face #include "config_dbg.h" si il rezolva in directorul propriu).
              Pe DB3 valoarea se schimba deci in AMINDOUA fisierele - asta e si
              regula scrisa in capul lor: blocurile se tin identice.

      Garda de mai jos exista fiindca un macro nedefinit ar face "#if X == 0" sa
      fie ADEVARAT si ar activa blocul in tacere, pe amindoua platformele. */
#if !defined(DEBUG_SKIP_PGM)
   #error "DEBUG_SKIP_PGM nu e vizibil in detect_car_search.c - vezi config_dbg.h"
#endif

#if DEBUG_SKIP_PGM == 0

   if( !detect_vin_search() )
   {
        led_detect_err_invalid_bcm();
        
        os_tsk_wait( 10000 ) ;
        
        TRACE( "\n\r[ DETECT : VIN_ERROR]\n\r") ;

        goto end_detect_phase;
   }
   
   TRACE( "\n\r[ DETECT : VIN_FOUND]\n\r") ;

   /* detect_vin_search() pune el car.bits.car = GOLF7 (detect_vin_search.c:34);
      nu citeste VIN-ul, il presupune. */

   if( !detect_pgm() )
   {
        led_detect_err_invalid_bcm();
        
        os_tsk_wait( 10000 ) ;
        
        TRACE( "\n\r[ DETECT : PGM_ERROR]\n\r") ;
		TRACE_CAN2( "\r\nPGMERR" );
   }

#else

   /* BANC: fara VIN si fara PGM. detect_vin_search() era cel care facea
      car.bits.car = GOLF7; fara atribuirea asta car.bits.car ramine NO_CAR,
      poarta din detect_init_nvfs nu trece, detectia se reia la fiecare pornire
      si nu se ajunge niciodata la func_init(). */
   car.bits.car = GOLF7 ;

#endif /* DEBUG_SKIP_PGM */

   end_detect_phase:
   {
        nvfs_wr( nvfs_fw_data , &car  );
        nvram_usr_commit();
                      
        wdog_v_force_reset( WDOG_V_RESET_FIRMWARE , (char*)__FUNCTION__ , __LINE__ ) ;     
   }
}


//--------------------------------------------------------------------------//

