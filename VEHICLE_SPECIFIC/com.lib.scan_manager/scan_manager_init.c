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
static Car_Computer car_computer_table[CAR_COMPUTER_MAX];
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

void scan_manager_init( Scan_Tx scan_tx , Scan_Atomic atomic)
{
    scan_manager.power_requests = 0;        
    scan_manager.stc_timeout    = 0; 
    scan_manager.stc_count      = 0;
    scan_manager.scan_tx        = scan_tx;
    scan_manager.atomic         = atomic ;

    scan_manager_queue_init( &scan_manager.used_session );
    scan_manager_queue_init( &scan_manager.free_session );

    ///init the scan objects
    {
        UInt8 i = 0;

        for( i = 0; i < CAR_COMPUTER_MAX ; i ++)
        {                        
//             car_computer_table[i].hw_addr       =0;
//             car_computer_table[i].sc_addr       =0;
//             car_computer_table[i].callback      =0;            
//             car_computer_table[i].state         =DISCONNECTED;       
//             car_computer_table[i].timeout_rx    =0;
//             car_computer_table[i].timeout_tx    =0;
//             car_computer_table[i].timeout_answer=0;                       
//             car_computer_table[i].tx_data       =0;
//             car_computer_table[i].tx_data_size  =0;
//             car_computer_table[i].tx_data_index =0;
//             car_computer_table[i].tx_frame_index=0;    
//             car_computer_table[i].rx_data       =0;
//             car_computer_table[i].rx_data_size  =0;
//             car_computer_table[i].rx_data_index =0; 
//             car_computer_table[i].rx_frame_index=0;  
			memset( &car_computer_table[i] , 0x00 , sizeof(car_computer_table[0]) ) ;
			car_computer_table[i].state         =DISCONNECTED;

            scan_manager_queue_insert_car_computer(&scan_manager.free_session,&car_computer_table[i]);                    
        }
    }
}

//--------------------------------------------------------------------------//
