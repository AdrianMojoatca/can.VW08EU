/*==========================================================================*/
// $Id: timeout_f.h 33257 2015-08-19 12:45:49Z martin.bouchard $
/*==========================================================================*/

#ifndef _TIMEOUT_F_H_
#define _TIMEOUT_F_H_

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/

#include "exec.h"
#include "timeout_private.h"

/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

Timeout timeout_f_create (UInt32  timeout, Exec_Func func) ;
void	timeout_f_update (Timeout timeout, Exec_Func new_func) ;

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

/* SS4 Slice 5: thin FW creates a one-shot timeout (with an FW callback) via the
   API table -> CORE's timeout system. */
#if defined(FW_DB3_BUILD)
#include "core_contract_db3_boot_handshake.h"
#define timeout_f_create(to, func) \
        ((Timeout)core_contract_db3_boot_api_table_get()->timeout_f_create_fn((uint32_t)(to), (core_db3_fn_void_ptr)(func)))
#endif

#endif



