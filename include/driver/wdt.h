/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Watchdog start / stop / pause macros (header only)
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_WDT_H_
#define _DRIVER_WDT_H_

#include <driver/config.h>

// -------------------------------------------------------------------------------------

#include <msp430.h>
#include <stdint.h>

// -------------------------------------------------------------------------------------

#ifndef __WDT_INTERVAL_TIMER_MODE__
#define _TMSEL_   (0x0000)
#else
#define _TMSEL_   WDTTMSEL
#endif

// -------------------------------------------------------------------------------------

/**
 * Stop WDT
 */
#define WDT_hold() \
    WDTCTL = WDTPW | WDTHOLD | (WDTCTL & (WDTSSEL | WDTTMSEL | WDTIS));

/**
 * Start / continue
 */
#define WDT_start() \
    __WDT_set__(0, WDTCTL & (WDTSSEL | WDTTMSEL | WDTIS), 0, _TMSEL_);

/**
 * Clear WDT internal counter
 */
#define WDT_clr() \
    __WDT_set__(WDTCNTCL, WDTCTL & (WDTHOLD | WDTSSEL | WDTTMSEL | WDTIS), 0, _TMSEL_);

/**
 * WDT timer clock source select
 *  - SMCLK
 *  - ACLK
 *  - VLOCLK
 *  - BCLK (or something else, see device-specific datasheet)
 */
#define WDT_ssel(source) \
    __WDT_set__(__WDT_param_expand__(WDTSSEL__, source), WDTCTL & (WDTHOLD | WDTIS), 0, _TMSEL_);

/**
 * Clear and set WDT for specified clock cycle count
 *  - 64        2^6 (64) clock cycles, 64 us at 1MHz
 *  - 512
 *  - 8192
 *  - 32K
 *  - 512K      2^19 (512K) clock cycles, 500 ms at 1MHz
 *  - 8192K     2^23 (8192K) clock cycles, 8 s at 1MHz
 *  - 128M
 *  - 2G
 */
#define WDT_clr_interval(clock_cycle_cnt) \
    __WDT_set__(__WDT_param_expand__(WDTIS__, clock_cycle_cnt), WDTCTL & WDTSSEL, WDTCNTCL, _TMSEL_);

/**
 * Clear and set WDT for specified clock cycle count, set clock source
 */
#define WDT_clr_ssel_interval(source, clock_cycle_cnt) \
    __WDT_set__(__WDT_param_expand__(WDTIS__, clock_cycle_cnt), __WDT_param_expand__(WDTSSEL__, source), WDTCNTCL, _TMSEL_);

/**
 * Expand and concatenate macro params
 */
#define __WDT_param_expand__(prefix, param) \
    prefix ## param

// -------------------------------------------------------------------------------------

#ifndef __WDT_DISABLE__

/**
 * Save current WDT state, stop WDT
 */
#define WDT_backup_hold() \
    __WDT_backup__(); \
    WDT_hold();

/**
 * Save current WDT state, clear and set WDT for specified clock cycle count
 */
#define WDT_backup_clr_interval(clock_cycle_cnt) \
    __WDT_backup__(); \
    __WDT_set__(__WDT_param_expand__(WDTIS__, clock_cycle_cnt), _WDT_STATE_ & WDTSSEL, WDTCNTCL, _TMSEL_);

/**
 * Save current WDT state, clear and set WDT for specified clock cycle count, set clock source
 */
#define WDT_backup_clr_ssel_interval(source, clock_cycle_cnt) \
    __WDT_backup__(); \
    WDT_clr_ssel_interval(source, clock_cycle_cnt);

/**
 * Recover saved state of WDT
 */
#define WDT_restore() \
    WDTCTL = WDTPW | _WDT_STATE_;

/**
 * Recover saved state of WDT, clear WDT
 */
#define WDT_clr_restore() \
    WDTCTL = WDTPW | WDTCNTCL | _WDT_STATE_;

// ---------- private --------------------

/**
 * Save current WDT state - the _WDTIS_ variable has to be local to allow nesting.
 */
#define __WDT_backup__() \
    uint8_t _WDT_STATE_ = WDTCTL;

/**
 * Write parameters to WDTCTL
 */
#define __WDT_set__(param_1, param_2, param_3, mode_select) \
    WDTCTL = WDTPW | (param_1) | (param_2) | (param_3) | (mode_select);

#else
#define WDT_backup_hold() WDT_hold()
#define WDT_backup_clr_interval(...)
#define WDT_backup_clr_ssel_interval(...)
#define WDT_restore()
#define WDT_clr_restore()
#define __WDT_backup__()
#define __WDT_set__(...)
#endif


#endif /* _DRIVER_WDT_H_ */
