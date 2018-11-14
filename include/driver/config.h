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
//#define __CRITICAL_SECTION_DISABLE__

/**
 * adjust WDT clock source for __critical_section_WDT_interval_enter() {@see __WDT_ssel()}, default [SMCLK]
 */
//#define __CRITICAL_SECTION_WDT_DEFAULT_SOURCE__     SMCLK

/**
 * count of general-purpose vector slots usually wrapped by drivers, default [8]
 */
//#define __VECTOR_SLOT_COUNT__     8

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
 * disable possibility to set SP register 20-bit wide when TI compiler is used (with GCC this has no effect)
 */
//#define __STACK_POINTER_20_BIT_SUPPORT_DISABLE__

// -------------------------------------------------------------------------------------


#endif /* _DRIVER_CONFIG_H_ */
