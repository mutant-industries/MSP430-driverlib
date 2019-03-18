/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Driverlib configuration file
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_CONFIG_H_
#define _DRIVER_CONFIG_H_


/**
 * override default behavior of driver disposal {@see disposable.h}
 */
//#define __RESOURCE_MANAGEMENT_ENABLE__

// -------------------------------------------------------------------------------------

/**
 * switch WDT to interval timer mode and enable SW timeout handler
 */
//#define __WDT_INTERVAL_TIMER_MODE__

/**
 * define all but __WDT_hold() macros empty
 */
//#define __WDT_DISABLE__

// -------------------------------------------------------------------------------------

/**
 * completely disable GIE manipulation in critical sections
 */
//#define __INTERRUPT_SUSPEND_DISABLE__

/**
 * adjust WDT clock source for interrupt_suspend_WDT_interval() {@see __WDT_ssel()}, default [SMCLK]
 */
//#define __INTERRUPT_SUSPEND_WDT_DEFAULT_SOURCE__      SMCLK

/**
 * count of general-purpose vector slots usually wrapped by drivers, default [8]
 */
//#define __VECTOR_SLOT_COUNT__     8

// -------------------------------------------------------------------------------------

/**
 * MSP430 1xx, 2xx, 3xx and 4xx have only one Timer_A and (except 3xx) one Timer_B. On these devices the driver
 * cannot support both timers, since both have different (max) number of CCRn channels and IV register behaves differently.
 * On these devices the support for both timers can be achieved by
 *  - either defining __TIMER_A_LEGACY_SUPPORT__ and manually editing _IV_register on Timer_driver_t for Timer_B after registered
 *  - or manually editing _IV_register on Timer_driver_t for Timer_A after registered
 * In the first case the possibility to use overflow handle on Timer_B is lost. In the second case the possibility
 * to use overflow handle on Timer_A is lost.
 *  - if used with PrimerOS kernel, one of those drivers with no support for overflow handle can be used to create
 * handles for kernel timing, since it does not need the overflow handle anyway.
 */
//#define __TIMER_A_LEGACY_SUPPORT__

/**
 * MSP430 1xx, 2xx, 3xx and 4xx port registers direct access support
 *  - on these devices there are no PxIV (interrupt vector generator) registers, therefore it is not supported
 * to register interrupt handlers via vector_register_handler(IO_pin_handle)
 */
//#define __IO_PORT_LEGACY_SUPPORT__

/**
 * count of DMA channels, MSP430FR5xx and 6xx define count in __MSP430_HAS_DMA__ already, default [6]
 *  - redefine to save some redundant pointers on DMA driver
 */
//#define __DMA_CONTROLLER_CHANNEL_COUNT__      6

/**
 * enable UART auto baudrate control manipulation via driver API
 */
//#define __UART_AUTO_BAUDRATE_CONTROL_ENABLE__

/**
 * enable UART IrDA control manipulation via driver API
 */
//#define __UART_IrDA_CONTROL_ENABLE__

// -------------------------------------------------------------------------------------

/**
 * disable possibility to set SP register 20-bit wide when TI compiler is used (with GCC this has no effect)
 */
//#define __STACK_POINTER_20_BIT_SUPPORT_DISABLE__

// -------------------------------------------------------------------------------------


#endif /* _DRIVER_CONFIG_H_ */
