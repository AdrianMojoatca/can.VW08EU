/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: bus_status_notify_input_keyless_trunk.c 33257 2015-08-19 12:45:49Z martin.bouchard $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/

#include "bus_status_private.h"

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

void bus_status_notify_input_keyless_trunk (void *arg)
{
	/* VW08EU 15.09.2026: pina acum de aici nu pleca NIMIC pe D2D - fratii acestui
	   handler (lock, unlock, hood, brake, handbrake) cheama toti d2d_tx, doar
	   trunk-ul ramasese comentat. Simptom raportat de tester: apasarea butonului
	   OEM de portbagaj se vedea pe trace, dar byte-ul nu ajungea la starter.

	   Nu e o inventie: exact asta scrie in CORE-ul de DS4, unde functia a fost
	   deja evoluata de Falcon (cele doua comentarii de mai jos sint ale lor).
	   DB3 e doar o poza mai veche a aceluiasi fisier. Singura diferenta fata de
	   DS4: bus_status_arm e variabila aici si functie acolo.

	   DB2 trimitea doar SET_KEYLESS_DISARM (0xDD). 0xDF e cel pe care il astepta
	   starterul, confirmat la banc pe DB3 in 1.00.02. Pe ruta asta se acopera si
	   varianta DATA[7]==0x26, fiindca INPUT_OEM_TRUNK se ridica pentru amindoua. */
	/* 15.09.2026: DISARM scos deliberat. In mod normal la trunk release trebuie
	   sa plece DOAR trunk release; unele masini fac ele disarm cind se deschide
	   portbagajul, altele nu. Se probeaza pe masina si se decide dupa. Ca sa-l
	   pui la loc, decomenteaza linia de mai jos - nimic altceva. */
	//d2d_tx(SET_KEYLESS_DISARM);  // legacy lie being told for SmartStart maybe ?
	d2d_tx(SET_KEYLESS_TRUNK);   // at least tell the truth for those who care
	bus_status_arm = 0;          // nu dezarmeaza nimic, doar curata flag-ul de tamper
}
