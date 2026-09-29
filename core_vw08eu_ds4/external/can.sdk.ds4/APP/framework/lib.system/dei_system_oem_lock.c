/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: dei_system_oem_lock.c 22422 2013-09-02 11:59:24Z florin.olariu $
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

void dei_system_oem_lock( void *pld  )
{		  
	UInt8 pld_val;
	if( get_ss_panic()  )
	{
		dei_system_panic_stop(); 

		return ;
	}
  
	if( (get_ign_status() && !get_rs_gwr()) )return ; 		
	
  if( !get_ss_lock() )
	{
		notify_send( NOTIFY_LOCK , DEI_NOTIFY_LOCK_PLD );
		
		dei_system_ch3_link_arm();		
		dei_system_ch4_link_arm();		
		dei_system_ch5_link_arm();		
		dei_system_ch6_link_arm();		
	}
				
	/* v20 VW08EU (25.09.2026): pulsul de Lock pe iesirea programabila, in Remote
	   Start. E simetricul lui dei_system_rf_lock() din acelasi director, care emite
	   aceeasi comanda cu acelasi payload cind incuierea vine de pe telecomanda
	   aftermarket.

	   DE CE era nevoie: in dei_out_init.c, INPUT_OEM_LOCK e inregistrat DOAR pe
	   dei_out_ground_when_locked si dei_out_gwa_on. Pulsul de Lock atirna de
	   INPUT_LOCK1_ON, pe care calea OEM nu il emitea niciodata - nici aici, nici in
	   dei_ss_lock_oem() sau dei_ss_arm_actions(), care schimba doar starea. De aici
	   asimetria observata pe masina: de pe telecomanda OEM iesirea de Unlock pulsa,
	   cea de Lock nu.

	   Atentie, pista falsa: in dei_out_lock() exista
	       if( DEI_OUT_PAYLOAD( payload ) & CMD_LOCK_OEM ) return ;
	   care pare sa fie explicatia, dar CMD_LOCK_OEM nu e atribuit NICAIERI in
	   arbore - e definit in cmd.h si testat doar acolo. Acel return e cod mort.

	   DE CE numai in Remote Start: in rest masina se incuie singura la apasarea
	   telecomenzii - de altfel exact de acolo aflam noi ca s-a incuiat, din cadrul
	   de pe magistrala - deci un puls in plus ar fi o a doua actionare, inutila.
	   In Remote Start BCM-ul nu raspunde la telecomanda, deci actionam noi.
	   Daca se dovedeste ca il vrem intotdeauna, se scoate doar conditia de mai jos.

	   Payload-ul e copiat intentionat din dei_system_rf_lock(), nu simplificat.
	   Asa Comfort Closure ramine tratat corect si daca instalatorul il activeaza de
	   pe web (FEAT_DEI_SYS05, acum 1 = No Comfort Closure): cu 1 se ajunge in
	   dei_out_lock_without_comfort(), iar cu 2 sau 3 bitul CMD_LOCK_WITH_COMFORT
	   duce la ramurile de confort. Fara acel bit s-ar intra in action_comfort_lock()
	   fara nicio ramura activa - zero puls, dar cu cc_is_in_progress setat, care ar
	   bloca si incuierile urmatoare. */
	if( get_rs_gwr() )
	{
		#define COMFORT	(((dei_feature_get(DEI_FEAT_COMFORT_CLOSURE)!=1) && !get_ss_one_time_bypass()) ? CMD_LOCK_WITH_COMFORT : 0)

		cmd_q_rs_insert_hhu_num( dei_sc_get_hhu_num() , INPUT_LOCK1_ON , 100 , ( COMFORT | CMD_LOCK_WITH_ARM ) ) ;

		#undef COMFORT
	}

	pld_val = *cmd_get_pld( pld );
	if(  pld_val != 3 )  // 3 = (CMD_LOCK_WITH_COMFORT | CMD_LOCK_WITH_ARM)
		set_ss_lock( TRUE );
}

//--------------------------------------------------------------------------//

