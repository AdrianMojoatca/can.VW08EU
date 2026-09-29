/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: dei_system_oem_unlock.c 17934 2012-10-29 19:23:36Z martin.bouchard $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "dei_system_private.h"
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

void dei_system_oem_unlock( void  )
{		
	if( get_ss_panic()  )
	{
		dei_system_panic_stop(); 

		return ;
	}
							
	notify_send( NOTIFY_UNLOCK , DEI_NOTIFY_UNLOCK_PLD );

  	if( get_ss_lock() )
	{
		dei_system_ch3_link_disarm();		
		dei_system_ch4_link_disarm();		
		dei_system_ch5_link_disarm();		
		dei_system_ch6_link_disarm();		
	}
	
	/* v21 VW08EU (25.09.2026): pulsul de Unlock pe iesirea programabila, in Remote
	   Start. Perechea modificarii din dei_system_oem_lock.c (v20) si simetricul lui
	   dei_system_rf_unlock().

	   Aceeasi cauza ca la Lock: in dei_out_init.c, INPUT_OEM_UNLOCK e inregistrat
	   doar pe dei_out_ground_when_locked si dei_out_gwa_off. Pulsul de Unlock atirna
	   de INPUT_UNLOCK_ALL1_ON / INPUT_UNLOCK_DRIVER1_ON, pe care calea OEM nu le
	   emitea niciodata. Initial am crezut ca unlock-ul mergea deja in Remote Start;
	   testul pe masina din 25.09.2026 a aratat ca nu mergea nici el.

	   Alegerea "toate usile" vs. "numai usa soferului" urmeaza aceeasi optiune DEI
	   ca in dei_system_rf_unlock() - FEAT_DEI_SYS35, acum 2 = Off, deci toate usile.
	   Macroul e definit local fiindca al lui rf_unlock nu e vizibil aici: in
	   amalgamarea din system.c fisierul asta e inclus la linia 14, iar rf_unlock la
	   31. Din acelasi motiv nu preluam si bitul CMD_UNLOCK_USER2 - el depinde de
	   parametrul "remote", pe care dei_system_oem_unlock() nu il primeste, si
	   oricum distinge intre telecomenzile aftermarket, nu intre cheile OEM.

	   Nu reproducem nici mecanismul de "al doilea unlock" (dei_system_2nd_unlock si
	   temporizatorul lui): apartine dublei apasari pe telecomanda aftermarket. */
	if( get_rs_gwr() )
	{
		#define OEM_UNLOCK_DRIVER_PRIORITY_OFF  ( dei_feature_get( DEI_FEAT_DRIVER_DOOR_UNLOCK_PRIORITY ) == 2 )

		if( OEM_UNLOCK_DRIVER_PRIORITY_OFF )
			cmd_q_rs_insert_hhu_num( dei_sc_get_hhu_num() , INPUT_UNLOCK_ALL1_ON    , 100 , CMD_UNLOCK_WITH_DISARM ) ;
		else
			cmd_q_rs_insert_hhu_num( dei_sc_get_hhu_num() , INPUT_UNLOCK_DRIVER1_ON , 100 , CMD_UNLOCK_WITH_DISARM ) ;

		#undef OEM_UNLOCK_DRIVER_PRIORITY_OFF
	}

	set_ss_lock( FALSE );
	if(dei_system_locked_by_ign_control)
	  dei_system_locked_by_ign_control = FALSE;
	if(dei_system_mts_by_passive_locking)
	  dei_system_mts_by_passive_locking = FALSE;
}

//--------------------------------------------------------------------------//

