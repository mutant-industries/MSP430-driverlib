/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Critical section enter / exit, WDT-protected critical section (header only)
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_CRITICAL_H_
#define _DRIVER_CRITICAL_H_

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
#define critical_section_WDT_interval_enter(clock_cycle_cnt) \
    critical_section_enter(); \
    WDT_backup_clr_ssel_interval(__CRITICAL_SECTION_WDT_DEFAULT_SOURCE__, clock_cycle_cnt);

/**
 * Save status register, disable interrupt,
 * save current WDT state, clear and set WDT for specified clock cycle count, set WDT clock source
 */
#define critical_section_WDT_ssel_interval_enter(source, clock_cycle_cnt) \
    critical_section_enter(); \
    WDT_backup_clr_ssel_interval(source, clock_cycle_cnt);

/**
 * Recover saved state of status register, recover saved state of WDT, clear WDT
 */
#define critical_section_WDT_exit() \
    WDT_clr_restore(); \
    critical_section_exit();

// -------------------------------------------------------------------------------------

#ifndef __CRITICAL_SECTION_DISABLE__

/**
 * Save status register and disable interrupt. The _SR_ variable has to be local to allow nesting.
 */
#define critical_section_enter() \
    uint16_t _SR_ = __get_SR_register(); \
    __disable_interrupt();

/**
 * Recover saved state of status register.
 */
#define critical_section_exit() \
    __set_interrupt_state(_SR_);

#else
#define critical_section_enter()
#define critical_section_exit()
#endif


#endif /* _DRIVER_CRITICAL_H_ */
