/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_disarm_p.c 4160 2012-10-03 12:49:32Z florin.olariu $
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
static Can_Sync_Flags disarm_3D0  ( Can_Msg *can_msg );
static Can_Sync_Flags disarm_184  ( Can_Msg *can_msg );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean disarm_done = FALSE ; 
Boolean ign_is_on = FALSE;
static UInt8 count = 0;
extern Boolean unlock_ign_status;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_disarm_start( void  )
{
    	
	ATOMIC( disarm_done = FALSE );
    
    func_can_wake_enter();
    	
    //Can_Sync_Id can_id  
	//UInt32      sync_offset   
	//UInt32      sync_on     
	//UInt32      sync_off  
	//UInt8       sync_repeat 
        	    
    can_sync_loop(FUNC_CAN_DEV ,(Can_Sync_Func)disarm_3D0,(Can_Sync_Arg)NULL,0x3D0,0);
    
    TRACE_FUNC("\n\r[DISARM_P_START]\n\r"); 	
}

/*--------------------------------------------------------------------------*/

Boolean func_disarm_exec( void  )
{    

	return disarm_done ; 	
}

/*--------------------------------------------------------------------------*/

void func_disarm_stop( void  )
{
    func_can_wake_leave();
    	
    can_sync_remove(FUNC_CAN_DEV ,(Can_Sync_Func)disarm_3D0 );
    
    TRACE_FUNC("\n\r[DISARM_P_STOP(%s)]\n\r",disarm_done ? "ok":"error"); 		
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags disarm_3D0( Can_Msg *can_msg )
{    
    
    ATOMIC(disarm_done = ((can_msg->data._8_[0]&0x06)==0x00)) ;
	
    can_msg->data._8_[0] |= 0x08 ;             
    	    		
    return disarm_done ? CAN_SYNC_COMPLETE : CAN_SYNC_TX; 
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags disarm_184( Can_Msg *can_msg )
{
    ATOMIC( disarm_done = (can_msg->data._8_[1] == 0x02) || (can_msg->data._8_[1] == 0xAA) || (can_msg->data._8_[1] == 0xA0) );

    return  disarm_done ? CAN_SYNC_COMPLETE:CAN_SYNC_NO_ACTION  ; 
}

/*--------------------------------------------------------------------------*/

