// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <stddef.h>
#include <compiler.h>
#include <driver/timer.h>
#include <driver/cpu.h>
#include <driver/interrupt.h>

// -------------------------------------------------------------------------------------

/**
 * Input divider expansion register
 */
#ifdef TAIDEX_0
#define _TIMER_HAS_IDEX_
#endif

/**
 * Standard timer register offsets from base address, compatible across all devices.
 */
#ifdef OFS_TAxCTL
#define OFS_TxCTL           OFS_TAxCTL
#define OFS_TxCCTL0         OFS_TAxCCTL0
#define OFS_TxR             OFS_TAxR
#define OFS_TxCCR0          OFS_TAxCCR0
#ifdef _TIMER_HAS_IDEX_
#define OFS_TxEX0           OFS_TAxEX0
#endif
#else
#define OFS_TxCTL           (0x0000)
#define OFS_TxCCTL0         (0x0002)
#define OFS_TxR             (0x0010)
#define OFS_TxCCR0          (0x0012)
#define OFS_TxEX0           (0x0020)
#endif

/**
 * OFS_TxIV in 1xx, 2xx, 3xx and 4xx families depends on timer, OFS_TAxIV != OFS_TBxIV
 */
#if defined (OFS_TAxIV) && defined (OFS_TBxIV) && OFS_TAxIV == OFS_TBxIV
#define OFS_TxIV    OFS_TAxIV
#elif defined(__TIMER_A_LEGACY_SUPPORT__)
// legacy Timer_A IV register offset (base addr 0x160, IV register addr 0x12E)
#define OFS_TxIV        -(0x32)
#else
// legacy Timer_B IV register offset (base addr 0x180, IV register addr 0x11E)
#define OFS_TxIV        -(0x62)
#endif

/**
 * Max threshold of two consecutive reads of counter register
 */
#define TIMER_THRESHOLD     (50)

// -------------------------------------------------------------------------------------

static uint8_t _start(Timer_channel_handle_t *_this) {
    uint16_t CTL_register;
    uint8_t result = TIMER_OK;

    interrupt_suspend();

    // check whether driver is not disposed already
    if ( ! (CTL_register = _this->_driver->_CTL_register)) {
        result = TIMER_DRIVER_NOT_REGISTERED;
    }
    else if ( ! _this->active) {
        if ( ! _this->_driver->_active_handles_cnt) {
            hw_register_16(CTL_register) |= _this->_driver->_mode | TACLR;
        }

        // vector.trigger() functionality not preserved when in capture mode
        if (_this->capture_mode || _this->handle_type == OVERFLOW) {
            vector_set_enabled(_this, true);
            vector_clear_interrupt_flag(_this);
        }
        else {
            hw_register_16(_this->_CCTLn_register) &= ~CAP;
        }

        _this->_driver->_active_handles_cnt++;
        _this->active = true;
    }

    interrupt_restore();

    return result;
}

static uint8_t _stop(Timer_channel_handle_t *_this) {
    uint16_t CTL_register;
    uint8_t result = TIMER_OK;

    interrupt_suspend();

    // check whether driver is not disposed already
    if ( ! (CTL_register = _this->_driver->_CTL_register)) {
        result = TIMER_DRIVER_NOT_REGISTERED;
    }
    else if (_this->active) {
        if (_this->_driver->_active_handles_cnt == 1) {
            hw_register_16(CTL_register) &= ~MC;
        }

        if (_this->capture_mode || _this->handle_type == OVERFLOW) {
            vector_set_enabled(_this, false);
        }

        // set capture mode (no matter what current operation of handle is)
        //  - this preserves vector.trigger() functionality and disables interrupt trigger on compare
        if (_this->handle_type != OVERFLOW) {
            hw_register_16(_this->_CCTLn_register) |= CAP;
        }

        _this->_driver->_active_handles_cnt--;
        _this->active = false;
    }

    interrupt_restore();

    return result;
}

static uint8_t _reset(Timer_channel_handle_t *_this) {
    uint16_t CTL_register;
    uint8_t result = TIMER_REFUSED;

    interrupt_suspend();
    // check whether driver is not disposed already
    if ( ! (CTL_register = _this->_driver->_CTL_register)) {
        result = TIMER_DRIVER_NOT_REGISTERED;
    }
    // only reset if _this is the only active handle of driver or no handles are active
    else if ( ! _this->_driver->_active_handles_cnt || (_this->_driver->_active_handles_cnt == 1 && _this->active)) {
        hw_register_16(CTL_register) |= TACLR;

        result = TIMER_OK;
    }

    interrupt_restore();

    return result;
}

static uint8_t _get_counter(Timer_channel_handle_t *_this, uint16_t *target) {
    uint16_t vote_one, vote_two;
    uint16_t TxR_register;
    // check whether driver is not disposed already
    if ((TxR_register = _this->_driver->_CTL_register + OFS_TxR) == OFS_TxR) {
        return TIMER_DRIVER_NOT_REGISTERED;
    }

    vote_one = hw_register_16(TxR_register);
    vote_two = hw_register_16(TxR_register);

    // cycle until diff of two consecutive votes is below allowed threshold
    while ((vote_one < vote_two && vote_two - vote_one > TIMER_THRESHOLD)
           || (vote_one > vote_two && vote_one - vote_two > TIMER_THRESHOLD)) {

        vote_one = vote_two;
        vote_two = hw_register_16(TxR_register);
    }

    *target = vote_two;

    return TIMER_OK;
}

static uint8_t _unsupported_operation() {
    return TIMER_UNSUPPORTED_OPERATION;
}

// -------------------------------------------------------------------------------------

static void _set_capture_mode(Timer_channel_handle_t *_this, uint16_t mode, uint16_t input_select, uint16_t input_synchronize) {

    // allow writing CCTLn register without stopping the handle to enable software-initiated capture trigger
    if ( ! _this->capture_mode) {
        // make sure that handle is not active when changing mode
        _this->stop(_this);
        // disable interrupts in case when changing from compare mode, handle.start() must be called to initiate capture
        vector_set_enabled(_this, false);
    }

    hw_register_16(_this->_CCTLn_register) = (hw_register_16(_this->_CCTLn_register) & ~(CM | CCIS | SCS | OUTMOD)) |
            (CAP | mode | input_select | input_synchronize);

    _this->capture_mode = true;
}

static bool _is_capture_overflow_set(Timer_channel_handle_t *_this) {
    bool capture_overflow_set;

    if ((capture_overflow_set = (bool) (hw_register_16(_this->_CCTLn_register) & COV))) {
        hw_register_16(_this->_CCTLn_register) &= ~COV;
    }

    return capture_overflow_set;
}

static void _set_compare_mode(Timer_channel_handle_t *_this, uint16_t output_mode) {
    // make sure that handle is not active when changing mode
    _this->stop(_this);
    // clear possible capture overflow flag, capture mode, no capture, set (optional) output mode
    hw_register_16(_this->_CCTLn_register) = (hw_register_16(_this->_CCTLn_register) & ~(CM | CAP | OUTMOD)) | CAP | output_mode;
    // compare mode - since timer started in capture mode with no capture, interrupt never triggers
    _this->capture_mode = false;
    // enable vector.trigger() functionality
    vector_set_enabled(_this, true);
}

static uint16_t _get_capture_value(Timer_channel_handle_t *_this) {
    return hw_register_16(_this->_CCRn_register);
}

static void _set_compare_value(Timer_channel_handle_t *_this, uint16_t value) {
    hw_register_16(_this->_CCRn_register) = value;
}

// -------------------------------------------------------------------------------------

static void _shared_vector_handler(Timer_driver_t *driver) {
    uint16_t interrupt_source;
    Timer_channel_handle_t *handle;

    if ( ! (interrupt_source = hw_register_16(driver->_IV_register))) {
        return;
    }

    handle = *((Timer_channel_handle_t **) (((uintptr_t)(&driver->_CCR0_handle)) + (interrupt_source * _DATA_POINTER_SIZE_ / 2)));

    handle->_handler(handle->_handler_arg_1, handle->_handler_arg_2);
}

static Vector_slot_t * _register_handler_shared(Timer_channel_handle_t *_this, vector_slot_handler_t handler, void *arg_1, void *arg_2) {

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
    _this->_handler_arg_1 = arg_1;
    _this->_handler_arg_2 = arg_2;

    return _this->_driver->_slot;
}

// -------------------------------------------------------------------------------------

// Timer_channel_handle_t destructor
static dispose_function_t _timer_channel_handle_dispose(Timer_channel_handle_t *_this) {

    _this->stop(_this);
    _this->_handler = NULL;
    _this->_handler_arg_1 = NULL;
    _this->_handler_arg_2 = NULL;

    if (_this->handle_type == OVERFLOW) {
        _this->_driver->_overflow_handle = NULL;
    }
    else {
        uint8_t CCRx;
        Timer_channel_handle_t **handle_ref = &_this->_driver->_CCR0_handle;
        // release driver->handle reference
        for (CCRx = 0; CCRx < _this->_driver->_available_handles_cnt; CCRx++, handle_ref++) {
            if (*handle_ref == _this) {
                *handle_ref = NULL;
                break;
            }
        }
    }

    _this->start = (uint8_t (*)(Timer_channel_handle_t *)) _unsupported_operation;
    _this->stop = (uint8_t (*)(Timer_channel_handle_t *)) _unsupported_operation;
    _this->reset = (uint8_t (*)(Timer_channel_handle_t *)) _unsupported_operation;

    if (_this->handle_type != OVERFLOW) {
        _this->set_capture_mode = (void (*)(Timer_channel_handle_t *, uint16_t, uint16_t, uint16_t)) _unsupported_operation;
        _this->is_capture_overflow_set = (bool (*)(Timer_channel_handle_t *)) _unsupported_operation;
        _this->set_compare_mode = (void (*)(Timer_channel_handle_t *, uint16_t)) _unsupported_operation;
        _this->set_compare_value = (void (*)(Timer_channel_handle_t *, uint16_t)) _unsupported_operation;
    }

    // timer counter and CCR can still be read after disposed

    _this->handle_type = (Timer_handle_type) NULL;

    return _this->_dispose_hook;
}

// Timer_channel_handle_t constructor
static uint8_t _channel_handle_register(Timer_driver_t *_this, Timer_channel_handle_t *handle,
                  Timer_handle_type handle_type, dispose_function_t dispose_hook) {

    uint8_t CCRx = 0;
    Timer_channel_handle_t **handle_ref = NULL;
    uint8_t vector_no = _this->_shared_vector_no;
    uint16_t interrupt_control_register, IE_mask, IFG_mask;

    interrupt_suspend();

    switch (handle_type) {
        case MAIN:
            // try assign main handle if requested
            if (_this->_CCR0_handle) {
                break;
            }

            vector_no = _this->_main_vector_no;
            handle_ref = &_this->_CCR0_handle;

            break;

        case SHARED:
            // try assign shared handle if requested
            handle_ref = &_this->_CCR1_handle;

            for (CCRx = 1; CCRx < _this->_available_handles_cnt; CCRx++, handle_ref++) {
                if ( ! (*handle_ref)) {
                    break;
                }
            }

            if (CCRx == _this->_available_handles_cnt) {
                handle_ref = NULL;
            }

            break;

        case OVERFLOW:
            // try assign overflow handle if requested
            if (_this->_overflow_handle) {
                break;
            }

            handle_ref = &_this->_overflow_handle;

            break;
    }

    if ( ! handle_ref) {
        interrupt_restore();
        return TIMER_NO_HANDLE_AVAILABLE;
    }

    // driver->handle reference
    *handle_ref = handle;

    // handle->driver reference
    handle->_driver = _this;

    // state
    interrupt_control_register = handle_type == OVERFLOW ? _this->_CTL_register : _this->_CTL_register + OFS_TxCCTL0 + (CCRx * 2);
    IE_mask = handle_type == OVERFLOW ? TAIE : CCIE;
    IFG_mask = handle_type == OVERFLOW ? TAIFG : CCIFG;

    vector_handle_register(&handle->vector, (dispose_function_t) _timer_channel_handle_dispose, vector_no,
                           interrupt_control_register, IE_mask, interrupt_control_register, IFG_mask);

    interrupt_restore();

    handle->_handler = NULL;
    handle->_handler_arg_1 = NULL;
    handle->_handler_arg_2 = NULL;
    handle->_dispose_hook = dispose_hook;

    // public
    if (handle_type != MAIN) {
        // disable assignment of raw handler to shared vector
        handle->vector.register_raw_handler = (uint8_t (*)(Vector_handle_t *, interrupt_service_t, bool)) _unsupported_operation;
        // override default register_handler on vector handle
        handle->_register_handler_parent = handle->vector.register_handler;
        handle->vector.register_handler = (Vector_slot_t *(*)(Vector_handle_t *,
                vector_slot_handler_t, void *, void *)) _register_handler_shared;
    }

    handle->start = _start;
    handle->stop = _stop;
    handle->reset = _reset;
    handle->get_counter = _get_counter;
    handle->handle_type = handle_type;
    handle->active = false;

    if (handle_type != OVERFLOW) {
        handle->_CCTLn_register = _this->_CTL_register + OFS_TxCCTL0 + (CCRx * 2);
        handle->_CCRn_register = _this->_CTL_register + OFS_TxCCR0 + (CCRx * 2);

        // clear possible interrupt flag and capture overflow flag, capture mode, no capture
        hw_register_16(handle->_CCTLn_register) = (hw_register_16(handle->_CCTLn_register) & ~(CCIFG | COV | CM)) | CAP;
        // compare mode by default - since timer started in capture mode with no capture, interrupt never triggers
        handle->capture_mode = false;
        // enable vector.trigger() functionality
        vector_set_enabled(handle, true);
        // reset capture / compare value
        hw_register_16(handle->_CCRn_register) = 0;

        handle->set_capture_mode = _set_capture_mode;
        handle->is_capture_overflow_set = _is_capture_overflow_set;
        handle->set_compare_mode = _set_compare_mode;
        handle->get_capture_value = _get_capture_value;
        handle->set_compare_value = _set_compare_value;
    }
    else {
        handle->set_capture_mode = (void (*)(Timer_channel_handle_t *, uint16_t, uint16_t, uint16_t)) _unsupported_operation;
        handle->is_capture_overflow_set = (bool (*)(Timer_channel_handle_t *)) _unsupported_operation;
        handle->set_compare_mode = (void (*)(Timer_channel_handle_t *, uint16_t)) _unsupported_operation;
        handle->get_capture_value = (uint16_t (*)(Timer_channel_handle_t *)) _unsupported_operation;
        handle->set_compare_value = (void (*)(Timer_channel_handle_t *, uint16_t)) _unsupported_operation;
    }

    return TIMER_OK;
}

// -------------------------------------------------------------------------------------

// Vector_handle_t destructor
static dispose_function_t _timer_driver_dispose(Timer_driver_t *_this) {
    uint8_t CCRx;
    Timer_channel_handle_t **handle_ref = &_this->_CCR0_handle;

    // timer stop, clear interrupt flag
    hw_register_16(_this->_CTL_register + OFS_TxCTL) &= ~(TASSEL | ID | MC | TAIE | TAIFG);

    dispose(_this->_slot);
    dispose(_this->_overflow_handle);

    for (CCRx = 0; CCRx < _this->_available_handles_cnt; CCRx++, handle_ref++) {
        dispose(*handle_ref);
    }

    // reset by 16-bit access (zerofill is 8-bit) so that _CTL_register is either set or not set but never half set
    _this->_CTL_register = NULL;

    zerofill(_this);

    return NULL;
}

// Timer_driver_t constructor
void timer_driver_register(Timer_driver_t *driver, Timer_config_t *config, uint16_t base,
            uint8_t main_vector_no, uint8_t shared_vector_no, uint8_t available_handles_cnt) {

    zerofill(driver);

    // private
    driver->_CTL_register = base;
    driver->_main_vector_no = main_vector_no;
    driver->_shared_vector_no = shared_vector_no;
    driver->_IV_register = base + OFS_TxIV;
    driver->_mode = config->mode;
    driver->_available_handles_cnt = available_handles_cnt;

    // public
    driver->channel_handle_register = _channel_handle_register;

    // timer stop, clear interrupt flag
    hw_register_16(driver->_CTL_register) &= ~(TASSEL | ID | MC | TAIE | TAIFG);
    // clock source - divider, clear timer
    hw_register_16(driver->_CTL_register + OFS_TxCTL) |= config->clock_source | config->clock_source_divider | TACLR;
#ifdef _TIMER_HAS_IDEX_
    // input divider expansion
    hw_register_16(driver->_CTL_register + OFS_TAxEX0) = config->clock_source_divider_expansion;
#endif

    __dispose_hook_register(driver, _timer_driver_dispose);
}
