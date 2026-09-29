/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: sense_ignition.c 4227 2012-10-18 13:32:07Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "sense_private.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
Boolean unlock_ign_status;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

/* Cite esantioane trimitem ca sa fortam imediat starea de frina de mina.
   Debouncer-ul din CORE (idb_debounce, sense_rs_can_cfg->handbrake_debounce = 3)
   cere N esantioane CONSECUTIVE ca sa comute. Trimitem N+1, la fel ca
   sense_rs_handbrake_reset() din CORE, care nu e rutat spre FW. */
#define HB_FORCE_SAMPLES   4U

void sense_ignition_3C0( Can_Msg *can_msg )
{
    sense_rs_ign_can( CAST_BOOL(DATA[2]&0x02) ); 
	if ( CAST_BOOL(DATA[2]&0x02) )  unlock_ign_status = TRUE;
	else unlock_ign_status = FALSE; 

	/* PORTARE VW08EU 31.08.2026 - regula: cit timp contactul e OFF pe CAN,
	   frina de mina TREBUIE raportata ON. Fara date de la vehicul raportam
	   conservator, ca sa nu porneasca RS-ul.

	   CORE aplica deja regula - sense_rs_handbrake_can() primeste
	   (instant_level || !get_ign_status()) - dar o evalueaza DOAR cind vine un
	   cadru 0x30B. In masina 0x30B curge continuu, deci fortarea prinde in 3
	   cadre si nu se vede; la banc, daca se opreste trimiterea lui 0x30B dupa
	   IGN OFF, nimeni nu mai reevalueaza si starea ramine cea veche.

	   Aici o facem determinista: la fiecare cadru de contact cu IGN OFF
	   impingem N+1 esantioane de TRUE, deci starea comuta imediat, indiferent
	   daca mai vine sau nu 0x30B. Apelul e idempotent - odata ce nivelul e
	   TRUE, idb_debounce nu mai produce front, deci nu se rescrie iesirea si nu
	   se retrimite nimic pe D2D. */
	if( !get_ign_status() )
	{
		UInt8 i;

		for( i = 0U ; i < HB_FORCE_SAMPLES ; i++ )
		{
			sense_rs_handbrake_can( TRUE );
		}
	}
}
	
//--------------------------------------------------------------------------//

