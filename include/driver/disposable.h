/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  Dispose interface, relation to kernel if not standalone
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_DISPOSABLE_H_
#define _DRIVER_DISPOSABLE_H_

#include <driver/config.h>

// --------------------------------------------------------------------------------------

typedef struct Dispose_hook Dispose_hook_t;
typedef struct Disposable Disposable_t;

/**
 * Pointer to function with parameter type (*) returning pointer to function with parameter type (*) returning...
 *  - @see https://stackoverflow.com/a/17536529
 */
typedef void (*(*dispose_function_t)(Dispose_hook_t *))(void);

/**
 * Dispose function wrapper structure
 *   - has to be first member of struct, because any struct to be disposed is always cast to Dispose_hook_t*
 *   - dispose_hook handles cleanup of resource / struct, and optionally returns pointer another dispose hook, which allows chaining
 */
struct Dispose_hook {
    // hook executed on dispose
    dispose_function_t _dispose_hook;

};

// --------------------------------------------------------------------------------------

#ifdef __RESOURCE_MANAGEMENT_ENABLE__
#include <resource.h>
#else

struct Disposable {
    Dispose_hook_t _;

};

#define __dispose_hook_register(handle, dispose_hook) \
        ((Dispose_hook_t *) (handle))->_dispose_hook = (dispose_function_t) (dispose_hook);

#endif

// --------------------------------------------------------------------------------------

/**
 * Execute dispose hook chain on given handle, shortcut to __do_dispose(Dispose_hook_t *)
 */
#define dispose(handle) \
    __do_dispose((Dispose_hook_t *) (handle));

/**
 * Typesafe dispose(*), internal use only
 */
void __do_dispose(Dispose_hook_t *handle);


#endif /* _DRIVER_DISPOSABLE_H_ */
