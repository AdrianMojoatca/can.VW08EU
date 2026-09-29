/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_private.h 1976 2010-10-30 17:27:24Z florin.olariu $
/*==========================================================================*/
#ifndef __CLUTCH_PRIVATE_H__
#define __CLUTCH_PRIVATE_H__
/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "clutch.h"
#include "trace.h"   /* DIAGNOSTIC 17.09.2026: trace pe traseul de apasare a ambreiajului. De scos. */
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   TEMPORAR - BANC 31.08.2026 - DE PUS CLUTCH_BENCH_SLOW PE 0 DUPA TEST

   La viteza normala PWM-ul de ambreiaj e la 500 Hz (perioada 2 ms), deci pe un
   LED nu se vede clipire, ci doar luminozitate partiala - greu de judecat cu
   ochiul. Cu comutatorul pe 1, scalam TOATE cele trei constante cu acelasi
   factor, deci PERIOADA creste dar DUTY-ul ramine identic:

       factor  100 -> perioada  200 ms = 5   Hz
       factor 1000 -> perioada 2000 ms = 0,5 Hz  (setat acum)
       duty neschimbat: 25% la RELEASE, 74% la FULL_PRESS

   clutch_value e in tacte ABSOLUTE, nu fractiune din perioada - de aceea toate
   trei trebuie scalate impreuna. Daca s-ar scala doar perioada, duty-ul ar
   deveni 0,25% si 0,74% si n-ai vedea nimic.

   Scalam si pauzele din clutch_press(): cele 200 ms originale ar fi mai scurte
   decit o perioada. Formula (10 x FACTOR) da mereu 5 perioade pe faza.

   NU folosi build-ul asta cu un actuator real conectat - PWM incetinit pe un
   ambreiaj adevarat nu are sens. E strict pentru LED-uri pe placa de test.
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#define CLUTCH_BENCH_SLOW             0

#if CLUTCH_BENCH_SLOW == 1
    #warning CLUTCH PWM INCETINIT PENTRU BANC - pune CLUTCH_BENCH_SLOW pe 0
    #define CLUTCH_SLOW_FACTOR        1000U
    #define CLUTCH_PRESS_STEP_MS      (10U * CLUTCH_SLOW_FACTOR)
#else
    #define CLUTCH_SLOW_FACTOR        1U
    #define CLUTCH_PRESS_STEP_MS      200
#endif

/* Valorile de baza, din DB2: perioada 2 ms, release 500 us, full press 1487 us
   (vers 1_07 - 7V). Parantezate, ca scalarea sa nu strice expresii de tipul
   CLUTCH_PWM_PERIOD - clutch_value. */
#define CLUTCH_PWM_PERIOD             (72*2000 * CLUTCH_SLOW_FACTOR)
#define CLUTCH_PWM_RELEASE            (72*500  * CLUTCH_SLOW_FACTOR)
#define CLUTCH_PWM_FULL_PRESS         (72*1487 * CLUTCH_SLOW_FACTOR)



/*--------------------------------------------------------------------------*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
void  clutch_fiq    ( void );
void  clutch_update ( UInt32 new_value );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
extern Tmr         clutch_tmr   ; 
extern Boolean     clutch_on    ;
extern UInt32      clutch_value ;
extern Boolean     clutch_active;
/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/


/* ==== portare VW08EU: iesirile de ambreiaj, rutate prin contract ==========
   In DB2 veneau din config_gpio.h (ramura DBALL):
     VW03_CLUTCH_PWM_OUT(x)  OUT_PHAL2_TX(!x)         -> ISO2_TX, INVERSAT
     VW03_CLUTCH_PWM_CUT(x)  OUT_RELAY1(x)            -> acelasi releu cu frana
     VW03_CLUTCH_SENSOR(x)   OUT_RELAY2(x);OUT_5(x)   -> doi pini

   Pe thin-FW GPIO-ul e al CORE-ului, seedat prin contract, deci trecem prin clase.
   Inversarea PWM-ului sta AICI, la apelant, exact ca in DB2 - nu in contract. */
#include "gpio_iface.h"

#ifndef CLUTCH_PWM_OUT
#define CLUTCH_PWM_OUT(x)     gpio_iface_clutch_pwm((x) ? FALSE : TRUE)
#endif

/* Aceeasi clasa cu FUNC_BRAKE_OUT: in DB2 amindoua sint OUT_RELAY1. */
#ifndef CLUTCH_PWM_CUT
#define CLUTCH_PWM_CUT(x)     gpio_iface_clutch(x)
#endif

#ifndef CLUTCH_SENSOR
#define CLUTCH_SENSOR(x)      { gpio_iface_clutch_sensor_a(x); gpio_iface_clutch_sensor_b(x); }
#endif
#endif
