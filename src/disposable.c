// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/disposable.h>
#include <stddef.h>
#include <driver/interrupt.h>


void __do_dispose(Dispose_hook_t *handle) {

    if ( ! handle || ! handle->_dispose_hook) {
        return;
    }

    interrupt_suspend();

    dispose_function_t dispose_hook = handle->_dispose_hook;
    // dispose() thread-safety, also optimization when same resource is disposed several times without re-registering
    handle->_dispose_hook = NULL;

    interrupt_restore();

    while (dispose_hook) {
        dispose_hook = (dispose_function_t) (*dispose_hook)(handle);
    }
}

void __do_zerofill(void *handle, uint16_t size) {
    uintptr_t i;

    for (i = (uintptr_t) handle; i < (((uintptr_t) handle) + size); i++) {
        *((uint8_t *) i) = 0;
    }
}
