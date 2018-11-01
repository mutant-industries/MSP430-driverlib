/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Interrupt vector wrapper
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_VECTOR_H_
#define _DRIVER_VECTOR_H_

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/disposable.h>

// -------------------------------------------------------------------------------------

/**
 * Vector handle public api return codes
 */
#define VECTOR_OK                   (0x00)
#define VECTOR_IFG_REG_NOT_SET      (0x10)
#define VECTOR_IFG_MASK_NOT_SET     (0x11)
#define VECTOR_IE_REG_NOT_SET       (0x12)
#define VECTOR_IE_MASK_NOT_SET      (0x13)

// -------------------------------------------------------------------------------------

/**
 * MSP430-gcc defines RESET_VECTOR number as ("reset"), therefore to determine reset vector number,
 * the 'preceding vector + 1' is used. Preceding is either SYSNMI_VECTOR or NMI_VECTOR on all devices.
 */
#ifdef  __MSP430__
// msp430-gcc
#ifdef SYSNMI_VECTOR
#define RESET_VECTOR_NO             (SYSNMI_VECTOR + 1)
#else
#define RESET_VECTOR_NO             (NMI_VECTOR + 1)
#endif
#else
// TI compiler
#define RESET_VECTOR_NO             RESET_VECTOR
#endif

/**
 * Get vector address by vector number (assume SYSRIVECT is not set).
 */
#define __vector_ptr(no) \
        ((uint16_t *) (0xFFFE - ((RESET_VECTOR_NO - (no)) * 2)))

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
 * __attribute__((interrupt(VECTOR(DEVICE[, DEVICE_ID[, ID_SEPARATOR]]))))
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
 * Interrupt vector descriptor
 */
typedef struct Vector_slot {
    // enable dispose(Vector_slot_t *)
    Disposable_t _disposable;
    // vector interrupt service handler
    void (*_handler)(void *);
    // vector interrupt service handler parameter
    void *_handler_param;
    // address of interrupt vector
    uint8_t _vector_no;
    // original vector handler, restored on dispose
    uint16_t _vector_original_content;

} Vector_slot_t;

/**
 * Single interrupt vector handle structure
 */
typedef struct Vector_handle {
    // enable dispose(Vector_handle_t *)
    Disposable_t _disposable;
    // address of interrupt vector
    uint8_t _vector_no;
    // interrupt enable register and mask
    uint16_t _IE_register;
    uint16_t _IE_mask;
    // interrupt flag register and mask
    uint16_t _IFG_register;
    uint16_t _IFG_mask;
    // function to be called on dispose
    dispose_function_t _dispose_hook;
    // -------- state --------
    // assigned slot via register_handler()
    Vector_slot_t *_slot;
    // original vector handler, restored on dispose
    uint16_t _vector_original_content;
    // -------- public api --------
    // trigger interrupt, so that registered handler shall be executed
    uint8_t (*trigger)(struct Vector_handle *_this);
    // clearing (or reading IV reg) only required when flags are not cleared by HW
    uint8_t (*clear_interrupt_flag)(struct Vector_handle *_this);
    // set / reset interrupt enable flag
    uint8_t (*set_enabled)(struct Vector_handle *_this, bool enabled);
    // register interrupt service routine for this vector, if reversible set, the original handler shall be restored on dispose
    uint8_t (*register_raw_handler)(struct Vector_handle *_this, void (*handler)(void), bool reversible);
    // assign and register slot for this vector, so that handler shall be called with handler_param on interrupt
    Vector_slot_t *(*register_handler)(struct Vector_handle *_this, void (*handler)(void *), void *handler_param);
    // when vector_handle is disposed, possible assigned slot is also disposed - calling this function disables this behavior
    uint8_t (*disable_slot_release_on_dispose)(struct Vector_handle *_this);
    // interrupt enable state
    bool enabled;

} Vector_handle_t;

// -------------------------------------------------------------------------------------

void vector_handle_register(Vector_handle_t *handle, dispose_function_t dispose_hook,
        uint8_t vector_no, uint16_t IE_register, uint16_t IE_mask, uint16_t IFG_register, uint16_t IFG_mask);


#endif /* _DRIVER_VECTOR_H_ */
