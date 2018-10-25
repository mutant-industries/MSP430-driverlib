/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Interrupt vector wrapper
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _SYS_DRIVER_VECTOR_H_
#define _SYS_DRIVER_VECTOR_H_

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/disposable.h>

// -------------------------------------------------------------------------------------

/**
 * Get vector address by vector number (assume SYSRIVECT is not set).
 */
#define __vector_ptr(no) \
        ((uint16_t *) (0xFFFE - ((RESET_VECTOR - (no)) * 2)))

/**
 * Get vector content by vector number.
 */
#define __vector(no) \
        (*__vector_ptr(no))

/**
 * Set interrupt vector handler.
 */
#define __vector_set(no, function) \
        __vector(no) = (uint16_t) function

/**
 * #Pragma vector=VECTOR(DEVICE[, DEVICE_ID[, ID_SEPARATOR]])
 *   - VECTOR(AES256)                   -> AES256_VECTOR
 *   - VECTOR(PORT, BUTTON_PORT)        -> PORT5_VECTOR when BUTTON_PORT defined as '5'
 *   - VECTOR(EUSCI, IF_MAIN, _)        -> EUSCI_A0_VECTOR when IF_MAIN defined as 'A0'
 */
#define __VOID__
// variable-args macro  - https://stackoverflow.com/a/11763277
#define _VECTOR_GET_MACRO(_1,_2,_3,NAME,...) NAME
#define VECTOR(...) _VECTOR_GET_MACRO(__VA_ARGS__, _VECTOR_3, _VECTOR_2, _VECTOR_1)(__VA_ARGS__)

#define _VECTOR_1(DEV)                              _VECTOR_EX_(DEV, __VOID__, __VOID__)
#define _VECTOR_2(DEV, DEV_ID)                      _VECTOR_EX_(DEV, DEV_ID, __VOID__)
#define _VECTOR_3(DEV, DEV_ID, ID_SEPARATOR)        _VECTOR_EX_(DEV, DEV_ID, ID_SEPARATOR)
// concatenation of expanded parameters
#define _VECTOR_EX_(DEV, DEV_ID, ID_SEPARATOR)      _VECTOR_EX_2(DEV, DEV_ID, ID_SEPARATOR)
#define _VECTOR_EX_2(DEV, DEV_ID, ID_SEPARATOR)     DEV ## ID_SEPARATOR ## DEV_ID ## _VECTOR

// -------------------------------------------------------------------------------------

/**
 * Single vector handle structure.
 */
typedef struct Vector_handle {
    // enable dispose(Vector_handle_t *)
    Disposable_t _disposable;
    // trigger interrupt, so that registered handler shall be executed
    void (*trigger)(struct Vector_handle *_this);
    // clearing (or reading IV reg) only required when flags are not cleared by HW
    void (*clear_interrupt_flag)(struct Vector_handle *_this);
    // set / reset interrupt enable flag
    void (*set_enabled)(struct Vector_handle *_this, bool enabled);
    // register interrupt service routine for this vector, if reversible set, the original handler shall be restored on dispose
    void (*register_handler)(struct Vector_handle *_this, void (*handler)(void), bool reversible);
    // interrupt enable state
    bool enabled;
    // address of interrupt vector
    uint16_t _vector_no;
    // original vector handler, restored on dispose
    uint16_t _vector_original_content;
    // interrupt enable register and mask
    volatile uint16_t _IE_register;
    uint16_t _IE_mask;
    // interrupt flag register and mask
    volatile uint16_t _IFG_register;
    uint16_t _IFG_mask;
    // function to be called on dispose
    dispose_function_t _dispose_hook;

} Vector_handle_t;

// -------------------------------------------------------------------------------------

/**
 * Default structure 'constructor'
 */
void vector_handle_register(Vector_handle_t *handle, dispose_function_t dispose_hook,
        uint16_t vector_no, uint16_t IE_register, uint16_t IE_mask, uint16_t IFG_register, uint16_t IFG_mask);


#endif /* _SYS_DRIVER_VECTOR_H_ */
