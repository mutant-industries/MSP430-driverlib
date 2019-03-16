/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  DMA controller driver for MSP430 F5xx_6xx, FR5xx_6xx and later devices
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_DMA_H_
#define _DRIVER_DMA_H_

#include <msp430.h>
#include <stdint.h>
#include <driver/cpu.h>
#include <driver/disposable.h>
#include <driver/vector.h>

// -------------------------------------------------------------------------------------

/**
 * Total DMA channel count
 */
#ifndef __DMA_CONTROLLER_CHANNEL_COUNT__

#ifdef __MSP430_HAS_DMA__
#define __DMA_CONTROLLER_CHANNEL_COUNT__        __MSP430_HAS_DMA__
#else
#define __DMA_CONTROLLER_CHANNEL_COUNT__        6
#endif

#endif

/**
 * Source address, destination address and size registers offsets from channel CTL register
 */
#if ! defined(OFS_DMAxSA) && ! defined(OFS_DMAxDA) && ! defined(OFS_DMAxSZ)
#define OFS_DMAxSA      0x02
#define OFS_DMAxDA      0x06
#define OFS_DMAxSZ      0x0A
#endif

/**
 * Offset of channel control register from base
 *  - _DMA_CHANNEL_CTL_OFFSET_(2)             -> OFS_DMA2CTL
 *  - _DMA_CHANNEL_CTL_OFFSET_(CHANNEL_NO)    -> OFS_DMA1CTL when CHANNEL_NO defined as '1'
 */
#define _DMA_CHANNEL_CTL_OFFSET_(NO)      _DMA_CHANNEL_CTL_OFFSET_EX_(NO)
// concatenation of expanded parameter
#define _DMA_CHANNEL_CTL_OFFSET_EX_(NO)   OFS_DMA## NO ##CTL

// -------------------------------------------------------------------------------------

#define _DMA_driver_(_driver)                   ((DMA_driver_t *) (_driver))
#define _DMA_channel_handle_(_handle)           ((DMA_channel_handle_t *) (_handle))

// -------------------------------------------------------------------------------------

/**
 * DMA driver public API access
 */
#define DMA_driver_channel_register(_driver, _handle, _channel_index) \
    _DMA_driver_(_driver)->channel_handle_register(_DMA_driver_(_driver), _DMA_channel_handle_(_handle), _channel_index, _DMA_CHANNEL_CTL_OFFSET_(_channel_index))
// ENNMI[_0], ROUNDROBIN[_0], DMARMWDIS[_0]
#define DMA_driver_set_control(_driver, _enable_NMI, _round_robin_priority, read_modify_write_disable) \
    hw_register_8(_DMA_driver_(_driver)->_control_register) = (_enable_NMI | _round_robin_priority | read_modify_write_disable)

#define DMA_channel_set_enabled(_handle, _enabled) \
    _DMA_channel_handle_(_handle)->set_enabled(_DMA_channel_handle_(_handle), _enabled)
#define DMA_channel_select_trigger(_handle, _trigger) \
    _DMA_channel_handle_(_handle)->select_trigger(_DMA_channel_handle_(_handle), _trigger)
#define DMA_channel_set_control(_handle, _dma_level, _src_type, _dst_type, _src_increment, _dst_increment, _transfer_mode) \
    _DMA_channel_handle_(_handle)->set_control(_DMA_channel_handle_(_handle), _dma_level, _src_type, _dst_type, _src_increment, _dst_increment, _transfer_mode)
#define DMA_channel_is_abort_set(_handle) \
    _DMA_channel_handle_(_handle)->is_abort_set(_DMA_channel_handle_(_handle))
#define DMA_channel_request(_handle) \
    hw_register_16(_DMA_channel_handle_(_handle)->_CTL_register) |= DMAREQ
#define DMA_channel_request_cancel(_handle) \
    hw_register_16(_DMA_channel_handle_(_handle)->_CTL_register) &= ~DMAREQ
#define DMA_channel_source_address(_handle) \
    hw_register_addr(_DMA_channel_handle_(_handle)->_CTL_register + OFS_DMAxSA)
#define DMA_channel_destination_address(_handle) \
    hw_register_addr(_DMA_channel_handle_(_handle)->_CTL_register + OFS_DMAxDA)
#define DMA_channel_size(_handle) \
    hw_register_16(_DMA_channel_handle_(_handle)->_CTL_register + OFS_DMAxSZ)

/**
 * DMA driver public API return codes
 */
#define DMA_OK                              (0x00)
#define DMA_UNSUPPORTED_OPERATION           (0x20)
#define DMA_CHANNEL_REGISTERED_ALREADY      (0x21)

// -------------------------------------------------------------------------------------

typedef struct DMA_driver DMA_driver_t;
typedef struct DMA_channel_handle DMA_channel_handle_t;

/**
 * DMA driver control
 */
struct DMA_driver {
    // enable dispose(DMA_driver_t *)
    Disposable_t _disposable;
    // base of DMA registers, (address of corresponding DMACTL0 register)
    uint16_t _base;
    // driver control register (address of corresponding DMACTL4 register on x5xx_x6xx devices)
    uint16_t _control_register;
    // DMA interrupt vector number
    uint8_t _vector_no;
    // interrupt vector register
    uint16_t _IV_register;

    // -------- state --------
    // shared vector slot
    Vector_slot_t *_slot;

#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 0
    DMA_channel_handle_t *_channel0_handle;
#endif
#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 1
    DMA_channel_handle_t *_channel1_handle;
#endif
#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 2
    DMA_channel_handle_t *_channel2_handle;
#endif
#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 3
    DMA_channel_handle_t *_channel3_handle;
#endif
#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 4
    DMA_channel_handle_t *_channel4_handle;
#endif
#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 5
    DMA_channel_handle_t *_channel5_handle;
#endif
#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 6
    DMA_channel_handle_t *_channel6_handle;
#endif
#if __DMA_CONTROLLER_CHANNEL_COUNT__ > 7
    DMA_channel_handle_t *_channel7_handle;
#endif

    // -------- public --------
    // register handle for given channel, where ctl_offset is offset of corresponding DMAxCTL register from DMA_BASE
    uint8_t (*channel_handle_register)(DMA_driver_t *_this, DMA_channel_handle_t *handle, uint8_t channel_no, uint16_t ctl_offset);

};

/**
 * Single DMA channel wrapper
 */
struct DMA_channel_handle {
    // vector wrapper, enable dispose(DMA_channel_handle_t *)
    Vector_handle_t vector;
    // channel control register
    uint16_t _CTL_register;
    // trigger select register
    uint16_t _TSEL_register;
    // channel index (0 - 7)
    uint8_t _channel_index;
    // DMA driver reference
    DMA_driver_t *_driver;
    // backup of original Vector_handle_t.register_handler
    Vector_slot_t *(*_register_handler_parent)(Vector_handle_t *_this, vector_slot_handler_t, void *, void *);

    // -------- state --------
    // vector interrupt service handler
    vector_slot_handler_t _handler;
    // vector interrupt service handler arguments
    void *_handler_arg_1;
    void *_handler_arg_2;

    // -------- public --------
    // DMAEN flag on DMA channel setter
    uint8_t (*set_enabled)(DMA_channel_handle_t *_this, bool enabled);
    // DMA transfer trigger setter
    //  - trigger:
    //      DMA5TSEL__DMAREQ
    //      DMA5TSEL__TA0CCR0
    //      DMA5TSEL__TA0CCR2
    //      DMA5TSEL__TA1CCR0
    //      DMA5TSEL__TA1CCR2
    //      ...
    uint8_t (*select_trigger)(DMA_channel_handle_t *_this, uint8_t trigger);
    // DMA control
    // params:
    //  - dma level: DMALEVEL__EDGE|DMALEVEL__LEVEL
    //  - src type: DMASRCBYTE__WORD|DMASRCBYTE__BYTE
    //  - dst type: DMADSTBYTE__WORD|DMADSTBYTE__BYTE
    //  - src increment: DMASRCINCR_0|DMASRCINCR_2 (src address is decremented)|DMASRCINCR_3 (src address is incremented)
    //  - dst increment: DMADSTINCR_0|DMADSTINCR_2 (dst address is decremented)|DMADSTINCR_3 (dst address is incremented)
    //  - transfer mode:
    //      DMADT_0 (Single transfer)
    //      DMADT_1 (Block transfer)
    //      DMADT_2 (Burst-block transfer)
    //      DMADT_4 (Repeated single transfer)
    //      DMADT_5 (Repeated block transfer)
    //      DMADT_6 (Repeated burst-block transfer)
    uint8_t (*set_control)(DMA_channel_handle_t *_this, uint16_t dma_level, uint16_t src_type, uint16_t dst_type,
            uint16_t src_increment, uint16_t dst_increment, uint16_t transfer_mode);
    // read and reset DMAABORT
    bool (*is_abort_set)(DMA_channel_handle_t *_this);

};

// -------------------------------------------------------------------------------------

void DMA_driver_register(DMA_driver_t *driver);


#endif /* _DRIVER_DMA_H_ */
