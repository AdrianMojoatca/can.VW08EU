/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_lock.c 4137 2012-09-20 16:41:44Z florin.olariu $
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
static Can_Sync_Flags lock_3D0  ( Can_Msg *can_msg );
static Can_Sync_Flags lock_184  ( Can_Msg *can_msg );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean lock_done ;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_lock_start( void  )
{
    UInt8 i;
	Can_Msg lock_can_msg;
	Can_Msg wake_lock_can_msg;
	
	if ( !get_ign_status() ) // 11_09_2017
	{
    	ATOMIC( lock_done = FALSE );
        
    	func_can_wake_enter();
    	    
		TRACE_CAN2( "\r\nLOCK" );
	
    	can_sync_loop(FUNC_CAN_DEV ,(Can_Sync_Func)lock_3D0,(Can_Sync_Arg)NULL,0x3D0,0);
    	can_sync_loop(FUNC_CAN_DEV ,(Can_Sync_Func)lock_184,(Can_Sync_Arg)NULL,0x184,0);
    }

	else  // 11_09_2017
	{

		wake_lock_can_msg.data._8_[0]=0x00;
		wake_lock_can_msg.data._8_[1]=0x00;
		wake_lock_can_msg.data._8_[2]=0x00;
		wake_lock_can_msg.data._8_[3]=0x00;
		wake_lock_can_msg.data._8_[4]=0x00;
		wake_lock_can_msg.data._8_[5]=0x00;
		wake_lock_can_msg.data._8_[6]=0x00;
		wake_lock_can_msg.data._8_[7]=0x00;
		wake_lock_can_msg.id = 0x000;
		wake_lock_can_msg.length = 8;
		wake_lock_can_msg.is_ext = 0;
		wake_lock_can_msg.is_rtr = 0;
		can_usr_tx( FUNC_CAN_DEV , &wake_lock_can_msg , FALSE , 100 );
	
		os_tsk_wait(200);
		
	
		
		lock_can_msg.data._8_[0]=0x40;
		lock_can_msg.data._8_[1]=0x00;
		lock_can_msg.data._8_[2]=0x00;
		lock_can_msg.data._8_[3]=0xC0;
		lock_can_msg.data._8_[4]=0x80;
		lock_can_msg.data._8_[5]=0x00;
		lock_can_msg.data._8_[6]=0x00;
		lock_can_msg.data._8_[7]=0x00;
		lock_can_msg.id = 0x3D0;
		lock_can_msg.length = 8;
		lock_can_msg.is_ext = 0;
		lock_can_msg.is_rtr = 0;
		can_usr_tx( FUNC_CAN_DEV , &lock_can_msg , FALSE , 100 );

			
		ATOMIC( lock_done = TRUE );	

		return;	
	}
	
	
	

    TRACE_FUNC("\n\r[LOCK_START]\n\r"); 		
}

/*--------------------------------------------------------------------------*/

Boolean func_lock_exec( void  )
{    
    return lock_done ; 	
}

/*--------------------------------------------------------------------------*/

void func_lock_stop( void )
{
    func_can_wake_leave();	

    can_sync_remove( FUNC_CAN_DEV , lock_3D0 ); 
    can_sync_remove( FUNC_CAN_DEV , lock_184 );

    TRACE_FUNC("\n\r[LOCK_STOP(%s)]\n\r",lock_done ? "ok":"error"); 	; 		
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags lock_3D0( Can_Msg *can_msg )
{            
    can_msg->data._8_[0] |= 0x40 ; 		

    return  lock_done ? CAN_SYNC_COMPLETE:CAN_SYNC_TX ; 
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags lock_184( Can_Msg *can_msg )
{    
    ATOMIC( lock_done = (((can_msg->data._8_[1]&0x50))));

    return lock_done ? CAN_SYNC_COMPLETE : CAN_SYNC_NO_ACTION ;
}

/*--------------------------------------------------------------------------*/
