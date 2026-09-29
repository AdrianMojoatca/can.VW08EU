/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: sense_handbrake.c 4280 2012-11-16 10:43:59Z florin.olariu $
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

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean hbrake_3DA_found = FALSE;
static Boolean hbrake_30B_found = FALSE;
static Boolean hbrake_30D_found = FALSE;
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

//--------------------------------------------------------------------------//

void sense_handbrake_3DA( Can_Msg *can_msg ) 
{    
    static Boolean last_status = FALSE;

    if(get_ign_status()&&get_rpm())
    {
        if(!hbrake_3DA_found && !hbrake_30B_found && !hbrake_30D_found)
        {
            if(last_status && !CAST_BOOL(DATA[1]&0x02))
            {
                hbrake_3DA_found = TRUE ; 
            }

            last_status = CAST_BOOL(DATA[1]&0x02);
        }
    }

    if(hbrake_3DA_found)
    {    
        sense_rs_handbrake_can( CAST_BOOL(DATA[1]&0x02) );
    }
}
	
//--------------------------------------------------------------------------//

void sense_handbrake_30B( Can_Msg *can_msg )
{
    static Boolean last_status = FALSE;

	/* PORTARE VW08EU 28.08.2026 - fara debounce local, fara scrierea iesirii.

	   DEBOUNCE: singurul filtru ramane cel din CORE (sense_rs_can_handbrake.c),
	   idb_debounce cu handbrake_debounce = 3 esantioane CONSECUTIVE. Contorul
	   local de 5 nu filtra nimic: nu reseta contorul opus, deci numara
	   cumulativ, nu consecutiv, si doar decima intrarea - transforma cele 3
	   esantioane ale CORE-ului in 15 cadre. Apelul la FIECARE cadru e forma
	   corecta: idb filtreaza pe esantioane, deci are sens doar alimentat la
	   cadenta magistralei.

	   IESIRE FIZICA: HBRAKE_SENSE_SET/CLR sint gpio_iface_handbrake_on()/off(),
	   adica exact ce cheama CORE pe frontul debounce-uit. Scrise si de aici,
	   direct din bitul brut, suprascriau decizia filtrata la fiecare cadru.
	   CORE detine iesirea - la fel ca in DB2, unde tot sense_rs_can_handbrake.c
	   o scria (liniile 74/78 acolo).

	   Aceeasi forma o are deja sense_handbrake_3DA, mai sus in fisier. */

	if ( hbrake_30B_found && !get_ign_status() ) 
	{
		/* Fara contact nu vin date de frina, deci raportam conservator catre RS
		   ca frina de mina E TRASA. CORE face acelasi lucru din proprie
		   initiativa - sense_rs_handbrake_can() primeste
		   (instant_level || !get_ign_status()) - deci apelul de aici e
		   redundant; il pastram ca sa ramina explicita intentia. */
		sense_rs_handbrake_can(TRUE);
		return;
	}

    if(get_ign_status())
    {
        /* Excludere mutuala: castiga primul ID care se manifesta dupa
           power on. Toate trei handlerele testeaza acelasi set complet. */
        if(!hbrake_3DA_found && !hbrake_30B_found && !hbrake_30D_found)
        {
            if(last_status && !CAST_BOOL(DATA[2]&0x80))
            {
                hbrake_30B_found = TRUE ; 
            }

            last_status = CAST_BOOL(DATA[2]&0x80);
        }
    }
    
    if(hbrake_30B_found) 
    {   
        sense_rs_handbrake_can( CAST_BOOL(DATA[2]&0x80) );
    }
}


void sense_handbrake_30D( Can_Msg *can_msg )
{
    static Boolean last_status = FALSE;

	/* PORTARE VW08EU 28.08.2026 - fara debounce local, fara scrierea iesirii.

	   DEBOUNCE: singurul filtru ramane cel din CORE (sense_rs_can_handbrake.c),
	   idb_debounce cu handbrake_debounce = 3 esantioane CONSECUTIVE. Contorul
	   local de 5 nu filtra nimic: nu reseta contorul opus, deci numara
	   cumulativ, nu consecutiv, si doar decima intrarea - transforma cele 3
	   esantioane ale CORE-ului in 15 cadre. Apelul la FIECARE cadru e forma
	   corecta: idb filtreaza pe esantioane, deci are sens doar alimentat la
	   cadenta magistralei.

	   IESIRE FIZICA: HBRAKE_SENSE_SET/CLR sint gpio_iface_handbrake_on()/off(),
	   adica exact ce cheama CORE pe frontul debounce-uit. Scrise si de aici,
	   direct din bitul brut, suprascriau decizia filtrata la fiecare cadru.
	   CORE detine iesirea - la fel ca in DB2, unde tot sense_rs_can_handbrake.c
	   o scria (liniile 74/78 acolo).

	   Aceeasi forma o are deja sense_handbrake_3DA, mai sus in fisier. */

	if ( hbrake_30D_found && !get_ign_status() ) 
	{
		/* Fara contact nu vin date de frina, deci raportam conservator catre RS
		   ca frina de mina E TRASA. CORE face acelasi lucru din proprie
		   initiativa - sense_rs_handbrake_can() primeste
		   (instant_level || !get_ign_status()) - deci apelul de aici e
		   redundant; il pastram ca sa ramina explicita intentia. */
		sense_rs_handbrake_can(TRUE);
		return;
	}

    if(get_ign_status())
    {
        /* Excludere mutuala: castiga primul ID care se manifesta dupa
           power on. Toate trei handlerele testeaza acelasi set complet. */
        if(!hbrake_3DA_found && !hbrake_30B_found && !hbrake_30D_found)
        {
            if(last_status && !CAST_BOOL(DATA[0]&0x04))
            {
                hbrake_30D_found = TRUE ; 
            }

            last_status = CAST_BOOL(DATA[0]&0x04);
        }
    }
    
    if(hbrake_30D_found) 
    {   
        sense_rs_handbrake_can( CAST_BOOL(DATA[0]&0x04) );
    }
}
	
//--------------------------------------------------------------------------//
