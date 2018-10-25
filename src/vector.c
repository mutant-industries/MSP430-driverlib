// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <stddef.h>
#include <driver/vector.h>

// -------------------------------------------------------------------------------------

static void _trigger(Vector_handle_t *_this) {
    *((volatile uint16_t *) _this->_IFG_register) |= _this->_IFG_mask;
}

static void _clear_interrupt_flag(Vector_handle_t *_this) {
    *((volatile uint16_t *) _this->_IFG_register) &= ~_this->_IFG_mask;
}

static void _set_enabled(Vector_handle_t *_this, bool enabled) {
    if (_this->enabled == enabled) {
        return;
    }

    _this->enabled = enabled;

    if ( ! _this->_IE_register || ! _this->_IE_mask) {
        return;
    }

    if (enabled) {
        *((volatile uint16_t *) _this->_IE_register) |= _this->_IE_mask;
    }
    else {
        *((volatile uint16_t *) _this->_IE_register) &= ~_this->_IE_mask;
    }
}

static void _register_handler(Vector_handle_t *_this, void (*handler)(void), bool reversible) {
    if (reversible && ! _this->_vector_original_content) {
        _this->_vector_original_content = __vector(_this->_vector_no);
    }

    __vector_set(_this->_vector_no, handler);
}

// -------------------------------------------------------------------------------------

static dispose_function_t _vector_deregister(Vector_handle_t *_this) {

    if (_this->set_enabled) {
        _this->set_enabled(_this, false);
    }

    if (_this->_vector_original_content) {
        __vector_set(_this->_vector_no, _this->_vector_original_content);
    }

    _this->_vector_original_content = NULL;

    return _this->_dispose_hook;
}

// -------------------------------------------------------------------------------------

// default constructor
void vector_handle_register(Vector_handle_t *handle, dispose_function_t dispose_hook,
        uint16_t vector_no, uint16_t IE_register, uint16_t IE_mask, uint16_t IFG_register, uint16_t IFG_mask) {

    // private
    handle->_vector_no= vector_no;
    handle->_IE_register = IE_register;
    handle->_IE_mask = IE_mask;
    handle->_IFG_register = IFG_register;
    handle->_IFG_mask = IFG_mask;
    handle->_dispose_hook = dispose_hook;

    // state
    if (IE_register && IE_mask) {
        handle->enabled = (bool) (*((volatile uint16_t *) IE_register) & IE_mask);
    }

    // public api
    handle->trigger = _trigger;
    handle->clear_interrupt_flag = _clear_interrupt_flag;
    handle->set_enabled = _set_enabled;
    handle->register_handler = _register_handler;

    __register_dispose_hook(handle, _vector_deregister);
}
