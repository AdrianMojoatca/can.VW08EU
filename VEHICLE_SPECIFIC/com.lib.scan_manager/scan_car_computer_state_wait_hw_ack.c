/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: reg_atomic_init.c 14828 2012-05-28 17:30:47Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "scan_manager_private.h"
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
//! \desc  place description here
//!        

//! \assume place assumptions here

//! \notes  place notes here

//! \param  p1        describe the p1 paramter here
//! \param  p2        describe the p2 paramter here

//! \return r         describe the returned value here

//--------------------------------------------------------------------------//


void scan_car_computer_state_wait_hw_ack(Car_Computer *computer ,Scan_Internal_Event event , UInt8 *data ,UInt8 size)
{
    switch( event )
    {
        case TIMEOUT_RX_EXPIRED:
        {
            scan_car_computer_set_state(computer,CONNECTED);/// reset the state to power on
            
            if(computer->callback)
            {
                computer->callback(EVT_RX_TIMEOUT,NULL,0);
            }                        
        }
        break;
        
        case CAN_FRAME_RECEIVED:
        {
            if((data[0]&0xF0)==0x30)
            {
                scan_car_computer_set_state(computer,CF_PENDING);

                computer->timeout_tx = CAR_COMPUTER_TX_TIMEOUT ;                                
            }
            else
            {
                scan_car_computer_set_state(computer,CONNECTED);/// reset the state to CONNECTED        
                
                if(computer->callback)
                {
                    computer->callback(EVT_ACK_ERROR,NULL,0);
                }                
            }
        }break;

        default : break;
    }
}
        
//--------------------------------------------------------------------------//
         
