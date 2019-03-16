// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/DMA.h>
#include <stddef.h>
#include <driver/interrupt.h>


/**
 * DMA controller support check, required:
 *  - DMA_BASE - address od DMACTL0
 *  - OFS_DMAIV - offset of IV register from base (DMAIV - DMA_BASE)
 *  - DMA_VECTOR - number of DMA interrupt vector
 *  - OFS_DMACTL4 - offset of DMA config register (x2xx and x4xx are not supported)
 *  - OFS_DMAxSA, OFS_DMAxDA, OFS_DMAxSZ - offsets of DMA source / destination address and size registers from DMAxCTL
 */
#if defined(DMA_BASE) && defined(OFS_DMAIV) && defined(DMA_VECTOR) && defined(OFS_DMACTL4)

// -------------------------------------------------------------------------------------

static uint8_t _unsupported_operation() {
    return DMA_UNSUPPORTED_OPERATION;
}

// -------------------------------------------------------------------------------------

static uint8_t _set_enabled(DMA_channel_handle_t *_this, bool enabled) {

    hw_register_16(_this->_CTL_register) = (hw_register_16(_this->_CTL_register) & ~DMAEN) | (enabled ? DMAEN_1 : DMAEN_0);

    return DMA_OK;
}

static uint8_t _select_trigger(DMA_channel_handle_t *_this, uint8_t trigger) {

    // DMAxTSEL bits should be modified only when the DMAEN bit is 0 (otherwise, unpredictable DMA triggers may occur)
    _set_enabled(_this, false);

    hw_register_8(_this->_TSEL_register) = trigger;

    return DMA_OK;
}

static uint8_t _set_control(DMA_channel_handle_t *_this, uint16_t dma_level, uint16_t src_type, uint16_t dst_type,
        uint16_t src_increment, uint16_t dst_increment, uint16_t transfer_mode) {

    // set disabled, reset REQ and ABORT, persist IE and IFG, set requested control flags
    hw_register_16(_this->_CTL_register) = (hw_register_16(_this->_CTL_register) & (DMAIE | DMAIFG)) |
            (dma_level | src_type | dst_type | src_increment | dst_increment | transfer_mode);

    return DMA_OK;
}

static bool _is_abort_set(DMA_channel_handle_t *_this) {
    bool abort_set;

    if ((abort_set = (bool) (hw_register_16(_this->_CTL_register) & DMAABORT))) {
        hw_register_16(_this->_CTL_register) &= ~DMAABORT;
    }

    return abort_set;
}

// -------------------------------------------------------------------------------------

static void _shared_vector_handler(DMA_driver_t *driver) {
    uint8_t interrupt_channel_index;
    uint16_t interrupt_source;
    DMA_channel_handle_t *handle;

    if ( ! (interrupt_source = hw_register_16(driver->_IV_register))) {
        return;
    }

    // IV -> channel number (0x00 - no interrupt, 0x02 - DMA0IFG interrupt, 0x04 - DMA1IFG interrupt...)
    interrupt_channel_index = (uint8_t) (interrupt_source / 2 - 1);

    handle = ((DMA_channel_handle_t **) &driver->_channel0_handle)[interrupt_channel_index];

    // execute handler with given handler arguments
    handle->_handler(handle->_handler_arg_1, handle->_handler_arg_1);
}

static Vector_slot_t *_register_handler_shared(DMA_channel_handle_t *_this, vector_slot_handler_t handler, void *arg_1, void *arg_2) {

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

// DMA_channel_handle_t destructor
static dispose_function_t _dma_channel_handle_dispose(DMA_channel_handle_t *_this) {

    // register reset
    _this->set_enabled(_this, false);

    // reset driver -> handle reference
    (&_this->_driver->_channel0_handle)[_this->_channel_index] = NULL;
    // reset handle -> driver reference
    _this->_driver = NULL;

    _this->_handler = NULL;
    _this->_handler_arg_1 = NULL;
    _this->_handler_arg_2 = NULL;

    // reset state of control registers
    _this->select_trigger(_this, DMA0TSEL__DMAREQ);
    _this->set_control(_this, DMALEVEL__EDGE, DMASRCBYTE__WORD, DMADSTBYTE__WORD, DMASRCINCR_0, DMADSTINCR_0, DMADT_0);

    _this->set_enabled = (uint8_t (*)(DMA_channel_handle_t *, bool)) _unsupported_operation;
    _this->select_trigger = (uint8_t (*)(DMA_channel_handle_t *, uint8_t)) _unsupported_operation;
    _this->set_control = (uint8_t (*)(DMA_channel_handle_t *, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t)) _unsupported_operation;
    _this->is_abort_set = (bool (*)(DMA_channel_handle_t *)) _unsupported_operation;

    return NULL;
}

// DMA_channel_handle_t constructor
static uint8_t _dma_channel_handle_register(DMA_driver_t *_this, DMA_channel_handle_t *handle, uint8_t channel_index, uint16_t ctl_offset) {
    DMA_channel_handle_t **handle_ref;

    handle->_CTL_register = _this->_base + ctl_offset;
    // TSEL 8-bit access, DMA0TSEL - DMA0TSEL_0 (DMA_BASE + 0), DMA1TSEL - DMA0TSEL_1 (DMA_BASE + 1)...
    handle->_TSEL_register = _this->_base + channel_index;
    handle->_channel_index = channel_index;

    interrupt_suspend();

    handle_ref = &_this->_channel0_handle;

    // check whether handle for given channel is registered already
    if (handle_ref[channel_index]) {
        interrupt_restore();
        // current channel is already registered for another handle
        return DMA_CHANNEL_REGISTERED_ALREADY;
    }

    handle_ref[channel_index] = handle;

    // reset SW transfer request flag, NMI abort flag
    hw_register_16(handle->_CTL_register) &= ~(DMAREQ | DMAABORT);

    interrupt_restore();

    // handle->driver reference
    handle->_driver = _this;

    vector_handle_register(&handle->vector, (dispose_function_t) _dma_channel_handle_dispose, _this->_vector_no,
            handle->_CTL_register, DMAIE, handle->_CTL_register, DMAIFG);

    handle->_handler = NULL;
    handle->_handler_arg_1 = NULL;
    handle->_handler_arg_2 = NULL;

    // disable assignment of raw handler to shared vector
    handle->vector.register_raw_handler = (uint8_t (*)(Vector_handle_t *, interrupt_service_t, bool)) _unsupported_operation;
    // override default register_handler on vector handle
    handle->_register_handler_parent = handle->vector.register_handler;
    handle->vector.register_handler = (Vector_slot_t *(*)(Vector_handle_t *,
            vector_slot_handler_t, void *, void *)) _register_handler_shared;

    // public
    handle->set_enabled = _set_enabled;
    handle->select_trigger = _select_trigger;
    handle->set_control = _set_control;
    handle->is_abort_set = _is_abort_set;

    return DMA_OK;
}

// -------------------------------------------------------------------------------------

// DMA_driver_t destructor
static dispose_function_t _dma_driver_dispose(DMA_driver_t *_this) {

    uint8_t handle;
    DMA_channel_handle_t **handle_ref = &_this->_channel0_handle;

    // register new handles is now disabled
    _this->channel_handle_register = (uint8_t (*)(DMA_driver_t *, DMA_channel_handle_t *, uint8_t, uint16_t)) _unsupported_operation;

    // restore original vector content
    dispose(_this->_slot);

    for (handle = 0; handle < __DMA_CONTROLLER_CHANNEL_COUNT__; handle++, handle_ref++) {
        dispose(*handle_ref);
    }

    // reset control register state
    DMA_driver_set_control(_this, ENNMI_0, ROUNDROBIN_0, DMARMWDIS_0);

    _this->_base = NULL;
    _this->_control_register = NULL;

    return NULL;
}

// DMA_driver_t constructor
void DMA_driver_register(DMA_driver_t *driver) {

    zerofill(driver);

    driver->_base = DMA_BASE;
    driver->_control_register = DMA_BASE + OFS_DMACTL4;
    driver->_IV_register = driver->_base + OFS_DMAIV;
    driver->_vector_no = DMA_VECTOR;

    // public
    driver->channel_handle_register = _dma_channel_handle_register;

    __dispose_hook_register(driver, _dma_driver_dispose);
}

#endif /* DMA controller support check */
