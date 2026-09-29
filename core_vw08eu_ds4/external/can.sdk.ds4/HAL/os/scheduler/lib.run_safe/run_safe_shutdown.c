
#include "run_safe_private.h"
#include "config_gpio.h"
#include "cmd.h"
#include "sense_notify.h"
#include "status.h"
#include "rf.h"
#include "prog.h"

void gpio_iface_brake(Boolean state);

/* v6.1 VW08EU (14.09.2026): "shutdown with brake" se face IDENTIC pe DB3 si DS4.
   Foloseste calea de ACTUATIE (gpio_iface_brake -> RELAY1), nu linia de status.
   Aici era BRAKE_SENSE_SET/CLR, iar cind statusul a trecut pe DAC_RELAY (v5) a
   plecat si el acolo - gresit. DAC_RELAY e doar sense de frina si are rost doar
   pe DB3, unde exista un remote starter extern care primeste informatia pe D2D
   sau pe fir (W2W). Ghidul de instalare DB2 confirma: "RS - Brake Status" e
   functie din grupul RS = Engine Start & Status, livrata "D2D and Wire-to-Wire".
   Pe DB3 acest fisier foloseste gpio_iface_brake - vezi run_safe_shutdown.c de
   acolo, structura e identica. */

/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

#define RF_IS_DIS !!(prog_retrieve_option (FEAT_RF_OUTPUT) == NO_RF)

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
                                   
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

/*==========================================================================*/
/*        I N L I N E   F U N C T I O N S   &   T E M P L A T E S           */
/*==========================================================================*/
void run_safe_shutdown (void)
{
	os_tsk_wait (100)           ;	// add delay to avoid sending door open and brake press too close in D2D
    
    sense_notify_brake_skip(TRUE) ;
     
    if(RF_IS_DIS)
      {
        gpio_iface_brake(TRUE);
      }
	
    ATOMIC (brake.bit.brake = 1);
    					  
    cmd_q_insert(INPUT_BRAKE_STATUS_PRESS, COMMAND_INSERT_TIMEOUT);
	
	os_tsk_wait (500)           ;
	
	cmd_q_insert(INPUT_BRAKE_STATUS_RELEASE, COMMAND_INSERT_TIMEOUT);

    ATOMIC (brake.bit.brake = 0);
    
    if (RF_IS_DIS)
    {
      gpio_iface_brake(FALSE);
    }
    
    sense_notify_brake_skip(FALSE) ;
    
    #if DBG_RUN_SAFE == 1
        dbg_tx_text("\r\nRunSafe Shutdown") ;
    #endif 

}
/*==========================================================================*/

