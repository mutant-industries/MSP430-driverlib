/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Critical section enter / exit, WDT-protected critical section (header only)
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _SYS_DRIVER_CRITICAL_H_
#define _SYS_DRIVER_CRITICAL_H_

#include <stdint.h>
#include <driver/config.h>
#include <driver/wdt.h>

// -------------------------------------------------------------------------------------

#ifndef __CRITICAL_SECTION_WDT_DEFAULT_SOURCE__
#define __CRITICAL_SECTION_WDT_DEFAULT_SOURCE__   SMCLK
#endif

// -------------------------------------------------------------------------------------

/**
 * Save status register, disable interrupt,
 * save current WDT state, clear and set WDT for specified clock cycle count, set default WDT clock source
 */
#define __critical_section_WDT_interval_enter(clock_cycle_count) \
    __critical_section_enter(); \
    __WDT_backup_clr_ssel_interval(__CRITICAL_SECTION_WDT_DEFAULT_SOURCE__, clock_cycle_count);

/**
 * Save status register, disable interrupt,
 * save current WDT state, clear and set WDT for specified clock cycle count, set WDT clock source
 */
#define __critical_section_WDT_ssel_interval_enter(source, clock_cycle_count) \
    __critical_section_enter(); \
    __WDT_backup_clr_ssel_interval(source, clock_cycle_count);

/**
 * Recover saved state of status register, recover saved state of WDT, clear WDT
 */
#define __critical_section_WDT_exit() \
    __WDT_clr_restore(); \
    __critical_section_exit();

// -------------------------------------------------------------------------------------

#ifndef __CRITICAL_SECTION_DISABLE__

/**
 * Save status register and disable interrupt. The _SR_ variable has to be local to allow nesting.
 */
#define __critical_section_enter() \
    uint16_t _SR_ = __get_SR_register(); \
    __disable_interrupt();

/**
 * Recover saved state of status register.
 */
#define __critical_section_exit() \
    __set_interrupt_state(_SR_);

#else
#define __critical_section_enter()
#define __critical_section_exit()
#endif


#endif /* _SYS_DRIVER_CRITICAL_H_ */
