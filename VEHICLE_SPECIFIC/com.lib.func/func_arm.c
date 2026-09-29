/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_arm.c 4229 2012-10-18 18:45:04Z florin.olariu $
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
static Can_Sync_Flags arm_3D0( Can_Msg *can_msg );
static Can_Sync_Flags arm_184( Can_Msg *can_msg );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean arm_done = FALSE ; 
static Boolean doors_status = FALSE ; 
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_arm_start( void  )
{
    UInt8 i;
	Can_Msg lock_can_msg;
	Can_Msg wake_lock_can_msg;	
       
	
	func_can_wake_enter();

	//Can_Sync_Id can_id  
	//UInt32      sync_offset   
	//UInt32      sync_on     
	//UInt32      sync_off  
	//UInt8       sync_repeat 
        	    
	TRACE_CAN2( "\r\nARM" );

	if ( !get_ign_status() ) // 11_09_2017
	{
    	can_sync_loop (FUNC_CAN_DEV ,(Can_Sync_Func)arm_3D0,(Can_Sync_Arg)NULL,0x3D0 ,0 );
		ATOMIC( arm_done = FALSE ); 
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
			
		ATOMIC( arm_done = TRUE );
		
		return;	
	}
    TRACE_FUNC("\n\r[ARM_START]\n\r");
}

/*--------------------------------------------------------------------------*/

Boolean func_arm_exec( void )
{    
	return arm_done  ; 	
}

/*--------------------------------------------------------------------------*/

void func_arm_stop( void )
{
    func_can_wake_leave();	

    can_sync_remove(FUNC_CAN_DEV , arm_3D0);
	
	// mih	 //ADI - 04_12_2017 - mih
	//timeout_start(wnd_rol_tmr);
	//OUT_RELAY2(TRUE)    ;
	// end mih 	
    
    if(arm_done)
    {
        TRACE_FUNC("\n\r[ARM_STOP]\n\r");
    }
    else
    {
		TRACE_FUNC("\n\r[ARM_ERROR]\n\r");
    }
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags arm_3D0( Can_Msg *can_msg )
{            
	doors_status = get_door_status();
	
    ATOMIC(arm_done = ((can_msg->data._8_[0]&0x02)==0x02) || doors_status ); // do not wait lock confirmation if door is opened, because we get wnd roll-up
    
    can_msg->data._8_[0] |= 0x10 ;
       
    return  arm_done ? CAN_SYNC_COMPLETE | CAN_SYNC_TX : CAN_SYNC_TX ; 
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags arm_184( Can_Msg *can_msg )
{            
    ATOMIC( arm_done = (((can_msg->data._8_[1]&0x50))));
            
    return  arm_done ? CAN_SYNC_COMPLETE : CAN_SYNC_NO_ACTION ; 
}

/*--------------------------------------------------------------------------*/

// mih
void wnd_roll_tmr_func ( void *arg)
{
	//OUT_RELAY2(FALSE);
}

