// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/IO.h>
#include <stddef.h>
#include <compiler.h>
#include <driver/interrupt.h>


// maximum count of 8-bit addressable ports
#define MAX_PORT_COUNT  12

// array of pointers to registered drivers, persistent to allow wakeup on FRAM devices
__persistent IO_port_driver_t *registered_drivers[MAX_PORT_COUNT] = {0};

static uint8_t _unsupported_operation() {
    return IO_UNSUPPORTED_OPERATION;
}

// -------------------------------------------------------------------------------------

static void _shared_vector_handler(IO_port_driver_t *driver) {
    uint8_t interrupt_pin_no;
    uint16_t interrupt_source;
    IO_pin_handle_t *handle;

    if ( ! (interrupt_source = hw_register_16(driver->_IV_register))) {
        return;
    }

    // IV -> pin number (0x00 - no interrupt, 0x02 - PxIFG.0 interrupt, 0x04 - PxIFG.1 interrupt...)
    interrupt_pin_no = (uint8_t) (interrupt_source / 2 - 1);

    handle = ((IO_pin_handle_t **) &driver->_pin0_handle)[interrupt_pin_no];

    // execute handler with given handler_arg and PIN_X that triggered interrupt
    handle->_handler(handle->_handler_arg, (void *) (((uint16_t) 0x0001) << interrupt_pin_no));
}

static Vector_slot_t *_register_handler_shared(IO_pin_handle_t *_this, vector_slot_handler_t handler, void *arg) {

    interrupt_suspend();

    if ( ! _this->_driver->_slot) {
        _this->_driver->_slot = _this->_register_handler_parent(&_this->vector,
                (vector_slot_handler_t) _shared_vector_handler, _this->_driver, NULL);
    }

    interrupt_restore();

    if ( ! _this->_driver->_slot) {
        return NULL;
    }

    // handle dispose preserves created vector slot
    vector_disable_slot_release_on_dispose(_this);

    _this->_handler = handler;
    _this->_handler_arg = arg;

    return _this->_driver->_slot;
}

// -------------------------------------------------------------------------------------

// IO_port_driver_t destructor
static dispose_function_t _pin_handle_dispose(IO_pin_handle_t *_this) {
    IO_pin_handle_t **handle_ref = &_this->_driver->_pin0_handle;
    uint8_t pin;

    _this->_handler = NULL;
    _this->_handler_arg = NULL;

    // register interrupt handler is now disabled
    _this->vector.register_handler = (Vector_slot_t *(*)(Vector_handle_t *,
            vector_slot_handler_t, void *, void *)) _unsupported_operation;
#ifdef __IO_PORT_LEGACY_SUPPORT__
    // disable assignment of raw handler to shared vector
    _this->vector.register_raw_handler = (uint8_t (*)(Vector_handle_t *, interrupt_service_t, bool)) _unsupported_operation;
#endif

    // release driver->handle references
    for (pin = 0; pin < 8; pin++, handle_ref++) {
        if (*handle_ref == _this) {
            *handle_ref = NULL;
        }
    }

    // reset default control register values
    IO_pin_handle_reg_reset(_this, DIR);
    IO_pin_handle_reg_reset(_this, REN);
#ifdef OFS_PxSELC
    IO_pin_handle_reg_reset(_this, SELC);
#else
    IO_pin_handle_reg_reset(_this, SEL0);
#endif

    // direct register access is still allowed after disposed

    return NULL;
}

// IO_port_driver_t constructor
static uint8_t _pin_handle_register(IO_port_driver_t *_this, IO_pin_handle_t *handle, uint8_t pin_mask) {
    // enable 16-bit register access in vector (set / clear interrupt flag, interrupt enable / disable)
    uint16_t base_register_16, pin_mask_16;
    IO_pin_handle_t **handle_ref;
    uint8_t pin;

    handle->_base_register = _this->_base_register;
    handle->_pin_mask = pin_mask;

    interrupt_suspend();

    // check whether handles for given pins are registered already
    for (pin = 1, handle_ref = &_this->_pin0_handle; pin; pin <<= 1, handle_ref++) {
        if (pin & pin_mask && *handle_ref) {
            interrupt_restore();
            // current pin is already registered for another handle
            return IO_PIN_HANDLE_REGISTERED_ALREADY;
        }
    }

    // driver->handle references
    for (pin = 1, handle_ref = &_this->_pin0_handle; pin; pin <<= 1, handle_ref++) {
        if (pin & pin_mask) {
            *handle_ref = handle;
        }
    }

    interrupt_restore();

    base_register_16 = handle->_base_register;
    pin_mask_16 = pin_mask;

#ifndef __IO_PORT_LEGACY_SUPPORT__
    if (base_register_16 & 0x0001) {
        // 16-bit address alignment
        base_register_16--;
        // adjust pin mask to correspond to 16-bit access
        pin_mask_16 <<= 8;
    }
#endif

    // handle->driver reference
    handle->_driver = _this;

    vector_handle_register(&handle->vector, (dispose_function_t) _pin_handle_dispose, _this->_vector_no,
            base_register_16 + OFS_PxIE, pin_mask_16, base_register_16 + OFS_PxIFG, pin_mask_16);

    handle->_handler = NULL;
    handle->_handler_arg = NULL;

#ifndef __IO_PORT_LEGACY_SUPPORT__
    // disable assignment of raw handler to shared vector
    handle->vector.register_raw_handler = (uint8_t (*)(Vector_handle_t *, interrupt_service_t, bool)) _unsupported_operation;
    // override default register_handler on vector handle
    handle->_register_handler_parent = handle->vector.register_handler;
    handle->vector.register_handler = (Vector_slot_t *(*)(Vector_handle_t *,
            vector_slot_handler_t, void *, void *)) _register_handler_shared;
#else
    // no support for vector handlers if device has no Px_IV register
    handle->vector.register_handler = (Vector_slot_t *(*)(Vector_handle_t *,
            vector_slot_handler_t, void *, void *)) _unsupported_operation;
#endif

    return IO_OK;
}

// -------------------------------------------------------------------------------------

// IO_port_driver_t destructor
static dispose_function_t _IO_port_driver_dispose(IO_port_driver_t *_this) {
    IO_pin_handle_t **handle_ref = &_this->_pin0_handle;
    uint8_t pin;

    // disable low-power mode wakeup reinit
    registered_drivers[_this->_port_no - 1] = NULL;

    // register new handles is now disabled
    _this->pin_handle_register = (uint8_t (*)(IO_port_driver_t *, IO_pin_handle_t *, uint8_t)) _unsupported_operation;

    // restore original vector content
    dispose(_this->_slot);

    for (pin = 0; pin < 8; pin++, handle_ref++) {
        dispose(*handle_ref);
    }

    // direct register access is still allowed after disposed

    return NULL;
}

// IO_port_driver_t constructor
void IO_port_driver_register(IO_port_driver_t *driver, uint8_t port_no, uint16_t base, uint8_t vector_no,
        void (*port_init)(IO_port_driver_t *), uint8_t low_power_mode_pin_reset_filter) {

    zerofill(driver);

    driver->_base_register = base;
    driver->_vector_no = vector_no;
    driver->_port_no = port_no;
    driver->_IV_register = base + 0x0E;
    driver->_low_power_mode_pin_reset_filter = low_power_mode_pin_reset_filter;

    // PORT_1 -> IV register 0x20E, PORT_2 -> IV register 0x21E, PORT_3 -> IV register 0x22E, PORT_4 -> IV register 0x23E...
    if (base & 0x0001) {
        driver->_IV_register += 0x000F;
    }

    // store global driver reference
    registered_drivers[port_no - 1] = driver;

    if (port_init) {
#ifndef __IO_PORT_LEGACY_SUPPORT__
        // wakeup available only on FRAM devices
        driver->_port_init = port_init;
#endif
        // execute port initialization
        port_init(driver);
    }

    // public
    driver->pin_handle_register = _pin_handle_register;

    __dispose_hook_register(driver, _IO_port_driver_dispose);
}

// -------------------------------------------------------------------------------------

void IO_wakeup_reinit() {

#ifndef __IO_PORT_LEGACY_SUPPORT__
    IO_port_driver_t *port, **port_ref;
    IO_pin_handle_t *handle, **handle_ref;
    uint8_t port_index, handle_index;

    // initialize port registers exactly the same way as they were configured before the device entered LPMx.5
    for (port_index = 0, port_ref = registered_drivers; port_index < MAX_PORT_COUNT; port_index++, port_ref++) {
        if ((port = *port_ref) == NULL || port->_port_init == NULL) {
            continue;
        }

        // initialize port registers
        port->_port_init(port);

        if ( ! port->_slot) {
            continue;
        }

        port->_slot = NULL;

        // reinit port vector slot if set
        for (handle_index = 0, handle_ref = &port->_pin0_handle; handle_index < 8; handle_index++, handle_ref++) {
            // search for first handle with registered interrupt handler
            if ((handle = *handle_ref) != NULL && handle->_handler) {
                // reinit (non-persistent) port vector slot
                port->_slot = handle->_register_handler_parent(&handle->vector,
                        (vector_slot_handler_t) _shared_vector_handler, port, NULL);

                // slot is registered just once per port
                break;
            }
        }
    }
#endif

    IO_unlock();

#ifndef __IO_PORT_LEGACY_SUPPORT__
    // disable interrupts so that handler with highest priority shall be triggered first
    interrupt_suspend();

    // enable port interrupts if configured before the device entered LPMx.5
    for (port_index = 0, port_ref = registered_drivers; port_index < MAX_PORT_COUNT; port_index++, port_ref++) {
        if ((port = *port_ref) == NULL || port->_port_init == NULL) {
            continue;
        }

        // search for first handle with registered interrupt handler
        for (handle_index = 0, handle_ref = &port->_pin0_handle; handle_index < 8; handle_index++, handle_ref++) {
            // set corresponding interrupt enable bits if vector interrupts were enabled
            if ((handle = *handle_ref) != NULL && handle->vector.enabled) {
                vector_set_enabled(&handle->vector, true);
            }
        }
    }

    interrupt_restore();
#endif
}

void IO_low_power_mode_prepare() {
    IO_port_driver_t *port, **port_ref;
    uint8_t port_index, pin_function_reset_mask;

    // prepare all registered port drivers for low-power mode
    for (port_index = 0, port_ref = registered_drivers; port_index < MAX_PORT_COUNT; port_index++, port_ref++) {
        if ((port = *port_ref) == NULL) {
            continue;
        }

        // restore original vector content (otherwise it would be lost)
        if (port->_slot) {
            dispose(port->_slot);
        }

        // by default reset all pins to general-purpose IO
        pin_function_reset_mask = PIN_0 | PIN_1 | PIN_2 | PIN_3 | PIN_4 | PIN_5 | PIN_6 | PIN_7;
        // filter reset mask if filter set
        pin_function_reset_mask ^= port->_low_power_mode_pin_reset_filter;

#ifdef OFS_PxSELC
        IO_port_reg_reset(port, SELC, pin_function_reset_mask);
#else
        IO_port_reg_reset(port, SEL0, pin_function_reset_mask);
#endif
    }
}
