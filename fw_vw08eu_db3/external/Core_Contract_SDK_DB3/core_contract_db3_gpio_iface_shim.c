#include "core_framework.h"
#include "core_contract_db3_fw_api.h"
#include "core_contract_db3_gpio_output.h"   /* pulls gpio_pl.h with the FW redirect (see that header) */
#include "core_contract_db3_firmware_config.h"
#include "core_contract_db3_can_config.h"
#include "core_contract_db3_pts_ctl_config.h"
#include "core_contract_db3_prog_config.h"
#include "core_contract_db3_status_config.h"

#if defined(CORE_DB3_BUILD)

#include "os.h"         /* SS4 Slice 4: os_evt + os_tsk_wait (CORE only) */
#include "timeout.h"    /* SS4 Slice 5: timeout start/stop (CORE only)  */
#include "timeout_f.h"  /* SS4 Slice 5: timeout_f_create (CORE only)    */
#include "nvfs.h"       /* SS4 Slice 6: nvfs_rd/wr/... (CORE only)      */
#include "nvfs_usr.h"   /* SS4 Slice 6: nvfs_fw_data entry name         */
#include "can_usr.h"    /* SS4 Slice 7a: can_usr_* (CORE only)          */
#include "can_sync.h"   /* SS4 Slice 7a: can_sync_* (CORE only)         */
#include "can_per.h"    /* SS4 Slice 7a: can_per_init (CORE only)       */
#include "process.h"    /* SS4 Slice 7b: process_* (CORE only)          */
#include "tsk_rs.h"     /* SS4 Slice 7b: tsk_rs_* (CORE only)           */
#include "hazard.h"     /* SS4 Slice 7b: hazard_* (CORE only)           */
#include "lf.h"         /* SS4 Slice 7b: lf_init (CORE only)            */
#include "sense_rs_can.h"  /* SS4 Slice 8 (CORE only) */
#include "status_system.h" /* SS4 Slice 8 */
#include "d2d.h"           /* SS4 Slice 8 */
#include "gpio_pl.h"       /* SS4 Slice 8 */
#include "reset.h"         /* SS4 Slice 8 */
#include "wdog_v.h"        /* SS4 Slice 8 */
#include "timeout_q.h"     /* SS4 Slice 8 */
#include "led_detect.h"    /* SS4 Slice 8 */
#include "cmd.h"           /* SS4 Slice 8 (cmd_lut_insert_table1) */
#include "prog.h"          /* SS4 Slice 8d (prog_set_exit_callback) */
#include "tmr.h"          /* v2.1 ambreiaj: tmr_create / ena / dis / update / int_sta */
#include "fiq.h"          /* v2.1 ambreiaj: fiq_register                              */
#include "per_exec.h"     /* v2.0 VW08EU: per_exec_register / critical_enter / leave */
#include "dl.h"           /* v2.0 VW08EU: dl_cfg                                    */
#include "can_per.h"      /* v2.0 VW08EU: can_per_add / can_per_remove (deja inclus mai sus, pastrat explicit) */

#if defined(__CC_ARM)
#define CORE_DB3_SHARED_ABI_INFO_PLACEMENT __attribute__((at(CORE_DB3_SHARED_ABI_INFO_ADDR), used))
#define CORE_DB3_SHARED_API_TABLE_PLACEMENT __attribute__((at(CORE_DB3_SHARED_API_TABLE_ADDR), used))
#elif defined(__GNUC__)
#define CORE_DB3_SHARED_ABI_INFO_PLACEMENT __attribute__((section("CORE_DB3_SHARED_ABI_INFO"), used))
#define CORE_DB3_SHARED_API_TABLE_PLACEMENT __attribute__((section("CORE_DB3_SHARED_API_TABLE"), used))
#else
#define CORE_DB3_SHARED_ABI_INFO_PLACEMENT
#define CORE_DB3_SHARED_API_TABLE_PLACEMENT
#endif

static const core_db3_abi_info_t* core_contract_db3_shared_abi_info_get(void);
extern Boolean trace_print(const char *str, ...);   /* SS4 Slice 3: routed to FW via table */

/* v2.0 VW08EU: declarate numai in sense_rs_can_private.h, iar shim-ul nu include
   headere private. Au legatura externa (sense_rs_can_oem_alarm.c), deci sint rutabile. */
extern void sense_rs_can_oem_alarm_arm(void);
extern void sense_rs_can_oem_alarm_disarm(void);
extern void sense_rs_can_oem_alarm_trigger(void);

const core_db3_abi_info_t g_core_db3_shared_abi_info CORE_DB3_SHARED_ABI_INFO_PLACEMENT =
{
    CORE_DB3_ABI_MAGIC,
    CORE_DB3_ABI_VERSION_MAJOR,
    CORE_DB3_ABI_VERSION_MINOR,
    CORE_DB3_API_TABLE_VERSION,
    CORE_DB3_CAPABILITY_DEFAULT_MASK,
    0U
};

const core_db3_api_table_t g_core_db3_shared_api_table CORE_DB3_SHARED_API_TABLE_PLACEMENT =
{
    CORE_DB3_API_TABLE_MAGIC,
    sizeof(core_db3_api_table_t),
    CORE_DB3_ABI_VERSION_MAJOR,
    CORE_DB3_ABI_VERSION_MINOR,
    CORE_DB3_API_TABLE_VERSION,
    CORE_DB3_CAPABILITY_DEFAULT_MASK,

    core_contract_db3_shared_abi_info_get,

    core_contract_db3_api_gpio_handbrake_set,
    core_contract_db3_api_gpio_doors_set,
    core_contract_db3_api_gpio_lights_set,
    core_contract_db3_api_gpio_trunk_set,
    core_contract_db3_api_gpio_brake_set,
    core_contract_db3_api_gpio_pts1_set,
    core_contract_db3_api_gpio_pts2_set,
    core_contract_db3_api_gpio_rap_set,
    core_contract_db3_api_gpio_rf_loop_set,
    core_contract_db3_api_gpio_ignition_set,
    core_contract_db3_api_gpio_hood_set,
    core_contract_db3_api_gpio_key_wrap_set,

    core_contract_db3_gpio_rf_loop_toggle,

    /* GROUP: config - DATA pointers to CORE's single config instances (SS4 Slice 2) */
    &g_firmware_config,
    &g_can_config,
    &g_pts_ctl_config,
    &g_prog_config,
    &g_status_config,

    /* GROUP: diag (SS4 Slice 3) */
    (core_db3_fn_trace_t)trace_print,

    /* GROUP: os/rtos (SS4 Slice 4) */
    (core_db3_fn_ptr_void)os_evt_create,
    (core_db3_fn_evt_clr_t)os_evt_clr,
    (core_db3_fn_evt_v_t)os_evt_signal_v,
    (core_db3_fn_evt_v_t)os_evt_wait_v,
    (core_db3_fn_void_u32)os_tsk_wait,

    /* GROUP: timeout (SS4 Slice 5) */
    (core_db3_fn_timeout_f_t)timeout_f_create,
    (core_db3_fn_void_ptr)timeout_start,
    (core_db3_fn_void_ptr)timeout_stop,

    /* GROUP: nvfs (SS4 Slice 6) */
    (core_db3_fn_nvfs_rw_t)nvfs_rd,
    (core_db3_fn_nvfs_rw_t)nvfs_wr,
    (core_db3_fn_nvfs_wrc_t)nvfs_wr_create,
    (core_db3_fn_nvfs_del_t)nvfs_delete,
    (core_db3_fn_u32_void)nvram_usr_commit,

    /* GROUP: can (SS4 Slice 7a) */
    (core_db3_fn_void_u32)can_per_init,
    (core_db3_fn_void_u32)can_usr_init,
    (core_db3_fn_void_u32_ptr)can_usr_rx_reg_fast,
    (core_db3_fn_void_u32_ptr_u32)can_usr_rx_reg_slow,
    (core_db3_fn_can_tx_t)can_usr_tx,
    (core_db3_fn_void_u32_u32)can_sync_init,
    (core_db3_fn_can_sync_add_t)can_sync_add,
    (core_db3_fn_u32_u32_ptr)can_sync_remove,

    /* GROUP: process/task/hazard (SS4 Slice 7b) */
    (core_db3_fn_ptr_ptr)process_create_cfg,
    (core_db3_fn_void_ptr_u32)process_launch,
    (core_db3_fn_void_void)tsk_rs_init,
    (core_db3_fn_void_u32_ptr)tsk_rs_register,
    (core_db3_fn_void_u32)tsk_rs_update_cfg,
    (core_db3_fn_void_ptr_u32)hazard_init,
    (core_db3_fn_void_u32)hazard_status_update,
    (core_db3_fn_void_void)lf_init,

    /* GROUP: sense_rs (SS4 Slice 8) */
    (core_db3_fn_void_ptr)sense_rs_can_init,
    (core_db3_fn_u32_u32)sense_rs_ign_can,
    (core_db3_fn_u32_u32)sense_rs_brake_can,
    (core_db3_fn_u32_u32)sense_rs_driver_door_can,
    (core_db3_fn_u32_u32)sense_rs_other_doors_can,
    (core_db3_fn_u32_u32)sense_rs_handbrake_can,
    (core_db3_fn_u32_u32)sense_rs_hood_can,
    (core_db3_fn_u32_u32)sense_rs_oem_lock_can,
    (core_db3_fn_u32_u32)sense_rs_oem_trunk_can,
    (core_db3_fn_u32_u32)sense_rs_oem_unlock_can,
    (core_db3_fn_void_u32)sense_rs_rpm_can,
    (core_db3_fn_u32_u32)sense_rs_skd_can,
    (core_db3_fn_void_u32)sense_rs_speed_can,
    (core_db3_fn_u32_u32)sense_rs_trunk_can,
    (core_db3_fn_void_void)sense_rs_can_brake_skip_enter,
    (core_db3_fn_void_void)sense_rs_can_brake_skip_leave,

    /* GROUP: status_system (SS4 Slice 8) */
    (core_db3_fn_void_u32)set_brake_source,
    (core_db3_fn_void_u32)set_door_source,
    (core_db3_fn_void_u32)set_handbrake_source,
    (core_db3_fn_void_u32)set_ign_source,
    (core_db3_fn_void_u32)set_rpm_source,
    (core_db3_fn_void_u32)set_speed_source,
    (core_db3_fn_void_u32)set_trunk_source,
    (core_db3_fn_void_u32)set_rke_lock_use,
    (core_db3_fn_void_u32)set_rke_trunk_use,
    (core_db3_fn_void_u32)set_rke_unlock_use,
    (core_db3_fn_void_u32)set_rs_gwr,
    (core_db3_fn_void_u32)set_rs_ign,
    (core_db3_fn_void_u32)set_rs_start,
    (core_db3_fn_u32_void)get_ign_status,
    (core_db3_fn_u32_void)get_rs_gwr,
    (core_db3_fn_u32_void)get_ss_lock,
    (core_db3_fn_u32_void)get_trunk_status,

    /* GROUP: misc (SS4 Slice 8) */
    (core_db3_fn_void_void)led_detect_started,
    (core_db3_fn_void_void)led_detect_done,
    (core_db3_fn_void_void)led_detect_in_progress,
    (core_db3_fn_void_void)led_detect_err_can0,
    (core_db3_fn_void_void)led_detect_err_invalid_ignition,
    (core_db3_fn_void_u32)d2d_tx,
    (core_db3_fn_void_u32)gpio_pl_set,
    (core_db3_fn_void_u32)gpio_pl_clr,
    (core_db3_fn_void_ptr)reset_register,
    (core_db3_fn_void_u32_ptr_u32)wdog_v_force_reset,
    (core_db3_fn_void_ptr_u32_u32)cmd_lut_insert_table1,
    (core_db3_fn_timeout_q_ins_t)timeout_q_insert,
    (core_db3_fn_u32_ptr)timeout_q_remove,
    &nvfs_fw_data,

    /* GROUP: prog (SS4 Slice 8d) */
    (core_db3_fn_void_ptr)prog_set_exit_callback,

    /* GROUP: dbg config (v1.1) - DATA pointer to CORE's single dbg config instance */
    &g_dbg_config,

    /* GROUP: status_system hood source (v1.2) */
    (core_db3_fn_void_u32)set_hood_source,

    /* GROUP: gpio clutch->RELAY1 (v1.3) */
    core_contract_db3_api_gpio_clutch_set,

    /* ================= GROUP VW08EU (v2.0) - 25 de servicii ================= */

    /* sense / status_system (7) */
    (core_db3_fn_void_u32)sense_rs_can_set_dl_status,
    (core_db3_fn_u32_void)get_ss_trigger,
    (core_db3_fn_u32_u32)sense_rs_oem_autolock_can,
    (core_db3_fn_u32_void)get_hood_status,
    (core_db3_fn_u32_u32)sense_rs_neutral_can,
    (core_db3_fn_u32_void)get_rpm,
    (core_db3_fn_u32_void)get_door_status,

    /* sense OEM alarm (3) */
    (core_db3_fn_void_void)sense_rs_can_oem_alarm_arm,
    (core_db3_fn_void_void)sense_rs_can_oem_alarm_disarm,
    (core_db3_fn_void_void)sense_rs_can_oem_alarm_trigger,

    /* per_exec, pentru com.lib.scan (3) */
    (core_db3_fn_per_exec_reg_t)per_exec_register,
    (core_db3_fn_u32_void)per_exec_critical_enter,
    (core_db3_fn_void_void)per_exec_critical_leave,

    /* os, pentru com.lib.scan_new (4) */
    (core_db3_fn_ptr_void)os_irq_create,
    (core_db3_fn_u32_ptr)os_irq_set,
    (core_db3_fn_os_irq_wait_v_t)os_irq_wait_v,
    (core_db3_fn_ptr_u32)os_mem_request,

    /* led_detect - restul familiei era deja rutat (2) */
    (core_db3_fn_void_void)led_detect_err_can1,
    (core_db3_fn_void_void)led_detect_err_invalid_bcm,

    /* func / lib.fw (6) */
    (core_db3_fn_void_u32_ptr)dl_cfg,
    (core_db3_fn_can_per_add_t)can_per_add,
    (core_db3_fn_u32_ptr)can_per_remove,
    (core_db3_fn_void_u32)can_usr_rx_filter_disable,
    (core_db3_fn_u32_void)tsk_rs_invalid_state,
    (core_db3_fn_void_u32)lf_autopulse_enable,

    /* GROUP ambreiaj (v2.1): timer + FIQ pentru com.lib.clutch */
    (core_db3_fn_void_ptr)fiq_register,
    (core_db3_fn_ptr_void)tmr_create,
    (core_db3_fn_void_ptr)tmr_ena,
    (core_db3_fn_void_ptr)tmr_dis,
    (core_db3_fn_void_ptr_u32_u32)tmr_update,
    (core_db3_fn_u32_ptr)tmr_int_sta,

    /* GROUP gpio ambreiaj (v2.1) */
    core_contract_db3_api_gpio_clutch_pwm_set,
    core_contract_db3_api_gpio_clutch_sensor_a_set,
    core_contract_db3_api_gpio_clutch_sensor_b_set,

    {0}
};

static const core_db3_abi_info_t* core_contract_db3_shared_abi_info_get(void)
{
    return &g_core_db3_shared_abi_info;
}

/* v5 VW08EU (11.09.2026) - STATUSUL de frina, separat de actuatie.
   In DB2 erau doua iesiri distincte: BRAKE_SENSE_SET/CLR mergeau pe
   _401_OUT_DAC_RELAY (P1.4), iar VW03_FUNC_BRAKE_OUT pe OUT_RELAY1. La portare
   au fost contopite intr-una singura, deci statusul de frina clantea releul 1.
   Vezi comentariul de la CORE_DB3_GPIO_IFACE_BRAKE_SENSE_OUTPUT.

   Sta INAUNTRUL lui CORE_DB3_BUILD, spre deosebire de gpio_iface_brake de mai
   jos: toti apelantii sint in CORE (sense_rs_can_brake.c, sense_notify_brake.c),
   deci nu are nevoie de slot in tabela de API si nu are ce cauta in FW. Merge
   direct la contract, fara drumul dus-intors prin tabela. */
void gpio_iface_brake_sense(Boolean state)
{
    core_contract_db3_gpio_brake_sense_set(state);
}

#endif

void gpio_iface_doors_open(void)
{
    core_contract_db3_fw_gpio_doors_open();
}

void gpio_iface_doors_closed(void)
{
    core_contract_db3_fw_gpio_doors_closed();
}

void gpio_iface_trunk_open(void)
{
    core_contract_db3_fw_gpio_trunk_open();
}

void gpio_iface_trunk_closed(void)
{
    core_contract_db3_fw_gpio_trunk_closed();
}

void gpio_iface_handbrake_on(void)
{
    core_contract_db3_fw_gpio_handbrake_on();
}

void gpio_iface_handbrake_off(void)
{
    core_contract_db3_fw_gpio_handbrake_off();
}

void gpio_iface_lights_on(void)
{
    core_contract_db3_fw_gpio_lights_on();
}

void gpio_iface_lights_off(void)
{
    core_contract_db3_fw_gpio_lights_off();
}

void gpio_iface_brake(Boolean state)
{
    core_contract_db3_fw_gpio_brake_set(state);
}

void gpio_iface_clutch(Boolean state)
{
    core_contract_db3_fw_gpio_clutch_set(state);
}

/* v2.1 VW08EU: iesirile de ambreiaj. Inversarea PWM-ului (DB2: OUT_PHAL2_TX(!x))
   NU se face aici - sta la apelant, in clutch_private.h, ca sa ramina vizibila. */
void gpio_iface_clutch_pwm(Boolean state)
{
    core_contract_db3_fw_gpio_clutch_pwm_set(state);
}

void gpio_iface_clutch_sensor_a(Boolean state)
{
    core_contract_db3_fw_gpio_clutch_sensor_a_set(state);
}

void gpio_iface_clutch_sensor_b(Boolean state)
{
    core_contract_db3_fw_gpio_clutch_sensor_b_set(state);
}

void gpio_iface_rf_loop(Boolean state)
{
    core_contract_db3_fw_gpio_rf_loop_set(state);
}

void gpio_iface_pts_1(Boolean state)
{
    core_contract_db3_fw_gpio_pts1_set(state);
}

void gpio_iface_pts_2(Boolean state)
{
    core_contract_db3_fw_gpio_pts2_set(state);
}

void gpio_iface_rap(Boolean state)
{
    core_contract_db3_fw_gpio_rap_set(state);
}

void gpio_iface_ignition_on(void)
{
    core_contract_db3_fw_gpio_ignition_on();
}

void gpio_iface_ignition_off(void)
{
    core_contract_db3_fw_gpio_ignition_off();
}

void gpio_iface_hood_open(void)
{
    core_contract_db3_fw_gpio_hood_open();
}

void gpio_iface_hood_closed(void)
{
    core_contract_db3_fw_gpio_hood_closed();
}

void gpio_iface_key_wrap_on(void)
{
    core_contract_db3_fw_gpio_key_wrap_on();
}

void gpio_iface_key_wrap_off(void)
{
    core_contract_db3_fw_gpio_key_wrap_off();
}

void gpio_iface_led_red_on(void)
{
    core_contract_db3_gpio_led_red_on();
}

void gpio_iface_led_red_off(void)
{
    core_contract_db3_gpio_led_red_off();
}

void gpio_iface_led_green_on(void)
{
    core_contract_db3_gpio_led_green_on();
}

void gpio_iface_led_green_off(void)
{
    core_contract_db3_gpio_led_green_off();
}

void gpio_iface_led_blue_on(void)
{
    core_contract_db3_gpio_led_blue_on();
}

void gpio_iface_led_blue_off(void)
{
    core_contract_db3_gpio_led_blue_off();
}

void gpio_iface_rsr_lock_on(void)
{
    core_contract_db3_gpio_rsr_lock_on();
}

void gpio_iface_rsr_lock_off(void)
{
    core_contract_db3_gpio_rsr_lock_off();
}

void gpio_iface_rsr_unlock_on(void)
{
    core_contract_db3_gpio_rsr_unlock_on();
}

void gpio_iface_rsr_unlock_off(void)
{
    core_contract_db3_gpio_rsr_unlock_off();
}

void gpio_iface_rsr_trunk_on(void)
{
    core_contract_db3_gpio_rsr_trunk_on();
}

void gpio_iface_rsr_trunk_off(void)
{
    core_contract_db3_gpio_rsr_trunk_off();
}

void gpio_iface_rf_loop_toggle(void)
{
    core_contract_db3_fw_gpio_rf_loop_toggle();
}
