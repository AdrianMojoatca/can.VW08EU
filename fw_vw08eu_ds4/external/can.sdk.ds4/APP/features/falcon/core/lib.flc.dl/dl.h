/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: out.h 1046 2010-04-08 12:51:11Z florin.olariu $
/*==========================================================================*/

#ifndef __DL_H__
#define __DL_H__

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "device.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/
typedef enum
{
    DL_OEM_DISARM_WITH_TRANSPONDER,
    DL_DRIVER_DOOR_PRIORITY       ,    
    DL_EVENT_CALLBACK             ,
    DL_OUT_CALLBACK               
}DL_CFG;

#if LIB_DL_DISABLE_ANALOG == 0
typedef enum
{
    DL_OUT_LOCK0   ,
    DL_OUT_LOCK1   ,
    DL_OUT_UNLOCK0 ,
    DL_OUT_UNLOCK1 ,
    DL_OUT_TRUNK0  ,
    DL_OUT_TRUNK1  ,
}DL_OUT;
#endif

typedef enum
{
    DL_EVT_ARM        ,
    DL_EVT_DISARM     ,
    DL_EVT_LOCK       ,
    DL_EVT_UNLOCK_DD  ,
    DL_EVT_UNLOCK_ALL ,
    DL_EVT_TRUNK      ,
}DL_EVT;

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
#if LIB_DL_DISABLE_ANALOG == 0
typedef void( *Dl_Out)(DL_OUT out);
#endif
typedef void( *Dl_Evt)(DL_EVT evt);
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/

void dl_init(void);
void dl_cfg (DL_CFG cfg,void* param);



/* PORTARE VW08EU - rutare prin tabela de API (bloc nou pe DS4). */
#if defined(FW_DS4_BUILD)
#include "core_contract_ds4_boot_handshake.h"
#define dl_cfg(cfg, param)               (core_contract_ds4_boot_api_table_get()->dl_cfg_fn((uint32_t)(cfg), (void*)(param)))
#endif /* FW_DS4_BUILD */
#endif


