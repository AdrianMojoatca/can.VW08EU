/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: tmr.h 33257 2015-08-19 12:45:49Z martin.bouchard $
/*==========================================================================*/

#ifndef __TMR_H__
#define __TMR_H__

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/

#include "os.h"

/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

typedef struct s_Tmr * Tmr ;

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

void    tmr_init    (void) ;
Tmr     tmr_create  (void) ;
void    tmr_update  (Tmr tmr, UInt32 tick_count, Boolean use_fiq_tc) ;
UInt8   tmr_start   (Tmr tmr, UInt32 tick_count, Boolean use_fiq_tc) ;
UInt8   tmr_start_fiq_rx  (Tmr tmr, UInt32 tick_count) ;
void    tmr_dis     (Tmr tmr) ;
void    tmr_ena     (Tmr tmr) ;
Boolean tmr_int_sta (Tmr tmr) ;
Boolean tmr_is_ena  (Tmr tmr) ;

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/

/* v2.1 VW08EU: rutare prin tabela de API (grupul de ambreiaj). */
#if defined(FW_DB3_BUILD)
#include "core_contract_db3_boot_handshake.h"
#define tmr_create()          ((Tmr)core_contract_db3_boot_api_table_get()->tmr_create_fn())
#define tmr_ena(t)            (core_contract_db3_boot_api_table_get()->tmr_ena_fn((void*)(t)))
#define tmr_dis(t)            (core_contract_db3_boot_api_table_get()->tmr_dis_fn((void*)(t)))
#define tmr_update(t, v, fq)  (core_contract_db3_boot_api_table_get()->tmr_update_fn((void*)(t), (uint32_t)(v), (uint32_t)(fq)))
#define tmr_int_sta(t)        (core_contract_db3_boot_api_table_get()->tmr_int_sta_fn((void*)(t)))
#endif /* FW_DB3_BUILD */

#endif


