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

//#define AUDI_TEST

typedef enum
{
    STATE_IDLE      ,
    
    STATE_GET_INFO1 , 
    STATE_GET_INFO2 , 
    STATE_GET_INFO3 , 
    STATE_GET_INFO4 , 
    STATE_GET_INFO5 , 
    
    STATE_SEED_REQ  ,
    STATE_AUTH_ACK  ,
    
    STATE_ADAPT_REQ ,
    STATE_ADAPT_ACK ,        
    STATE_ADAPT_DONE ,        
}PGM_STATE;

#define PGM_TIMEOUT_ANSWER      (1500)
#define PGM_TIMEOUT             (30000)
#define PGM_BUFF_SIZE           (128)
#define PGM_ADAPT_SIZE          (64)

#ifdef AUDI_TEST
#define PGM_PASSWORD            (20113)
#else
#define PGM_PASSWORD            (31347)
#endif

typedef struct s_Pgm_Data
{
    Irq           irq;
    UInt8         buff[PGM_BUFF_SIZE]; 
    UInt8         adapt[PGM_ADAPT_SIZE];
    UInt8         adapt_size;
    PGM_STATE     state    ;
    Can_Dev_Id    dev_id   ; 
    Car_Computer  *ce;    
    Boolean       done;
}Pgm_Data;


/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
static void pgm_callback     (Scan_Evt event, UInt8* data , UInt8 size);
static void pgm_get_info1    (void);
static void pgm_get_info2    (void);
static void pgm_get_info3    (void);
static void pgm_get_info4    (void);
static void pgm_get_info5    (void);
static void pgm_req_seed     (void);
static void pgm_req_auth     (UInt32 seed,UInt32 pass);
static void pgm_req_adapt    (void);
static void pgm_set_adapt    (void);
static void pgm_set_adapt_def(void);
static void pgm_req_open     (void);
static void pgm_req_close    (void);
static void pgm_failed       (void);
static void pgm_state_manager(UInt8 *data , UInt16 size , Boolean error );
static void pgm_trace_data   (UInt8 *data , UInt16 size);

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Pgm_Data *pgm_data;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

//--------------------------------------------------------------------------//

Boolean vw06_scan_NEW_pgm2( Can_Dev_Id dev_id )
{
    pgm_data = os_mem_request(sizeof(*pgm_data));
    
    memset(pgm_data,0,sizeof(*pgm_data));
    
    pgm_data->irq    = os_irq_create();    
    //pgm_data->dev_id = dev_id;
	pgm_data->dev_id = CAN_DEV_0; // use FT for scan - vers. 1_04 - 04_04_2018
    
    pgm_req_open();
    
    os_irq_wait(pgm_data->irq,TRUE,PGM_TIMEOUT);
    
    pgm_req_close();
    
    return pgm_data->done; 
}

//--------------------------------------------------------------------------//

void pgm_callback(Scan_Evt event, UInt8* data , UInt8 size)
{
    switch(event)
    {
        case EVT_CONNECTED :
        {                        
            pgm_get_info1();
        }
        break;
        
        case EVT_RX_INDEX_ERROR:
        {
            TRACE("[PGM_TIGUAN] >> ERROR : index \n\r");
            
            pgm_state_manager(NULL,0,TRUE);
        }
        break;
        
        case EVT_RX_TIMEOUT:
        {
            TRACE("[PGM_TIGUAN] >> ERROR : timeout \n\r");
            
            pgm_state_manager(NULL,0,TRUE);
        }
        break;
        
        case EVT_ACK_ERROR:
        {
            TRACE("[PGM_TIGUAN] >> ERROR : ack \n\r");
            
            pgm_state_manager(NULL,0,TRUE);
        }
        break;

        case EVT_DATA_RECEIVED :
        {
            TRACE("[PGM_TIGUAN] >> Data received \n\r");
            
            pgm_trace_data(data,size);
            
            pgm_state_manager(data,size,FALSE);                    
        }
        break;
                
                
        default : break;
    }              
}

//--------------------------------------------------------------------------// 
          
static void pgm_get_info1(void)
{
    const UInt8 data[] = {0x10,0x03};
    
    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));

    pgm_data->state = STATE_GET_INFO1; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);

    TRACE("[PGM_TIGUAN] >> GetInfo1 \n\r");

    pgm_trace_data(pgm_data->buff,sizeof(data));
}

//--------------------------------------------------------------------------// 
          
static void pgm_get_info2(void)
{
    static const UInt8 data[] = {0x22,0xF1,0x9E};
    
    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));

    pgm_data->state = STATE_GET_INFO2; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);

    TRACE("[PGM_TIGUAN] >> GetInfo2 \n\r");

    pgm_trace_data(pgm_data->buff,sizeof(data));
}

//--------------------------------------------------------------------------// 

static void pgm_get_info3(void)
{
    const UInt8 data[] = {0x22,0xF1,0xA2};
    
    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));

    pgm_data->state = STATE_GET_INFO3; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);

    TRACE("[PGM_TIGUAN] >> GetInfo3 \n\r");

    pgm_trace_data(pgm_data->buff,sizeof(data));
}

//--------------------------------------------------------------------------// 

static void pgm_get_info4(void)
{
    const UInt8 data[] = {0x22,0xF1,0x99};
    
    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));

    pgm_data->state = STATE_GET_INFO4; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);

    TRACE("[PGM_TIGUAN] >> GetInfo4 \n\r");

    pgm_trace_data(pgm_data->buff,sizeof(data));
}

//--------------------------------------------------------------------------// 

static void pgm_get_info5(void)
{
    const UInt8 data[] = {0x2E,0xF1,0x99};
    
    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));

    pgm_data->state = STATE_GET_INFO5; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);

    TRACE("[PGM_TIGUAN] >> GetInfo5 \n\r");

    pgm_trace_data(pgm_data->buff,sizeof(data));
}

//--------------------------------------------------------------------------// 

static void pgm_req_seed(void)
{
    const UInt8 data[] = {0x27,0x03};

    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));

    pgm_data->state = STATE_SEED_REQ; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);

    TRACE("[PGM_TIGUAN] >> ReqSeed \n\r");

    pgm_trace_data(pgm_data->buff,sizeof(data));
}

//--------------------------------------------------------------------------// 
          
static void pgm_req_auth(UInt32 seed , UInt32 pass )
{
    const UInt8 data[] = {0x27,0x04,0x00,0x00,0x00,0x00};
    
    UInt32 auth;    

    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));
    
    auth = seed + pass;

    pgm_data->buff[2] = ((auth >> 24)&0xFF);
    pgm_data->buff[3] = ((auth >> 16)&0xFF);
    pgm_data->buff[4] = ((auth >> 8 )&0xFF);
    pgm_data->buff[5] = ((auth >> 0 )&0xFF);
    
    pgm_data->state = STATE_AUTH_ACK; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);

    TRACE("[PGM_TIGUAN] >> ReqAuth %08X\n\r" , auth);

    pgm_trace_data(pgm_data->buff,sizeof(data));
}

//--------------------------------------------------------------------------// 

static void pgm_req_adapt(void)
{
    const UInt8 data[] = {0x22,0x0D,0x07};

    memcpy(pgm_data->buff,(UInt8*)data,sizeof(data));

    pgm_data->state = STATE_ADAPT_REQ; 
     
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(data),PGM_TIMEOUT_ANSWER);    

    TRACE("[PGM_TIGUAN] >> ReqAdaptation \n\r");

    pgm_trace_data(pgm_data->buff,sizeof(data));
}
    
//--------------------------------------------------------------------------// 

static void pgm_set_adapt_def(void)
{        
    const UInt8 default_data[] = {0x2E,0xF1,0x98,0x01,0x81,0xCE,0x2D,0x1E,0xD6};
                
    TRACE("[PGM_TIGUAN] >> Found invalid values , load default \n\r");   
                
    memcpy(pgm_data->buff,(UInt8*)default_data,sizeof(default_data));
        
    scan_send( pgm_data->ce,pgm_data->buff,sizeof(default_data),PGM_TIMEOUT_ANSWER);    
        
    TRACE("[PGM_TIGUAN] >> SetAdaptation \n\r");    
    
    pgm_trace_data(pgm_data->buff,sizeof(default_data));
        
    pgm_data->state = STATE_ADAPT_ACK;         
}

//--------------------------------------------------------------------------// 

static void pgm_set_adapt(void)
{        
    pgm_data->state = STATE_ADAPT_DONE; 
        
    TRACE("[PGM_TIGUAN] >> Found valid values , use the values \n\r");   
        
    pgm_data->adapt[0]  = 0x2E;            
    //pgm_data->adapt[4] |= 0x80; // commented first release vw08eu - new scan pgm only this changed !!!
	pgm_data->adapt[5] |= 0x01;   // first release vw08eu - new scan pgm only this changed !!!!
      
    
    scan_send( pgm_data->ce,pgm_data->adapt,pgm_data->adapt_size,PGM_TIMEOUT_ANSWER);   
        
    TRACE("[PGM_TIGUAN] >> SetAdaptation \n\r");    
    
    pgm_trace_data(pgm_data->adapt,pgm_data->adapt_size);       
}

//--------------------------------------------------------------------------// 

static void pgm_req_open(void)
{
    pgm_data->state = STATE_IDLE;
    
    scan_init(pgm_data->dev_id);    

    scan_power(TRUE);

    pgm_data->ce = scan_open("CE",0x70E,0x778,pgm_data->buff,pgm_callback );
    
    TRACE("[PGM_TIGUAN] >> Open scan session\n\r");
}

//--------------------------------------------------------------------------// 

static void pgm_req_close( void )
{
    if(pgm_data->ce == NULL )
    {
        scan_close( pgm_data->ce );

        scan_power(FALSE);
    
        pgm_data->ce = NULL;
        
        TRACE("[PGM_TIGUAN] >> Close scan session\n\r");
    }
}

//--------------------------------------------------------------------------//

static void pgm_failed(void)
{
    pgm_req_close();
                
    os_tsk_wait(1000);
                
    pgm_req_open();
}

//--------------------------------------------------------------------------//

static void pgm_state_manager(UInt8 *data , UInt16 size , Boolean error )
{

	switch(pgm_data->state)
    {        
        case STATE_GET_INFO1 :
        {  
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to get Info1 \n\r");
            }
            
            error ? pgm_failed() : pgm_get_info2();                        
        }
        break;
        
        case STATE_GET_INFO2 :
        {
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to get Info2 \n\r");
            }
            
            error ? pgm_failed() : pgm_get_info3();                                                
        }
        break;
        
        case STATE_GET_INFO3 : 
        {
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to get Info3 \n\r");
            }
                        
            error ? pgm_failed() : pgm_req_seed();                                                
        }
        break;
    
        case STATE_SEED_REQ  :
        {
            UInt32 seed = 0;
            
            if(!error)
            {
                error = !((data[1] == 0x03));
                    
                if(!error)
                {
                    UInt32  seed_msb = data[2];
                    UInt32  seed_mid1= data[3];
                    UInt32  seed_mid0= data[4];
                    UInt32  seed_lsb = data[5];
                    
                    seed = ((seed_msb << 24)&0xFF000000) |
                           ((seed_mid1<< 16)&0x00FF0000) |
                           ((seed_mid0<< 8 )&0x0000FF00) |
                           ((seed_lsb << 0 )&0x000000FF);
                }                    
                
            }
            
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to request SEED \n\r");
            }
            else
            {
                TRACE("[PGM_TIGUAN] >> Request SEED --> %08X\n\r",seed);
            }
                        
            error ? pgm_failed() : pgm_req_auth(seed,PGM_PASSWORD);   
        }
        break;
        
        case STATE_AUTH_ACK  :
        {
            if(!error)
            {
                error = !((data[0] == 0x67));
            }
            
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to authenticate \n\r");
            }
            
            error ? pgm_failed() : pgm_req_adapt();
        }
        break;
        
        case STATE_ADAPT_REQ :
        {
            if(!error)
            {
                error = !((data[0] == 0x62) && (data[1] == 0x0D));
            }
            
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to request the adaptation \n\r");
            }
            else
            {
                if(size < PGM_ADAPT_SIZE)
                {
                    memcpy(pgm_data->adapt,data,size);
                    
                    pgm_data->adapt_size = size;                    
                }
                else
                {
                    error = TRUE;
                    
                    TRACE("[PGM_TIGUAN] >> Failed to request the adaptation(out of memory) \n\r");
                }
            }
            
            if(error)
            {
                pgm_failed();
            }
            else
            {
                //if( (data[7] == 0x00) && (data[8] == 0x00) )  // commented in vers. 1_05
				if( (data[7] == 0x00) ) // vers. 1_05 
                {
                    pgm_get_info4();
                }
                else
                {
                    pgm_set_adapt();
                }
            }                                                
        }
        break;
        
        case STATE_GET_INFO4:
        {
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to get Info 4 \n\r");
                
                pgm_failed();
            }
            else
            {
                pgm_get_info5();
            }                        
        }
        break;
        
        case STATE_GET_INFO5:
        {
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to get Info 5 \n\r");
                
                pgm_failed();
            }
            else
            {
                pgm_set_adapt_def();
            }                        
        }
        break;
        
        case STATE_ADAPT_ACK : 
        {
            if(!error)
            {
               error = !((data[0] == 0x6E) && (data[1] == 0xF1));
            }
            
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to set the adaptation \n\r");
                
                pgm_failed();
            }
            else
            {
                pgm_set_adapt();
            }
                        
        }
        break;   
        
        case STATE_ADAPT_DONE : 
        {            
            if(error)
            {
                TRACE("[PGM_TIGUAN] >> Failed to set the adaptation \n\r");
            }
            
            if(error)
            {
                pgm_failed();
            }
            else
            {
                pgm_data->done = TRUE;
                
                pgm_req_close();
                
                os_irq_set(pgm_data->irq);                
            }
        }
        break;   
        
        
        default:break;
    }
}

//--------------------------------------------------------------------------//

static void pgm_trace_data(UInt8 *data , UInt16 size)
{
    UInt16 i = 0 ;
        
    TRACE("\tsize       >> %d \n\r");
    TRACE("\tdata       >> ");
    
    for(i = 0 ; i < size; i++)
    {
        TRACE(" %02x",data[i]);
    }
    
    TRACE("\n\r");    
}

//--------------------------------------------------------------------------//

