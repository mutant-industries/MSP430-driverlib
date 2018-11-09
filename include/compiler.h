/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Definitions based on used compiler, memory model etc.
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

// -------------------------------------------------------------------------------------

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
 * Memory model small (up to 64KB) / memory model large
 */
#if defined(_TI_COMPILER_) && defined(__LARGE_DATA_MODEL__) || defined(_GCC_COMPILER_) && defined(__MSP430X_LARGE__)
#define _MEMORY_MODEL_LARGE_
#endif

/**
 * Size of compiler-generated pointers based on memory model
 */
#if defined(_MEMORY_MODEL_LARGE_)
#define _POINTER_SIZE_      4
#else
#define _POINTER_SIZE_      2
#endif

// -------------------------------------------------------------------------------------


#endif /* _COMPILER_H_ */
