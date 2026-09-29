/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: reg_atomic_init.c 14828 2012-05-28 17:30:47Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "vw06_scan_NEW.h"
#include "scan.h"
#include "scan_manager.h"
#include <string.h>
/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/
#define STATE_READ1         0
#define STATE_READ2         1
#define STATE_READ3         3


#define STATE_WRITE1        4
#define STATE_WRITE2        5
#define STATE_WRITE3        6
#define STATE_WRITE4        7

#define TIMEOUT_ANSWER      1500
/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
static void pgm_callback(Scan_Evt event, UInt8* data , UInt8 size);

static void vw06_scan_NEW_pgm_open  ( Can_Dev_Id dev_id );
static void vw06_scan_NEW_pgm_close ( void );

static void vw06_scan_NEW_pgm_read1 ( void );
static void vw06_scan_NEW_pgm_read2 ( void );
static void vw06_scan_NEW_pgm_read3 ( void );

static void vw06_scan_NEW_pgm_write1( void );
static void vw06_scan_NEW_pgm_write2( void );
static void vw06_scan_NEW_pgm_write3( void );
static void vw06_scan_NEW_pgm_write4( void );

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

static const    UInt8 WRITE1_CMD[]  = {0x22,0xF1,0x99};
static const    UInt8 WRITE2_CMD[]  = {0x2E,0xF1,0x99,0x17,0x03,0x01};
static const    UInt8 WRITE3_CMD[]  = {0x2E,0xF1,0x98,0x00,0x00,0x0E,0x2C,0x18,0xB5};


static const    UInt8 READ1_CMD[] = {0x10,0x03};
static const    UInt8 READ2_CMD[] = {0x22,0xF1,0x9E};
static const    UInt8 READ3_CMD[] = {0x22,0x06,0x00};

static          UInt8 buf_read[48];
static volatile UInt8 retry_count  ;
static volatile Boolean pgm_done   ;
static          UInt8 state        ;
static          Car_Computer *central_electric;
static          UInt8 pgm_data[48];
static          UInt8 pgm_size;    

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

Boolean vw06_scan_NEW_pgm1( Can_Dev_Id dev_id )
{
    pgm_done = FALSE;
    
    vw06_scan_NEW_pgm_open(dev_id);
    
	os_tsk_wait(1000);
    
    vw06_scan_NEW_pgm_read1 ();os_tsk_wait(1000);
    vw06_scan_NEW_pgm_read2 ();os_tsk_wait(1000);
    vw06_scan_NEW_pgm_read3 ();os_tsk_wait(1000);
    vw06_scan_NEW_pgm_write1();os_tsk_wait(1000);
    vw06_scan_NEW_pgm_write2();os_tsk_wait(1000);
    vw06_scan_NEW_pgm_write3();os_tsk_wait(1000);
    vw06_scan_NEW_pgm_write4();os_tsk_wait(1000);
    vw06_scan_NEW_pgm_close ();
    
    
    return pgm_done ; 
}

//--------------------------------------------------------------------------//

void pgm_callback(Scan_Evt event, UInt8* data , UInt8 size)
{
    switch(event)
    {
        case EVT_CONNECTED :
        {
            
        }
        break;

        case EVT_DATA_RECEIVED :
        {
            switch( state )
            {
                case STATE_READ3 :
                {            
                    UInt8 i = 0 ;
                    
                    buf_read[0 ]  =  0x2E;
                    buf_read[11] |=  0x04;
                    buf_read[11] &= ~0x02;
                    
                    pgm_size = size ; 
                                        
                    memcpy(&pgm_data[0],&buf_read[0],size) ;

                }break;
                
                case STATE_WRITE4:
                {
                    if(buf_read[2] == 0x78) pgm_done = TRUE ;
                }
                break;

                default : break;
            }
                    
        }
        break;
                
        case EVT_RX_INDEX_ERROR:
        case EVT_RX_TIMEOUT    :
        case EVT_ACK_ERROR     :
        {
            
        }
        break;
        
        default : break;
    }              
}

//--------------------------------------------------------------------------// 
          
void vw06_scan_NEW_pgm_read1( void )
{
     state = STATE_READ1 ; 
     
     scan_send( central_electric , (UInt8 *)READ1_CMD , sizeof(READ1_CMD) , TIMEOUT_ANSWER);
}

//--------------------------------------------------------------------------// 
          
void vw06_scan_NEW_pgm_read2( void )
{
     state = STATE_READ2 ; 
     
     scan_send( central_electric , (UInt8 *)READ2_CMD , sizeof(READ2_CMD) , TIMEOUT_ANSWER);
}

//--------------------------------------------------------------------------// 

void vw06_scan_NEW_pgm_read3( void )
{
     state = STATE_READ3 ; 
     
     scan_send( central_electric , (UInt8 *)READ3_CMD , sizeof(READ3_CMD) , TIMEOUT_ANSWER);
}

//--------------------------------------------------------------------------// 


void vw06_scan_NEW_pgm_write1( void )
{
     state = STATE_WRITE1 ; 
     
     scan_send( central_electric , (UInt8 *)WRITE1_CMD , sizeof(WRITE1_CMD) , TIMEOUT_ANSWER);
}

//--------------------------------------------------------------------------// 
          
void vw06_scan_NEW_pgm_write2( void )
{
     state = STATE_WRITE2 ; 
     
     scan_send( central_electric , (UInt8 *)WRITE2_CMD , sizeof(WRITE2_CMD) , TIMEOUT_ANSWER);
}

//--------------------------------------------------------------------------// 

void vw06_scan_NEW_pgm_write3( void )
{
     state = STATE_WRITE3 ; 
     
     scan_send( central_electric , (UInt8 *)WRITE3_CMD , sizeof(WRITE3_CMD) , TIMEOUT_ANSWER);
}

//--------------------------------------------------------------------------// 

void vw06_scan_NEW_pgm_write4( void )
{
     state = STATE_WRITE4 ; 
     
     scan_send( central_electric , (UInt8 *)pgm_data , pgm_size , TIMEOUT_ANSWER);
}

//--------------------------------------------------------------------------// 

void vw06_scan_NEW_pgm_open( Can_Dev_Id dev_id)
{
    scan_init(dev_id);    

    scan_power(TRUE);

    central_electric = scan_open("CE",0x17FC008B,0x17FE008B,buf_read,pgm_callback );
}

//--------------------------------------------------------------------------// 

void vw06_scan_NEW_pgm_close( void )
{
    scan_close( central_electric );

    scan_power(FALSE);
}

//--------------------------------------------------------------------------// 

    