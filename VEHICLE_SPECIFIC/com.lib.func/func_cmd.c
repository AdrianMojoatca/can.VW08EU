/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: niss_bp_init.c 1282 2010-07-13 19:33:32Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "func_private.h"
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
static void execute_gas_cap( void );
static void execute_aux    ( Func_Aux_Type aux_type );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean skip_lock = FALSE;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_evt_callback(DL_EVT evt)
{
	return; // 29_03_2018 - vers. 1_01
    switch(evt)
    {
		case DL_EVT_ARM        :TRACE_CAN2( "\r\nDL_A" ); process_launch(  func_prc[FUNC_ARM          ] ,0);break;        
		//case DL_EVT_DISARM     :TRACE_CAN2( "\r\nDL_D" ); process_launch(  func_prc[FUNC_DISARM       ] ,0);break; // a fost folosita         pt. unsafe lock
		case DL_EVT_LOCK       :TRACE_CAN2( "\r\nDL_L" ); process_launch(  func_prc[FUNC_ARM          ] ,0);break;        
//        case DL_EVT_TRUNK      :process_launch(  func_prc[FUNC_TRUNK_RELEASE] ,0);break;
		case DL_EVT_TRUNK      :TRACE_CAN2( "\r\nDL_T" ); func_cmd_trunk(); break;
        
		case DL_EVT_UNLOCK_DD  :
		{
			TRACE_CAN2( "\r\nDL_UD" );
			
			if( get_ign_status() ) 
			{
				ATOMIC(func_do_unlock_all=TRUE;);
			} 
			else
			{
				ATOMIC(func_do_unlock_all=FALSE;);
			}
			
			process_launch(func_prc[FUNC_DISARM] ,0);
		}break;        
		case DL_EVT_UNLOCK_ALL :TRACE_CAN2( "\r\nDL_UA" ); ATOMIC(func_do_unlock_all=TRUE );process_launch(func_prc[FUNC_DISARM] ,0);break;        

		default:break;
    }
    
    switch(evt)
    {
//        case DL_EVT_ARM  :
//        case DL_EVT_LOCK :
//         {
//             switch(func_cfg->confort)
//             {
//                 case CONFORT_WITH_LOCK:
//                 {
//                     if(!get_rs_gwr())
//                     {
//                         process_launch( func_prc[ FUNC_WRU ] , 0 ) ;
//                     }
//                 }
//                 break;
//                 
//                 
//                 case CONFORT_WITH_LOCK_AND_RS:
//                 {
//                     process_launch( func_prc[ FUNC_WRU ] , 0 ) ;
//                 }
//                 break;
//                 
//                 default:break;
//             }
//         
//             if( !get_ign_status() )
//             {
//                 process_launch( func_prc[ FUNC_LIGHT_DISABLE ] , 0 );        
//             }            
//         }
//         break;
//         
//         case DL_EVT_DISARM:
//         {
//             if( !get_ign_status() )
//             {
//                 process_launch( func_prc[ FUNC_LIGHT_DISABLE ] , 0 );  
//             }    
//         }
//         break;
//         
        default:break;
    }        
} 
      
void func_out_callback(DL_OUT out)
{    
    switch(out)
    {
//        case DL_OUT_LOCK0   :FUNC_OUT_LOCK(FALSE)  ;func_dac_lock  (FALSE);break;
//        case DL_OUT_LOCK1   :FUNC_OUT_LOCK(TRUE)   ;func_dac_lock  (TRUE );break;
//        case DL_OUT_UNLOCK0 :FUNC_OUT_UNLOCK(FALSE);func_dac_unlock(FALSE);break;
//        case DL_OUT_UNLOCK1 :FUNC_OUT_UNLOCK(TRUE) ;func_dac_unlock(TRUE );break;
        
        case DL_OUT_TRUNK0  :FUNC_TRUNK_OUT(FALSE) ; TRACE_CAN2( "\r\nTR_0" ); break;
        case DL_OUT_TRUNK1  :FUNC_TRUNK_OUT(TRUE ) ; TRACE_CAN2( "\r\nTR_1" ); break;
        
        default:break;
    }
}

/*--------------------------------------------------------------------------*/

// void func_cmd_lock( void )
// {    
//     if( skip_lock ) return ;
//     
//     process_launch( func_prc[ FUNC_ARM ] , 0 );
//     
//     if( func_cfg->confort && ( !get_ign_status() || get_rs_gwr() ) )
//     {
//         process_launch( func_prc[ FUNC_WRU ] , 0 ) ;
//     }    
//     
//     if( !get_ign_status() )
//     {
//         process_launch( func_prc[ FUNC_LIGHT_DISABLE ] , 0 );        
//     }

// 	TRACE_FUNC("[%s]\n\r",__FUNCTION__);
// }
                   
/*--------------------------------------------------------------------------*/

// void func_cmd_unlock( void )
// {            
// 	timeout_q_remove( (void*)func_gwr_off_relock );

//     ATOMIC(skip_lock = FALSE);
//     
//     if( !get_ign_status() )
//     {
//         process_launch( func_prc[ FUNC_DISARM ] , 0 );    
//         process_launch( func_prc[ FUNC_LIGHT_DISABLE ] , 0 );  
//     }
//     else
//     {        
//         process_launch( func_prc[ FUNC_DISARM ] , 0 );      
//     }
//     
// 	TRACE_FUNC("[%s]\n\r",__FUNCTION__);                    
// }
                   
/*--------------------------------------------------------------------------*/

void func_cmd_trunk( void )
{    
    timeout_q_remove( (void*)func_gwr_off_relock );
	
    ATOMIC(skip_lock = FALSE);
    
	if( !get_ign_status() )        
    {
        // following code was removed in version 1_14 - 19_03_2018
		//process_launch( func_prc[ FUNC_DISARM ] , 0 );    
    }
            
    process_launch( func_prc[ FUNC_TRUNK_RELEASE ] , 0 ); 
	
	TRACE_FUNC("[%s]\n\r",__FUNCTION__);       
}
                   
/*--------------------------------------------------------------------------*/

void func_cmd_gwr_on ( void )
{
//     if(get_ign_status()==FALSE)
//     {
//         //process_launch( func_prc[ FUNC_RAP ] , 0 );
//         
//         ATOMIC(func_smart_key_power=TRUE);
//         
//         //FUNC_SMART_KEY_POWER(TRUE);
//         
//         TRACE_FUNC("\n\r[SMART_KEY_POWER_ON]\n\r");
//     }   
}

void func_cmd_ign_off( void )
{
//     if(func_smart_key_power && !get_rs_gwr())
//     {
//         ATOMIC(func_smart_key_power=FALSE);
//         
//         //FUNC_SMART_KEY_POWER(FALSE);
//         
//         TRACE_FUNC("\n\r[SMART_KEY_POWER_OFF]\n\r");
//     }
}

/*--------------------------------------------------------------------------*/

void func_cmd_gwr_off( void )
{            
//     if(func_smart_key_power && !get_ign_status())
//     {
//         ATOMIC(func_smart_key_power=FALSE);
//         
//         //FUNC_SMART_KEY_POWER(FALSE);
//         
//         TRACE_FUNC("\n\r[SMART_KEY_POWER_OFF]\n\r");
//     }
    
    timeout_q_remove( (void*)func_gwr_off_relock );
    
    ATOMIC(skip_lock = TRUE);

	timeout_q_insert( (Timeout_Q_Func)func_gwr_off_relock , NULL , FUNC_GWR_OFF_RELOCK_TIMEOUT , 1 , 1 ); 

	TRACE_FUNC("[%s]\n\r",__FUNCTION__);
}
                   
/*--------------------------------------------------------------------------*/

Timeout_Q_Status func_gwr_off_relock( void )
{
	ATOMIC(skip_lock = FALSE);
    
    if( get_ss_lock() && !get_ign_status() )
	{
		process_launch( func_prc[ FUNC_ARM ] , 0 );        
	}

	TRACE_FUNC("[%s]\n\r",__FUNCTION__);
	
	return TIMEOUT_Q_COMPLETE ; 
}		

/*--------------------------------------------------------------------------*/

void func_cmd_oem_lock( void )
{        
    timeout_q_remove( (void*)func_gwr_off_relock );
	
    ATOMIC(skip_lock = FALSE);
    
	process_launch( func_prc[ FUNC_OEM_LOCK ] , 0 );              
    
    if( func_cfg->confort && ( !get_ign_status() || get_rs_gwr() ) )
    {
        //process_launch( func_prc[ FUNC_WRU ] , 0 );
    }
    
//    process_launch( func_prc[ FUNC_LIGHT_DISABLE ] , 0 );

	/* Aici a stat, 03-08.09.2026, o sonda de banc care injecta
	   cmd_q_insert(INPUT_PARKING_ON) ca sa se poata declansa lantul de hazard de
	   la un cadru 0x593. A fost necesara cit timp cautam de ce nu pulsa OUT_1;
	   cauza reala era alta (seed-ul din config_gpio.h umbrea contractul, vezi
	   RELEASE_NOTES_DS4.md). Scoasa dupa ce hazardul a fost confirmat la banc pe
	   panica si pe RS, deci exista declansatori reali pe DS4. */

	TRACE_FUNC("[%s]\n\r",__FUNCTION__);
}

                   
/*--------------------------------------------------------------------------*/

void func_cmd_oem_unlock( void )
{    
    timeout_q_remove( (void*)func_gwr_off_relock );
	
    ATOMIC(skip_lock = FALSE);
    
	//process_launch( func_prc[ FUNC_LIGHT_DISABLE ] , 0 );        

    process_launch( func_prc[ FUNC_OEM_UNLOCK ] , 0 ); 
	
	TRACE_FUNC("[%s]\n\r",__FUNCTION__);       
}
                   
/*--------------------------------------------------------------------------*/

void func_cmd_oem_trunk( void )
{    
	timeout_q_remove( (void*)func_gwr_off_relock );

    ATOMIC(skip_lock = FALSE);
    
//    process_launch( func_prc[ FUNC_LIGHT_DISABLE ] , 0 );        

    process_launch( func_prc[ FUNC_OEM_TRUNK ] , 0 );   
	
	TRACE_FUNC("[%s]\n\r",__FUNCTION__);     
}
                   
/*--------------------------------------------------------------------------*/

void func_cmd_light_on( void *arg )
{    
    ATOMIC( func_light_done = FALSE );
        
//    process_launch( func_prc[ FUNC_LIGHT ] , 0 );    
}
                   
/*--------------------------------------------------------------------------*/

void func_cmd_light_off( void *arg )
{    
    ATOMIC( func_light_done = TRUE );                    
}

/*--------------------------------------------------------------------------*/

void func_cmd_aux1( void )
{
    //execute_aux( func_cfg->aux1 );    
}

/*--------------------------------------------------------------------------*/

void func_cmd_aux2( void )
{
    //execute_aux( func_cfg->aux2 );    
}

/*--------------------------------------------------------------------------*/

void func_cmd_aux3( void )
{
    //execute_aux( func_cfg->aux3 );    
}

/*--------------------------------------------------------------------------*/

// void execute_aux( Func_Aux_Type aux_type )
// {
// 	switch( aux_type )
// 	{
// 		case FUNC_AUX_WRU       : process_launch( func_prc[ FUNC_WRU        ] , 0 ); break;
//         case FUNC_AUX_WRD_FRONT : process_launch( func_prc[ FUNC_WRD_FRONT  ] , 0 ); break;
//         case FUNC_AUX_WRD_REAR  : process_launch( func_prc[ FUNC_WRD_REAR   ] , 0 ); break;
//         
//         case FUNC_AUX_GAS_CAP   : execute_gas_cap(); break;
//         
//         
//         default : break ;
// 	}
// }

/*--------------------------------------------------------------------------*/	

// void execute_gas_cap( void )
// {
// 	Boolean disarm_active = FALSE ; 
//     
// 	if( !get_ign_status() )        
// 	{
//     	disarm_active = TRUE ; 

//         process_launch( func_prc[ FUNC_DISARM ] , 0 );    
//     }
//             
//     process_launch( func_prc[ FUNC_GAS_CAP    ] , 0 );
//             
//     if( disarm_active )        
//     {
//     	process_launch( func_prc[ FUNC_RELOCK_TRUNK_RELEASE ] , 0 );    
//     }	
// }

/*--------------------------------------------------------------------------*/


