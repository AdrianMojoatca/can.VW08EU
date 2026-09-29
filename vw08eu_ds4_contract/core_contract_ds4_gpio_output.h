#ifndef __CORE_CONTRACT_DS4_GPIO_OUTPUT_H__
#define __CORE_CONTRACT_DS4_GPIO_OUTPUT_H__

#include "device.h"
#include "core_contract_ds4_gpio_config.h"
#include "gpio.h"

/* DS4 PROVISIONAL (FIXME): the POS_OUT1/POS_OUT2 selectors are inherited from the
   DB3-derived map, but DS4 has NO GPIO_PL_POS_OUT6/POS_OUT2 (see 933_gpio.h). To
   unblock the DS4 build, the switch bodies below drive spare VALID DS4 pins
   (POS_OUT1 selector -> POS_OUT6, POS_OUT2 selector -> POS_OUT7). The real DS4
   function->output assignment (via config_933_gpio.h) is DEFERRED. DO NOT bench/
   vehicle-flash with brake on the placeholder pin before that mapping is finalized. */

static void core_contract_ds4_gpio_output_on(gpio_iface_output_selector_t output_selector)
{
    switch (output_selector)
    {
        case GPIO_IFACE_OUTPUT_NEG_OUT1: gpio_pl_set(GPIO_PL_NEG_OUT1); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT2: gpio_pl_set(GPIO_PL_NEG_OUT2); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT3: gpio_pl_set(GPIO_PL_NEG_OUT3); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT4: gpio_pl_set(GPIO_PL_NEG_OUT4); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT5: gpio_pl_set(GPIO_PL_NEG_OUT5); break;
        case GPIO_IFACE_OUTPUT_POS_OUT1: gpio_pl_set(GPIO_PL_POS_OUT6); break;
        case GPIO_IFACE_OUTPUT_POS_OUT2: gpio_pl_set(GPIO_PL_POS_OUT7); break;
        case GPIO_IFACE_OUTPUT_RELAY1: gpio_pl_set(GPIO_PL_RLY1_CTL); break;
        case GPIO_IFACE_OUTPUT_RELAY2: gpio_pl_set(GPIO_PL_RLY2_CTL); break;
        case GPIO_IFACE_OUTPUT_RF_LOOP_POWER: gpio_pl_set(GPIO_PL_RFLOOP_POWER); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT13: gpio_pl_set(GPIO_PL_NEG_OUT13); break;
        case GPIO_IFACE_OUTPUT_LED_GREEN: gpio_pl_clr(GPIO_PL_LED_GREEN); break;
        case GPIO_IFACE_OUTPUT_LED_RED: gpio_pl_clr(GPIO_PL_LED_RED); break;
        case GPIO_IFACE_OUTPUT_LED_BLUE: gpio_pl_clr(GPIO_PL_LED_BLUE); break;
        case GPIO_IFACE_OUTPUT_DAC_POWER: gpio_pl_set(GPIO_PL_DAC_PWR_CTL); break;
        /* v17 VW08EU (22.09.2026): ISO2_TX si ISO1_TX LIPSEAU din switch.
           Erau declarate in enum si CLUTCH_PWM_OUTPUT le folosea, dar aici
           cadeau pe default: break; - scriere inghitita in tacere, fara warning,
           fiindca "case NONE: default:" acopera orice valoare noua din enum.
           Simptom: pe DS4 ISO2 (PWM-ul de ambreiaj) nu se activa; pe DB3 mergea,
           fiindca acolo cele sase cazuri exista din v2.1 / v4.1. */
        case GPIO_IFACE_OUTPUT_ISO2_TX: gpio_pl_set(GPIO_PL_ISO2_TX); break;
        case GPIO_IFACE_OUTPUT_ISO1_TX: gpio_pl_set(GPIO_PL_ISO1_TX); break;
        /* v18 VW08EU (22.09.2026): RELAY3, numai pe DS4. Preia senzorul A de
           ambreiaj, ca RELAY2 sa ramina liber pentru GWR ON. */
        case GPIO_IFACE_OUTPUT_RELAY3: gpio_pl_set(GPIO_PL_RLY3_CTL); break;
        case GPIO_IFACE_OUTPUT_NONE:
        default:
            break;
    }
}

static void core_contract_ds4_gpio_output_off(gpio_iface_output_selector_t output_selector)
{
    switch (output_selector)
    {
        case GPIO_IFACE_OUTPUT_NEG_OUT1: gpio_pl_clr(GPIO_PL_NEG_OUT1); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT2: gpio_pl_clr(GPIO_PL_NEG_OUT2); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT3: gpio_pl_clr(GPIO_PL_NEG_OUT3); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT4: gpio_pl_clr(GPIO_PL_NEG_OUT4); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT5: gpio_pl_clr(GPIO_PL_NEG_OUT5); break;
        case GPIO_IFACE_OUTPUT_POS_OUT1: gpio_pl_clr(GPIO_PL_POS_OUT6); break;
        case GPIO_IFACE_OUTPUT_POS_OUT2: gpio_pl_clr(GPIO_PL_POS_OUT7); break;
        case GPIO_IFACE_OUTPUT_RELAY1: gpio_pl_clr(GPIO_PL_RLY1_CTL); break;
        case GPIO_IFACE_OUTPUT_RELAY2: gpio_pl_clr(GPIO_PL_RLY2_CTL); break;
        case GPIO_IFACE_OUTPUT_RF_LOOP_POWER: gpio_pl_clr(GPIO_PL_RFLOOP_POWER); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT13: gpio_pl_clr(GPIO_PL_NEG_OUT13); break;
        case GPIO_IFACE_OUTPUT_LED_GREEN: gpio_pl_set(GPIO_PL_LED_GREEN); break;
        case GPIO_IFACE_OUTPUT_LED_RED: gpio_pl_set(GPIO_PL_LED_RED); break;
        case GPIO_IFACE_OUTPUT_LED_BLUE: gpio_pl_set(GPIO_PL_LED_BLUE); break;
        case GPIO_IFACE_OUTPUT_DAC_POWER: gpio_pl_clr(GPIO_PL_DAC_PWR_CTL); break;
        /* v17 VW08EU (22.09.2026): ISO2_TX si ISO1_TX LIPSEAU din switch.
           Erau declarate in enum si CLUTCH_PWM_OUTPUT le folosea, dar aici
           cadeau pe default: break; - scriere inghitita in tacere, fara warning,
           fiindca "case NONE: default:" acopera orice valoare noua din enum.
           Simptom: pe DS4 ISO2 (PWM-ul de ambreiaj) nu se activa; pe DB3 mergea,
           fiindca acolo cele sase cazuri exista din v2.1 / v4.1. */
        case GPIO_IFACE_OUTPUT_ISO2_TX: gpio_pl_clr(GPIO_PL_ISO2_TX); break;
        case GPIO_IFACE_OUTPUT_ISO1_TX: gpio_pl_clr(GPIO_PL_ISO1_TX); break;
        /* v18 VW08EU (22.09.2026): RELAY3, numai pe DS4. Preia senzorul A de
           ambreiaj, ca RELAY2 sa ramina liber pentru GWR ON. */
        case GPIO_IFACE_OUTPUT_RELAY3: gpio_pl_clr(GPIO_PL_RLY3_CTL); break;
        case GPIO_IFACE_OUTPUT_NONE:
        default:
            break;
    }
}

static void core_contract_ds4_gpio_output_toggle(gpio_iface_output_selector_t output_selector)
{
    switch (output_selector)
    {
        case GPIO_IFACE_OUTPUT_NEG_OUT1: gpio_pl_tog(GPIO_PL_NEG_OUT1); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT2: gpio_pl_tog(GPIO_PL_NEG_OUT2); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT3: gpio_pl_tog(GPIO_PL_NEG_OUT3); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT4: gpio_pl_tog(GPIO_PL_NEG_OUT4); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT5: gpio_pl_tog(GPIO_PL_NEG_OUT5); break;
        case GPIO_IFACE_OUTPUT_POS_OUT1: gpio_pl_tog(GPIO_PL_POS_OUT6); break;
        case GPIO_IFACE_OUTPUT_POS_OUT2: gpio_pl_tog(GPIO_PL_POS_OUT7); break;
        case GPIO_IFACE_OUTPUT_RELAY1: gpio_pl_tog(GPIO_PL_RLY1_CTL); break;
        case GPIO_IFACE_OUTPUT_RELAY2: gpio_pl_tog(GPIO_PL_RLY2_CTL); break;
        case GPIO_IFACE_OUTPUT_RF_LOOP_POWER: gpio_pl_tog(GPIO_PL_RFLOOP_POWER); break;
        case GPIO_IFACE_OUTPUT_NEG_OUT13: gpio_pl_tog(GPIO_PL_NEG_OUT13); break;
        case GPIO_IFACE_OUTPUT_LED_GREEN: gpio_pl_tog(GPIO_PL_LED_GREEN); break;
        case GPIO_IFACE_OUTPUT_LED_RED: gpio_pl_tog(GPIO_PL_LED_RED); break;
        case GPIO_IFACE_OUTPUT_LED_BLUE: gpio_pl_tog(GPIO_PL_LED_BLUE); break;
        case GPIO_IFACE_OUTPUT_DAC_POWER: gpio_pl_tog(GPIO_PL_DAC_PWR_CTL); break;
        /* v17 VW08EU (22.09.2026): ISO2_TX si ISO1_TX LIPSEAU din switch.
           Erau declarate in enum si CLUTCH_PWM_OUTPUT le folosea, dar aici
           cadeau pe default: break; - scriere inghitita in tacere, fara warning,
           fiindca "case NONE: default:" acopera orice valoare noua din enum.
           Simptom: pe DS4 ISO2 (PWM-ul de ambreiaj) nu se activa; pe DB3 mergea,
           fiindca acolo cele sase cazuri exista din v2.1 / v4.1. */
        case GPIO_IFACE_OUTPUT_ISO2_TX: gpio_pl_tog(GPIO_PL_ISO2_TX); break;
        case GPIO_IFACE_OUTPUT_ISO1_TX: gpio_pl_tog(GPIO_PL_ISO1_TX); break;
        /* v18 VW08EU (22.09.2026): RELAY3, numai pe DS4. Preia senzorul A de
           ambreiaj, ca RELAY2 sa ramina liber pentru GWR ON. */
        case GPIO_IFACE_OUTPUT_RELAY3: gpio_pl_tog(GPIO_PL_RLY3_CTL); break;
        case GPIO_IFACE_OUTPUT_NONE:
        default:
            break;
    }
}

static void core_contract_ds4_gpio_handbrake_on(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_handbrake_output());
}

static void core_contract_ds4_gpio_handbrake_off(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_handbrake_output());
}

static void core_contract_ds4_gpio_doors_open(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_doors_output());
}

static void core_contract_ds4_gpio_doors_closed(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_doors_output());
}

static void core_contract_ds4_gpio_lights_on(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_lights_output());
}

static void core_contract_ds4_gpio_lights_off(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_lights_output());
}

static void core_contract_ds4_gpio_trunk_open(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_trunk_output());
}

static void core_contract_ds4_gpio_trunk_closed(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_trunk_output());
}

static void core_contract_ds4_gpio_brake_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_brake_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_brake_output());
    }
}

/* v5 VW08EU (11.09.2026): STATUSUL de frina, distinct de actuatie. Vezi
   comentariul de la CORE_DS4_GPIO_IFACE_BRAKE_SENSE_OUTPUT. Nu e rutat prin
   tabela de API - toti apelantii sint in CORE. */
static void core_contract_ds4_gpio_brake_sense_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_brake_sense_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_brake_sense_output());
    }
}


/* S6 VW08EU: iesirile de ambreiaj. CLUTCH imparte pinul cu BRAKE (RELAY1), ca in DB2
   unde VW03_CLUTCH_PWM_CUT(x) era OUT_RELAY1(x). CLUTCH_SENSOR_B e inca pe NONE. */
static void core_contract_ds4_gpio_clutch_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_clutch_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_clutch_output());
    }
}

static void core_contract_ds4_gpio_clutch_pwm_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_clutch_pwm_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_clutch_pwm_output());
    }
}

static void core_contract_ds4_gpio_clutch_sensor_a_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_clutch_sensor_a_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_clutch_sensor_a_output());
    }
}

static void core_contract_ds4_gpio_clutch_sensor_b_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_clutch_sensor_b_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_clutch_sensor_b_output());
    }
}
static void core_contract_ds4_gpio_pts_1_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_pts_1_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_pts_1_output());
    }
}

static void core_contract_ds4_gpio_pts_2_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_pts_2_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_pts_2_output());
    }
}

static void core_contract_ds4_gpio_rap_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_rap_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_rap_output());
    }
}

static void core_contract_ds4_gpio_rf_loop_set(Boolean state)
{
    if (state == TRUE)
    {
        core_contract_ds4_gpio_output_on(get_gpio_iface_rf_loop_output());
    }
    else
    {
        core_contract_ds4_gpio_output_off(get_gpio_iface_rf_loop_output());
    }
}

static void core_contract_ds4_gpio_ignition_on(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_ignition_output());
}

static void core_contract_ds4_gpio_ignition_off(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_ignition_output());
}

static void core_contract_ds4_gpio_hood_open(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_hood_output());
}

static void core_contract_ds4_gpio_hood_closed(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_hood_output());
}

static void core_contract_ds4_gpio_key_wrap_on(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_key_wrap_output());
}

static void core_contract_ds4_gpio_key_wrap_off(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_key_wrap_output());
}

static void core_contract_ds4_gpio_led_red_on(void)
{
    core_contract_ds4_gpio_output_on(GPIO_IFACE_OUTPUT_LED_RED);
}

static void core_contract_ds4_gpio_led_red_off(void)
{
    core_contract_ds4_gpio_output_off(GPIO_IFACE_OUTPUT_LED_RED);
}

static void core_contract_ds4_gpio_led_green_on(void)
{
    core_contract_ds4_gpio_output_on(GPIO_IFACE_OUTPUT_LED_GREEN);
}

static void core_contract_ds4_gpio_led_green_off(void)
{
    core_contract_ds4_gpio_output_off(GPIO_IFACE_OUTPUT_LED_GREEN);
}

static void core_contract_ds4_gpio_led_blue_on(void)
{
    core_contract_ds4_gpio_output_on(GPIO_IFACE_OUTPUT_LED_BLUE);
}

static void core_contract_ds4_gpio_led_blue_off(void)
{
    core_contract_ds4_gpio_output_off(GPIO_IFACE_OUTPUT_LED_BLUE);
}

static void core_contract_ds4_gpio_rsr_lock_on(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_rsr_lock_output());
}

static void core_contract_ds4_gpio_rsr_lock_off(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_rsr_lock_output());
}

static void core_contract_ds4_gpio_rsr_unlock_on(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_rsr_unlock_output());
}

static void core_contract_ds4_gpio_rsr_unlock_off(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_rsr_unlock_output());
}

static void core_contract_ds4_gpio_rsr_trunk_on(void)
{
    core_contract_ds4_gpio_output_on(get_gpio_iface_rsr_trunk_output());
}

static void core_contract_ds4_gpio_rsr_trunk_off(void)
{
    core_contract_ds4_gpio_output_off(get_gpio_iface_rsr_trunk_output());
}

static void core_contract_ds4_gpio_rf_loop_toggle(void)
{
    core_contract_ds4_gpio_output_toggle(get_gpio_iface_rf_loop_output());
}

#endif
