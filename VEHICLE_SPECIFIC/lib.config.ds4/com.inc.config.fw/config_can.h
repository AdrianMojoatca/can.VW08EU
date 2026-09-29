/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: config_firmware.h 82 2008-03-21 19:35:35Z martin.bouchard $
/*==========================================================================*/

//             <<< Use Configuration Wizard in Context Menu >>>             //

#ifndef __CONFIG_CAN_H__
#define __CONFIG_CAN_H__

#define CORE_CONFIG_CAN_VERSION_1_01

/*==========================================================================*/
/*                                N O T E S                                 */
/*==========================================================================*/
/* This file is included by 'device.h' and represents global compile-time   */
/* configuration parameters.  These parameters can be edited through the    */
/* configuration tool in the RealView IDE.                                  */
/*==========================================================================*/

//------------------------------------------------------------------------//
// <h> CAN0 CONFIGURATION(FT or SW)
// <q0> 1. FT/SW SELECTION
// <i>  FT = checked
// <i>  SW = unchecked
// <o1> 2. BAUD RATE  <0=>33.3K  <1=>50K  <2=>83.3  <3=>100K  <4=>125K	 <5=>250K  <6=>500K
// <o2> 3. RX BUFFER SIZE      <1-255>
// <o3> 4. TX BUFFER SIZE      <1-255>
// </h>
#define CAN_DEV_0_FT_SW                1   // 1 = FT     , 0 = SW
#define CAN_DEV_0_SPEED                6   // 
#define CAN_DEV_0_RX_BUFFER_SIZE       5
#define CAN_DEV_0_TX_BUFFER_SIZE       5

// <h> CAN1 CONFIGURATION(HS)
// <o0> 1.BAUD RATE  <0=>125K  <1=>250K  <2=>500K
// <o1> 2.RX BUFFER SIZE      <1-255>
// <o2> 3.TX BUFFER SIZE      <1-255>
// </h>
/* ATENTIE - ACEST FISIER ESTE SURSA REALA A TIMINGULUI CAN PE DS4, NU CONTRACTUL.
   Pe DS4, can_usr_private.h face #include "config_can.h" (adica exact fisierul
   asta). Pe DB3 acelasi header face #include "core_contract_db3_can_config.h".
   Ladder-ul care alege BTR/TSEG/SJW e un #if pe CAN_DEV_1_SPEED, rezolvat la
   COMPILARE in can_usr_private.h - iar acel TU se compileaza in CORE si nu vede
   niciodata wrapper-ul de contract. Prin urmare CORE_CAN_DEV_1_SPEED_DEFAULT din
   core_contract_ds4_can_config.h NU are efect pe DS4; a fost pus pe 2 la
   02.09.2026 si viteza a ramas 125K.
   Acelasi tipar de umbrire ca la config_gpio.h - vezi comentariul de acolo.
   09.09.2026: 0 (125K) -> 2 (500K). Valoarea 0 era mostenita din seed-ul
   FRD37EU, care nu foloseste CAN1 si deci n-a avut niciodata motiv s-o
   corecteze. Golf 7 are magistrala pe CAN1 (tan / tan-black) la 500K, ca in DB2
   si ca pe DB3. Simptomul: CAN_DEV_1 nu receptiona nimic si nici nu intra in
   bus-off la transmisie - la bit-time de 4x nu gasea magistrala libera. */
#define CAN_DEV_1_SPEED                2   // 500K
#define CAN_DEV_1_RX_BUFFER_SIZE       5
#define CAN_DEV_1_TX_BUFFER_SIZE       5
//------------------------------------------------------------------------//

#endif
