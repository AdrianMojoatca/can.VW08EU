/*<#======================================================================#>*/
/*              CONTRACT WRAPPER PENTRU CONFIG_CAN.H (DB3)              */
/*<#======================================================================#>*/

#ifndef __CAN_CONFIG_WRAPPER_H__
#define __CAN_CONFIG_WRAPPER_H__

#include <stdint.h>

#define __CONFIG_CAN_H__
#define CORE_CONFIG_CAN_VERSION_1_01

#define CORE_CAN_DEV_0_FT_SW_DEFAULT          (1U)
#define CORE_CAN_DEV_0_SPEED_DEFAULT          (6U)
#define CORE_CAN_DEV_0_RX_BUFFER_SIZE_DEFAULT (5U)
#define CORE_CAN_DEV_0_TX_BUFFER_SIZE_DEFAULT (5U)
/* v4 VW08EU (26.08.2026): CAN1 la 500 kbit/s, ca in DB2.
   Dovada: DBALL_egal_DB2\code\common\inc.com.config\config_can.h -> CAN_DEV_1_SPEED = 2
   Ladder-ul pentru CAN1 (can_usr_private.h): 0 = 125K, 1 = 250K, 2 = 500K.
   Era 0 (125K), din seed-ul FRD37EU. Pe CAN1 stau func, sense si detectia de
   contact (CAN_DEV_FUNC_SENSE / CAN_DEV_DET_IGN din detect.h), deci la 125K
   modulul nu recepta nimic de pe magistrala vehiculului. */
#define CORE_CAN_DEV_1_SPEED_DEFAULT          (2U)
#define CORE_CAN_DEV_1_RX_BUFFER_SIZE_DEFAULT (5U)
#define CORE_CAN_DEV_1_TX_BUFFER_SIZE_DEFAULT (5U)

#define CAN_DEV_0_FT_SW          CORE_CAN_DEV_0_FT_SW_DEFAULT
#define CAN_DEV_0_SPEED          CORE_CAN_DEV_0_SPEED_DEFAULT
#define CAN_DEV_0_RX_BUFFER_SIZE CORE_CAN_DEV_0_RX_BUFFER_SIZE_DEFAULT
#define CAN_DEV_0_TX_BUFFER_SIZE CORE_CAN_DEV_0_TX_BUFFER_SIZE_DEFAULT
/* ATENTIE, asimetrie reala intre cele doua canale:
     dev0 -> can_usr_rx_dev0_init.c foloseste CAN_CFG_CAN_DEV_0_* , adica
             get_can_dev_0_*() la RUNTIME. Deci set_can_dev_0_speed() are efect.
     dev1 -> can_usr_rx/tx_dev1_init.c folosesc CAN_DEV_1_* , macrouri rezolvate
             la COMPILARE (#if in can_usr_private.h alege BTR/TSEG).
   Prin urmare set_can_dev_1_speed() / _buffer_size() sint INERTE pentru timing:
   singurul loc care schimba viteza CAN1 e CORE_CAN_DEV_1_SPEED_DEFAULT, mai sus. */
#define CAN_DEV_1_SPEED          CORE_CAN_DEV_1_SPEED_DEFAULT
#define CAN_DEV_1_RX_BUFFER_SIZE CORE_CAN_DEV_1_RX_BUFFER_SIZE_DEFAULT
#define CAN_DEV_1_TX_BUFFER_SIZE CORE_CAN_DEV_1_TX_BUFFER_SIZE_DEFAULT

typedef struct {
    uint32_t can_dev_0_ft_sw;
    uint32_t can_dev_0_speed;
    uint32_t can_dev_0_rx_buffer_size;
    uint32_t can_dev_0_tx_buffer_size;
    uint32_t can_dev_1_speed;
    uint32_t can_dev_1_rx_buffer_size;
    uint32_t can_dev_1_tx_buffer_size;
} can_config_contract_t;

#if defined(FW_DB3_BUILD)
/* SS4 Slice 2 single-instance: FW reads/writes CORE's single config instance
   through the API-table DATA pointer (no separate FW storage). */
can_config_contract_t* core_db3_cfg_can(void);
#define g_can_config (*core_db3_cfg_can())
#else
extern can_config_contract_t g_can_config;

#ifdef CAN_CONFIG_DEFINE_STORAGE
can_config_contract_t g_can_config;
#endif
#endif

#define set_can_dev_0_ft_sw(value) (g_can_config.can_dev_0_ft_sw = (uint32_t)(value))
#define get_can_dev_0_ft_sw() (g_can_config.can_dev_0_ft_sw)
#define set_can_dev_0_speed(value) (g_can_config.can_dev_0_speed = (uint32_t)(value))
#define get_can_dev_0_speed() (g_can_config.can_dev_0_speed)
#define set_can_dev_0_rx_buffer_size(value) (g_can_config.can_dev_0_rx_buffer_size = (uint32_t)(value))
#define get_can_dev_0_rx_buffer_size() (g_can_config.can_dev_0_rx_buffer_size)
#define set_can_dev_0_tx_buffer_size(value) (g_can_config.can_dev_0_tx_buffer_size = (uint32_t)(value))
#define get_can_dev_0_tx_buffer_size() (g_can_config.can_dev_0_tx_buffer_size)
#define set_can_dev_1_speed(value) (g_can_config.can_dev_1_speed = (uint32_t)(value))
#define get_can_dev_1_speed() (g_can_config.can_dev_1_speed)
#define set_can_dev_1_rx_buffer_size(value) (g_can_config.can_dev_1_rx_buffer_size = (uint32_t)(value))
#define get_can_dev_1_rx_buffer_size() (g_can_config.can_dev_1_rx_buffer_size)
#define set_can_dev_1_tx_buffer_size(value) (g_can_config.can_dev_1_tx_buffer_size = (uint32_t)(value))
#define get_can_dev_1_tx_buffer_size() (g_can_config.can_dev_1_tx_buffer_size)

#define can_config_set_defaults_from_defines() do { \
    set_can_dev_0_ft_sw((uint32_t)(CORE_CAN_DEV_0_FT_SW_DEFAULT)); \
    set_can_dev_0_speed((uint32_t)(CORE_CAN_DEV_0_SPEED_DEFAULT)); \
    set_can_dev_0_rx_buffer_size((uint32_t)(CORE_CAN_DEV_0_RX_BUFFER_SIZE_DEFAULT)); \
    set_can_dev_0_tx_buffer_size((uint32_t)(CORE_CAN_DEV_0_TX_BUFFER_SIZE_DEFAULT)); \
    set_can_dev_1_speed((uint32_t)(CORE_CAN_DEV_1_SPEED_DEFAULT)); \
    set_can_dev_1_rx_buffer_size((uint32_t)(CORE_CAN_DEV_1_RX_BUFFER_SIZE_DEFAULT)); \
    set_can_dev_1_tx_buffer_size((uint32_t)(CORE_CAN_DEV_1_TX_BUFFER_SIZE_DEFAULT));\
} while(0)

#define CAN_CFG_CAN_DEV_0_FT_SW (get_can_dev_0_ft_sw())
#define CAN_CFG_CAN_DEV_0_SPEED (get_can_dev_0_speed())
#define CAN_CFG_CAN_DEV_0_RX_BUFFER_SIZE (get_can_dev_0_rx_buffer_size())
#define CAN_CFG_CAN_DEV_0_TX_BUFFER_SIZE (get_can_dev_0_tx_buffer_size())
#define CAN_CFG_CAN_DEV_1_SPEED (get_can_dev_1_speed())
#define CAN_CFG_CAN_DEV_1_RX_BUFFER_SIZE (get_can_dev_1_rx_buffer_size())
#define CAN_CFG_CAN_DEV_1_TX_BUFFER_SIZE (get_can_dev_1_tx_buffer_size())

void core_contract_db3_can_config_init_defaults(void);
void core_contract_db3_can_config_init_vehicle_specific(void);

#endif /* __CAN_CONFIG_WRAPPER_H__ */
