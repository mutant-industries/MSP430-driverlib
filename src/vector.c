// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <stddef.h>
#include <lib/cpp/repeat.h>
#include <driver/vector.h>
#include <driver/cpu.h>
#include <driver/interrupt.h>

// -------------------------------------------------------------------------------------

#ifndef __VECTOR_SLOT_COUNT__
#define __VECTOR_SLOT_COUNT__       8
#endif

// -------------------------------------------------------------------------------------

static Vector_slot_t _vector_slot_array[__VECTOR_SLOT_COUNT__];

// interrupt handler function name generator
#define __interrupt_handler_name_generator(no) _vector_slot_ ## no
#define __interrupt_handler_array_generator(no, _) __interrupt_handler_name_generator(no),

// interrupt handler generator
#define __interrupt_handler_generator(no, _)                                                \
__naked __interrupt void __interrupt_handler_name_generator(no) () {                        \
    __asm__("   "__pushm__" #5, R15");                                                      \
    Vector_slot_t *slot = &_vector_slot_array[no];                                          \
    slot->_handler(slot->_handler_arg_1, slot->_handler_arg_2);                             \
    __asm__("   "__popm__" #5, R15");                                                       \
    reti;                                                                                   \
}

// generate slot interrupt handler definitions
REPEAT(__VECTOR_SLOT_COUNT__, __interrupt_handler_generator)

// generate array of pointers to previously generated handlers
static interrupt_service_t _vector_slot_handler_array[__VECTOR_SLOT_COUNT__] = {
    REPEAT(__VECTOR_SLOT_COUNT__, __interrupt_handler_array_generator)
};

// -------------------------------------------------------------------------------------

static uint8_t _trigger(Vector_handle_t *_this) {
    if ( ! _this->_IFG_register) {
        return VECTOR_IFG_REG_NOT_SET;
    }
    if ( ! _this->_IFG_mask) {
        return VECTOR_IFG_MASK_NOT_SET;
    }

    hw_register_16(_this->_IFG_register) |= _this->_IFG_mask;

    return VECTOR_OK;
}

static uint8_t _clear_interrupt_flag(Vector_handle_t *_this) {
    if ( ! _this->_IFG_register) {
        return VECTOR_IFG_REG_NOT_SET;
    }
    if ( ! _this->_IFG_mask) {
        return VECTOR_IFG_MASK_NOT_SET;
    }

    hw_register_16(_this->_IFG_register) &= ~_this->_IFG_mask;

    return VECTOR_OK;
}

static uint8_t _set_enabled(Vector_handle_t *_this, bool enabled) {
    if (_this->enabled == enabled) {
        return VECTOR_OK;
    }

    _this->enabled = enabled;

    if ( ! _this->_IE_register) {
        return VECTOR_IE_REG_NOT_SET;
    }

    if ( ! _this->_IE_mask) {
        return VECTOR_IE_MASK_NOT_SET;
    }

    if (enabled) {
        hw_register_16(_this->_IE_register) |= _this->_IE_mask;
    }
    else {
        hw_register_16(_this->_IE_register) &= ~_this->_IE_mask;
    }

    return VECTOR_OK;
}

static uint8_t _register_raw_handler(Vector_handle_t *_this, interrupt_service_t handler, bool reversible) {
    if (reversible && ! _this->_vector_original_content) {
        _this->_vector_original_content = __vector(_this->_vector_no);
    }

    __vector_set(_this->_vector_no, handler);

    return VECTOR_OK;
}

// -------------------------------------------------------------------------------------

// Vector_slot_t destructor
static dispose_function_t _vector_slot_dispose(Vector_slot_t *_this) {
    if (_this->_vector_original_content) {
        __vector_set(_this->_vector_no, _this->_vector_original_content);
    }

    zerofill(_this);

    return NULL;
}

// Vector_slot_t constructor
static void _vector_slot_register(Vector_slot_t *slot, uint8_t vector_no, interrupt_service_t  interrupt_handler,
              vector_slot_handler_t handler, void *arg_1, void *arg_2) {

    // private
    slot->_handler = handler;
    slot->_handler_arg_1 = arg_1;
    slot->_handler_arg_2 = arg_2;
    slot->_vector_no = vector_no;
    slot->_vector_original_content = __vector(slot->_vector_no);

    __vector_set(slot->_vector_no, interrupt_handler);

    __dispose_hook_register(slot, _vector_slot_dispose);
}

// -------------------------------------------------------------------------------------

static Vector_slot_t *_register_handler(Vector_handle_t *_this, vector_slot_handler_t handler, void *arg_1, void *arg_2) {
    Vector_slot_t *slot_iter = _vector_slot_array;
    interrupt_service_t *slot_handler_iter = _vector_slot_handler_array;
    uint8_t i;

    if ( ! _this->_vector_no) {
        return NULL;
    }

    interrupt_suspend();

    dispose(_this->_slot);

    for (i = 0; i < __VECTOR_SLOT_COUNT__; i++, slot_iter++, slot_handler_iter++) {
        if ( ! slot_iter->_handler) {
            _this->_slot = slot_iter;
            _vector_slot_register(_this->_slot, _this->_vector_no, *slot_handler_iter, handler, arg_1, arg_2);

            break;
        }
    }

    interrupt_restore();

    return _this->_slot;
}

static uint8_t _disable_slot_release_on_dispose(Vector_handle_t *_this) {
    _this->_slot = NULL;

    return VECTOR_OK;
}

// -------------------------------------------------------------------------------------

// Vector_handle_t destructor
static dispose_function_t _vector_handle_dispose(Vector_handle_t *_this) {

    if (_this->set_enabled) {
        _this->set_enabled(_this, false);
    }

    dispose(_this->_slot);

    if (_this->_vector_original_content) {
        __vector_set(_this->_vector_no, _this->_vector_original_content);
    }

    _this->_vector_no = NULL;
    _this->_IE_register = NULL;
    _this->_IE_mask = NULL;
    _this->_IFG_register = NULL;
    _this->_IFG_mask = NULL;
    _this->_vector_original_content = NULL;
    _this->_slot = NULL;

    return _this->_dispose_hook;
}

// Vector_handle_t constructor
void vector_handle_register(Vector_handle_t *handle, dispose_function_t dispose_hook,
        uint8_t vector_no, uint16_t IE_register, uint16_t IE_mask, uint16_t IFG_register, uint16_t IFG_mask) {

    // private
    handle->_vector_no = vector_no;
    handle->_IE_register = IE_register;
    handle->_IE_mask = IE_mask;
    handle->_IFG_register = IFG_register;
    handle->_IFG_mask = IFG_mask;
    handle->_dispose_hook = dispose_hook;

    // state
    handle->_slot = NULL;
    handle->_vector_original_content = NULL;

    // public
    handle->trigger = _trigger;
    handle->clear_interrupt_flag = _clear_interrupt_flag;
    handle->set_enabled = _set_enabled;
    handle->register_raw_handler = _register_raw_handler;
    handle->register_handler = _register_handler;
    handle->disable_slot_release_on_dispose = _disable_slot_release_on_dispose;

    if (IE_register && IE_mask) {
        handle->enabled = (bool) (hw_register_16(IE_register) & IE_mask);
    }

    __dispose_hook_register(handle, _vector_handle_dispose);
}
