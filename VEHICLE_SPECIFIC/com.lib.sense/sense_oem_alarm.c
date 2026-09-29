/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: sense_oem.c 4227 2012-10-18 13:32:07Z florin.olariu $
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
static Timeout_Q_Status mechanical_key_timeout( void );

static void             trace_can_msg(Can_Msg *can_msg);
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean          is_mechanical_key  = FALSE             ;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//
/* PORTARE VW08EU: pe thin-FW, _arm/_disarm/_trigger sint rutate prin tabela de API,
   deci sint macrouri fara parametri (sense_rs_can.h, blocul FW_DB3_BUILD / FW_DS4_BUILD). Un
   prototip `nume ( void )` ar fi o invocare de macrou cu un argument -> eroare #55.
   Declaratiile ramin pentru un build ne-thin. _arming NU e rutat (nu e apelat de
   nicaieri in VW08EU), deci prototipul lui sta neconditionat. */
#if !defined(FW_DB3_BUILD) && !defined(FW_DS4_BUILD)
void          sense_rs_can_oem_alarm_disarm  ( void );
void          sense_rs_can_oem_alarm_arm     ( void );
void          sense_rs_can_oem_alarm_trigger ( void );
#endif
void          sense_rs_can_oem_alarm_arming  ( void );

void sense_oem_alarm_583( Can_Msg *can_msg )
{
	Boolean oem_alarm_is_armed = (DATA[3]&0x80) == 0x80;
	
	if( oem_alarm_is_armed )
	{
		sense_rs_can_oem_alarm_arm();
	}
	else
	{
		sense_rs_can_oem_alarm_disarm();
	}
	
	if( ((DATA[3]&0x0F) == 0x01) || // alarm triggered driver door
		((DATA[3]&0x0F) == 0x02) || // alarm triggered front pass door
		((DATA[3]&0x0F) == 0x03) || // alarm triggered 
		((DATA[3]&0x0F) == 0x04) || // alarm triggered 
		((DATA[3]&0x0F) == 0x05)    // alarm triggered 
		|| ( oem_alarm_is_armed && get_hood_status())   // alarm triggered by hood 
	  )
	{
		sense_rs_can_oem_alarm_trigger();
	}
}

//--------------------------------------------------------------------------//
