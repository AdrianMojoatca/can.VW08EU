/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: func_arm.c 2977 2011-07-18 16:29:02Z florin.olariu $
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
static Can_Sync_Flags dl_type_detect( Can_Msg *can_msg );
/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/
static Boolean do_lock   = FALSE;
static Boolean do_unlock = FALSE; 
/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/

void func_oem_lock_start( void  )
{
     can_sync_loop(FUNC_CAN_DEV ,(Can_Sync_Func)dl_type_detect,(Can_Sync_Arg)NULL,0x3D0 ,0 );
     
     /* v23 VW08EU (25.09.2026): calea CAN de incuiere ramine pe DB3 si e scoasa
        numai pe DS4. Fisierul e in VEHICLE_SPECIFIC, deci comun ambelor platforme.

        DE CE difera:
          DS4 (933) - modulul are iesire de Lock (Output 3, 22-pin/3) si de cind cu
            v20 ea e comandata din dei_system_oem_lock. Cadrele CAN devin redundante,
            iar pe masina se vedeau ca zgomot: func_lock_start() cu contactul PORNIT
            transmite id 0x000 (opt octeti de zero, trezire) si apoi id 0x3D0 cu
            40 00 00 C0 80 00 00 00. Adrian a cerut sa dispara.
          DB3 (403) - modulul NU are iesire de Lock. In ghidul lui, 10-pin/1 si /2
            sint "Lock Input" / "Unlock Input", adica modulul PRIMESTE comanda de la
            remote starter-ul extern si o executa pe magistrala. Acolo cadrele CAN
            nu sint reminiscenta, sint chiar mecanismul: fara ele, apasarea
            telecomenzii OEM in Remote Start nu ar mai incuia nimic.

        Verificat in techsheet si in ghidurile generate, nu presupus. */
#ifndef PLATFORM_933
     if( get_rs_gwr() )
     {
         do_lock = TRUE;
         
		 os_tsk_wait(700);
         func_lock_start();
     }
#endif
}

/*--------------------------------------------------------------------------*/

Boolean func_oem_lock_exec( void )
{
    return do_lock ? func_lock_exec():FALSE ; 
}

/*--------------------------------------------------------------------------*/

void func_oem_lock_stop( void )
{
    can_sync_remove(FUNC_CAN_DEV , (Can_Sync_Func)dl_type_detect );    
    
    if( do_lock )
    {
        func_lock_stop();
        
        do_lock = FALSE;
    }
}

/*--------------------------------------------------------------------------*/

Can_Sync_Flags dl_type_detect( Can_Msg *can_msg )
{            
    ATOMIC(func_dl_type = (can_msg->data._8_[0]&0x06));
            
    return  CAN_SYNC_NO_ACTION ; 
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

void func_oem_unlock_start( void  )
{
    /* v23 VW08EU (25.09.2026): la fel ca la lock, calea CAN de descuiere ramine pe
       DB3 si e scoasa numai pe DS4. Motivatia completa e in comentariul din
       func_oem_lock_start().

       Pe DS4 dispare si cadrul 0x000: func_unlock_all_start() cheama
       func_can_wake_enter(), care porneste can_per_loop( can_wake_tx , ... , 20 ),
       iar can_wake_tx trimite PERIODIC func_can_table[FUNC_CAN_WAKE] = id 0x000 cu
       opt octeti de zero. Dispare si retransmiterea lui 0x3D0: unlock_all_3D0()
       facea data[0] |= 0xA0 si intorcea CAN_SYNC_TX, de trei ori.

       GEAMANUL RAMAS, pe amindoua platformele: func_oem_trunk_start() de mai jos
       cheama tot func_unlock_all_start(), tot pazit de get_rs_gwr(), deci butonul
       de portbagaj de pe telecomanda OEM produce inca 0x000 in Remote Start si pe
       DS4. Lasat intentionat - comportamentul lui pe masina nu a fost verificat. */
#ifndef PLATFORM_933
    if( get_rs_gwr() )
    {
        do_unlock = TRUE;
         
        func_unlock_all_start();
    }
#endif
}

/*--------------------------------------------------------------------------*/

Boolean func_oem_unlock_exec( void )
{
    return do_unlock ? func_unlock_all_exec():FALSE ; 
}

/*--------------------------------------------------------------------------*/

void func_oem_unlock_stop( void )
{
    if( do_unlock )
    {
        func_unlock_all_stop();
        
        do_unlock = FALSE;
    }
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

void func_oem_trunk_start( void  )
{
    if( get_rs_gwr() )
    {
		
		do_unlock = TRUE;
         
        func_unlock_all_start();
    }
}

/*--------------------------------------------------------------------------*/

Boolean func_oem_trunk_exec( void )
{
	return do_unlock ? func_unlock_all_exec():FALSE ;	 
}

/*--------------------------------------------------------------------------*/

void func_oem_trunk_stop( void )
{
    if( do_unlock )
    {
        func_unlock_all_stop();
        
        do_unlock = FALSE;
    }
}

/*--------------------------------------------------------------------------*/


