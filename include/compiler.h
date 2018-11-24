/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Definitions based on used compiler, memory model etc.
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

// -------------------------------------------------------------------------------------

#define __VOID__

/**
 * TI compiler and MSP430-gcc are supported
 */
#if defined(__TI_COMPILER_VERSION__)
#define _TI_COMPILER_
#elif defined(__GNUC__)
#define _GCC_COMPILER_
#else
#error "unsupported compiler"
#endif

/**
 * Memory data model small (up to 64KB) / memory data model large
 *  - defines size of data pointers, defines whether 'X' instructions are to be used (MOVA, CMPA...)
 */
#if defined(_TI_COMPILER_) && defined(__LARGE_DATA_MODEL__) || defined(_GCC_COMPILER_) && defined(__MSP430X_LARGE__)
#define _DATA_MODEL_LARGE_
#endif

/**
 * Memory code model small (up to 64KB) / memory code model large
 *  - defines size of code pointers, defines whether 'X' call instructions are to be used (CALLA, RETA, BRA...)
 */
#if defined(_TI_COMPILER_) && defined(__LARGE_CODE_MODEL__) || defined(_GCC_COMPILER_) && defined(__MSP430X_LARGE__)
#define _CODE_MODEL_LARGE_
#endif

/**
 * Size of compiler-generated pointers based on memory model
 */
#if defined(_DATA_MODEL_LARGE_)
#define _DATA_POINTER_SIZE_      4
#else
#define _DATA_POINTER_SIZE_      2
#endif

// -------------------------------------------------------------------------------------

/**
 * Interrupt function attribute, default placement in _isr section
 */
#define __interrupt_no(no) \
    __attribute__((interrupt, section(".text:_isr")))

#define __interrupt \
    __interrupt_no(__VOID__)

/**
 * Naked function attribute - no compiler-generated prologue / epilogue
 */
#define __naked \
    __attribute__((naked))

/**
 * Noinit variable attribute - no initialization by C runtime startup code
 */
#define __noinit \
    __attribute__((noinit))

/**
 * Persistent variable attribute - initialize only on program load
 */
#define __persistent \
    __attribute__((persistent))

// -------------------------------------------------------------------------------------

#endif /* _COMPILER_H_ */
