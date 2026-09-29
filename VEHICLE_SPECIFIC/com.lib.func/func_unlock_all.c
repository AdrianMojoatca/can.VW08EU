/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_unlock_all.c 3497 2011-10-12 12:25:49Z florin.olariu $
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
static Can_Sync_Flags unlock_all_3D0  ( Can_Msg *can_msg );
static Can_Sync_Flags unlock_all_184  ( Can_Msg *can_msg );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean unlock_all_done ;
#if UNLOCK_METHOD1 == 1
static SInt8 unlock_all_counter = 0 ; 
#endif
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_unlock_all_start( void  )
{
    
	
	ATOMIC( unlock_all_done = FALSE );
	
#if UNLOCK_METHOD1 == 1
	unlock_all_counter = 3;
#endif
        
    func_can_wake_enter();
    	    
    if(func_do_unlock_all)
    {
		TRACE_CAN2( "\r\nUNLCKA" );
	
		can_sync_loop(FUNC_CAN_DEV ,(Can_Sync_Func)unlock_all_3D0,(Can_Sync_Arg)NULL,0x3D0,0);
		can_sync_loop(FUNC_CAN_DEV ,(Can_Sync_Func)unlock_all_184,(Can_Sync_Arg)NULL,0x184,0);
    }
    else
    {
		TRACE_CAN2( "\r\nUNLCKS" );

		unlock_all_done = TRUE;
    }

    TRACE_FUNC("\n\r[UNLOCK_START]\n\r"); 		
}

/*--------------------------------------------------------------------------*/

Boolean func_unlock_all_exec( void  )
{    
    return unlock_all_done ; 	
}

/*--------------------------------------------------------------------------*/

void func_unlock_all_stop( void )
{
    func_can_wake_leave();	

    can_sync_remove( FUNC_CAN_DEV , unlock_all_3D0 ); 
    can_sync_remove( FUNC_CAN_DEV , unlock_all_184 );
	
#if UNLOCK_METHOD1 == 1
	unlock_all_counter = 0;
#endif
    
    TRACE_FUNC("\n\r[UNLOCK_STOP]\n\r"); 		    
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags unlock_all_3D0( Can_Msg *can_msg )
{            
#if UNLOCK_METHOD1 == 1
	if( unlock_all_counter > 0)
		unlock_all_counter--;
	else
		unlock_all_done = TRUE;
#endif

	can_msg->data._8_[0] |= 0xA0 ;

    return  unlock_all_done ? CAN_SYNC_COMPLETE : CAN_SYNC_TX  ; 
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags unlock_all_184( Can_Msg *can_msg )
{    
#if UNLOCK_METHOD1 == 1
#else
    ATOMIC( unlock_all_done = (can_msg->data._8_[1]&0xF0 == 0xA0) );
#endif

    return unlock_all_done  ? CAN_SYNC_COMPLETE:CAN_SYNC_NO_ACTION ;
}

/*--------------------------------------------------------------------------*/
