/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: reg_private.h 14828 2012-05-28 17:30:47Z florin.olariu $
/*==========================================================================*/

#ifndef __SCAN_MANAGER_PRIVATE_H__
#define __SCAN_MANAGER_PRIVATE_H__

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "scan_manager.h"
#include "core_framework.h"
#include <string.h>
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*-------------------------------------------------------------------------*/

#define CAR_COMPUTER_MAX          4
#define CAR_COMPUTER_TX_TIMEOUT   5
#define STC_DELAY                 4
#define STC_LIVE_TIMEOUT          100
#define STC_COUNT_POWER_READY     5

/*-------------------------------------------------------------------------*/

typedef enum
{
    CAN_FRAME_RECEIVED , 
    TIMEOUT_TX_EXPIRED ,
    TIMEOUT_RX_EXPIRED

}Scan_Internal_Event;

/*-------------------------------------------------------------------------*/

typedef enum
{
    WAIT_HW_ANSWER , 
    WAIT_HW_ACK    ,
    CF_PENDING     ,
    DISCONNECTED   ,
    CONNECTED      ,

}Scan_State;

/*-------------------------------------------------------------------------*/

typedef struct s_Car_Computer
{
    struct s_Car_Computer *rhs;
    struct s_Car_Computer *lhs;

#if DEBUG_TEXT == 1
    const UInt8 *name;
#endif
    
    UInt16 hw_addr ;
    UInt16 sc_addr ;

    Car_Computer_Callback callback;
    
    Scan_State    state   ;
        
    UInt16   timeout_rx;
    UInt16   timeout_tx;

    UInt8    data_buf[8];

    UInt16   timeout_answer;
                       
    UInt8*  tx_data;
    UInt8   tx_data_size;
    UInt8   tx_data_index;
    UInt8   tx_frame_index;
    
    UInt8*  rx_data;
    UInt8   rx_data_size;
    UInt8   rx_data_index; 
    UInt8   rx_frame_index;  
    
}Car_Computer ;

/*-------------------------------------------------------------------------*/

typedef struct s_Computer_List
{
    struct s_Car_Computer *rhs;
    struct s_Car_Computer *lhs;    
}Computer_List;    

/*-------------------------------------------------------------------------*/

typedef struct s_Scan_Manager
{
    Computer_List used_session;
    Computer_List free_session;
        
    UInt8   power_requests;
        
    UInt16  stc_timeout  ;
    UInt8   stc_count    ;

    Scan_Tx         scan_tx;
    Scan_Atomic     atomic ;
            
}Scan_Manager;

/*-------------------------------------------------------------------------*/



 

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

void scan_manager_tick_state( void );
void scan_manager_tick_power( void );
void scan_manager_tx_stc    ( void );

void scan_car_computer_set_state           (Car_Computer *computer,Scan_State state);
void scan_car_computer_state_handler       (Car_Computer *computer ,Scan_Internal_Event event , UInt8 *data ,UInt8 size);
void scan_car_computer_state_wait_hw_ack   (Car_Computer *computer ,Scan_Internal_Event event , UInt8 *data ,UInt8 size);
void scan_car_computer_state_cf_pending    (Car_Computer *computer ,Scan_Internal_Event event , UInt8 *data ,UInt8 size);
void scan_car_computer_state_wait_hw_answer(Car_Computer *computer ,Scan_Internal_Event event , UInt8 *data ,UInt8 size);

void          scan_car_computer_tx_sf      (Car_Computer* computer);
void          scan_car_computer_tx_cf_first(Car_Computer* computer);
Boolean       scan_car_computer_tx_cf_next (Car_Computer* computer);
void          scan_car_computer_tx_ack     (Car_Computer* computer);
void          scan_car_computer_tx_cs      (Car_Computer* computer);

void          scan_manager_queue_init               (Computer_List* list);
void          scan_manager_queue_insert_car_computer(Computer_List* list , Car_Computer* computer);
Car_Computer* scan_manager_queue_remove_car_computer(Computer_List* list);
void          scan_manager_queue_unlink_car_computer(Car_Computer *computer );


void scan_manager_trace_rx   ( Car_Computer *computer);
void scan_manager_trace_tx   ( Car_Computer *computer);
void scan_manager_trace_state( Car_Computer *computer);

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
extern Scan_Manager scan_manager;
/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/

#endif

