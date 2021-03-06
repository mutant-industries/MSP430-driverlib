/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Stack manipulation and CPU-specific related macros
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_CPU_H_
#define _DRIVER_CPU_H_

#include <stdint.h>
#include <compiler.h>

// -------------------------------------------------------------------------------------

/**
 * Return from interrupt - skip possible function epilogue generated by compiler
 */
#define reti \
    __asm__(" reti");

// -------------------------------------------------------------------------------------

/**
 * 8-bit SFR register manipulation
 */
#define hw_register_8(x) \
    (*((volatile uint8_t *)((uint16_t) (x))))

/**
 * 16-bit SFR register manipulation
 */
#define hw_register_16(x) \
    (*((volatile uint16_t *)((uint16_t) (x))))

/**
 * address (16-bit / 20-bit) SFR register manipulation
 */
#define hw_register_addr(x) \
    (*((volatile void **)((uint16_t) (x))))

/**
 * Core registers data type - depends on data pointer size (and thus on instructions used)
 */
#if defined(_DATA_MODEL_LARGE_) || defined(_CODE_MODEL_LARGE_)
typedef uintptr_t data_pointer_register_t;
#else
typedef uint16_t data_pointer_register_t;
#endif

/**
 * Instructions dependant on data memory model
 */
#if defined(_DATA_MODEL_LARGE_) || defined(_CODE_MODEL_LARGE_)
#define __mov__     "MOV.A"
#define __pushm__   "PUSHM.A"
#define __popm__    "POPM.A"
#else
#define __mov__     "MOV.W"
#define __pushm__   "PUSHM.W"
#define __popm__    "POPM.W"
#endif


#endif /* _DRIVER_CPU_H_ */
