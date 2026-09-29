/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_trunk_release.c 3491 2011-10-12 07:13:08Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "func_private.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
Boolean tr_ign_is_on = FALSE;

/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_trunk_release_start( void )
{
    // following code was removed in version 1_14 - 19_03_2018
	/*if ( get_ign_status() ) tr_ign_is_on = TRUE;
	else tr_ign_is_on = FALSE;
	if (!tr_ign_is_on)
	{
		FUNC_PTS_OUT(TRUE);
		os_tsk_wait(500);
		FUNC_PTS_OUT(FALSE);
	}
	*/ 
	
	TRACE_FUNC("\n\r[TRUNK_START]\n\r"); 		
}

/*--------------------------------------------------------------------------*/

Boolean func_trunk_release_exec( void )
{
    // ADI - vers. 1.09 - 14.12.2016
	Can_Msg unlock_can_msg;
	Can_Msg wake_unlock_can_msg;

	if ( get_ign_status() )
	{
		
		os_tsk_wait(1000);
		
		wake_unlock_can_msg.data._8_[0]=0x00;
		wake_unlock_can_msg.data._8_[1]=0x00;
		wake_unlock_can_msg.data._8_[2]=0x00;
		wake_unlock_can_msg.data._8_[3]=0x00;
		wake_unlock_can_msg.data._8_[4]=0x00;
		wake_unlock_can_msg.data._8_[5]=0x00;
		wake_unlock_can_msg.data._8_[6]=0x00;
		wake_unlock_can_msg.data._8_[7]=0x00;
		wake_unlock_can_msg.id = 0x000;
		wake_unlock_can_msg.length = 8;
		wake_unlock_can_msg.is_ext = 0;
		wake_unlock_can_msg.is_rtr = 0;
		can_usr_tx( FUNC_CAN_DEV , &wake_unlock_can_msg , FALSE , 100 );
	
		os_tsk_wait(200);
	
		// ADI - 13.12.2016
		unlock_can_msg.data._8_[0]=0x00;
		unlock_can_msg.data._8_[1]=0x08;
		unlock_can_msg.data._8_[2]=0x00;
		unlock_can_msg.data._8_[3]=0x00;
		unlock_can_msg.data._8_[4]=0x8C;
		unlock_can_msg.data._8_[5]=0x00;
		unlock_can_msg.data._8_[6]=0x00;
		unlock_can_msg.data._8_[7]=0x00;
		unlock_can_msg.id = 0x3D0;
		unlock_can_msg.length = 8;
		unlock_can_msg.is_ext = 0;
		unlock_can_msg.is_rtr = 0;
		can_usr_tx( FUNC_CAN_DEV , &unlock_can_msg , FALSE , 100 );
	
		// following code was removed in version 1_14 - 19_03_2018
		/*if ( !tr_ign_is_on )
		{
			FUNC_PTS_OUT(TRUE);
			os_tsk_wait(500);
			FUNC_PTS_OUT(FALSE);
			tr_ign_is_on = FALSE;
		}*/

		return TRUE;
	}
	
	
	// and addition

	
	return FALSE;
}

/*--------------------------------------------------------------------------*/

void func_trunk_release_stop( void )
{
    TRACE_FUNC("\n\r[TRUNK_STOP]\n\r"); 		
}

/*--------------------------------------------------------------------------*/
