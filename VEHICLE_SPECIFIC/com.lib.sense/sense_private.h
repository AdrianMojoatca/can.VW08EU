/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: sense_private.h 4134 2012-09-17 08:48:12Z florin.olariu $
/*==========================================================================*/
#ifndef __SENSE_PRIVATE_H__
#define __SENSE_PRIVATE_H__
/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "sense.h"
#include "trace.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

//--------------------------------------------------------------------------//

#define DATA 			can_msg->data._8_
#define CAST_BOOL(x)   	!!(x)

//--------------------------------------------------------------------------//

#define  SENSE_MECHANICAL_KEY_TIMEOUT      100

//--------------------------------------------------------------------------//

#define SENSE_WINDOW_VENT                  0x20

//--------------------------------------------------------------------------//

#if DBG_SENSE == 1
#define     TRACE_SENSE      TRACE
#else
#define     TRACE_SENSE(...)   ;
#endif

//--------------------------------------------------------------------------//

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

void sense_doors_3D0               ( Can_Msg *can_msg );
void sense_doors_3D1               ( Can_Msg *can_msg );
void sense_doors_583               ( Can_Msg *can_msg );
void sense_locks_3D0               ( Can_Msg *can_msg );

void sense_trunk_583               ( Can_Msg *can_msg );
void sense_hood_65A                ( Can_Msg *can_msg );

void sense_ignition_3C0            ( Can_Msg *can_msg );

void sense_oem_3D0                 ( Can_Msg *can_msg );
void sense_oem_583                 ( Can_Msg *can_msg );
void sense_oem_593                 ( Can_Msg *can_msg );

void sense_left_front_window_3D0   ( Can_Msg *can_msg );
void sense_right_front_window_3D1  ( Can_Msg *can_msg );
void sense_left_rear_window_3CE    ( Can_Msg *can_msg );
void sense_right_rear_window_3CF   ( Can_Msg *can_msg );

void sense_brake_3BE               ( Can_Msg *can_msg );

void sense_handbrake_3DA           ( Can_Msg *can_msg );
void sense_handbrake_30B           ( Can_Msg *can_msg );

void sense_rpm_3DC                 ( Can_Msg *can_msg );

void sense_hazard_366              ( Can_Msg *can_msg );
void sense_speed_0FD               ( Can_Msg *can_msg );
void sense_park_3DC                ( Can_Msg *can_msg );

void sense_oem_alarm_583           ( Can_Msg *can_msg );

void update_sense_out_door_trunk   ( void );

void sense_handbrake_30D           ( Can_Msg *can_msg );  // ADI

void sense_oem_184                 ( Can_Msg *can_msg ); // 14_09_2017



/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
extern Boolean all_doors_status;
extern Boolean trunk_status;
/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/


/* ---- portare VW08EU: punti catre facilitatile template-ului ---- */
/* DB2 avea un canal de trace separat (trace_can2). SDK-ul nostru are un singur
   TRACE, rutat prin tabela de API sub FW_DB3_BUILD. Pastram apelurile intacte. */
#ifndef TRACE_CAN2
#define TRACE_CAN2      TRACE_SENSE
#endif

/* In DB2 aceasta iesire era deja neutralizata in config_gpio.h (varianta reala,
   _401_OUT_3, era comentata). O pastram no-op ca sa nu schimbam comportamentul. */
#ifndef OUT_SENSE_DOOR_TRUNK
#define OUT_SENSE_DOOR_TRUNK(x)   ;
#endif

/* HBRAKE_SENSE_SET/CLR: in DB2 veneau din config_gpio.h, care pe thin-FW nu e
   accesibil (configurarea e detinuta de CORE si seedata prin contract). Rutam
   prin contract, ca restul indicatorilor de sense. */
#include "gpio_iface.h"   /* declaratiile shim-ului de contract; erau scrise de mina aici */
#ifndef HBRAKE_SENSE_SET
#define HBRAKE_SENSE_SET    gpio_iface_handbrake_on()
#endif
#ifndef HBRAKE_SENSE_CLR
#define HBRAKE_SENSE_CLR    gpio_iface_handbrake_off()
#endif

#endif

