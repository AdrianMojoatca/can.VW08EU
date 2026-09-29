/*<#======================================================================#>*/
/*                          FILE HEADER GOES HERE                           */
/*<#======================================================================#>*/

/*==========================================================================*/
// $Id: task_main.c 33258 2015-08-19 12:46:35Z martin.bouchard $
/*==========================================================================*/

/*==========================================================================*/
/*                             I N C L U D E S                              */
/*==========================================================================*/

#include "os.h"
#include "core.h"
#include "wake.h"
#include "dbg.h"
#include "fiq.h"
#include "irq_user.h"

#if defined(FW_DB3_BUILD)
#include "fw.h"
#endif

#include "core_contract_db3_gpio_output.h"
#include "core_contract_db3_misc_gpio.h"
#include "core_contract_db3_boot_handshake.h"
#include "core_contract_db3_fw_handoff.h"
#include "core_contract_db3_fw_descriptor.h"
#include "core_contract_db3_fw_api.h"

#ifdef OUT_1
#undef OUT_1
#endif

/*==========================================================================*/
/*      D E F I N E S  -  E N U M E R A T I O N S  -  T Y P E D E F S       */
/*==========================================================================*/

/*==========================================================================*/
/*                  F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/

static int get_cmd (void) ;
void gpio_iface_doors_open (void);
void gpio_iface_doors_closed (void);

/*==========================================================================*/
/*                            V A R I A B L E S                             */
/*==========================================================================*/

Tsk tsk_main ;

static UInt8 * const RAM_MEM = (UInt8 *) 0x7FE00000 ;

/*==========================================================================*/
/*                 F U N C T I O N   D E F I N I T I O N S                  */
/*==========================================================================*/

void task_main (void * tsk_arg)
{   
	UInt8 i;
#if defined(CORE_DB3_BUILD)
	const core_db3_fw_descriptor_t* fw_desc = (const core_db3_fw_descriptor_t*)(CORE_DB3_FLASH_FW_START_ADDR);
	uint32_t handoff_valid;
#endif

	#ifdef DEV_LPC_176x
	
	// Timer 1
		PCONP |= (1u << 2) ;		// POWER the TIMER

	PCLKSEL0 &= ~(3u << 4) ;
	PCLKSEL0 |=  (1u << 4) ;    // Divide CPU clock by 1

	
	// Timer 2
	PCONP |= (1u << 22) ;		// POWER the TIMER

	PCLKSEL1 &= ~(3u << 12) ;
	PCLKSEL1 |=  (1u << 12) ;    // Divide CPU clock by 1
	
	
	// Timer 3
		PCONP |= (1u << 23) ;		// POWER the TIMER

	PCLKSEL1 &= ~(3u << 14) ;
	PCLKSEL1 |=  (1u << 14) ;    // Divide CPU clock by 1

	core_contract_db3_dac_power_set(FALSE);

	#endif

#if defined(FW_DB3_BUILD)
	fw_identity_init() ;	// Vehicle-specific contract identity init (must run before core_init)
#endif

#if defined(CORE_DB3_BUILD)
	core_contract_db3_gpio_led_red_off();
	core_contract_db3_gpio_led_green_off();

	if (core_contract_db3_fw_descriptor_is_valid(fw_desc))
	{
		/* ===== single-boot (thin-FW): CORE stays LIVE and invokes FW via descriptor ===== */
		/* fw_identity_init fills CORE's SINGLE config instance (via API-table DATA
		   pointers) BEFORE core_init consumes it; then CORE invokes the FW. */
		fw_desc->fw_ram_init_fn();
		fw_desc->fw_identity_init_fn();
		/* v1.1: FW owns the runtime dbg config iff its ABI minor >= 10 (else CORE self-inits). */
		g_core_fw_owns_dbg = (((fw_desc->abi_version & 0xFFFFu) >= 10u) ? TRUE : FALSE);
		core_init();
		core_contract_db3_gpio_led_green_on();
		core_contract_db3_gpio_led_red_off();
		fw_desc->fw_init_fn();
	}
	else
	{
		/* ===== legacy re-boot handoff path (old monolithic FW, no FWD3 descriptor) ===== */
		core_init();

		handoff_valid = core_contract_db3_fw_handoff_is_valid();

		if (handoff_valid != 0U)
		{
			core_contract_db3_gpio_led_green_on();
			core_contract_db3_gpio_led_red_off();
			core_contract_db3_fw_handoff_jump();

			/* jump should not return; if it does, flag it on both LEDs */
			core_contract_db3_gpio_led_red_on();
			core_contract_db3_gpio_led_green_on();
		}
		else
		{
			core_contract_db3_gpio_led_red_on();
			core_contract_db3_gpio_led_green_off();
		}
	}

	while (1)
	{
		TRACE_RX();
		os_tsk_wait(1000);
	}
#endif

	core_init 	() ;		// Platform core init


#if defined(FW_DB3_BUILD)
	{
		const core_db3_api_table_t* core_api_table = core_contract_db3_boot_api_table_get();
		core_db3_boot_handshake_result_t abi_handshake_result;

	abi_handshake_result = core_contract_db3_boot_handshake_validate(
		CORE_DB3_ABI_VERSION_MINOR,
		CORE_DB3_CAPABILITY_DEFAULT_MASK,
		core_api_table);

	if (abi_handshake_result != CORE_DB3_BOOT_HANDSHAKE_OK)
	{
		core_contract_db3_gpio_led_red_on();
		while (1)
		{
			os_tsk_wait(1000);
		}
	}
    }
#endif

	/* a small test for the IO's */
	for ( i = 0; i < 3; i++ )
	{
		/* asa nu - GPIO_IFACE_OUTPUT_NEG_OUT1 poate fi inlocuit cu alt OUT de catre programatorul "outside core" */

		/*core_contract_db3_gpio_output_on(GPIO_IFACE_CFG_DOORS_OUTPUT);	 // asa merge
		os_tsk_wait(500);
		core_contract_db3_gpio_output_off(GPIO_IFACE_CFG_DOORS_OUTPUT);
		os_tsk_wait(500);*/
		
		/* asa da - programatorul "outside core" nu poate schimba out-ul pt. doors */
		 
		#if defined(FW_DB3_BUILD)
		core_contract_db3_fw_gpio_doors_open();
		#else
		gpio_iface_doors_open();
		#endif
		os_tsk_wait(500);
		#if defined(FW_DB3_BUILD)
		core_contract_db3_fw_gpio_doors_closed();
		#else
		gpio_iface_doors_closed();
		#endif
		os_tsk_wait(500);
	}
	/* end small test */

#ifdef DEV_LPC_176x
  	fiq_init();
	irq_user_init();
#endif

#if defined(FW_DB3_BUILD)
	fw_init 	() ;		// Platform & fw specific init
#endif

	os_tsk_wait (3000) ; 	// wait for LED to turn OFF before going to Sleep
	
    #if (DEBUG_TEXT == 1) || USE_TRACE == 1
    #warning "Fw Debug Enable"
    
    while( 1 )
      {
        TRACE_RX();
      }
    
    #else
      wake_leave () ;
    #endif	   	
}	

