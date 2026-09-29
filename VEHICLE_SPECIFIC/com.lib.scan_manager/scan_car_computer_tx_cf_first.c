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

void scan_car_computer_tx_cf_first(Car_Computer *computer)
{
    UInt8 i;

    computer->tx_frame_index = 0;
    computer->tx_data_index  = 0;
     
    computer->data_buf[0] = 0x10 | computer->tx_frame_index;
    computer->data_buf[1] =        computer->tx_data_size;
    

//     for(i = 0 ; i < 6 ; i++)
//     {
//         computer->data_buf[i+2] = computer->tx_data[computer->tx_data_index++];
//     }
	memcpy(&computer->data_buf[0+2] , &computer->tx_data[computer->tx_data_index] , 6) ;
	computer->tx_data_index += 6;

    scan_manager.stc_timeout = STC_LIVE_TIMEOUT ;

    scan_manager.scan_tx( computer->hw_addr , computer->data_buf , 8);

    computer->tx_frame_index ++ ;
}

//--------------------------------------------------------------------------//

