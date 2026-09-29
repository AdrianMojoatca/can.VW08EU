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

void scan_car_computer_send(Car_Computer *computer,
                            UInt8* data, 
                            UInt8  size,                            
                            UInt16 timeout_answer)
{
    
    if(scan_manager.atomic)scan_manager.atomic(TRUE);
    {
        if( computer && computer->state == CONNECTED )
        {    
            UInt8 i = 0;
                
            computer->tx_data_size   = size;
            computer->tx_data        = data;            
            computer->timeout_answer = timeout_answer;
            computer->timeout_rx     = timeout_answer;

			TRACE_ZONE(	scan_manager_trace_tx(computer) );
    
            if( size < 7 )
            {
                scan_car_computer_set_state(computer,WAIT_HW_ANSWER) ;
                
                scan_car_computer_tx_sf(computer);
            }
            else
            {
                scan_car_computer_set_state(computer,WAIT_HW_ACK) ; 
    
                scan_car_computer_tx_cf_first(computer);
            }
        }            
    }
    if(scan_manager.atomic)scan_manager.atomic(FALSE);
}

//--------------------------------------------------------------------------//
