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

void scan_manager_release_car_computer(Car_Computer* computer) 
{
    if(scan_manager.atomic)scan_manager.atomic(TRUE);
    {
        Car_Computer *curr , *temp ;

        for( curr = scan_manager.used_session.rhs ; curr != (Car_Computer*)&scan_manager.used_session ; curr = curr->rhs )
        {
            if( curr == computer )
            {
                temp = curr->rhs ; 
        			
                scan_manager_queue_unlink_car_computer( curr ) ; 
        
        	    scan_manager_queue_insert_car_computer( &scan_manager.free_session , curr ) ; 
        			
        		scan_car_computer_tx_cs(curr); /// close the session
                    
                if( curr->callback )
                {
                    curr->callback( EVT_DISCONNECTED , NULL , 0);
                }
                
                curr = temp->lhs ;                                     
            }
        }        
    }
    if(scan_manager.atomic)scan_manager.atomic(FALSE);
}

//--------------------------------------------------------------------------//
