/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Interrupts enable / disable, WDT-protected interrupt suspend (header only)
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_INTERRUPT_H_
#define _DRIVER_INTERRUPT_H_

#include <stdint.h>
#include <driver/config.h>
#include <driver/wdt.h>

// -------------------------------------------------------------------------------------

#ifndef __INTERRUPT_SUSPEND_WDT_DEFAULT_SOURCE__
#define __INTERRUPT_SUSPEND_WDT_DEFAULT_SOURCE__   SMCLK
#endif

// -------------------------------------------------------------------------------------

/**
 * Global interrupt enable / disable
 */
#define interrupt_enable() \
    __enable_interrupt();

#define interrupt_disable() \
    __disable_interrupt();

// -------------------------------------------------------------------------------------

/**
 * Save status register, disable interrupt,
 * save current WDT state, clear and set WDT for specified clock cycle count, set default WDT clock source
 */
#define interrupt_suspend_WDT_interval(clock_cycle_cnt) \
    interrupt_suspend(); \
    WDT_backup_clr_ssel_interval(__INTERRUPT_SUSPEND_WDT_DEFAULT_SOURCE__, clock_cycle_cnt);

/**
 * Save status register, disable interrupt,
 * save current WDT state, clear and set WDT for specified clock cycle count, set WDT clock source
 */
#define interrupt_suspend_WDT_ssel_interval(source, clock_cycle_cnt) \
    interrupt_suspend(); \
    WDT_backup_clr_ssel_interval(source, clock_cycle_cnt);

/**
 * Recover saved state of status register, recover saved state of WDT, clear WDT
 */
#define interrupt_restore_WDT() \
    WDT_clr_restore(); \
    interrupt_restore();

// -------------------------------------------------------------------------------------

#ifndef __INTERRUPT_SUSPEND_DISABLE__

/**
 * Save status register and disable interrupt. The _SR_ variable has to be local to allow nesting.
 */
#define interrupt_suspend() \
    uint16_t _SR_ = __get_SR_register(); \
    interrupt_disable();

/**
 * Recover saved state of status register.
 */
#define interrupt_restore() \
    interrupt_restore_with(0);

/**
 * Recover saved state of status register, set additional status register bits.
 */
#define interrupt_restore_with(bits) \
    __set_interrupt_state(_SR_ | (bits));

#else
#define interrupt_suspend()
#define interrupt_restore_with(bits)
#endif


#endif /* _DRIVER_INTERRUPT_H_ */
