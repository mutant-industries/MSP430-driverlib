// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/disposable.h>


void __do_dispose(Dispose_hook_t *handle) {

    if ( ! handle) {
        return;
    }

    dispose_function_t dispose_hook = handle->_dispose_hook;

    while (dispose_hook) {
        dispose_hook = (dispose_function_t) (*dispose_hook)(handle);
    }
}
