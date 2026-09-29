/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: detect.h 4241 2012-10-25 18:16:25Z florin.olariu $
/*==========================================================================*/

#ifndef __DETECT_H__
#define __DETECT_H__

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "core_framework.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

//--------------------------------------------------------------------------//

typedef union u_Car
{
    struct
    {
        unsigned car             :8 ;        
        unsigned year            :8 ;        
        unsigned can0_detected   :1 ;
        unsigned can1_detected   :1 ;
    }bits ; 

    UInt32 bytes ;

}Car ; 

//--------------------------------------------------------------------------//

typedef enum
{
    NO_CAR   = 0  ,         
    GOLF7                          
}Car_Type ; 

//--------------------------------------------------------------------------//

/* Magistrala vehiculului: CAN_DEV_1 pe AMBELE platforme.

   Golf 7, HS 500K, pe LPC CAN1, pini P0.0/P0.1, transceiver HS cu standby pe
   P0.30. Firele: Tan / Tan-Black. La fel ca in DB2 si pe DB3.

   Intre 02 si 08.09.2026 aici a stat un comutator, VEHICLE_ON_CAN_DEV_0, care pe
   DS4 muta canalul pe CAN_DEV_0. Motivul: pe placa de test DS4 nu se receptiona
   nimic pe CAN_DEV_1, iar cadrele intrau curat pe CAN_DEV_0. Trei dovezi
   independente pareau sa arate ca asa e cablat modulul 933 - dar era doar
   conectarea de pe placa de test, care nu are marcajele firelor. Adrian a
   reconectat fizic pe Tan / Tan-Black si comutatorul a fost scos: nu se lasa in
   cod un comutator mort, cineva il intoarce fara sa stie de ce exista.

   Ce a RAMAS din episod, si merita pastrat: indirectarile de mai jos. Inainte,
   mai multe locuri scriau CAN_DEV_1 direct, ocolind aliasul primit ca parametru
   (sense_init, detect_ign_search, detect_init_nvfs, trei fisiere func). Pe DB2 si
   DB3 valorile coincideau, deci nu se vedea nimic; orice mutare de canal le lasa
   insa in urma, in tacere. Acum toate trec prin CAN_DEV_VEHICLE. */
#define CAN_DEV_VEHICLE       CAN_DEV_1
#define VEHICLE_CAN_DETECTED  (car.bits.can1_detected)

#define CAN_DEV_OBD         CAN_DEV_0
#define CAN_DEV_FUNC_SENSE  CAN_DEV_VEHICLE
#define CAN_DEV_DET_PGM     CAN_DEV_VEHICLE
#define CAN_DEV_DET_IGN     CAN_DEV_VEHICLE

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
void     detect_car_search( void );
void     detect_reset_init( void );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
extern Car      car ;
/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/

/*==========================================================================*/

#endif


