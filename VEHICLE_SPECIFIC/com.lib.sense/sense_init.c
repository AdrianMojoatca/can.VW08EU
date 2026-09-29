/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: sense_init.c 4280 2012-11-16 10:43:59Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "sense_private.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

void sense_init( Can_Dev_Id can_id )
{
    sense_rs_can_init(NULL);

	set_ign_source(SOURCE_DATA);  // THIS IS FUCKIN' 1
	set_door_source(SOURCE_DATA); // THIS IS FUCKIN' 2
	set_trunk_source(SOURCE_DATA); // THIS IS FUCKIN' 3
	set_rke_lock_use(RKE_ENABLE);  // THIS IS FUCKIN' 4
	set_rke_trunk_use(RKE_ENABLE); // THIS IS FUCKIN' 5
	set_handbrake_source(SOURCE_DATA); // THIS IS FUCKIN' 6
	set_hood_source(SOURCE_DATA); // hood: route hood sense to CAN data (was missing -> get_hood_status stuck FALSE)
	set_brake_source(SOURCE_DATA); // THIS IS FUCKIN' 7
	set_rke_unlock_use(RKE_ENABLE); // THIS IS FUCKIN' 8

	set_speed_source(SOURCE_DATA); // THIS IS FUCKIN' 9
	set_rpm_source(SOURCE_DATA); // THIS IS FUCKIN' 10
    
    all_doors_status = FALSE;
    trunk_status     = FALSE;

    can_register:
    {
        static const Can_Usr_Array1 can_table[] = 
        {
            { (Reg_Func)sense_doors_3D0             , 0x3D0 },
            { (Reg_Func)sense_doors_3D1             , 0x3D1 },
            { (Reg_Func)sense_doors_583             , 0x583 },
      
            
            { (Reg_Func)sense_trunk_583             , 0x583 },
            { (Reg_Func)sense_hood_65A              , 0x65A },
            
            { (Reg_Func)sense_ignition_3C0          , 0x3C0 },        
            
            { (Reg_Func)sense_oem_3D0               , 0x3D0 },  
            { (Reg_Func)sense_oem_583               , 0x583 },
            { (Reg_Func)sense_oem_593               , 0x593 },

            { (Reg_Func)sense_left_front_window_3D0 , 0x3D0 },     
            { (Reg_Func)sense_right_front_window_3D1, 0x3D1 },   
            { (Reg_Func)sense_left_rear_window_3CE  , 0x3CE },    
            { (Reg_Func)sense_right_rear_window_3CF , 0x3CF },
            
        	//{ (Reg_Func)sense_handbrake_3DA         , 0x3DA },   
            { (Reg_Func)sense_handbrake_30B         , 0x30B }, // !!!   
			{ (Reg_Func)sense_handbrake_30D         , 0x30D },  // ADI

            { (Reg_Func)sense_brake_3BE             , 0x3BE },   

            { (Reg_Func)sense_rpm_3DC               , 0x3DC },                          
            { (Reg_Func)sense_hazard_366            , 0x366 },                          
            { (Reg_Func)sense_locks_3D0             , 0x3D0 },                          
            { (Reg_Func)sense_park_3DC              , 0x3DC },                          
            { (Reg_Func)sense_speed_0FD             , 0x0FD },
            { (Reg_Func)sense_oem_alarm_583         , 0x583 },

			{ (Reg_Func)sense_oem_184               , 0x184 },  // 14_09_2017
        };

        /* PORTARE VW08EU: era CAN_USR_REG1(CAN_DEV_1, ...) hardcodat (ADI, 1_03,
           02_04_2018), ocolind parametrul. Valoarea coincidea pe DB2/DB3, dar pe
           DS4 canalul vehiculului e CAN_DEV_0, deci toate cele 24 de inregistrari
           ajungeau pe un canal pe care nu vine nimic - sense complet mut, fara
           niciun semn. Parametrul vine din cfg.can_dev_id = CAN_DEV_FUNC_SENSE,
           adica exact canalul vehiculului. */
        CAN_USR_REG1(can_id , can_table);
    }
}

//--------------------------------------------------------------------------//
