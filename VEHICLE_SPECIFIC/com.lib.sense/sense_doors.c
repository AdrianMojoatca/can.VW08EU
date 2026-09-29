/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: sense_doors.c 4224 2012-10-18 09:45:06Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "sense_private.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/
#define OTHER_DOORS_STATUS   rear_driver || front_pass || rear_pass  
#define ALL_DOORS_STATUS     rear_driver || front_pass || rear_pass || front_driver
/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean front_driver = FALSE ;
static Boolean front_pass   = FALSE ;
static Boolean rear_driver  = FALSE ;
static Boolean rear_pass    = FALSE ;

extern Boolean ignore_doors_sense; // this is defined in func_rs for RAP (simulates a door open on wire)
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

void sense_doors_3D0( Can_Msg *can_msg )
{
	front_driver = CAST_BOOL( DATA[0]&0x01);

	sense_rs_driver_door_can( front_driver );

	all_doors_status = ALL_DOORS_STATUS ;
	
	update_sense_out_door_trunk();
}

//--------------------------------------------------------------------------//

void sense_doors_3D1( Can_Msg *can_msg )
{
    front_pass = CAST_BOOL(DATA[0]&0x01);            

    sense_rs_other_doors_can( OTHER_DOORS_STATUS );

	all_doors_status = ALL_DOORS_STATUS ;
	
	update_sense_out_door_trunk();}

//--------------------------------------------------------------------------//

void sense_doors_583( Can_Msg *can_msg )
{
    rear_driver = CAST_BOOL(DATA[3]&0x04);            
    rear_pass   = CAST_BOOL(DATA[3]&0x08);        

    sense_rs_other_doors_can( OTHER_DOORS_STATUS );
	all_doors_status   = ALL_DOORS_STATUS ;
	
	update_sense_out_door_trunk();
}

//--------------------------------------------------------------------------//
void update_sense_out_door_trunk( void )
{
	if( trunk_status || all_doors_status )
	{
		OUT_SENSE_DOOR_TRUNK( TRUE );
		TRACE_CAN2( "\r\nTRSO" );
	}
	else
	{
		OUT_SENSE_DOOR_TRUNK( FALSE );
		TRACE_CAN2( "\r\nTRSC" );
	}
}

//--------------------------------------------------------------------------//
