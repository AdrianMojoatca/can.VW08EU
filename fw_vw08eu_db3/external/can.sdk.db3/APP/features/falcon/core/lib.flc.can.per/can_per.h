/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: can_per.h 19497 2013-02-25 13:41:22Z martin.bouchard $
/*==========================================================================*/

#ifndef __CAN_PER_H__
#define __CAN_PER_H__

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "can.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

typedef enum
{
    CAN_PER_NO_ACTION   = 0       ,
    
    CAN_PER_COMPLETE    = 1u << 0 , 
    
    CAN_PER_DEV0        = 1u << 1 , 
    
    CAN_PER_DEV1        = 1u << 2 

}Can_Per_Flags ; 


//---------------------------------------------------------------------------//

typedef void *	Can_Per_Arg ;

typedef Can_Per_Flags	(*Can_Per_Func)( Can_Msg *, Can_Per_Arg );	

//---------------------------------------------------------------------------//

#define can_per_loop(func,arg,offet,repeat)  can_per_add(func,arg,offet,repeat,0)

//---------------------------------------------------------------------------//


/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

void    can_per_init       ( UInt8 size ) ; 
Boolean can_per_add        ( Can_Per_Func can_per_func , Can_Per_Arg can_per_arg , UInt32 offset ,  UInt32 repeat_period , UInt8  repeat_count );
Boolean can_per_remove     ( void *func_or_arg );

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/


/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/

/* SS4 Slice 7a: route can_per_init through the table to CORE. */
#if defined(FW_DB3_BUILD)
#include "core_contract_db3_boot_handshake.h"
#define can_per_init(size) (core_contract_db3_boot_api_table_get()->can_per_init_fn((uint32_t)(size)))
#endif

/* v2.0 VW08EU: rutare prin tabela de API (grupul VW08EU, 25 de sloturi). */
#if defined(FW_DB3_BUILD)
#include "core_contract_db3_boot_handshake.h"
#define can_per_add(f, a, o, p, c)       (core_contract_db3_boot_api_table_get()->can_per_add_fn((void*)(f), (void*)(a), (uint32_t)(o), (uint32_t)(p), (uint32_t)(c)))
#define can_per_remove(x)                (core_contract_db3_boot_api_table_get()->can_per_remove_fn((void*)(x)))
#endif /* FW_DB3_BUILD */

#endif



