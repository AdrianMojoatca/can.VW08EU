/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: sense_oem.c 4227 2012-10-18 13:32:07Z florin.olariu $
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
static Timeout_Q_Status mechanical_key_timeout( void );

static void             trace_can_msg(Can_Msg *can_msg);
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean          is_mechanical_key  = FALSE             ;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

void sense_oem_583( Can_Msg *can_msg )
{   
    static UInt8 LAST_B4 = 0x00;
	static SInt8 lock_counter = -1;
	static Boolean lock_pulse_active = FALSE;
	static Boolean lock_cmd = FALSE;
	static Boolean lock_edge_pulse = FALSE;
	
	lock_pulse_active = CAST_BOOL(DATA[4]&0x20);
	lock_edge_pulse   = CAST_BOOL(DATA[4]&0x20) && !CAST_BOOL(LAST_B4&0x20);
	
	if ( lock_pulse_active )
	{
		if( lock_edge_pulse )
			lock_counter = 0;
		
		if ( !lock_cmd )
		{
			if( lock_edge_pulse || (lock_counter >= 16) )
			{
				TRACE_CAN2( "\r\nOEML" );
			}
		}
		
		lock_cmd = lock_edge_pulse || (lock_counter >= 16);
		
		if( lock_counter >= 16 )
			lock_counter = 0;
		
		lock_counter++;
	}
	else
	{
		lock_cmd = FALSE;
	}
	
	//sense_rs_oem_lock_can  ( lock_cmd ); // modified to improve detection for 3x LOCK	 // removed in vers. 1.08
	sense_rs_oem_lock_can(CAST_BOOL(DATA[4]&0x20) && !CAST_BOOL(LAST_B4&0x20));
    sense_rs_oem_unlock_can( CAST_BOOL(DATA[4]&0x10) && !CAST_BOOL(LAST_B4&0x10) );        
    sense_rs_oem_trunk_can ( CAST_BOOL(DATA[4]&0x04) && !CAST_BOOL(LAST_B4&0x04) );

    //keyless sense - type1
    {
         static UInt8 LAST_B2 = 0x00;
        
        #define CAR_IS_LOCKED()                     ((DATA[2]&0x80) != 0x80)
        #define CAR_IS_UNLOCKED()                   ((DATA[2]&0x80) == 0x80)
        #define CAR_IS_ARMED()                      ((DATA[2]&0x30) == 0x30)
        #define CAR_IS_DISARMED()                   ((DATA[2]&0x30) == 0x00)
        
        #define CAR_WAS_LOCKED()                    ((LAST_B2&0x80) != 0x80)
        #define CAR_WAS_UNLOCKED()                  ((LAST_B2&0x80) == 0x80)
        #define CAR_WAS_ARMED()                     ((LAST_B2&0x30) == 0x30)
        #define CAR_WAS_DISARMED()                  ((LAST_B2&0x30) != 0x30)

                
        sense_rs_oem_lock_can  (!get_rs_gwr() && !is_mechanical_key && CAR_IS_ARMED   () && CAR_WAS_DISARMED());        
        //sense_rs_oem_unlock_can(!get_rs_gwr() && !is_mechanical_key && CAR_IS_DISARMED() && CAR_WAS_ARMED   ()); // 13.12.2016 - vers. 1_08

        sense_rs_oem_trunk_can (!get_rs_gwr() && !is_mechanical_key && CAR_IS_ARMED   () && CAR_IS_UNLOCKED () && CAR_WAS_LOCKED()); 	
		//sense_rs_oem_trunk_can (!get_rs_gwr() /*&& !is_mechanical_key*/ && CAR_IS_ARMED   () && CAR_IS_UNLOCKED () && CAR_WAS_LOCKED());  //mih

        sense_rs_oem_lock_can  (!get_rs_gwr() && !is_mechanical_key && CAR_IS_ARMED   () && CAR_WAS_UNLOCKED() && CAR_IS_LOCKED ());        

        #undef CAR_IS_LOCKED         
        #undef CAR_IS_UNLOCKED       
        #undef CAR_IS_ARMED          
        #undef CAR_IS_DISARMED       
        
        #undef CAR_WAS_LOCKED        
        #undef CAR_WAS_UNLOCKED      
        #undef CAR_WAS_ARMED         
        #undef CAR_WAS_DISARMED      

        LAST_B2 = DATA[2] ;

    }
    
    LAST_B4 = DATA[4] ;         
}
	
//--------------------------------------------------------------------------//

void sense_oem_593( Can_Msg *can_msg )
{
    static UInt8 LAST_B6 = 0x00; 
    
    // autolock
    sense_rs_oem_lock_can  ( (DATA[6] == 0x4F) && (LAST_B6 == 0x00) && !get_ign_status() );
                                  
    // keyless sense - type2
	sense_rs_oem_lock_can  ( ((DATA[6] == 0x46) || (DATA[6] == 0x47) || (DATA[6]==0x48) || (DATA[6]== 0x49)) && (LAST_B6 == 0x00) && !get_ign_status() ); // 14_09_2017
    //sense_rs_oem_unlock_can( ((DATA[6] == 0x86) || (DATA[6] == 0x87)) && (LAST_B6 == 0x00));
	sense_rs_oem_unlock_can( ((DATA[6] == 0x86) || (DATA[6] == 0x87) || (DATA[6]==0x88) || (DATA[6]==0x89)) && (LAST_B6 == 0x00));	// // 14_09_2017
    sense_rs_oem_trunk_can ((((DATA[6] == 0x8A) && (DATA[7] == 0x16)) && (LAST_B6 == 0x00)));
    sense_rs_oem_trunk_can ((((DATA[6] == 0x8A) && (DATA[7] == 0x26)) && (LAST_B6 == 0x00)));

    
    LAST_B6 = DATA[6];
}

//--------------------------------------------------------------------------//

void sense_oem_3D0 (Can_Msg *can_msg )
{
    static UInt8 LAST_B0 = 0x00 ;
    

    if( CAST_BOOL(DATA[0]&0x08) != CAST_BOOL(LAST_B0&0x08) )
    {
        ATOMIC( is_mechanical_key = TRUE );

        TRACE_SENSE("\n\r[SENSE_MECHANICAL_KEY_ON]\n\r");

        timeout_q_remove( (void *)mechanical_key_timeout  );

        timeout_q_insert( (Timeout_Q_Func)mechanical_key_timeout , NULL , SENSE_MECHANICAL_KEY_TIMEOUT , 1 , 1 );
    }
                    
    #ifndef PLATFORM_CM800
        sense_rs_oem_lock_can( CAST_BOOL(DATA[0]&0x10) && !CAST_BOOL(LAST_B0&0x10) && !get_ss_trigger() ); 
    #endif
                        
    #if 0 
        #warning "Sensing not working properly on Skoda Octavia 3 - inside door handler trigger unlock"

        sense_rs_oem_lock_can    ( !get_ign_status() && !get_rs_gwr() && !get_ss_lock() && ((DATA[0]&0x06)==0x06) && ((LAST_B0&0x06)!=0x06) && !is_mechanical_key );        
        sense_rs_oem_unlock_can  ( !get_ign_status() && !get_rs_gwr() &&  get_ss_lock() && ((DATA[0]&0x06)!=0x06) && ((LAST_B0&0x06)==0x06) && !is_mechanical_key );        
    #endif
    
    sense_rs_oem_autolock_can( !is_mechanical_key && CAST_BOOL(DATA[0]&0x02) && !CAST_BOOL(LAST_B0&0x02) );
            
    LAST_B0 = DATA[0] ; 
}

// 14_09_2017
void sense_oem_184( Can_Msg *can_msg )
{
    static UInt8 LAST_B4 = 0x00; 
    
    // trunk by foot
	sense_rs_oem_trunk_can ((((DATA[4] == 0x0C)) && (LAST_B4 != 0x0C)));
    
    
    LAST_B4= DATA[4];
}

//--------------------------------------------------------------------------//

Timeout_Q_Status mechanical_key_timeout( void )
{
    ATOMIC( is_mechanical_key = FALSE ) ;
    
    TRACE_SENSE("\n\r[SENSE_MECHANICAL_KEY_TIMEOUT]\n\r"); 
    
    return TIMEOUT_Q_COMPLETE  ;
}

//--------------------------------------------------------------------------//

void trace_can_msg(Can_Msg *can_msg)
{
    static UInt64 last_data = 0 ;  ; 
    
    if(last_data != can_msg->data._64_[0])
    {
        UInt8 i = 0 ; 
        
        TRACE("\n\r|");
        
        for(i = 0 ; i < can_msg->length ; i ++)
        {
            TRACE("%02x|",can_msg->data._8_[i]);
        }
        
        TRACE("\n\r");
    }

    last_data = can_msg->data._64_[0];
}    

//--------------------------------------------------------------------------//
