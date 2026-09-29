/*<#======================================================================#>*/
/*              CONTRACT WRAPPER PENTRU CONFIG_GPIO.H (DB3)              */
/*<#======================================================================#>*/

#ifndef __CORE_CONTRACT_DB3_GPIO_CONFIG_H__
#define __CORE_CONTRACT_DB3_GPIO_CONFIG_H__

typedef enum {
    GPIO_IFACE_OUTPUT_NONE = 0U,
    GPIO_IFACE_OUTPUT_NEG_OUT1,
    GPIO_IFACE_OUTPUT_NEG_OUT2,
    GPIO_IFACE_OUTPUT_NEG_OUT3,
    GPIO_IFACE_OUTPUT_NEG_OUT4,
    GPIO_IFACE_OUTPUT_NEG_OUT5,
    GPIO_IFACE_OUTPUT_POS_OUT1,
    GPIO_IFACE_OUTPUT_POS_OUT2,
    GPIO_IFACE_OUTPUT_RELAY1,
    GPIO_IFACE_OUTPUT_RELAY2,
    GPIO_IFACE_OUTPUT_RF_LOOP_POWER,
    GPIO_IFACE_OUTPUT_LED_GREEN,
    GPIO_IFACE_OUTPUT_LED_RED,
    GPIO_IFACE_OUTPUT_LED_BLUE,
    GPIO_IFACE_OUTPUT_DAC_POWER,
    /* v2.1 VW08EU: PHAL2 / ISO2_TX (P0.11). Ambreiajul isi scoate PWM-ul pe acest
       pin. Adaugat la COADA enumerarii, ca sa nu schimbe valorile existente. */
    GPIO_IFACE_OUTPUT_ISO2_TX,
    /* v4.1 VW08EU (31.08.2026): PHAL1 / ISO1_TX (P0.10). Iesirea de raportare a
       frinei de mina. In DB2, config_gpio.h are HBRAKE_OUTPUT_SELECTION = 8, iar
       core_gpio.h duce optiunea 8 la _401_OUT_PHAL1_TX, adica P1.24 = ISO1_TX.
       La portare a fost pusa greasit pe NONE ("selectabil de instalator"), deci
       handbrake ON nu comanda niciun pin. Adaugat la COADA enumerarii.
       Pe DB3 pinul e liber: GPIO_PL_ISO1_TX apare doar in gpio_init.c. */
    GPIO_IFACE_OUTPUT_ISO1_TX
} gpio_iface_output_selector_t;

/* IESIRILE DE RAPORTARE A STARII DE PE CAN, v4.4 (01.09.2026) ----------------
   Confirmate de Adrian ca fiind identice cu DB2. Se refera DOAR la raportarea
   statusului de pe CAN, nu la iesirile de comanda (FUNC_*_OUT), care sint alta
   familie, cu alte selectoare in DB2 - nu le confunda.

     HANDBRAKE -> ISO1_TX    (DB2 sel 8 -> _401_OUT_PHAL1_TX)
     IGNITION  -> POS_OUT1   (DB2 sel 1 -> _401_OUT_1_PLUS)
     DOORS     -> NEG_OUT3   (DB2 sel 3 -> _401_OUT_3)
     TRUNK     -> NEG_OUT4   (DB2 sel 4 -> _401_OUT_4)
     HOOD      -> NONE       (DB2 sel 10 = niciun pin)
     KEY_WRAP  -> NONE       (DB2 sel 12 = niciun pin)

   ATENTIE, SUPRAPUNERE NEREZOLVATA: NEG_OUT3 e folosit si de RSR_UNLOCK_OUTPUT
   si de RSR_TRUNK_OUTPUT. Cu DOORS pe acelasi pin, trei functii scriu aceeasi
   iesire. In DB2 asa ceva nu era posibil: acolo core_gpio.h "otraveste" pinul
   ales pentru sense - la DOOR_OUTPUT_SELECTION = 3 defineste
   OUT_3(x) -> OUT_3_used_for_DOOR_SENSE(x), deci orice alta folosire a lui
   OUT_3 nu compileaza. Rezulta ca in DB2 RSR NU putea folosi OUT_3, deci
   maparea RSR din contractul nostru e o presupunere de la portare, nu ceva
   preluat. DE CLARIFICAT cu Adrian daca RSR se foloseste la VW08EU si pe ce
   iesiri. Pina atunci nu am schimbat RSR - maparea ceruta e cea de mai sus. */
/* HBRAKE_SENSE pe DB3 = ISO1_TX (P0.10) --------------------------------------
   Sursa: DB2 config_gpio.h are HBRAKE_OUTPUT_SELECTION = 8, iar core_gpio.h duce
   optiunea 8 la _401_OUT_PHAL1_TX = P1.24 = ISO1_TX. La portare fusese pusa
   greasit pe NONE ("selectabil de instalator"), deci handbrake ON nu comanda
   niciun pin.

   Nota de banc, 31.08.2026: prima incercare a produs un reset cu RSIR = 0x0C
   (WDTR + BODR = brown-out) la tranzitia handbrake OFF -> ON. Cauza NU era
   maparea si nici placa: era un jumper pus greasit pe placa de test, pe
   pull-up-ul de ISO1. Cu jumperul corect, maparea functioneaza. Se pastreaza
   consemnat ca sa nu se mai piarda timp pe aceeasi pista.

   ISO1 are pull-up-uri comutabile pe placa: GPIO_PL_PULL_12V_ISO1 (P0.21) si
   GPIO_PL_PULL_5V_ISO1 (P0.19). Firmware-ul nu le atinge; sint de configurat
   hardware. Daca reapare un brown-out la comanda lui ISO1_TX, verifica intii
   ACOLO, nu in cod. */

#define CORE_DB3_GPIO_IFACE_HANDBRAKE_OUTPUT   GPIO_IFACE_OUTPUT_ISO1_TX   /* v4.3 VW08EU (31.08.2026): DB2 HBRAKE_OUTPUT_SELECTION=8 -> _401_OUT_PHAL1_TX = ISO1_TX */
#define CORE_DB3_GPIO_IFACE_BRAKE_OUTPUT       GPIO_IFACE_OUTPUT_RELAY1   /* v2.1 VW08EU: acelasi releu cu CLUTCH, ca in DB2 */
/* BRAKE_SENSE pe DAC_RELAY ----------------------------------------------------
   v5 VW08EU (11.09.2026). In DB2 exista DOUA iesiri de frina, distincte:
     VW03_FUNC_BRAKE_OUT(x) = OUT_RELAY1(x)          -> actuatia (demaraj)
     BRAKE_SENSE_SET/CLR    = _401_OUT_DAC_RELAY(..) -> STATUSUL de frina
   (core_gpio.h:122-123 din DB2; _401_OUT_DAC_RELAY = P1.4).
   La portare cele doua au fost contopite intr-o singura intrare de contract,
   asa ca statusul de frina a ajuns sa clanteze releul 1 - se aude si se vede,
   raportat de Adrian la banc.
   DAC_RELAY nu e un releu obisnuit: comanda un convertor digital-analogic
   (CPC1008N pe DB2, vezi lib.dac/dac_set_relay_ctl.c). Numele de aici,
   DAC_POWER, e acelasi lucru - SDK-ul aliaza explicit
   OUT_DAC_RELAY(x) -> GPIO_PL_DAC_PWR_CTL, pe amindoua platformele
   (db3_gpio.h:19, 933_gpio.h:456). Pinul difera: P0.18 pe DB3, P1.4 pe DS4.
   Confuzia de semantica vine din faptul ca toate trei se numesc "RELAY".
   Cine foloseste statusul, deci trece pe aici: sense_rs_can_brake.c (sense_brake
   si, prin el, sense_rs_can_brake_force_on - pulsul de la oprirea la viteza) si
   sense_notify_brake.c. Actuatia ramine pe RELAY1: FUNC_BRAKE_OUT din func_rs.c
   si run_safe_shutdown.c. Exact impartirea din DB2. */
#define CORE_DB3_GPIO_IFACE_BRAKE_SENSE_OUTPUT GPIO_IFACE_OUTPUT_DAC_POWER
#define CORE_DB3_GPIO_IFACE_IGNITION_OUTPUT    GPIO_IFACE_OUTPUT_POS_OUT1   /* v4.4 VW08EU (01.09.2026): DB2 IGNITION_OUTPUT_SELECTION=1 -> _401_OUT_1_PLUS */
#define CORE_DB3_GPIO_IFACE_HOOD_OUTPUT        GPIO_IFACE_OUTPUT_NONE
#define CORE_DB3_GPIO_IFACE_KEY_WRAP_OUTPUT    GPIO_IFACE_OUTPUT_NONE
#define CORE_DB3_GPIO_IFACE_LIGHTS_OUTPUT      GPIO_IFACE_OUTPUT_NEG_OUT1   /* v4 VW08EU: FUNC_HAZARD_OUT -> OUT_1 */
#define CORE_DB3_GPIO_IFACE_DOORS_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT3   /* v4.4 VW08EU (01.09.2026): DB2 DOOR_OUTPUT_SELECTION=3 -> _401_OUT_3 */
#define CORE_DB3_GPIO_IFACE_TRUNK_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT4   /* v4.4 VW08EU (01.09.2026): DB2 TRUNK_OUTPUT_SELECTION=4 -> _401_OUT_4 */
#define CORE_DB3_GPIO_IFACE_PTS_1_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT2   /* v4 VW08EU: FUNC_PTS_OUT -> SPEED_PROT_NORMAL(OUT_2) */
#define CORE_DB3_GPIO_IFACE_PTS_2_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* v4 VW08EU: DB2 are o singura iesire PTS */
#define CORE_DB3_GPIO_IFACE_CLUTCH_OUTPUT      GPIO_IFACE_OUTPUT_RELAY1

/* v2.1 VW08EU - ambreiaj. Maparea vine din config_gpio.h (DB2, ramura DBALL):
     VW03_CLUTCH_PWM_OUT(x)  OUT_PHAL2_TX(!x)      -> ISO2_TX, inversat la apelant
     VW03_CLUTCH_PWM_CUT(x)  OUT_RELAY1(x)         -> aceeasi clasa cu CLUTCH/BRAKE
     VW03_CLUTCH_SENSOR(x)   OUT_RELAY2(x);OUT_5(x)-> doua clase, A si B
   NOTA: PWM-ul si RAP-ul stau pe acelasi pin fizic si in DB2 (RAP_ON_WIRE_SET ->
   ISO2_TX_SET). Nu sint niciodata active simultan: RAP e dupa shutdown, PWM-ul e
   la demaraj. Confirmat cu Adrian. */
#define CORE_DB3_GPIO_IFACE_CLUTCH_PWM_OUTPUT       GPIO_IFACE_OUTPUT_ISO2_TX
#define CORE_DB3_GPIO_IFACE_CLUTCH_SENSOR_A_OUTPUT  GPIO_IFACE_OUTPUT_RELAY2
#define CORE_DB3_GPIO_IFACE_CLUTCH_SENSOR_B_OUTPUT  GPIO_IFACE_OUTPUT_NEG_OUT5
#define CORE_DB3_GPIO_IFACE_RAP_OUTPUT         GPIO_IFACE_OUTPUT_ISO2_TX   /* v4 VW08EU: RAP_ON_WIRE_SET -> ISO2_TX_SET */
#define CORE_DB3_GPIO_IFACE_RF_LOOP_OUTPUT     GPIO_IFACE_OUTPUT_RF_LOOP_POWER
#define CORE_DB3_GPIO_IFACE_RSR_LOCK_OUTPUT    GPIO_IFACE_OUTPUT_NEG_OUT5
#define CORE_DB3_GPIO_IFACE_RSR_UNLOCK_OUTPUT  GPIO_IFACE_OUTPUT_NEG_OUT3
#define CORE_DB3_GPIO_IFACE_RSR_TRUNK_OUTPUT   GPIO_IFACE_OUTPUT_NEG_OUT3

#define get_gpio_iface_handbrake_output()   ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_HANDBRAKE_OUTPUT)
#define get_gpio_iface_brake_output()       ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_BRAKE_OUTPUT)
#define get_gpio_iface_brake_sense_output() ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_BRAKE_SENSE_OUTPUT)
#define get_gpio_iface_ignition_output()    ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_IGNITION_OUTPUT)
#define get_gpio_iface_hood_output()        ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_HOOD_OUTPUT)
#define get_gpio_iface_key_wrap_output()    ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_KEY_WRAP_OUTPUT)
#define get_gpio_iface_lights_output()      ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_LIGHTS_OUTPUT)
#define get_gpio_iface_doors_output()       ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_DOORS_OUTPUT)
#define get_gpio_iface_trunk_output()       ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_TRUNK_OUTPUT)
#define get_gpio_iface_pts_1_output()       ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_PTS_1_OUTPUT)
#define get_gpio_iface_pts_2_output()       ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_PTS_2_OUTPUT)
#define get_gpio_iface_clutch_output()      ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_CLUTCH_OUTPUT)
#define get_gpio_iface_clutch_pwm_output()      ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_CLUTCH_PWM_OUTPUT)
#define get_gpio_iface_clutch_sensor_a_output() ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_CLUTCH_SENSOR_A_OUTPUT)
#define get_gpio_iface_clutch_sensor_b_output() ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_CLUTCH_SENSOR_B_OUTPUT)
#define get_gpio_iface_rap_output()         ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_RAP_OUTPUT)
#define get_gpio_iface_rf_loop_output()     ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_RF_LOOP_OUTPUT)
#define get_gpio_iface_rsr_lock_output()    ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_RSR_LOCK_OUTPUT)
#define get_gpio_iface_rsr_unlock_output()  ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_RSR_UNLOCK_OUTPUT)
#define get_gpio_iface_rsr_trunk_output()   ((gpio_iface_output_selector_t)CORE_DB3_GPIO_IFACE_RSR_TRUNK_OUTPUT)

#define GPIO_IFACE_CFG_HANDBRAKE_OUTPUT  (get_gpio_iface_handbrake_output())
#define GPIO_IFACE_CFG_BRAKE_OUTPUT      (get_gpio_iface_brake_output())
#define GPIO_IFACE_CFG_BRAKE_SENSE_OUTPUT (get_gpio_iface_brake_sense_output())
#define GPIO_IFACE_CFG_IGNITION_OUTPUT   (get_gpio_iface_ignition_output())
#define GPIO_IFACE_CFG_HOOD_OUTPUT       (get_gpio_iface_hood_output())
#define GPIO_IFACE_CFG_KEY_WRAP_OUTPUT   (get_gpio_iface_key_wrap_output())
#define GPIO_IFACE_CFG_LIGHTS_OUTPUT     (get_gpio_iface_lights_output())
#define GPIO_IFACE_CFG_DOORS_OUTPUT      (get_gpio_iface_doors_output())
#define GPIO_IFACE_CFG_TRUNK_OUTPUT      (get_gpio_iface_trunk_output())
#define GPIO_IFACE_CFG_PTS_1_OUTPUT      (get_gpio_iface_pts_1_output())
#define GPIO_IFACE_CFG_PTS_2_OUTPUT      (get_gpio_iface_pts_2_output())
#define GPIO_IFACE_CFG_CLUTCH_OUTPUT     (get_gpio_iface_clutch_output())
#define GPIO_IFACE_CFG_CLUTCH_PWM_OUTPUT      (get_gpio_iface_clutch_pwm_output())
#define GPIO_IFACE_CFG_CLUTCH_SENSOR_A_OUTPUT (get_gpio_iface_clutch_sensor_a_output())
#define GPIO_IFACE_CFG_CLUTCH_SENSOR_B_OUTPUT (get_gpio_iface_clutch_sensor_b_output())
#define GPIO_IFACE_CFG_RAP_OUTPUT        (get_gpio_iface_rap_output())
#define GPIO_IFACE_CFG_RF_LOOP_OUTPUT    (get_gpio_iface_rf_loop_output())
#define GPIO_IFACE_CFG_RSR_LOCK_OUTPUT   (get_gpio_iface_rsr_lock_output())
#define GPIO_IFACE_CFG_RSR_UNLOCK_OUTPUT (get_gpio_iface_rsr_unlock_output())
#define GPIO_IFACE_CFG_RSR_TRUNK_OUTPUT  (get_gpio_iface_rsr_trunk_output())

#define CORE_DB3_GPIO_CONFIG_VERSION      4
#define CORE_DB3_GPIO_CONFIG_FREEZE_DATE  20260901UL

/*
 * GPIO_IFACE contract freeze guardrail.
 * Update these expected values only when intentionally revving contract version.
 */
#define CORE_DB3_GPIO_IFACE_EXPECT_HANDBRAKE_OUTPUT   GPIO_IFACE_OUTPUT_ISO1_TX   /* v4.3 VW08EU (31.08.2026): DB2 HBRAKE_OUTPUT_SELECTION=8 -> _401_OUT_PHAL1_TX = ISO1_TX */
#define CORE_DB3_GPIO_IFACE_EXPECT_BRAKE_OUTPUT       GPIO_IFACE_OUTPUT_RELAY1   /* v4: VW08EU, acelasi releu cu CLUTCH */
#define CORE_DB3_GPIO_IFACE_EXPECT_BRAKE_SENSE_OUTPUT GPIO_IFACE_OUTPUT_DAC_POWER   /* v5 VW08EU (11.09.2026): DB2 BRAKE_SENSE_SET -> _401_OUT_DAC_RELAY */
#define CORE_DB3_GPIO_IFACE_EXPECT_IGNITION_OUTPUT    GPIO_IFACE_OUTPUT_POS_OUT1   /* v4.4 VW08EU (01.09.2026): DB2 IGNITION_OUTPUT_SELECTION=1 -> _401_OUT_1_PLUS */
#define CORE_DB3_GPIO_IFACE_EXPECT_HOOD_OUTPUT        GPIO_IFACE_OUTPUT_NONE
#define CORE_DB3_GPIO_IFACE_EXPECT_KEY_WRAP_OUTPUT    GPIO_IFACE_OUTPUT_NONE
#define CORE_DB3_GPIO_IFACE_EXPECT_LIGHTS_OUTPUT      GPIO_IFACE_OUTPUT_NEG_OUT1   /* v4 VW08EU: FUNC_HAZARD_OUT -> OUT_1 */
#define CORE_DB3_GPIO_IFACE_EXPECT_DOORS_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT3   /* v4.4 VW08EU (01.09.2026): DB2 DOOR_OUTPUT_SELECTION=3 -> _401_OUT_3 */
#define CORE_DB3_GPIO_IFACE_EXPECT_TRUNK_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT4   /* v4.4 VW08EU (01.09.2026): DB2 TRUNK_OUTPUT_SELECTION=4 -> _401_OUT_4 */
#define CORE_DB3_GPIO_IFACE_EXPECT_PTS_1_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT2   /* v4 VW08EU: FUNC_PTS_OUT -> SPEED_PROT_NORMAL(OUT_2) */
#define CORE_DB3_GPIO_IFACE_EXPECT_PTS_2_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* v4 VW08EU: DB2 are o singura iesire PTS */
#define CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_OUTPUT      GPIO_IFACE_OUTPUT_RELAY1
#define CORE_DB3_GPIO_IFACE_EXPECT_RAP_OUTPUT         GPIO_IFACE_OUTPUT_ISO2_TX   /* v4 VW08EU: RAP_ON_WIRE_SET -> ISO2_TX_SET */
#define CORE_DB3_GPIO_IFACE_EXPECT_RF_LOOP_OUTPUT     GPIO_IFACE_OUTPUT_RF_LOOP_POWER
#define CORE_DB3_GPIO_IFACE_EXPECT_RSR_LOCK_OUTPUT    GPIO_IFACE_OUTPUT_NEG_OUT5
#define CORE_DB3_GPIO_IFACE_EXPECT_RSR_UNLOCK_OUTPUT  GPIO_IFACE_OUTPUT_NEG_OUT3
#define CORE_DB3_GPIO_IFACE_EXPECT_RSR_TRUNK_OUTPUT   GPIO_IFACE_OUTPUT_NEG_OUT3
/* v4 (VW08EU): clasele de ambreiaj */
#define CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_PWM_OUTPUT       GPIO_IFACE_OUTPUT_ISO2_TX
#define CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_A_OUTPUT  GPIO_IFACE_OUTPUT_RELAY2
#define CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_B_OUTPUT  GPIO_IFACE_OUTPUT_NEG_OUT5

#define CORE_DB3_GPIO_MAP_ASSERT(name, expr) typedef char core_db3_gpio_map_assert_##name[(expr) ? 1 : -1]

CORE_DB3_GPIO_MAP_ASSERT(handbrake_fixed, CORE_DB3_GPIO_IFACE_HANDBRAKE_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_HANDBRAKE_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(brake_fixed, CORE_DB3_GPIO_IFACE_BRAKE_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_BRAKE_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(brake_sense_fixed, CORE_DB3_GPIO_IFACE_BRAKE_SENSE_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_BRAKE_SENSE_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(ignition_fixed, CORE_DB3_GPIO_IFACE_IGNITION_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_IGNITION_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(hood_fixed, CORE_DB3_GPIO_IFACE_HOOD_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_HOOD_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(key_wrap_fixed, CORE_DB3_GPIO_IFACE_KEY_WRAP_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_KEY_WRAP_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(lights_fixed, CORE_DB3_GPIO_IFACE_LIGHTS_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_LIGHTS_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(doors_fixed, CORE_DB3_GPIO_IFACE_DOORS_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_DOORS_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(trunk_fixed, CORE_DB3_GPIO_IFACE_TRUNK_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_TRUNK_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(pts1_fixed, CORE_DB3_GPIO_IFACE_PTS_1_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_PTS_1_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(pts2_fixed, CORE_DB3_GPIO_IFACE_PTS_2_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_PTS_2_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(clutch_fixed, CORE_DB3_GPIO_IFACE_CLUTCH_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(rap_fixed, CORE_DB3_GPIO_IFACE_RAP_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_RAP_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(rf_loop_fixed, CORE_DB3_GPIO_IFACE_RF_LOOP_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_RF_LOOP_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(rsr_lock_fixed, CORE_DB3_GPIO_IFACE_RSR_LOCK_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_RSR_LOCK_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(rsr_unlock_fixed, CORE_DB3_GPIO_IFACE_RSR_UNLOCK_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_RSR_UNLOCK_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(rsr_trunk_fixed, CORE_DB3_GPIO_IFACE_RSR_TRUNK_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_RSR_TRUNK_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(clutch_pwm_fixed, CORE_DB3_GPIO_IFACE_CLUTCH_PWM_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_PWM_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(clutch_sensor_a_fixed, CORE_DB3_GPIO_IFACE_CLUTCH_SENSOR_A_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_A_OUTPUT);
CORE_DB3_GPIO_MAP_ASSERT(clutch_sensor_b_fixed, CORE_DB3_GPIO_IFACE_CLUTCH_SENSOR_B_OUTPUT == CORE_DB3_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_B_OUTPUT);

#undef CORE_DB3_GPIO_MAP_ASSERT

#endif /* __CORE_CONTRACT_DB3_GPIO_CONFIG_H__ */
