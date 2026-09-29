/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: reg_atomic_init.c 14828 2012-05-28 17:30:47Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "scan_manager_private.h"
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
//! \desc  place description here
//!        

//! \assume place assumptions here

//! \notes  place notes here

//! \param  p1        describe the p1 paramter here
//! \param  p2        describe the p2 paramter here

//! \return r         describe the returned value here

//--------------------------------------------------------------------------//

void scan_manager_queue_init (Computer_List* list)
{
  list->rhs = list->lhs = (Car_Computer*)list ;
}

/*--------------------------------------------------------------------------*/

Car_Computer* scan_manager_queue_remove_car_computer (Computer_List* list)
{
    Car_Computer *computer = list->lhs ; 
	
	if( computer ==  (Car_Computer *)list ) return NULL ; 

	list->lhs          = computer->lhs ; 	
	computer->lhs->rhs = (Car_Computer *)list ; 
	
	return computer ; 
}

/*--------------------------------------------------------------------------*/

void scan_manager_queue_insert_car_computer (Computer_List* list , Car_Computer* computer)
{
    list->rhs->lhs = computer ; 	
	computer->rhs  = list->rhs ; 	
	computer->lhs  = (Car_Computer*)list ; 
	list->rhs      = computer ;   
}

/*--------------------------------------------------------------------------*/

void  scan_manager_queue_unlink_car_computer(Car_Computer *computer )
{
    computer->lhs->rhs = computer->rhs ;
  	computer->rhs->lhs = computer->lhs ;
}
    
/*--------------------------------------------------------------------------*/

