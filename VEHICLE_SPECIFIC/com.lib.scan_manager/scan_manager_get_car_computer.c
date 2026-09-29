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

Car_Computer* scan_manager_get_car_computer (const UInt8 *name,
                                             UInt16 hw_addr   , 
                                             UInt16 sc_addr   ,
                                             UInt8* read_buf  ,  
                                             Car_Computer_Callback callback)                                             
{
    Car_Computer *computer = NULL;
    
    if(scan_manager.atomic)scan_manager.atomic(TRUE);
    {    
        computer = scan_manager_queue_remove_car_computer(&scan_manager.free_session);
    
        if( computer )
        {            
            scan_manager_queue_insert_car_computer  (&scan_manager.used_session,computer);
                    
            computer->hw_addr       =hw_addr;
            computer->sc_addr       =sc_addr;
            computer->callback      =callback;
			
			TRACE_ZONE( computer->name = name );
			
            computer->timeout_rx    = 0;
            computer->timeout_tx    = 0;
            computer->timeout_answer= 100; //1000ms by default
            computer->rx_data       = read_buf;
            computer->rx_data_index = 0;
            computer->rx_data_size  = 0;
            computer->rx_frame_index= 0;
              
            scan_car_computer_set_state(computer,DISCONNECTED);        
            
            scan_manager.stc_timeout=1; 
        }
    }
    if(scan_manager.atomic)scan_manager.atomic(FALSE);

    return computer;
}

//--------------------------------------------------------------------------//
