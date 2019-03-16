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

#define _timer_driver_(_driver)                 ((Timer_driver_t *) (_driver))
#define _timer_channel_handle_(_handle)         ((Timer_channel_handle_t *) (_handle))

/**
 * Timer driver public API access
 */
#define timer_driver_channel_register(_driver, _handle, _handle_type, _dispose_hook)        \
        (_timer_driver_(_driver)->channel_handle_register(_timer_driver_(_driver), _timer_channel_handle_(_handle), _handle_type, _dispose_hook))

#define timer_channel_start(_handle)                                                        \
        (_timer_channel_handle_(_handle)->start(_timer_channel_handle_(_handle)))
#define timer_channel_stop(_handle)                                                         \
        (_timer_channel_handle_(_handle)->stop(_timer_channel_handle_(_handle)))
#define timer_channel_reset(_handle)                                                        \
        (_timer_channel_handle_(_handle)->reset(_timer_channel_handle_(_handle)))
#define timer_channel_get_counter(_handle, _target)                                         \
        (_timer_channel_handle_(_handle)->get_counter(_timer_channel_handle_(_handle), (uint16_t *) (_target)))
#define timer_channel_set_capture_mode(_handle, _mode, _input_select, _input_synchronize)   \
        (_timer_channel_handle_(_handle)->set_capture_mode(_timer_channel_handle_(_handle), _mode, _input_select, _input_synchronize))
#define timer_channel_is_capture_overflow_set(_handle)                                      \
        (_timer_channel_handle_(_handle)->is_capture_overflow_set(_timer_channel_handle_(_handle)))
#define timer_channel_set_compare_mode(_handle, _output_mode)                               \
        (_timer_channel_handle_(_handle)->set_compare_mode(_timer_channel_handle_(_handle), _output_mode))
#define timer_channel_get_capture_value(_handle)                                            \
        (_timer_channel_handle_(_handle)->get_capture_value(_timer_channel_handle_(_handle)))
#define timer_channel_set_compare_value(_handle, _value)                                    \
        (_timer_channel_handle_(_handle)->set_compare_value(_timer_channel_handle_(_handle), (uint16_t) (_value)))
#define timer_channel_is_active(_handle)                                                    \
        _timer_channel_handle_(_handle)->active

/**
 * Timer driver public API return codes
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

typedef struct Timer_driver Timer_driver_t;
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
     *  - no capture / compare API
     */
    OVERFLOW = 3

} Timer_handle_type;

/**
 * Physical HW timer control
 */
struct Timer_driver {
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

    // -------- public --------
    // register handle of given type with optional dispose hook
    uint8_t (*channel_handle_register)(Timer_driver_t *_this, Timer_channel_handle_t *handle,
          Timer_handle_type handle_type, dispose_function_t dispose_hook);

};

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
    vector_slot_handler_t _handler;
    // vector interrupt service handler arguments
    void *_handler_arg_1;
    void *_handler_arg_2;
    // function to be called on dispose
    dispose_function_t _dispose_hook;
    // backup of original Vector_handle_t.register_handler
    Vector_slot_t *(*_register_handler_parent)(Vector_handle_t *_this, vector_slot_handler_t handler, void *arg_1, void *arg_2);

    // -------- public --------
    // enable interrupts triggered by handle-specific event, start timer driver if not started yet
    uint8_t (*start)(Timer_channel_handle_t *_this);
    // disable interrupts triggered by handle-specific event, stop timer driver if all handles are inactive to conserve power
    uint8_t (*stop)(Timer_channel_handle_t *_this);
    // reset content of counter register - possible only when _this is the only active handle or no handles are active
    uint8_t (*reset)(Timer_channel_handle_t *_this);
    // get content of counter register (voting system)
    uint8_t (*get_counter)(Timer_channel_handle_t *_this, uint16_t *target);
    // ---- capture mode ----
    // params:
    //  - capture mode: CM__RISING | CM__FALLING | CM__BOTH
    //  - capture input select: CCIS__CCIA | CCIS__CCIB |d CCIS__GND | CCIS__VCC
    //  - capture input synchronize: SCS__SYNC | SCS__ASYNC
    void (*set_capture_mode)(Timer_channel_handle_t *_this, uint16_t mode, uint16_t input_select, uint16_t input_synchronize);
    // read and reset COV
    bool (*is_capture_overflow_set)(Timer_channel_handle_t *_this);
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
    void (*set_compare_mode)(Timer_channel_handle_t *_this, uint16_t output_mode);
    // get content of CCRn register
    uint16_t (*get_capture_value)(Timer_channel_handle_t *_this);
    // set content of CCRn register
    void (*set_compare_value)(Timer_channel_handle_t *_this, uint16_t value);
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
