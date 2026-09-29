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
#include "core_framework.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/
#define SCAN_TRACE         TRACE
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

void scan_manager_trace_rx( Car_Computer *computer)
{
    UInt8 i ;

    SCAN_TRACE("[%s RX - ",computer->name);

    for(i=0;i<computer->rx_data_size;i++)
    {
        SCAN_TRACE("%02X ",computer->rx_data[i]);

        if( i%10 ) os_tsk_wait(10);
    }

    SCAN_TRACE("]\n\r");
}


//--------------------------------------------------------------------------//

void scan_manager_trace_tx( Car_Computer *computer)
{
    UInt8 i ;

    SCAN_TRACE("[%s TX - ",computer->name);

    for(i=0;i<computer->tx_data_size;i++)
    {
        SCAN_TRACE("%02X ",computer->tx_data[i]);

        if( i%10 ) os_tsk_wait(10);
    }

    SCAN_TRACE("]\n\r");
}


//--------------------------------------------------------------------------//

void scan_manager_trace_state(Car_Computer *computer)
{
    switch(computer->state)
    {
        case WAIT_HW_ANSWER : SCAN_TRACE("[%s WAIT_HW_ANSWER]\n\r",computer->name);break;
        case WAIT_HW_ACK    : SCAN_TRACE("[%s WAIT_HW_ACK   ]\n\r",computer->name);break;
        case CF_PENDING     : SCAN_TRACE("[%s CF_PENDING    ]\n\r",computer->name);break;
        case DISCONNECTED   : SCAN_TRACE("[%s DISCONNECTED  ]\n\r",computer->name);break;
        case CONNECTED      : SCAN_TRACE("[%s CONNECTED     ]\n\r",computer->name);break;

        default : break;
    }
}

//--------------------------------------------------------------------------//
