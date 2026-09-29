/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_rs.c 4279 2012-11-16 10:42:15Z florin.olariu $
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

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
Boolean ignore_doors_sense;

/* v8 VW08EU (15.09.2026): "pornirea curenta e a NOASTRA?"

   TRUE doar daca modulul a adus el contactul sus, adica a intrat efectiv pe
   ramura !get_ign_status() din func_rs_ign_on() si a apasat PTS. Pe VW08EU asta
   e singura cale prin care porneste motorul: iesirea de ignition a remote
   starter-ului nu alimenteaza masina (FUNC_RS_IGNITION2_OUT e no-op, H2/6 e
   doar status catre RS). Deci "contactul era deja sus la IGN_ON" inseamna,
   fara ambiguitate, ca masina a fost pornita de sofer.

   Se sterge la ambele capete de sesiune (GWR ON si GWR OFF), ca sa nu ramina
   TRUE de la o sesiune anterioara daca se pierde un GWR OFF. La reset porneste
   FALSE, adica in sensul sigur: nu oprim un motor despre care nu stim nimic.

   Foloseste DOAR pentru a paza pulsul de PTS de la shutdown. Restul secventei
   (frina, ambreiaj, statusuri) ramine neatinsa. */
static Boolean func_rs_start_is_ours = FALSE ;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_rs_init( void )
{
    tsk_rs_init();
	    
    tsk_rs_register(  TSK_RS_GWR_ON  , (Tsk_Rs_Func)func_rs_gwr_on  );
    tsk_rs_register(  TSK_RS_GWR_OFF , (Tsk_Rs_Func)func_rs_gwr_off );    
    tsk_rs_register(  TSK_RS_IGN_ON  , (Tsk_Rs_Func)func_rs_ign_on  );
    tsk_rs_register(  TSK_RS_IGN_OFF , (Tsk_Rs_Func)func_rs_ign_off );
    tsk_rs_register(  TSK_RS_STR_ON  , (Tsk_Rs_Func)func_rs_str_on  );
    tsk_rs_register(  TSK_RS_STR_OFF , (Tsk_Rs_Func)func_rs_str_off );
}

/*--------------------------------------------------------------------------*/

/* STATUSURILE RS - adaugate 10.09.2026, dupa modelul FRD37EU (func_rs.c:56,93,
   131,163,198,250). Cine executa actiunea ii publica si statusul.

   De ce sint necesare: pe DB2 statusurile erau scrise de un handler din CORE,
   lib.flc.401.core/core_init_other.c, inregistrat pe INPUT_GWR|INPUT_ON. Acea
   biblioteca NU are corespondent pe DB3 si nimic nu i-a preluat rolul, deci
   set_rs_gwr / set_rs_ign / set_rs_start aveau ZERO apelanti - existau doar ca
   definitie si ca intrare in shim-ul de contract. Consecinta: get_rs_gwr() era
   permanent FALSE, iar oprirea de siguranta la viteza din sense_rs_can_speed.c
     if( !!new_speed && get_rs_gwr() && get_rpm() && cfg->speed_shutdown_on )
   nu se putea declansa niciodata. Adrian a confirmat la banc: merge pe DB2, nu
   merge pe DB3. Vezi analiza completa in RELEASE_NOTES.

   Atentie, get_rs_gwr() nu e citit doar de protectia de viteza: mai apare in
   dl_lock.c, dl_unlock.c, dl_trunk.c, hazard_disable.c, lf_tsk.c si chiar mai
   jos in func_rs_ign_on (bucla de asteptare a contactului, care pina acum iesea
   instantaneu). Toate ramurile acelea se trezesc odata cu asta.

   set_rs_idle NU se scrie: are zero cititori pe DB3, iar FRD37EU nu-l cheama. */

void func_rs_gwr_on( void )
{
    TRACE_FUNC("[%s]\n\r",__FUNCTION__);
	TRACE_CAN2( "\r\nGWRON" );

	/* inceput de sesiune: pina se dovedeste contrariul in func_rs_ign_on(),
	   pornirea nu e a noastra. */
	func_rs_start_is_ours = FALSE ;

	set_rs_gwr (TRUE);

    FUNC_RF_LOOP_RSR(TRUE);
}

/*--------------------------------------------------------------------------*/

void func_rs_ign_on( void )
{
    Boolean done        = FALSE ;
    UInt8   retry_count = 60    ; 

	TRACE_FUNC("[%s]\n\r",__FUNCTION__);
	TRACE_CAN2( "\r\nIGNON" );
    
    FUNC_RS_IGNITION2_OUT( TRUE );

//	os_tsk_wait( 300 );

    if( !get_ign_status() )
    {
		/* contactul era jos si il urcam noi, apasind PTS: pornirea e a noastra,
		   deci avem dreptul sa o si oprim cu PTS la shutdown. */
		func_rs_start_is_ours = TRUE ;

		FUNC_PTS_OUT( TRUE );
        
        while( !done && get_rs_gwr() )
        {                        
            os_tsk_wait( 50 );
            
            if( get_ign_status() || !( -- retry_count ) )
            {
                done = TRUE ; 
            }
        }

		FUNC_PTS_OUT( FALSE );

        os_tsk_wait( 300 );
    }

	set_rs_ign (TRUE);

	TRACE_FUNC("[%s - %s]\n\r",__FUNCTION__ , get_ign_status() ? "done" : "error");
}
                    
/*--------------------------------------------------------------------------*/


void func_rs_str_on( void )
{
    Boolean done        = FALSE ;
    UInt8   retry_count = 10    ;
	
	TRACE_FUNC("[%s]\n\r",__FUNCTION__); 
	TRACE_CAN2( "\r\nSTRON" );
        
    brake_rpm_bypass_on:
    {
        sense_rs_can_brake_skip_enter();
        clutch_press();
    
        FUNC_BRAKE_OUT( TRUE );       

        os_tsk_wait( 200 );
    }
                           
	FUNC_PTS_OUT( TRUE );

	set_rs_start(TRUE);
}

/*--------------------------------------------------------------------------*/

void func_rs_str_off( void )
{            
	TRACE_FUNC("[%s]\n\r",__FUNCTION__);
	TRACE_CAN2( "\r\nSTROFF" );

	FUNC_PTS_OUT( FALSE );

    os_tsk_wait( 200 );
    
    FUNC_BRAKE_OUT( FALSE );
        
    clutch_release();
	
	os_tsk_wait( 200 );
	    
    sense_rs_can_brake_skip_leave();

	set_rs_start(FALSE);
}

/*--------------------------------------------------------------------------*/

void func_rs_ign_off( Boolean do_shutdown )
{
    Boolean done        = FALSE ;
    UInt8   retry_count = 10    ;
	
	FUNC_BRAKE_OUT( FALSE );
	
	sense_rs_can_brake_skip_leave(); 	     	
	        
    clutch_release();

	TRACE_FUNC("[%s]\n\r",__FUNCTION__);
	TRACE_CAN2( "\r\nIGNOFF" );

    FUNC_RS_IGNITION2_OUT( FALSE );

	/* func_rs_start_is_ours e conditia adaugata in v8 (15.09.2026). Fara ea,
	   daca soferul porneste masina normal si pleaca cu ea, iar apoi se apasa
	   Remote Start, secventa ajunge tot aici cu do_shutdown TRUE si cu contactul
	   sus - si pulsul de mai jos oprea motorul IN MERS. Vezi comentariul de la
	   declaratia variabilei. */
    if( do_shutdown && get_ign_status() && func_rs_start_is_ours )
    {
        TRACE_FUNC("[%s - do shutdown]\n\r",__FUNCTION__);

		/* FUNC_PTS_SHUTDOWN_OUT, nu FUNC_PTS_OUT: la oprirea pe viteza garda
		   tsk_rs_invalid_state() e deja zavorita de viteza si ar transforma
		   apasarea in FALSE. Vezi comentariul din func_private.h. */
		FUNC_PTS_SHUTDOWN_OUT( TRUE );

        while( !done )
        {
            os_tsk_wait( 300 );

            if( !get_ign_status() || !( -- retry_count ) )
            {
                done = TRUE ;
            }
        }

		FUNC_PTS_SHUTDOWN_OUT( FALSE );
    }
	else if( do_shutdown && get_ign_status() )
	{
		/* motorul merge, dar nu noi l-am pornit - nu-l oprim. */
		TRACE_FUNC("[%s - shutdown, dar pornirea nu e a noastra: fara PTS]\n\r",__FUNCTION__);
	}

	set_rs_ign (FALSE);
}

/*--------------------------------------------------------------------------*/

void func_rs_gwr_off( void )
{
    TRACE_FUNC("[%s]\n\r",__FUNCTION__);
	TRACE_CAN2( "\r\nGWROFF" );

	/* capat de sesiune: urmatorul shutdown nu mai are voie sa apese PTS decit
	   daca o noua secventa trece iar prin func_rs_ign_on() cu contactul jos. */
	func_rs_start_is_ours = FALSE ;

	set_rs_gwr (FALSE);

    FUNC_RF_LOOP_RSR(FALSE);
	
	if( get_rpm() == 0 /*&& get_ss_lock() == FALSE*/ ) // ADI - in vers. 01 we removed the get_ss_lock condition
	{
		process_launch( func_prc[ FUNC_RAP ] , 0 );

	}
}

/*--------------------------------------------------------------------------*/
