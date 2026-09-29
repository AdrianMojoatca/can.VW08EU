/*<#======================================================================#>*/
/*  config_prog_seed.c - DB3 vehicle-specific prog-config seeding.           */
/*  Strong override of the contract's __weak                                 */
/*  core_contract_db3_prog_config_apply_vehicle_specific(): copies the        */
/*  installer/web values from config_prog.h into g_prog_config (CORE).        */
/*  Called from core_contract_db3_prog_config_init_defaults() ->              */
/*  prog_build_feature_table_default() -> feature_table -> prog_retrieve_option.*/
/*  NVFS (nvfs_feat_fw) can still override feature_table at runtime.           */
/*<#======================================================================#>*/

#include "core_contract_db3_prog_config.h"
#include "config_prog.h"

void core_contract_db3_prog_config_apply_vehicle_specific(void);

void core_contract_db3_prog_config_apply_vehicle_specific(void)
{
    /* --- Controlled door lock sub-options --- */
    set_prog_ctl_doorlock_brake_enable(CTL_DOORLOCK_BRAKE_ENABLE);
    set_prog_ctl_doorlock_ignition_enable(CTL_DOORLOCK_IGNITION_ENABLE);
    set_prog_ctl_doorlock_speed_enable(CTL_DOORLOCK_SPEED_ENABLE);

    /* --- Custom feature sub-options (opt2..opt5) --- */
    set_prog_custom1_opt2_enable(CUSTOM1_OPT2_ENABLE);
    set_prog_custom1_opt3_enable(CUSTOM1_OPT3_ENABLE);
    set_prog_custom1_opt4_enable(CUSTOM1_OPT4_ENABLE);
    set_prog_custom1_opt5_enable(CUSTOM1_OPT5_ENABLE);
    set_prog_custom2_opt2_enable(CUSTOM2_OPT2_ENABLE);
    set_prog_custom2_opt3_enable(CUSTOM2_OPT3_ENABLE);
    set_prog_custom2_opt4_enable(CUSTOM2_OPT4_ENABLE);
    set_prog_custom2_opt5_enable(CUSTOM2_OPT5_ENABLE);
    set_prog_custom3_opt2_enable(CUSTOM3_OPT2_ENABLE);
    set_prog_custom3_opt3_enable(CUSTOM3_OPT3_ENABLE);
    set_prog_custom3_opt4_enable(CUSTOM3_OPT4_ENABLE);
    set_prog_custom3_opt5_enable(CUSTOM3_OPT5_ENABLE);
    set_prog_custom4_opt2_enable(CUSTOM4_OPT2_ENABLE);
    set_prog_custom4_opt3_enable(CUSTOM4_OPT3_ENABLE);
    set_prog_custom4_opt4_enable(CUSTOM4_OPT4_ENABLE);
    set_prog_custom4_opt5_enable(CUSTOM4_OPT5_ENABLE);

    /* --- Features (enable + value/default) --- */
    set_prog_feat_auto_lock_enable(FEAT_AUTO_LOCK_ENABLE);
    set_prog_feat_auto_lock_value(FEAT_AUTO_LOCK_VALUE);
    set_prog_feat_auto_unlock_enable(FEAT_AUTO_UNLOCK_ENABLE);
    set_prog_feat_auto_unlock_value(FEAT_AUTO_UNLOCK_VALUE);
    set_prog_feat_aux1_default(FEAT_AUX1_DEFAULT);
    set_prog_feat_aux1_enable(FEAT_AUX1_ENABLE);
    set_prog_feat_aux2_default(FEAT_AUX2_DEFAULT);
    set_prog_feat_aux2_enable(FEAT_AUX2_ENABLE);
    set_prog_feat_aux3_default(FEAT_AUX3_DEFAULT);
    set_prog_feat_aux3_enable(FEAT_AUX3_ENABLE);
    set_prog_feat_aux4_default(FEAT_AUX4_DEFAULT);
    set_prog_feat_aux4_enable(FEAT_AUX4_ENABLE);
    set_prog_feat_comfort_closure_enable(FEAT_COMFORT_CLOSURE_ENABLE);
    set_prog_feat_comfort_closure_value(FEAT_COMFORT_CLOSURE_VALUE);
    set_prog_feat_ctl_doorlock_enable(FEAT_CTL_DOORLOCK_ENABLE);
    set_prog_feat_ctl_doorlock_value(FEAT_CTL_DOORLOCK_VALUE);
    set_prog_feat_custom1_default_value(FEAT_CUSTOM1_DEFAULT_VALUE);
    set_prog_feat_custom1_enable(FEAT_CUSTOM1_ENABLE);
    set_prog_feat_custom2_default_value(FEAT_CUSTOM2_DEFAULT_VALUE);
    set_prog_feat_custom2_enable(FEAT_CUSTOM2_ENABLE);
    set_prog_feat_custom3_default_value(FEAT_CUSTOM3_DEFAULT_VALUE);
    set_prog_feat_custom3_enable(FEAT_CUSTOM3_ENABLE);
    set_prog_feat_custom4_default_value(FEAT_CUSTOM4_DEFAULT_VALUE);
    set_prog_feat_custom4_enable(FEAT_CUSTOM4_ENABLE);
    set_prog_feat_defroster_enable(FEAT_DEFROSTER_ENABLE);
    set_prog_feat_defroster_value(FEAT_DEFROSTER_VALUE);
    set_prog_feat_ext_light_supervision_enable(FEAT_EXT_LIGHT_SUPERVISION_ENABLE);
    set_prog_feat_ext_light_supervision_value(FEAT_EXT_LIGHT_SUPERVISION_VALUE);
    set_prog_feat_heated_seats_enable(FEAT_HEATED_SEATS_ENABLE);
    set_prog_feat_heated_seats_value(FEAT_HEATED_SEATS_VALUE);
    set_prog_feat_horn_confirmation_enable(FEAT_HORN_CONFIRMATION_ENABLE);
    set_prog_feat_horn_confirmation_value(FEAT_HORN_CONFIRMATION_VALUE);
    set_prog_feat_key_ign_arm_defeat_enable(FEAT_KEY_IGN_ARM_DEFEAT_ENABLE);
    set_prog_feat_key_ign_arm_defeat_value(FEAT_KEY_IGN_ARM_DEFEAT_VALUE);
    set_prog_feat_light_control_enable(FEAT_LIGHT_CONTROL_ENABLE);
    set_prog_feat_light_control_value(FEAT_LIGHT_CONTROL_VALUE);
    set_prog_feat_oem_security_enable(FEAT_OEM_SECURITY_ENABLE);
    set_prog_feat_oem_security_value(FEAT_OEM_SECURITY_VALUE);
    set_prog_feat_relock_enable(FEAT_RELOCK_ENABLE);
    set_prog_feat_relock_value(FEAT_RELOCK_VALUE);
    set_prog_feat_rf_output_enable(FEAT_RF_OUTPUT_ENABLE);
    set_prog_feat_rf_output_value(FEAT_RF_OUTPUT_VALUE);
    set_prog_feat_seats_temp_control_enable(FEAT_SEATS_TEMP_CONTROL_ENABLE);
    set_prog_feat_seats_temp_control_value(FEAT_SEATS_TEMP_CONTROL_VALUE);
    set_prog_feat_seats_temp_level_enable(FEAT_SEATS_TEMP_LEVEL_ENABLE);
    set_prog_feat_seats_temp_level_value(FEAT_SEATS_TEMP_LEVEL_VALUE);
    set_prog_feat_secure_lock_enable(FEAT_SECURE_LOCK_ENABLE);
    set_prog_feat_secure_lock_value(FEAT_SECURE_LOCK_VALUE);
    set_prog_feat_trunk_default(FEAT_TRUNK_DEFAULT);
    set_prog_feat_trunk_enable(FEAT_TRUNK_ENABLE);
    set_prog_feat_unlock_driver_enable(FEAT_UNLOCK_DRIVER_ENABLE);
    set_prog_feat_unlock_driver_value(FEAT_UNLOCK_DRIVER_VALUE);

    /* --- Heated seats level --- */
    set_prog_heated_seats_level_high_enable(HEATED_SEATS_LEVEL_HIGH_ENABLE);
    set_prog_heated_seats_level_low_enable(HEATED_SEATS_LEVEL_LOW_ENABLE);
    set_prog_heated_seats_level_med_enable(HEATED_SEATS_LEVEL_MED_ENABLE);

    /* --- Trunk / Aux icon options --- */
    set_prog_option_gas_cap_enable(OPTION_GAS_CAP_ENABLE);
    set_prog_option_head_lights_enable(OPTION_HEAD_LIGHTS_ENABLE);
    set_prog_option_left_slide_enable(OPTION_LEFT_SLIDE_ENABLE);
    set_prog_option_right_slide_enable(OPTION_RIGHT_SLIDE_ENABLE);
    set_prog_option_sunroof_closure_enable(OPTION_SUNROOF_CLOSURE_ENABLE);
    set_prog_option_tail_glass_enable(OPTION_TAIL_GLASS_ENABLE);
    set_prog_option_trunk_enable(OPTION_TRUNK_ENABLE);
    set_prog_option_windows_closure_enable(OPTION_WINDOWS_CLOSURE_ENABLE);
    set_prog_option_windows_vent_enable(OPTION_WINDOWS_VENT_ENABLE);
    set_prog_option_audio_enable(OPTION_AUDIO_ENABLE);
    set_prog_option_timed_enable(OPTION_TIMED_ENABLE);
    set_prog_option_latched_enable(OPTION_LATCHED_ENABLE);
    set_prog_option_pulsed_enable(OPTION_PULSED_ENABLE);
    set_prog_option_garage_door_enable(OPTION_GARAGE_DOOR_ENABLE);

    /* --- Secure lock smart sub-option --- */
    set_prog_secure_lock_smart_enable(SECURE_LOCK_SMART_ENABLE);
}
