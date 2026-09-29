/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: reg.h 14828 2012-05-28 17:30:47Z florin.olariu $
/*==========================================================================*/

#ifndef __SCAN_MANAGER_H__
#define __SCAN_MANAGER_H__

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "device.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

typedef enum
{
    EVT_DATA_RECEIVED   ,
    EVT_CONNECTED       ,
    EVT_DISCONNECTED    ,
    EVT_RX_TIMEOUT      ,
    EVT_RX_ERROR        ,
    EVT_RX_INDEX_ERROR  ,
    EVT_RX_UNKNOWN_ERROR,
    EVT_ACK_ERROR       ,    
}Scan_Evt;    

/*--------------------------------------------------------------------------*/

typedef void (*Car_Computer_Callback)( Scan_Evt event, UInt8* data , UInt8 size);
typedef void (*Scan_Tx              )( UInt16   id   , UInt8* data , UInt8 size);
typedef void (*Scan_Atomic          )( Boolean disable_interrupts );
/*--------------------------------------------------------------------------*/

typedef struct s_Car_Computer  Car_Computer ; 

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

void scan_manager_init     ( Scan_Tx scan_tx , Scan_Atomic atomic);
void scan_manager_tick     ( void );
void scan_manager_power_on ( void );
void scan_manager_power_off( void );

void scan_manager_can_event(UInt16 can_id , UInt8 *data , UInt8 size);

Car_Computer* scan_manager_get_car_computer (const UInt8 *name,
                                             UInt16 hw_addr   , 
                                             UInt16 sc_addr   ,
                                             UInt8* read_buf  , 
                                             Car_Computer_Callback callback);   

void scan_manager_release_car_computer(Car_Computer *computer);


void scan_car_computer_send(Car_Computer *computer,
                            UInt8* data, 
                            UInt8  size,                            
                            UInt16 timeout_answer);

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/


/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/

#endif



