/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: fw_init.c 15282 2012-06-12 13:29:20Z louis-philippe.rispoli $
/*==========================================================================*/


/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "fw.h"
//#include "prog_private.h"
//#include "key.h"
#include "detect_private.h"
#include "build_identity.h"
#include "core_contract_db3_firmware_config.h"
#include "core_contract_db3_can_config.h"
#include "core_contract_db3_gpio_config.h"
#include "core_contract_db3_gpio_output.h"  /* led_green_off: vezi fw_init() */
#include "core_contract_db3_pts_ctl_config.h"
#include "core_contract_db3_status_config.h"
#include "core_contract_db3_prog_config.h"
#include "core_contract_db3_prog_option_defs.h"
#include "tsk_rs.h"                        /* tsk_rs_update_cfg() in fw_options_set */
#include "core_contract_db3_boot_handshake.h" /* v1.1: api_table_version guard        */
#include "core_contract_db3_dbg_config.h"     /* v1.1: FW-owned runtime debug config  */
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/* v1.1: FW-owned master debug/trace switch (this FW's choice; CORE honors it).
   1 = debug on (default, parity with before); 0 = ship this FW silent (CORE still
   prints one identity banner). This is the per-FW knob for "one CORE, many FWs" -
   it must NOT come from config_dbg.h (the contract copy is shadowed-in via quoted
   include and is CORE's compile-time trace-presence switch). Override in the FW
   project Define list if preferred. */
#ifndef FW_TRACE_ENABLE
#define FW_TRACE_ENABLE 1
#endif

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
static void fw_options_set( void );
static void fw_prog_vehicle_specific_enable_apply( void );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
/* PORTARE VW08EU: numita `cfg`, ca in DB2. NU `func_cfg` - acela e globalul lui
   com.lib.func (`extern Func_Cfg* func_cfg` in func_private.h), iar func_init()
   il face sa pointeze aici. */
static Func_Cfg cfg ;

//extern Detect_Vars     detect_vars  ;      
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

void core_contract_db3_pts_ctl_config_apply_vehicle_specific(void)
{
    set_config_pts_ctl_version(1U);
    set_pts_ctl_timeout(750U);
    set_pts_ctl_pulse_timeout(4000U);
}

/*--------------------------------------------------------------------------*/

void core_contract_db3_status_config_apply_vehicle_specific(void)
{
    set_status_vin_enable(1U);
    set_status_rke_enable(1U);
    set_status_key_enable(1U);
    set_status_rpm_enable(1U);
    set_status_door_enable(1U);
    set_status_speed_enable(1U);
    set_status_brake_enable(1U);
    set_status_trans_enable(1U);
    set_status_alarm_enable(1U);
    set_status_engine_enable(1U);
    set_status_comfort_enable(1U);
    set_status_temperature_enable(1U);
    set_status_req_input_enable(1U);
    set_status_dtc_enable(1U);
    set_status_mileage_enable(1U);
    set_status_tpms_enable(1U);
    set_status_doorlock_enable(1U);

    set_status_oil_alarm_enable(1U);
    set_status_tire_alarm_enable(1U);
    set_status_seat_belts_enable(1U);
    set_status_seat_weights_enable(1U);
    set_status_ev_battery_lvl_enable(1U);
    set_status_ev_range_enable(1U);
    set_status_oil_life_enable(1U);
    set_status_fuel_lvl_enable(1U);
    set_status_fuel_cap_enable(1U);
    set_status_cabin_temp_enable(1U);
    set_status_tire_pressure_enable(1U);
    set_status_odometer_enable(1U);
    set_status_battery_voltage_enable(1U);

    set_function_honks_enable(1U);
    set_function_headlights_enable(1U);
    set_function_rib_enable(1U);
}

/*--------------------------------------------------------------------------*/

void core_contract_db3_prog_config_apply_vehicle_specific(void)
{
    /* Controlled door lock option flags */
    set_prog_ctl_doorlock_brake_enable(1U);
    set_prog_ctl_doorlock_ignition_enable(1U);
    set_prog_ctl_doorlock_speed_enable(1U);

    /* Custom1 available options */
    set_prog_custom1_opt2_enable(1U);
    set_prog_custom1_opt3_enable(1U);
    set_prog_custom1_opt4_enable(1U);
    set_prog_custom1_opt5_enable(0U);

    /* Custom2 available options */
    set_prog_custom2_opt2_enable(0U);
    set_prog_custom2_opt3_enable(0U);
    set_prog_custom2_opt4_enable(0U);
    set_prog_custom2_opt5_enable(0U);

    /* Custom3 available options */
    set_prog_custom3_opt2_enable(0U);
    set_prog_custom3_opt3_enable(0U);
    set_prog_custom3_opt4_enable(0U);
    set_prog_custom3_opt5_enable(0U);

    /* Custom4 available options */
    set_prog_custom4_opt2_enable(1U);
    set_prog_custom4_opt3_enable(1U);
    set_prog_custom4_opt4_enable(1U);
    set_prog_custom4_opt5_enable(1U);

    /* Feature gate + selected value */
    set_prog_feat_auto_lock_enable(0U);
    set_prog_feat_auto_lock_value(1U);

    set_prog_feat_auto_unlock_enable(0U);
    set_prog_feat_auto_unlock_value(1U);

    set_prog_feat_aux1_default(8U);
    set_prog_feat_aux1_enable(0U);

    set_prog_feat_aux2_default(7U);
    set_prog_feat_aux2_enable(0U);

    set_prog_feat_aux3_default(4U);
    set_prog_feat_aux3_enable(0U);

    set_prog_feat_aux4_default(7U);
    set_prog_feat_aux4_enable(0U);

    /* Comfort closure */
    set_prog_feat_comfort_closure_enable(0U);
    set_prog_feat_comfort_closure_value(1U);

    /* Controlled door lock */
    set_prog_feat_ctl_doorlock_enable(0U);
    set_prog_feat_ctl_doorlock_value(1U);

    /* Takeover */
    set_prog_feat_custom1_default_value(1U);
    set_prog_feat_custom1_enable(0U);

    /* Digital Aux1 */
    set_prog_feat_custom2_value(1U);
    set_prog_feat_custom2_enable(0U);

    /* Digital Aux2 */
    set_prog_feat_custom3_value(1U);
    set_prog_feat_custom3_enable(0U);

    /* Digital Aux3 */
    set_prog_feat_custom4_value(1U);
    set_prog_feat_custom4_enable(0U);

    set_prog_feat_defroster_enable(0U);
    set_prog_feat_defroster_value(1U);

    set_prog_feat_ext_light_supervision_enable(0U);
    set_prog_feat_ext_light_supervision_value(1U);

    set_prog_feat_heated_seats_enable(0U);
    set_prog_feat_heated_seats_value(1U);

    set_prog_feat_horn_confirmation_enable(0U);
    set_prog_feat_horn_confirmation_value(1U);

    set_prog_feat_key_ign_arm_defeat_enable(0U);
    set_prog_feat_key_ign_arm_defeat_value(2U);

    set_prog_feat_light_control_enable(0U);
    set_prog_feat_light_control_value(1U);

    set_prog_feat_oem_security_enable(0U);
    set_prog_feat_oem_security_value(1U);

    set_prog_feat_relock_enable(0U);
    set_prog_feat_relock_value(1U);

    set_prog_feat_rf_output_enable(1U);
    set_prog_feat_rf_output_value(1U);

    set_prog_feat_seats_temp_control_enable(0U);
    set_prog_feat_seats_temp_control_value(1U);

    set_prog_feat_seats_temp_level_enable(0U);
    set_prog_feat_seats_temp_level_value(1U);

    set_prog_feat_secure_lock_enable(0U);
    set_prog_feat_secure_lock_value(3U);

    set_prog_feat_trunk_default(0U);
    set_prog_feat_trunk_enable(0U);

    /* Unlock driver priority */
    set_prog_feat_unlock_driver_enable(0U);
    set_prog_feat_unlock_driver_value(1U);

    /* Heated seats level options */
    set_prog_heated_seats_level_high_enable(0U);
    set_prog_heated_seats_level_low_enable(0U);
    set_prog_heated_seats_level_med_enable(0U);

    /* Trunk/Aux available outputs */
    set_prog_option_gas_cap_enable(0U);
    set_prog_option_head_lights_enable(0U);
    set_prog_option_left_slide_enable(0U);
    set_prog_option_right_slide_enable(0U);
    set_prog_option_sunroof_closure_enable(0U);
    set_prog_option_tail_glass_enable(0U);
    set_prog_option_trunk_enable(0U);
    set_prog_option_windows_closure_enable(0U);
    set_prog_option_windows_vent_enable(0U);
    set_prog_option_audio_enable(0U);
    set_prog_option_timed_enable(0U);
    set_prog_option_latched_enable(0U);
    set_prog_option_pulsed_enable(0U);
    set_prog_option_garage_door_enable(0U);

    /* Smart OEM alarm control */
    set_prog_secure_lock_smart_enable(1U);
}

/*--------------------------------------------------------------------------*/

void fw_identity_init(void)
{
    /* SS4 Slice 2: writes CORE's SINGLE config instance via the API-table DATA
       pointers (g_*_config is redefined to (*core_db3_cfg_*()) under
       FW_DB3_BUILD). Runs before core_init, which then consumes it. */
    core_contract_db3_firmware_config_init_defaults();
    core_contract_db3_pts_ctl_config_init_defaults();
    set_fw_name(BUILD_FW_NAME);
    set_app_version(BUILD_FW_VERSION);
    set_fw_info(BUILD_FW_INFO);

    set_can_dev_0_ft_sw(1);
    set_can_dev_0_speed(6);
    set_can_dev_0_rx_buffer_size(8);
    set_can_dev_0_tx_buffer_size(8);

    /* v1.1: FW owns the runtime debug config; CORE honors it (the UART stays in CORE).
       Guarded on the table version so a pre-v1.1 CORE (no dbg_config slot) is untouched.
       Seed the per-module dbg_* + debug_text defaults, then apply THIS FW's master
       trace switch (the per-vehicle knob; see FW_TRACE_ENABLE above). */
    if (core_contract_db3_boot_api_table_get()->api_table_version >= 12U)
    {
        dbg_config_set_defaults_from_defines();
        set_use_trace(FW_TRACE_ENABLE);
    }
}

/*--------------------------------------------------------------------------*/

/* PORTARE VW08EU - fluxul e cel din DB2 (dball/lib.fw/fw_init.c):
   detect_car_search() e void, rezultatul detectiei vine din car.bits.*, iar
   poarta e can1_detected. Al doilea argument al lui func_init spune daca se
   initializeaza si CAN-ul de OBD (adica daca s-a detectat can0). */
void fw_init( void )
{
	/* PORTARE VW08EU - culoarea LED-ului in faza de detectie.
	   CORE preda controlul cu VERDELE APRINS: task_main.c face led_green_on()
	   + led_red_off() imediat inainte de fw_init_fn(). Detectia aprinde apoi
	   rosu peste el (led_detect_started -> led_flash(LED_RED,...)) si nimeni
	   nu stinge verdele, deci pe modul se vede PORTOCALIU.

	   In DB2 nu exista impartirea CORE/FW, nimic nu aprindea verdele, iar
	   detectia se vedea rosu curat. Stingem verdele aici, in primul punct in
	   care firmware-ul detine LED-ul, ca semnalizarea sa fie identica cu DB2.
	   Restul fazelor (led_detect_in_progress, _done, _err_*) isi pun singure
	   culoarea, deci e suficient o singura data. */
	core_contract_db3_gpio_led_green_off();

	detect_car_search();

	cfg.can_dev_id = car.bits.can1_detected ? CAN_DEV_FUNC_SENSE : CAN_DEV_COUNT ;

	if( cfg.can_dev_id != CAN_DEV_COUNT )
	{
		func_init( &cfg, car.bits.can0_detected ? TRUE : FALSE );

		fw_options_set();

		prog_set_exit_callback( (Prog_Func) fw_options_set );
	}
}
/*--------------------------------------------------------------------------*/

/* PORTARE VW08EU - echivalentul lui fw_options_set din DB2, exprimat prin
   accesorii de contract in loc de prog_retrieve_option(FEAT_*). Contractul e
   sursa de adevar pe thin-FW; maparea opt-cu-opt e mai jos, la fiecare bloc. */
void fw_options_set( void )
{
    fw_prog_vehicle_specific_enable_apply();

    /* DB2: cfg.confort = FALSE, neconditionat. */
    cfg.confort = FALSE;

    /* DB2: switch(prog_retrieve_option(FEAT_CUSTOM1)) cu cazurile 1..4.
       Aceeasi ordine ca in enumerarea PROG_TAKEOVER_* din contract. */
    /* v2 VW08EU (28.09.2026) - SHUTDOWN PE USA = OFF. Corectie fata de v1: Adrian a
       cerut A PATRA optiune din DB2, PROG_TAKEOVER_SKIP_DOOR_SHUTDOWN (valoarea 4 in
       core_contract_<platforma>_prog_option_defs.h), nu a treia.

       Blocul de aici era, mostenit din VW07EU:

           takeover_option = (UInt8)get_prog_feat_custom1_default_value();
           if (get_prog_feat_custom1_enable() == 0U)
               tsk_rs_update_cfg(TSK_RS_TAKEOVER_DISABLE);
           else
               switch (takeover_option) { ...cele patru cazuri PROG_TAKEOVER_*... }

       Ramura de sus se executa INTOTDEAUNA, dar nu fiindca asa s-a decis: e acelasi
       defect ca la ambreiaj (REPARATIE A). get_prog_feat_custom1_enable() citeste din
       g_prog_config, populat de
       core_contract_<platforma>_prog_config_apply_vehicle_specific() - hook __weak,
       declarat in CORE si definit in FW. CORE si FW sint imagini SEPARATE, deci hook-ul
       nu se cheama niciodata, g_prog_config ramine memset-at la 0 si accesorul intoarce
       0. Rezultat: tsk_rs_takeover_cfg = TSK_RS_TAKEOVER_DISABLE (= 0 in tsk_rs.h), si
       orice deschidere de usa in Remote Start opreste motorul. Acela e comportamentul
       VW07EU, si acolo ramine neschimbat.

       Pe VW08EU scriem direct cazul 4. Ce face flagul, din tsk_rs_private.h:70..73:

           TSK_RS_TAKEOVER_MANAGER_ENABLED()        = !(cfg & SKIP_DOOR_SHUTDOWN)
           TSK_RS_TAKEOVER_ALLOWED_BY_OEM()         = (cfg & OEM)         && !SKIP
           TSK_RS_TAKEOVER_ALLOWED_BY_AFTERMARKET() = (cfg & AFTERMARKET) && !SKIP

       Cu SKIP setat, toate trei devin FALSE, deci:
         - tsk_rs_tke_door_shutdown() iese fara sa faca nimic: usa deschisa in Remote
           Start NU mai opreste motorul, niciodata si neconditionat;
         - fereastra de 45 s (TSK_RS_TAKEOVER_TIMEOUT) nu se mai armeaza si nu mai
           conteaza - nu mai e nevoie de unlock inainte de deschiderea usii;
         - lock-ul nu mai inchide nimic, fiindca nu mai exista fereastra de inchis.

       Diferenta fata de v1 (OEM|AFTERMARKET): acolo preluarea era o EXCEPTIE de 45 s
       acordata de unlock, iar dupa expirare usa oprea iar motorul. Aici oprirea pe usa
       nu mai exista deloc. E varianta mai permisiva, si e cea ceruta.

       Cind se repara hook-ul de contract (vezi cele 89 de apeluri set_prog_* din
       RELEASE_NOTES), locul asta se intoarce la switch si optiunea devine programabila
       de pe web. */
    tsk_rs_update_cfg(TSK_RS_SKIP_DOOR_SHUTDOWN);

    /* DB2: FEAT_LIGHT_CONTROL 1 -> Flashing (implicit), 2 -> Disabled. */
    if (get_prog_feat_light_control_enable() != 0U)
    {
        lf_autopulse_enable( get_prog_feat_light_control_value() == 1U );
    }

    /* DB2: cele trei iesiri auxiliare sint fortate OFF pe VW08EU. */
    cfg.aux1 = FUNC_AUX_OFF;
    cfg.aux2 = FUNC_AUX_OFF;
    cfg.aux3 = FUNC_AUX_OFF;

    /* DB2: cfg.is_rsr = (prog_retrieve_option(FEAT_RF_OUTPUT) != NO_RF), cu
       NO_RF = 0x01 din rf.h. Pe contract, valoarea 1 e acelasi caz "fara RF".
       DE VERIFICAT LA BANC: echivalenta valorii 1 intre cele doua reprezentari. */
    /* v5 VW08EU (22.09.2026) - REPARATIE A, oglinda celei de pe DS4 (v14). Era:
           cfg.is_rsr = ( get_prog_feat_rf_output_value() != 1U );
       Valoarea aia nu se aplica NICIODATA: e scrisa in
       core_contract_db3_prog_config_apply_vehicle_specific() (linia 111), care e
       __weak in CORE si definita aici, in FW. CORE si FW sint imagini SEPARATE,
       deci CORE cheama versiunea lui goala, iar a noastra e eliminata de linker
       ca fiind moarta. Dovada pe DB3: zero aparitii ale simbolului in
       fw_db3_release.map. g_prog_config ramine memset-at la 0, deci
       get_prog_feat_rf_output_value() = 0 si is_rsr iesea TRUE -> clutch_disable(),
       adica ambreiaj mort, in tacere, exact ca pe DS4 inainte de 1.00.13.

       Pe VW08EU intentia e neechivoca si scrisa la linia 210: RF output = 1,
       adica starter extern pe D2D, deci is_rsr FALSE si ambreiaj ACTIV.
       Fixam direct, fara sa depindem de un canal care nu functioneaza.

       Reparatia de fond (sa cheme FW-ul hook-ul, ca sa se aplice si celelalte 88
       de optiuni set_prog_*) e separata si cere regresie - vezi RELEASE_NOTES. */
    cfg.is_rsr = FALSE;

    /* DB2: is_rsr ? clutch_disable() : clutch_enable(). Ambreiajul e scos
       temporar din build (vezi STARE_PORTARE.md), deci cele doua simboluri
       ramin nerezolvate la link pina la reincludere. */
    cfg.is_rsr ? clutch_disable() : clutch_enable();

    /* DB2: FEAT_UNLOCK_DRIVER 1 -> prioritate pe usa soferului, 2 -> toate. */
    if (get_prog_feat_unlock_driver_enable() != 0U)
    {
        dl_cfg( DL_DRIVER_DOOR_PRIORITY,
                (void*)( get_prog_feat_unlock_driver_value() == 1U ) );
    }

    dl_cfg( DL_OEM_DISARM_WITH_TRANSPONDER, (void*)FALSE );
}
/*--------------------------------------------------------------------------*/

static void fw_prog_vehicle_specific_enable_apply( void )
{
#define APPLY_VS_ENABLE_STATE(field) \
    do { \
        if (get_prog_##field() != 0U) { \
            enable_prog_##field(); \
        } else { \
            disable_prog_##field(); \
        } \
    } while (0);

    PROG_CONFIG_ENABLE_FIELDS(APPLY_VS_ENABLE_STATE);

#undef APPLY_VS_ENABLE_STATE
}

/*--------------------------------------------------------------------------*/



