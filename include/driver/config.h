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
 * Count of general-purpose vector slots usually wrapped by drivers, default [8]
 */
//#define __VECTOR_SLOT_COUNT__     8

// -------------------------------------------------------------------------------------


#endif /* _DRIVER_CONFIG_H_ */
