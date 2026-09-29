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

void scan_manager_tick_power( void )
{        
    if( scan_manager.power_requests)
    {
        if(!(--scan_manager.stc_timeout))
        {
            if(scan_manager.stc_count)
            {
                scan_manager.stc_count --;

                scan_manager.stc_timeout = STC_DELAY ;
            }
            else
            {
                scan_manager.stc_timeout = STC_LIVE_TIMEOUT ;
            }    

            scan_manager_tx_stc();                        
        }

        /// notify the active Scan objects about power on state
        {
            if( !scan_manager.stc_count )
            {
                Car_Computer *curr ;

                for( curr = scan_manager.used_session.rhs ; curr != (Car_Computer*)&scan_manager.used_session ; curr = curr->rhs )
                {
                    if(curr->state == DISCONNECTED)
                    {
                        scan_car_computer_set_state(curr,CONNECTED);
    
                        if( curr->callback )
                        {
                            curr->callback( EVT_CONNECTED , NULL , 0);
                        }                            
                    }                        
                }
            }
        }                              
    }
    else
    {
        /// notify the active Scan objects about power off state
        {
            Car_Computer *curr ;

            for( curr = scan_manager.used_session.rhs ; curr != (Car_Computer*)&scan_manager.used_session ; curr = curr->rhs )
            {
                if( curr->state != DISCONNECTED)
                {
                    scan_car_computer_tx_cs(curr); /// close the session
                    
                    scan_car_computer_set_state(curr,DISCONNECTED);
                    
                    if( curr->callback )
                    {
                        curr->callback( EVT_DISCONNECTED , NULL , 0);
                    }
                }                
            }
        }
    }                        
}

//--------------------------------------------------------------------------//
