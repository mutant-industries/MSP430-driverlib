// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/stack.h>


#if defined(_DATA_MODEL_LARGE_) && defined(_TI_COMPILER_) && ! defined(__STACK_POINTER_20_BIT_SUPPORT_DISABLE__)
volatile data_pointer_register_t __stack_pointer__;
#endif
