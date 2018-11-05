/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Timer generic driver for MSP430
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_TIMER_H_
#define _DRIVER_TIMER_H_

#include <msp430.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/disposable.h>
#include <driver/vector.h>

// -------------------------------------------------------------------------------------

/**
 * Timer driver public api return codes
 */
#define TIMER_OK                        (0x00)
#define TIMER_UNSUPPORTED_OPERATION     (0x20)
#define TIMER_NO_HANDLE_AVAILABLE       (0x21)
#define TIMER_REFUSED                   (0x22)
#define TIMER_DRIVER_NOT_REGISTERED     (0x23)

// -------------------------------------------------------------------------------------

/**
 * TIMER_BASE(A, 2)    -> TA2_BASE
 * TIMER_A_BASE(1)     -> TA1_BASE
 * TIMER_B_BASE(0)     -> TB0_BASE
 */
#define TIMER_BASE(prefix, no)  __TIMER_BASE_EX__(prefix, no)
#define TIMER_A_BASE(no)        __TIMER_BASE_EX__(A, no)
#define TIMER_B_BASE(no)        __TIMER_BASE_EX__(B, no)
// concatenation of expanded parameters
#define __TIMER_BASE_EX__(prefix, no) T ## prefix ## no ## _BASE

/**
 * __attribute__((interrupt(TIMER_VECTOR(prefix, timer_no, vector_no))))
 *   - TIMER_VECTOR(A, 2, 0)                            -> TIMER2_A0_VECTOR
 *   - TIMER_VECTOR(B, 0, TIMER_VECTOR_MAIN)            -> TIMER0_B0_VECTOR
 *   - TIMER_VECTOR(B, TIMER_NO, TIMER_VECTOR_SHARED)   -> TIMER1_B1_VECTOR when TIMER_NO defined as '1'
 * __attribute__((interrupt(TIMER_A_VECTOR(timer_no, vector_no))))
 *   - TIMER_A_VECTOR(2, 0)                             -> TIMER2_A0_VECTOR
 *   - TIMER_A_VECTOR(0, TIMER_VECTOR_MAIN)             -> TIMER0_A0_VECTOR
 *   - TIMER_A_VECTOR(TIMER_NO, TIMER_VECTOR_SHARED)    -> TIMER1_A1_VECTOR when TIMER_NO defined as '1'
 * __attribute__((interrupt(TIMER_VECTOR(timer_no, vector_no))))
 *   - TIMER_B_VECTOR(2, 0)                             -> TIMER2_B0_VECTOR
 *   - TIMER_B_VECTOR(0, TIMER_VECTOR_MAIN)             -> TIMER0_B0_VECTOR
 *   - TIMER_B_VECTOR(TIMER_NO, TIMER_VECTOR_SHARED)    -> TIMER1_B1_VECTOR when TIMER_NO defined as '1'
 */
#define TIMER_VECTOR_MAIN       0
#define TIMER_VECTOR_SHARED     1

#define TIMER_VECTOR(prefix, timer_no, vector_no)   VECTOR(__TIMER_VECTOR_EX__(TIMER, timer_no), __TIMER_VECTOR_EX__(prefix, vector_no), _)
#define TIMER_A_VECTOR(timer_no, vector_no)         VECTOR(__TIMER_VECTOR_EX__(TIMER, timer_no), __TIMER_VECTOR_EX__(A, vector_no), _)
#define TIMER_B_VECTOR(timer_no, vector_no)         VECTOR(__TIMER_VECTOR_EX__(TIMER, timer_no), __TIMER_VECTOR_EX__(B, vector_no), _)
// concatenation of expanded parameters
#define __TIMER_VECTOR_EX__(prefix, no)                   prefix ## no

// -------------------------------------------------------------------------------------

typedef struct Timer_channel_handle Timer_channel_handle_t;

typedef enum {
    /**
     * Main CCR0 register handle
     *  - register_handler needs one vector slot
     *  - register_raw_handler on vector is allowed
     */
    MAIN = 1,
    /**
     * Shared CCRn register handle (n > 0)
     *  - register_handler uses one shared vector slot for all shared handles and overflow handle
     *  - register_raw_handler on vector is disabled
     *  - interrupts on shared slot have ~8 cycles delay compared to main handle
     */
    SHARED = 2,
    /**
     * Timer overflow handle
     *  - register_raw_handler on vector is disabled
     *  - no capture / compare api
     */
    OVERFLOW = 3

} Timer_handle_type;

/**
 * Physical HW timer control
 */
typedef struct Timer_driver {
    // enable dispose(Timer_driver_t *)
    Disposable_t _disposable;
    // base of HW timer registers, (address of corresponding TxCTL register)
    uint16_t _CTL_register;
    // CCR0 interrupt vector number
    uint8_t _main_vector_no;
    // CCR1 - CCRn, overflow vector number
    uint8_t _shared_vector_no;
    // interrupt vector register
    uint16_t _IV_register;
    // stored mode control
    uint8_t _mode;
    // amount of CCRn registers
    uint8_t _available_handles_cnt;

    // -------- state --------
    // main (CCR0) handle
    Timer_channel_handle_t *_CCR0_handle;
    // up to six (CCRn) handles sharing one interrupt vector
    Timer_channel_handle_t *_CCR1_handle;
    Timer_channel_handle_t *_CCR2_handle;
    Timer_channel_handle_t *_CCR3_handle;
    Timer_channel_handle_t *_CCR4_handle;
#ifndef __TIMER_A_LEGACY_SUPPORT__
    Timer_channel_handle_t *_CCR5_handle;
    Timer_channel_handle_t *_CCR6_handle;
#endif
    // overflow handle with shared interrupt vector
    Timer_channel_handle_t *_overflow_handle;
    // shared vector slot
    Vector_slot_t *_slot;
    // active registers count ~ remaining handles count = _available_handles_cnt - _active_handles_cnt
    uint8_t _active_handles_cnt;

    // -------- public api --------
    // register handle of given type with optional dispose hook
    uint8_t (*channel_handle_register)(struct Timer_driver *_this, Timer_channel_handle_t *handle,
          Timer_handle_type handle_type, dispose_function_t dispose_hook);

} Timer_driver_t;

/**
 * Single CCRn wrapper / overflow event wrapper
 */
struct Timer_channel_handle {
    // vector wrapper, enable dispose(Timer_channel_handle_t *)
    Vector_handle_t vector;
    // HW timer driver reference
    Timer_driver_t *_driver;
    // capture / compare control register
    uint16_t _CCTLn_register;
    // capture / compare register
    uint16_t _CCRn_register;

    // -------- state --------
    // vector interrupt service handler
    void (*_handler)(void *);
    // vector interrupt service handler parameter
    void *_handler_param;
    // function to be called on dispose
    dispose_function_t _dispose_hook;
    // backup of original Vector_handle_t.register_handler
    Vector_slot_t *(*_register_handler_parent)(Vector_handle_t *_this, void (*handler)(void *), void *handler_param);

    // -------- public api --------
    // enable interrupts triggered by handle-specific event, start timer driver if not started yet
    uint8_t (*start)(struct Timer_channel_handle *_this);
    // disable interrupts triggered by handle-specific event, stop timer driver if all handles are inactive to conserve power
    uint8_t (*stop)(struct Timer_channel_handle *_this);
    // reset content of counter register - possible only when _this is the only active handle or no handles are active
    uint8_t (*reset)(struct Timer_channel_handle *_this);
    // get content of counter register (voting system)
    uint8_t (*get_counter)(struct Timer_channel_handle *_this, uint16_t *);
    // ---- capture mode ----
    // params:
    //  - capture mode: CM__RISING | CM__FALLING | CM__BOTH
    //  - capture input select: CCIS__CCIA | CCIS__CCIB |d CCIS__GND | CCIS__VCC
    //  - capture input synchronize: SCS__SYNC | SCS__ASYNC
    void (*set_capture_mode)(struct Timer_channel_handle *_this, uint16_t, uint16_t, uint16_t);
    // read and reset COV
    bool (*is_capture_overflow_set)(struct Timer_channel_handle *_this);
    // ---- compare mode ----
    // params:
    //  - output mode:
    //      OUTMOD_0 (output only)
    //      OUTMOD_1 (set)
    //      OUTMOD_2 (PWM toggle/reset)
    //      OUTMOD_3 (PWM set/reset)
    //      OUTMOD_4 (toggle)
    //      OUTMOD_5 (Reset)
    //      OUTMOD_6 (PWM toggle/set)
    //      OUTMOD_7 (PWM reset/set)
    void (*set_compare_mode)(struct Timer_channel_handle *_this, uint16_t);
    // get content of CCRn register
    uint16_t (*get_capture_value)(struct Timer_channel_handle *_this);
    // set content of CCRn register
    void (*set_compare_value)(struct Timer_channel_handle *_this, uint16_t value);
    // handle type, read-only
    Timer_handle_type handle_type;
    // running + interrupt enabled state
    bool active;
    // handle operation state - does not apply for overflow handle
    bool capture_mode;
};

/**
 * Timer driver init configuration
 */
typedef struct Timer_config {
    // TASSEL__TACLK | TASSEL__ACLK | TASSEL__SMCLK | TASSEL__INCLK
    uint16_t clock_source;
    // ID__1  | ID__2 | ID__4 | ID__8
    uint8_t clock_source_divider;
    // TAIDEX__1 | TAIDEX__2 | TAIDEX__3 | TAIDEX__4 | TAIDEX__5 | TAIDEX__6 | TAIDEX__7 | TAIDEX__8
    uint16_t clock_source_divider_expansion;
    // MC__UP | MC__CONTINUOUS | MC__UPDOWN
    uint8_t mode;

} Timer_config_t;

// -------------------------------------------------------------------------------------

void timer_driver_register(Timer_driver_t *driver, Timer_config_t *config, uint16_t base,
            uint8_t main_vector_no, uint8_t shared_vector_no, uint8_t available_handles_cnt);


#endif /* _DRIVER_TIMER_H_ */
