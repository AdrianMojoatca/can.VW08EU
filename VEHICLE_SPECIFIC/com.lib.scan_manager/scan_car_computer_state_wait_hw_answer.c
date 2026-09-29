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

void scan_car_computer_state_wait_hw_answer(Car_Computer *computer ,Scan_Internal_Event event , UInt8 *data ,UInt8 size)
{
    UInt8 i ;

    switch(event)
    {
        case TIMEOUT_RX_EXPIRED:
        {
            scan_car_computer_set_state(computer,CONNECTED);/// reset the state to CONNECTED
            
            computer->rx_data_index = 0;
            computer->rx_data_size  = 0;
            computer->rx_frame_index= 0;            
                        
            if(computer->callback)
            {
                computer->callback(EVT_RX_TIMEOUT,NULL,0);
            }                         
        }
        break;

        case CAN_FRAME_RECEIVED:
        {
            switch(data[0]&0xF0)
            {
                case 0x00 : // single frame received
                {
                    computer->rx_data_size = data[0]&0x0F ;
                    
//                     for(i=0; i < computer->rx_data_size ;i++)
//                     {
//                         computer->rx_data[i] = data[i+1];
//                     }
					memcpy(&computer->rx_data[0] , &data[0+1] , computer->rx_data_size) ;


                    scan_car_computer_set_state(computer,CONNECTED);

                    scan_manager_trace_rx(computer);

                    if(computer->callback)
                    {
                        computer->callback(EVT_DATA_RECEIVED , computer->rx_data , computer->rx_data_size);
                    }
                    
                    computer->rx_data_index = 0;
                    computer->rx_data_size  = 0;
                    computer->rx_frame_index= 0;  
                    computer->timeout_rx    = 0;                                      
                }
                break;

                case 0x10: // composite frame - first frame
                {                     
                     computer->rx_data_size  = data[1];
                     computer->rx_data_index = 0;
                     computer->rx_frame_index= 0;
                                                
//                      for(i=0; i < 6 ;i++)
//                      {
//                         computer->rx_data[computer->rx_data_index++] = data[i+2];
//                      }
					 memcpy(&computer->rx_data[computer->rx_data_index] , &data[0+2] , 6) ;
					 computer->rx_data_index += 6;

                     scan_car_computer_tx_ack(computer); // ack the first frame

                     computer->timeout_rx = computer->timeout_answer; 
                     
                     scan_car_computer_set_state(computer,CONNECTED);                    
                }
                break;

                
                case 0x20: // composite frame
                {
                    UInt8 frame_index = data[0]&0x0F;

                    computer->timeout_rx = computer->timeout_answer;

                    computer->rx_frame_index ++ ;

                    if( (computer->rx_frame_index != frame_index) || (computer->rx_data_index==0) )
                    {
                        scan_car_computer_set_state(computer,CONNECTED);
                        
                        computer->rx_data_index = 0;
                        computer->rx_data_size  = 0;
                        computer->rx_frame_index= 0;
                        computer->timeout_rx    = 0;                                                                

                        if(computer->callback)
                        {
                            computer->callback( EVT_RX_INDEX_ERROR , NULL , 0);
                        }                                                
                    }
                    else
                    {
                        for(i=0 ; i < 7 ; i ++ )
                        {
                            computer->rx_data[computer->rx_data_index++] = data[i+1];

                            if(computer->rx_data_index >= computer->rx_data_size )break;
                        }

                        if(computer->rx_data_index >= computer->rx_data_size )
                        {
                            scan_car_computer_set_state(computer,CONNECTED);
                            
                            scan_manager_trace_rx(computer);
                                                                                    
                            if(computer->callback)
                            {
                                computer->callback( EVT_DATA_RECEIVED , computer->rx_data , computer->rx_data_size);
                            }
                            
                            computer->rx_data_index = 0;
                            computer->rx_data_size  = 0;
                            computer->rx_frame_index= 0; 
                            computer->timeout_rx    = 0;                                                                                                    
                        }                        
                    }
                    
                }break;

                case 0xA0:  //session closed
                {
                    computer->state = DISCONNECTED;   

                    if(computer->callback)
                    {
                        computer->callback( EVT_DISCONNECTED , NULL , 0);
                    }                                                        
                }
                break;    

                default : 
                {
                    computer->state = CONNECTED;   

                    computer->rx_data_index = 0;
                    computer->rx_data_size  = 0;
                    computer->rx_frame_index= 0;                                                            

                    if(computer->callback)
                    {
                        computer->callback( EVT_RX_UNKNOWN_ERROR , NULL , 0);
                    }                                        
                }
                break;
                                
            }
        }break;       //case CAN_FRAME_RECEIVED:

        default : break;
    }
}

//--------------------------------------------------------------------------//

