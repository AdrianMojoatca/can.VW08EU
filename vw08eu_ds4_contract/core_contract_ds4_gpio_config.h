/*<#======================================================================#>*/
/*              CONTRACT WRAPPER PENTRU CONFIG_GPIO.H (DS4)              */
/*<#======================================================================#>*/

#ifndef __CORE_CONTRACT_DS4_GPIO_CONFIG_H__
#define __CORE_CONTRACT_DS4_GPIO_CONFIG_H__

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
    GPIO_IFACE_OUTPUT_NEG_OUT13   /* OUT_13 = NEG_OUT13 = P1.29, 22-pin/10 Light Blue/Black.
                                     v15 (17.09.2026): nu mai e folosit de niciun selector - pinul
                                     a trecut la GWR prin DEI (FEAT_DEI_SYS52). Intrarea ramine in
                                     enum, renumerotarea ar rupe ABI-ul. */,
    /* VW08EU/DS4: ambreiajul isi scoate PWM-ul pe ISO2_TX (P0.11), ca pe DB3.
       ISO1_TX (P0.10) adaugat pentru simetrie cu contractul DB3, unde poarta
       HBRAKE_SENSE; pe DS4 raportarea de stare nu comanda pini, deci ramine
       nefolosit deocamdata. Ambele adaugate la COADA, nu schimba valorile
       existente. */
    GPIO_IFACE_OUTPUT_ISO2_TX,
    GPIO_IFACE_OUTPUT_ISO1_TX,
    /* v18 VW08EU (22.09.2026): RELAY3 exista numai pe DS4 - DB3 are doua relee.
       GPIO_PL_RLY3_CTL = P1.16, initializat in 933_gpio_init.c. Adaugat tot la
       COADA, ca sa nu schimbe valorile existente. ATENTIE: o intrare noua aici
       NU face nimic pina nu primeste si cele trei case-uri din
       core_contract_ds4_gpio_output.h (set/clr/tog) - vezi ce a patit ISO2_TX. */
    GPIO_IFACE_OUTPUT_RELAY3
} gpio_iface_output_selector_t;

#define CORE_DS4_GPIO_IFACE_HANDBRAKE_OUTPUT   GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_BRAKE_OUTPUT       GPIO_IFACE_OUTPUT_RELAY1
/* BRAKE_SENSE pe DAC_RELAY - v5 VW08EU (11.09.2026), oglinda a fixului de pe DB3.
   In DB2 existau DOUA iesiri de frina, distincte:
     VW03_FUNC_BRAKE_OUT(x) = OUT_RELAY1(x)          -> actuatia (demaraj)
     BRAKE_SENSE_SET/CLR    = _401_OUT_DAC_RELAY(..) -> STATUSUL de frina
   (core_gpio.h:122-123 din DB2; _401_OUT_DAC_RELAY = P1.4 - acelasi pin ca aici).
   La portare au fost contopite, deci statusul clantea releul 1.
   DAC_RELAY nu e releu: comanda un convertor digital-analogic. Numele DAC_POWER
   de aici e acelasi lucru - SDK-ul aliaza OUT_DAC_RELAY(x) -> GPIO_PL_DAC_PWR_CTL
   (933_gpio.h:456), iar GPIO_PL_DAC_PWR_CTL e P1.4 (933_gpio.h:210).
   Pe statusul asta trec sense_rs_can_brake.c (inclusiv pulsul de la oprirea la
   viteza, prin sense_rs_can_brake_force_on) si sense_notify_brake.c.
   Actuatia ramine pe RELAY1: FUNC_BRAKE_OUT si run_safe_shutdown.c. */
#define CORE_DS4_GPIO_IFACE_BRAKE_SENSE_OUTPUT GPIO_IFACE_OUTPUT_DAC_POWER
#define CORE_DS4_GPIO_IFACE_IGNITION_OUTPUT    GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_HOOD_OUTPUT        GPIO_IFACE_OUTPUT_NONE
#define CORE_DS4_GPIO_IFACE_KEY_WRAP_OUTPUT    GPIO_IFACE_OUTPUT_NONE
#define CORE_DS4_GPIO_IFACE_LIGHTS_OUTPUT      GPIO_IFACE_OUTPUT_NEG_OUT1   /* VW08EU: FUNC_HAZARD_OUT (func_hazard_ctrl.c) -> OUT_1_NEGATIVE, ca in DB3 */
#define CORE_DS4_GPIO_IFACE_DOORS_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_TRUNK_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
/* v6 VW08EU (14.09.2026): PTS mutat OUT_5 -> OUT_2, ca in DB2 si pe DB3.
   DB2: VW03_FUNC_PTS_OUT(x) = SPEED_PROT_NORMAL( OUT_2, x ) (config_gpio.h:40).
   NEG_OUT5 fusese mostenit din seed-ul FRD37EU si nu a fost niciodata comparat
   cu referinta. Regula lui Adrian: IO-urile atinse de func_rs.c trebuie sa aiba
   aceeasi functie pe DB3 si DS4. */
#define CORE_DS4_GPIO_IFACE_PTS_1_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT2
#define CORE_DS4_GPIO_IFACE_PTS_2_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_CLUTCH_OUTPUT      GPIO_IFACE_OUTPUT_RELAY1
/* VW08EU/DS4 - ambreiaj. Aceleasi clase ca pe DB3, unde au fost validate la banc:
     CLUTCH_PWM      -> ISO2_TX  (P0.11), PWM 500 Hz, inversat la apelant
     CLUTCH_SENSOR_A -> RELAY2
     CLUTCH_SENSOR_B -> DE STABILIT: pe DB3 e NEG_OUT5, dar pe DS4 NEG_OUT5 e
                        ocupat de PTS_1. Lasat NONE pina se alege pinul. */
#define CORE_DS4_GPIO_IFACE_CLUTCH_PWM_OUTPUT       GPIO_IFACE_OUTPUT_ISO2_TX
/* v18 VW08EU (22.09.2026): senzorul A de ambreiaj mutat RELAY2 -> RELAY3.
   Regula lui Adrian pentru alocarea releelor:
     RELAY1 - aceeasi functie pe DB3 si DS4 (actuatie de frina / ambreiaj);
     RELAY2 - pe DB3 ramine senzorul de ambreiaj; pe DS4 devine GWR ON;
     RELAY3 - exista doar pe DS4 si preia ce face RELAY2 pe DB3.
   De ce se elibereaza RELAY2 tocmai pe DS4: pe DB3 releul de GWR il pune
   remote starter-ul extern, deci firmware-ul nu-l comanda; pe DS4 modulul e si
   starter, deci GWR-ul trebuie sa iasa din el.
   GWR-ul pe RELAY2 se face prin DEI (FEAT_DEI_SYS55 = 0x1C), nu prin contract.

   EXCEPTIE ASUMATA de la regula de paritate IO pentru func_rs.c: clutch_press()
   se cheama din func_rs_str_on(), deci senzorul A intra sub regula, si totusi
   difera acum intre platforme (DB3 RELAY2, DS4 RELAY3). E ceruta explicit,
   nu e un drift de portare - a nu se "repara" inapoi la un audit. */
#define CORE_DS4_GPIO_IFACE_CLUTCH_SENSOR_A_OUTPUT  GPIO_IFACE_OUTPUT_RELAY3
/* v10 VW08EU (16.09.2026): senzorul B primeste OUT_4 (NEG_OUT4, P1.21).
   Istoric: in DB2 semnalul de senzor merge simultan pe DOUA iesiri,
   VW03_CLUTCH_SENSOR(x) = OUT_RELAY2(x);OUT_5(x). Pe DS4 a stat pe OUT_5 pina
   cind OUT_5 a fost dat lui UNLOCK, apoi scurt pe NONE. OUT_4 e prima iesire
   libera si NEGATIVA, ca pe DB3 - Output 7 exista doar ca POS_OUT7 in BSP, deci
   ar fi inversat polaritatea pe o linie de senzor. DB3 ramine pe NEG_OUT5. */
#define CORE_DS4_GPIO_IFACE_CLUTCH_SENSOR_B_OUTPUT  GPIO_IFACE_OUTPUT_NEG_OUT4
#define CORE_DS4_GPIO_IFACE_RAP_OUTPUT         GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
/* v15 VW08EU (17.09.2026): RF Loop mutat inapoi pe RF_LOOP_POWER, adica EXACT ca
   pe DB3 (core_contract_db3_gpio_config.h:116). Era pe NEG_OUT13, mapare
   mostenita din ds4_contract-ul lui FRD37EU.
   De ce se schimba: NEG_OUT13 (P1.29) e 22-pin/10, Light Blue/Black, singura
   iesire negativa pe care se putea aseza GWR - si e fix pinul pe care FRD37EU
   isi scoate "Ground While Running Status". Nu poate avea doi stapini.
   De ce e sigur: pe 403 bucla RF are conectorul ei dedicat de 2 pini, nu un fir
   din I/O harness, iar pe 933 SDK-ul comanda deja bucla interna singur -
   rf_loop_init() inregistreaza rf_loop_pwr_set/clr pe INPUT_GWR_ON/OFF si scrie
   GPIO_PL_RFLOOP_POWER. FUNC_RF_LOOP_RSR() din func_rs.c ajunge acum pe acelasi
   pin, deci dubleaza ce face SDK-ul, exact ca pe DB3.
   DE VERIFICAT LA BANC: P1.1 e RFLOOP_POWER pe placa curenta; in 933_gpio.h
   linia de sub ea, comentata, e EN_FTCAN. Daca bypass-ul RF se poarta ciudat,
   aici se uita omul intii. */
#define CORE_DS4_GPIO_IFACE_RF_LOOP_OUTPUT     GPIO_IFACE_OUTPUT_RF_LOOP_POWER
#define CORE_DS4_GPIO_IFACE_RSR_LOCK_OUTPUT    GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_RSR_UNLOCK_OUTPUT  GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_RSR_TRUNK_OUTPUT   GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */

#define get_gpio_iface_handbrake_output()   ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_HANDBRAKE_OUTPUT)
#define get_gpio_iface_brake_output()       ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_BRAKE_OUTPUT)
#define get_gpio_iface_brake_sense_output() ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_BRAKE_SENSE_OUTPUT)
#define get_gpio_iface_ignition_output()    ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_IGNITION_OUTPUT)
#define get_gpio_iface_hood_output()        ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_HOOD_OUTPUT)
#define get_gpio_iface_key_wrap_output()    ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_KEY_WRAP_OUTPUT)
#define get_gpio_iface_lights_output()      ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_LIGHTS_OUTPUT)
#define get_gpio_iface_doors_output()       ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_DOORS_OUTPUT)
#define get_gpio_iface_trunk_output()       ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_TRUNK_OUTPUT)
#define get_gpio_iface_pts_1_output()       ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_PTS_1_OUTPUT)
#define get_gpio_iface_pts_2_output()       ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_PTS_2_OUTPUT)
#define get_gpio_iface_clutch_output()      ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_CLUTCH_OUTPUT)
#define get_gpio_iface_clutch_pwm_output()      ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_CLUTCH_PWM_OUTPUT)
#define get_gpio_iface_clutch_sensor_a_output() ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_CLUTCH_SENSOR_A_OUTPUT)
#define get_gpio_iface_clutch_sensor_b_output() ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_CLUTCH_SENSOR_B_OUTPUT)
#define get_gpio_iface_rap_output()         ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_RAP_OUTPUT)
#define get_gpio_iface_rf_loop_output()     ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_RF_LOOP_OUTPUT)
#define get_gpio_iface_rsr_lock_output()    ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_RSR_LOCK_OUTPUT)
#define get_gpio_iface_rsr_unlock_output()  ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_RSR_UNLOCK_OUTPUT)
#define get_gpio_iface_rsr_trunk_output()   ((gpio_iface_output_selector_t)CORE_DS4_GPIO_IFACE_RSR_TRUNK_OUTPUT)

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
#define GPIO_IFACE_CFG_RAP_OUTPUT        (get_gpio_iface_rap_output())
#define GPIO_IFACE_CFG_RF_LOOP_OUTPUT    (get_gpio_iface_rf_loop_output())
#define GPIO_IFACE_CFG_RSR_LOCK_OUTPUT   (get_gpio_iface_rsr_lock_output())
#define GPIO_IFACE_CFG_RSR_UNLOCK_OUTPUT (get_gpio_iface_rsr_unlock_output())
#define GPIO_IFACE_CFG_RSR_TRUNK_OUTPUT  (get_gpio_iface_rsr_trunk_output())

#define CORE_DS4_GPIO_CONFIG_VERSION      5
#define CORE_DS4_GPIO_CONFIG_FREEZE_DATE  20260707UL

/*
 * GPIO_IFACE contract freeze guardrail.
 * Update these expected values only when intentionally revving contract version.
 */
#define CORE_DS4_GPIO_IFACE_EXPECT_HANDBRAKE_OUTPUT   GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_BRAKE_OUTPUT       GPIO_IFACE_OUTPUT_RELAY1
#define CORE_DS4_GPIO_IFACE_EXPECT_BRAKE_SENSE_OUTPUT GPIO_IFACE_OUTPUT_DAC_POWER   /* v5 VW08EU (11.09.2026): DB2 BRAKE_SENSE_SET -> _401_OUT_DAC_RELAY */
#define CORE_DS4_GPIO_IFACE_EXPECT_IGNITION_OUTPUT    GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_HOOD_OUTPUT        GPIO_IFACE_OUTPUT_NONE
#define CORE_DS4_GPIO_IFACE_EXPECT_KEY_WRAP_OUTPUT    GPIO_IFACE_OUTPUT_NONE
#define CORE_DS4_GPIO_IFACE_EXPECT_LIGHTS_OUTPUT      GPIO_IFACE_OUTPUT_NEG_OUT1   /* VW08EU: FUNC_HAZARD_OUT (func_hazard_ctrl.c) -> OUT_1_NEGATIVE, ca in DB3 */
#define CORE_DS4_GPIO_IFACE_EXPECT_DOORS_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_TRUNK_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_PTS_1_OUTPUT       GPIO_IFACE_OUTPUT_NEG_OUT2   /* v6: ca in DB2/DB3 */
#define CORE_DS4_GPIO_IFACE_EXPECT_PTS_2_OUTPUT       GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_OUTPUT      GPIO_IFACE_OUTPUT_RELAY1
#define CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_PWM_OUTPUT       GPIO_IFACE_OUTPUT_ISO2_TX
#define CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_A_OUTPUT  GPIO_IFACE_OUTPUT_RELAY3   /* v18: RELAY3, vezi nota de mai sus */
#define CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_B_OUTPUT  GPIO_IFACE_OUTPUT_NEG_OUT4   /* v10: OUT_4, negativ ca pe DB3 */
#define CORE_DS4_GPIO_IFACE_EXPECT_RAP_OUTPUT         GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_RF_LOOP_OUTPUT     GPIO_IFACE_OUTPUT_RF_LOOP_POWER   /* v15: ca pe DB3 */
#define CORE_DS4_GPIO_IFACE_EXPECT_RSR_LOCK_OUTPUT    GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_RSR_UNLOCK_OUTPUT  GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */
#define CORE_DS4_GPIO_IFACE_EXPECT_RSR_TRUNK_OUTPUT   GPIO_IFACE_OUTPUT_NONE   /* VW08EU/DS4: raportarea de stare nu comanda pini; configurabil pe web */

#define CORE_DS4_GPIO_MAP_ASSERT(name, expr) typedef char core_ds4_gpio_map_assert_##name[(expr) ? 1 : -1]

CORE_DS4_GPIO_MAP_ASSERT(handbrake_fixed, CORE_DS4_GPIO_IFACE_HANDBRAKE_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_HANDBRAKE_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(brake_fixed, CORE_DS4_GPIO_IFACE_BRAKE_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_BRAKE_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(brake_sense_fixed, CORE_DS4_GPIO_IFACE_BRAKE_SENSE_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_BRAKE_SENSE_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(ignition_fixed, CORE_DS4_GPIO_IFACE_IGNITION_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_IGNITION_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(hood_fixed, CORE_DS4_GPIO_IFACE_HOOD_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_HOOD_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(key_wrap_fixed, CORE_DS4_GPIO_IFACE_KEY_WRAP_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_KEY_WRAP_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(lights_fixed, CORE_DS4_GPIO_IFACE_LIGHTS_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_LIGHTS_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(doors_fixed, CORE_DS4_GPIO_IFACE_DOORS_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_DOORS_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(trunk_fixed, CORE_DS4_GPIO_IFACE_TRUNK_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_TRUNK_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(pts1_fixed, CORE_DS4_GPIO_IFACE_PTS_1_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_PTS_1_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(pts2_fixed, CORE_DS4_GPIO_IFACE_PTS_2_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_PTS_2_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(clutch_fixed, CORE_DS4_GPIO_IFACE_CLUTCH_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(clutch_pwm_fixed, CORE_DS4_GPIO_IFACE_CLUTCH_PWM_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_PWM_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(clutch_sensor_a_fixed, CORE_DS4_GPIO_IFACE_CLUTCH_SENSOR_A_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_A_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(clutch_sensor_b_fixed, CORE_DS4_GPIO_IFACE_CLUTCH_SENSOR_B_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_CLUTCH_SENSOR_B_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(rap_fixed, CORE_DS4_GPIO_IFACE_RAP_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_RAP_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(rf_loop_fixed, CORE_DS4_GPIO_IFACE_RF_LOOP_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_RF_LOOP_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(rsr_lock_fixed, CORE_DS4_GPIO_IFACE_RSR_LOCK_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_RSR_LOCK_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(rsr_unlock_fixed, CORE_DS4_GPIO_IFACE_RSR_UNLOCK_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_RSR_UNLOCK_OUTPUT);
CORE_DS4_GPIO_MAP_ASSERT(rsr_trunk_fixed, CORE_DS4_GPIO_IFACE_RSR_TRUNK_OUTPUT == CORE_DS4_GPIO_IFACE_EXPECT_RSR_TRUNK_OUTPUT);

#undef CORE_DS4_GPIO_MAP_ASSERT

#endif /* __CORE_CONTRACT_DS4_GPIO_CONFIG_H__ */
