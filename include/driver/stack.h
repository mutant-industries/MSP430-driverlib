/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Stack pointer manipulation, deferred stack modification
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_STACK_H_
#define _DRIVER_STACK_H_

#include <stdint.h>
#include <compiler.h>
#include <driver/cpu.h>

// --------------------------------------------------------------------------------------

/**
 * Stack pointer set / get, data model small
 *  - compiler intrinsic functions __set_SP_register() / __get_SP_register() are used
 */
#if ! defined(_DATA_MODEL_LARGE_) || defined(_TI_COMPILER_) && defined(__STACK_POINTER_20_BIT_SUPPORT_DISABLE__)

#define stack_pointer_set(ptr) \
    __set_SP_register((unsigned short) (ptr));

#define stack_pointer_get(aptr) \
    *(unsigned short **) (aptr) = __get_SP_register();

#else

/**
 * Stack pointer set / get, data model large, TI compiler
 *  - no way to directly manipulate SP register
 */
#if defined(_TI_COMPILER_)
extern volatile data_pointer_register_t __stack_pointer__;

#define stack_pointer_set(ptr) \
    __stack_pointer__ = (data_pointer_register_t) (ptr); \
    __asm__("   "__mov__" &__stack_pointer__, SP");

#define stack_pointer_get(aptr) \
    __asm__("   "__mov__" SP, &__stack_pointer__"); \
    *(data_pointer_register_t **) (aptr) = __stack_pointer__;

/**
 * Stack pointer set / get, data model large, GCC
 *  - global register variable (R1 == SP), has to be defined in header file for some reason
 */
#elif defined(_GCC_COMPILER_)
volatile register data_pointer_register_t __stack_pointer__ __asm__("R1");

#define stack_pointer_set(ptr) \
    __stack_pointer__ = (data_pointer_register_t) (ptr);

#define stack_pointer_get(aptr) \
    *(data_pointer_register_t **) (aptr) = __stack_pointer__;

#endif

#endif

// --------------------------------------------------------------------------------------

/**
 * Stack save / restore processor registers
 */
#define stack_save_context(aptr) \
    __asm__("   "__pushm__" #12, R15"); \
    stack_pointer_get(aptr);

#define stack_restore_context(aptr) \
    stack_pointer_set(*(aptr)); \
    *(data_pointer_register_t **) (aptr) += 5; \
    __asm__("   "__popm__" #12, R15");

// --------------------------------------------------------------------------------------

/**
 * Initialize stack pointer according to base address and size of stack
 *  - on MSP430 devices stack pointer is initialized to highest address, growing stack decrements stack pointer
 *  - stack_base_address and stack_size should be even numbers (optional)
 *  - stack pointer has predecrement behavior, the border address is not going to be written to
 */
#define deferred_stack_pointer_init(aptr, stack_base_address, stack_size) \
    *(data_pointer_register_t **) (aptr) = (data_pointer_register_t) ((uintptr_t) (stack_base_address) + (uint16_t) (stack_size));

/**
 * Push return address on deferred stack
 */
#if defined(_CODE_MODEL_LARGE_)
#define deferred_stack_push_return_address(aptr, return_address) \
    (*(uint16_t **) (aptr))--; \
    **(uint16_t **) (aptr) = (uint16_t) (((uintptr_t) (return_address)) >> 16); \
    (*(uint16_t **) (aptr))--; \
    **(uint16_t **) (aptr) = (uint16_t) (return_address);
#else
#define deferred_stack_push_return_address(aptr, return_address) \
    (*(uint16_t **) (aptr))--; \
    **(uint16_t **) (aptr) = (uint16_t) (return_address);
#endif

// --------------------------------------------------------------------------------------

/**
 * Initialize context on deferred stack
 *  - store return address and decrement given 'aptr'
 *  - 5x decrement given 'aptr', store parameter to address, from which R12 shall be restored
 *  - after this operation, restore_context('aptr') and RETI can be executed, then:
 *   - execution starts at 'start_address', 'parameter' is passed to it
 */
#define deferred_stack_context_init(aptr, start_address, parameter) \
    uint16_t start_address_high = (uint16_t) (((uintptr_t) (start_address)) >> 16); \
    \
    (*(uint16_t **) (aptr))--; \
    **(uint16_t **) (aptr) = (uint16_t) (start_address); \
    (*(uint16_t **) (aptr))--; \
    **(uint16_t **) (aptr) = (start_address_high << 12) | GIE; \
    /* CPU registers ~ context */ \
    (*(data_pointer_register_t **) (aptr))--; /* R15 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R14 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R13 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R12 */ \
    **(data_pointer_register_t **) (aptr) = (data_pointer_register_t) (parameter); \
    (*(data_pointer_register_t **) (aptr))--; /* R11 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R10 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R9 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R8 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R7 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R6 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R5 */ \
    (*(data_pointer_register_t **) (aptr))--; /* R4 */

/**
 * Store return value on deferred stack to address, from which R12 shall be restored
 */
#define deferred_stack_store_return_value(ptr, value) \
    ((data_pointer_register_t *) (ptr))[8] = (data_pointer_register_t) (value);

// --------------------------------------------------------------------------------------

#endif /* _DRIVER_STACK_H_ */
