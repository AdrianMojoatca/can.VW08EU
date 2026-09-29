/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_init.c 4229 2012-10-18 18:45:04Z florin.olariu $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/
#include "func_private.h"
#include "detect.h"
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

void func_init( Func_Cfg *cfg, Boolean init_obd_can )
{        
    func_cfg = cfg;
    
    can_usr_init( CAN0_ENABLE | CAN0_RX_BUFFER_ENABLE | CAN0_TX_BUFFER_ENABLE | CAN1_ENABLE | CAN1_RX_BUFFER_ENABLE | CAN1_TX_BUFFER_ENABLE );

    if( init_obd_can )
    {
		/* PORTARE VW08EU - OBD CAN lasat neactivat, deliberat.
		   In DB2 blocul era:
		       obd_can_init( CAN_DEV_OBD, TRUE );
		       obd_can_activate();
		   Doua motive pentru care nu se transcrie direct:
		   1. lib.flc.obd_can exista numai pe partea de CORE si nu e rutata prin
		      tabela de API - pe partea de FW nu exista nici declaratia.
		   2. API-ul s-a scindat intre timp: obd_can_init(void) +
		      obd_can_init_can(dev, is_std, do_init_can). Semantica argumentului
		      do_init_can nu e stabilita pentru cazul nostru (CAN-ul e deja
		      initializat mai sus), deci nu ghicim.
		   In CORE apelurile echivalente sint comentate azi (core.c ~225), deci
		   OBD-ul nu e activat nici acolo. Vezi STARE_PORTARE.md. */
		TRACE("[VW08EU INIT : OBD detectat, activare AMANATA]\n\r");
    }
    else
    {
        TRACE("[VW08EU INIT : OBD disable]\n\r");
    }

    func_dl_type = FUNC_DL_UNKNOWN ;
		
    /* Canalul vine din cfg, NU se scrie direct aici. Cind se ajunge in func_init,
       cfg->can_dev_id e intotdeauna CAN_DEV_FUNC_SENSE - singura alternativa,
       CAN_DEV_COUNT, opreste apelul din fw_init. Deci a-l inlocui cu o valoare
       fixa nu aduce nimic, dar rupe canalele in doua in ziua in care aliasul se
       schimba: detectia si TX-ul pe un dev, can_sync si sense pe altul, iar
       sense-ul amuteste fara niciun semn. Vezi cele sase ocoliri reparate la
       04.09.2026 in RELEASE_NOTES_DS4.md. */
    can_sync_init( cfg->can_dev_id , 10 ) ;
    can_per_init (  10  );

    sense_init( cfg->can_dev_id );
    
    clutch_init   ();
	    
	func_rs_init  ();
    func_prc_init ();
    func_cmd_init ();
    func_eips_init(); 

	// mih
	//wnd_rol_tmr = timeout_f_create(10000,wnd_roll_tmr_func);  //ADI - 04_12_2017 - mih
}
              
//--------------------------------------------------------------------------//

