/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: detect_can_search.c 2134 2010-12-30 13:26:45Z florin.olariu $
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
void search_can0( Can_Msg *can_msg );
void search_can1( Can_Msg *can_msg );
void wakeup_obd_can( void );
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

void detect_can_search( void )
{
	car.bytes = 0 ; 

    os_evt_clr( detect_evt , 0xFFFF );

    can_usr_rx_reg_fast( CAN_DEV_0 , (Reg_Func)search_can0 ); 
    can_usr_rx_reg_fast( CAN_DEV_1 , (Reg_Func)search_can1 );
            
    os_evt_wait( detect_evt , CAN0_OK | CAN1_OK , 0 , SEARCH_TIMEOUT ) ;     

		#if DEBUG_SKIP_OBD_CAN_INIT == 1
		#else
			//wakeup OBD can
			{
				UInt8 i = 0;
				
				for( i = 0 ; i < 5 ; i ++){
					wakeup_obd_can();            
				}
			}
		#endif // DEBUG_SKIP_OBD_CAN_INIT
        
}


//--------------------------------------------------------------------------//

void search_can0( Can_Msg *can_msg )
{
    
    if( !car.bits.can0_detected )
    {
        car.bits.can0_detected = TRUE ;     
        
        os_evt_signal( detect_evt , CAN0_OK ) ;   
    }
}

//--------------------------------------------------------------------------//

void search_can1( Can_Msg *can_msg )
{
    if( !car.bits.can1_detected )
    {    
        car.bits.can1_detected = TRUE ; 
    
        os_evt_signal( detect_evt , CAN1_OK ) ;    
    }
}

//--------------------------------------------------------------------------//

void wakeup_obd_can( void )
{
         Can_Msg can_msg1 = {0x200 , 0 , 0 , 7 , 0 , 0x21 , 0xC0 , 0x00 , 0x10 , 0x00 , 0x03 , 0x01};
// const Can_Msg can_msg2 = {0x200 , 0 , 0 , 7 , 0 , 0x20 , 0xC0 , 0x00 , 0x10 , 0x00 , 0x03 , 0x01};    

    if( !VEHICLE_CAN_DETECTED )
    {
        can_usr_tx( CAN_DEV_OBD , (Can_Msg *)&can_msg1 , FALSE , 200);os_tsk_wait(500);
		
		can_msg1.data._8_[0] &= 0xFE; // set bit0 to 0

        can_usr_tx( CAN_DEV_OBD , (Can_Msg *)&can_msg1 , FALSE , 200);os_tsk_wait(500);
    }      
}

//--------------------------------------------------------------------------//
