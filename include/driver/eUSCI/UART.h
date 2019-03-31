/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  eUSCI (enhanced Universal Serial Communication Interface) UART driver
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_EUSCI_UART_H_
#define _DRIVER_EUSCI_UART_H_

#include <driver/eUSCI.h>
#include <stdbool.h>

// -------------------------------------------------------------------------------------

/**
 * Deglitch time control register offset
 */
#if ! defined(OFS_UCA0CTLW1)
#define OFS_UCA0CTLW1   (0x0002)
#endif

/**
 * Modulation control register offset
 */
#if ! defined(OFS_UCA0MCTLW)
#define OFS_UCA0MCTLW   (0x0008)
#endif

/**
 * Auto baudrate control register offset
 */
#if ! defined(OFS_UCA0ABCTL)
#define OFS_UCA0ABCTL   (0x0010)
#endif

/**
 * IrDA control register offset
 */
#if ! defined(OFS_UCA0IRCTL)
#define OFS_UCA0IRCTL   (0x0012)
#endif

// -------------------------------------------------------------------------------------

/**
 * UART_BASE(2)              -> EUSCI_A2_BASE
 * UART_BASE(INTERFACE_NO)   -> EUSCI_A1_BASE when INTERFACE_NO defined as '1'
 */
#define UART_BASE(no)    EUSCI_A_BASE(no)

/**
 * __attribute__((interrupt(UART_VECTOR(no))))
 *   - UART_VECTOR(2)             -> EUSCI_A2_VECTOR
 *   - UART_VECTOR(INTERFACE_NO)  -> EUSCI_A1_VECTOR when INTERFACE_NO defined as '1'
 */
#define UART_VECTOR(no)   EUSCI_A_VECTOR(no)

// -------------------------------------------------------------------------------------

#define _UART_driver_(_driver)                ((UART_driver_t *) (_driver))
#define UART_event_handler(_handler)          ((uart_event_handler_t) (_handler))

/**
 * UART driver public API access
 */
#define UART_set_baudrate_config(_driver, _config) \
    _UART_driver_(_driver)->set_baudrate_config(_UART_driver_(_driver), _config)
#define UART_set_transfer_config(_driver, _mode, _config) \
    _UART_driver_(_driver)->set_transfer_config(_UART_driver_(_driver), _mode, _config)
#define UART_set_loopback(_driver, _enabled) \
    _UART_driver_(_driver)->set_loopback(_UART_driver_(_driver), _enabled)
#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__
#define UART_set_auto_baudrate_detection(_driver, _enabled, _delimiter) \
    _UART_driver_(_driver)->set_auto_baudrate_detection(_UART_driver_(_driver), _enabled, _delimiter)
#endif
#ifdef __UART_IrDA_CONTROL_ENABLE__
#define UART_set_IrDA_control(_driver, _enabled, _config) \
    _UART_driver_(_driver)->set_IrDA_control(_UART_driver_(_driver), _enabled, _config)
#endif

// direct control register access
#define UART_control_reg(_driver) EUSCI_control_reg(_driver)
#define UART_deglitch_control_reg(_driver) _EUSCI_base_offset_reg_16(_driver, OFS_UCA0CTLW1)
#define UART_baudrate_control_reg(_driver) EUSCI_bitrate_control_reg(_driver)
#define UART_modulation_control_reg(_driver) _EUSCI_base_offset_reg_16(_driver, OFS_UCA0MCTLW)
#define UART_status_reg(_driver) EUSCI_status_reg(_driver)
#define UART_RX_buffer(_driver) EUSCI_RX_buffer(_driver)
#define UART_TX_buffer(_driver) EUSCI_TX_buffer(_driver)
#define UART_auto_baudrate_control_reg(_driver) _EUSCI_base_offset_reg_16(_driver, OFS_UCA0ABCTL)
#define UART_IrDA_control_reg(_driver) _EUSCI_base_offset_reg_16(_driver, OFS_UCA0IRCTL)
#define UART_IE_reg(_driver) EUSCI_IE_reg(_driver)
#define UART_IFG_reg(_driver) EUSCI_IFG_reg(_driver)
#define UART_IV_reg(_driver) EUSCI_IV_reg(_driver)
// RX / TX buffer address for DMA channel control
#define UART_RX_buffer_address(_driver) EUSCI_RX_buffer_address(_driver)
#define UART_TX_buffer_address(_driver) EUSCI_TX_buffer_address(_driver)

// software reset
#define UART_halt(_driver) EUSCI_reset_enable(_driver)
#define UART_reset_enable(_driver) EUSCI_reset_enable(_driver)
#define UART_reset_disable(_driver) EUSCI_reset_disable(_driver)
// status flags
#define UART_is_busy(_driver) ((bool) (UART_status_reg(_driver) & UCBUSY))
#define UART_is_break_condition(_driver) ((bool) (UART_status_reg(_driver) & UCBRK))
// status flags to be used when interrupts are not used
#define UART_is_RX_buffer_full(_driver) ((bool) (UART_IFG_reg(_driver) & UCRXIFG))
#define UART_is_TX_buffer_empty(_driver) ((bool) (UART_IFG_reg(_driver) & UCTXIFG))
// interrupt control
#define UART_interrupt_enable(_driver, _mask) EUSCI_interrupt_enable(_driver, _mask)
#define UART_interrupt_disable(_driver, _mask) EUSCI_interrupt_disable(_driver, _mask)
#define UART_receive_erroneous_char_interrupt_enable(_driver) UART_control_reg(_driver) |= UCRXEIE
#define UART_receive_erroneous_char_interrupt_disable(_driver) UART_control_reg(_driver) &= ~UCRXEIE
#define UART_receive_break_char_interrupt_enable(_driver) UART_control_reg(_driver) |= UCBRKIE
#define UART_receive_break_char_interrupt_disable(_driver) UART_control_reg(_driver) &= ~UCBRKIE
// auto baudrate / multiprocessor line control
#define UART_dormant_set(_driver) UART_control_reg(_driver) |= UCDORM
#define UART_dormant_reset(_driver) UART_control_reg(_driver) &= ~UCDORM
#define UART_transmit_address_set(_driver) UART_control_reg(_driver) |= UCTXADDR
#define UART_transmit_break_set(_driver) UART_control_reg(_driver) |= UCTXBRK

// getter, setter
#define UART_on_character_received(_driver) _UART_driver_(_driver)->_on_character_received
#define UART_on_transmit_buffer_empty(_driver) _UART_driver_(_driver)->_on_transmit_buffer_empty
#define UART_on_start_bit_received(_driver) _UART_driver_(_driver)->_on_start_bit_received
#define UART_on_transmit_complete(_driver) _UART_driver_(_driver)->_on_transmit_complete
#define UART_owner(_driver) EUSCI_owner(_driver)
#define UART_event_arg(_driver) EUSCI_event_arg(_driver)

/**
 * UART driver public API return codes
 */
#define UART_OK                         EUSCI_OK
#define UART_UNSUPPORTED_OPERATION      EUSCI_UNSUPPORTED_OPERATION

// -------------------------------------------------------------------------------------

/**
 * UART clock select and baudrate config
 *  - {@see http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html}
 */
typedef struct UART_baudrate_config {
    // UCSSEL__UCLK|UCSSEL__ACLK|UCSSEL__SMCLK
    uint8_t clock_source;
    // clock prescaler setting of the baudrate generator
    uint16_t clock_prescaler;
    // these bits determine the modulation pattern for BITCLK16 when UCOS16 = 1, ignored with UCOS16 = 0
    uint8_t first_modulation_stage;
    // these bits hold a free modulation pattern for BITCLK
    uint8_t second_modulation_stage;
    // oversampling enable, UCOS16 bit
    bool oversampling;

} UART_baudrate_config_t;

/**
 * UART transfer mode config
 *  - by default UART mode and listed default values are configured
 */
typedef struct UART_transfer_config {
    // UCPEN_0|UCPEN_1, default [UCPEN_1]
    uint16_t parity_enable;
    // UCPAR__ODD|UCPAR__EVEN, default [UCPAR__EVEN]
    uint16_t parity_select;
    // UCMSB_0 (LSB first)|UCMSB_1 (MSB first), default [UCMSB_0]
    uint16_t receive_direction;
    // UC7BIT__8BIT|UC7BIT__7BIT, default [UC7BIT__8BIT]
    uint16_t character_length;
    // UCSPB_0 (one stop bit)|UCSPB_1 (two stop bits), default [UCSPB_0]
    uint16_t stop_bit_select;

} UART_transfer_config_t;

#ifdef __UART_IrDA_CONTROL_ENABLE__

/**
 * UART IrDA encoder / decoder config
 */
typedef struct UART_IrDA_config {
    // UCIRTXCLK_0 (BRCLK)|UCIRTXCLK_1 (BITCLK16 when UCOS16 = 1. Otherwise, BRCLK)
    uint16_t transmit_pulse_clock;
    // UCIRTXPL0|UCIRTXPL1|UCIRTXPL2|UCIRTXPL3|UCIRTXPL4|UCIRTXPL5
    uint16_t transmit_pulse_length;
    // UCIRRXFE_0 (receive filter disabled)|UCIRRXFE_1 (Receive filter enabled)
    uint16_t receive_filter_enabled;
    // UCIRRXPL__HIGH (IrDA transceiver delivers a high pulse when a light pulse is seen)|UCIRRXPL__LOW
    uint16_t receive_input_polarity;
    // UCIRRXFL0|UCIRRXFL1|UCIRRXFL2|UCIRRXFL3|UCIRRXFL4|UCIRRXFL5
    uint16_t receive_filter_length;

} UART_IrDA_config_t;

#endif

// -------------------------------------------------------------------------------------

typedef struct UART_driver UART_driver_t;
typedef eusci_event_handler_t uart_event_handler_t;

struct UART_driver {
    // eUSCI driver inherit, enable dispose(UART_driver_t *)
    EUSCI_driver_t eusci;
    // configure input clock and baudrate (SW reset shall be set)
    uint8_t (*set_baudrate_config)(UART_driver_t *_this, UART_baudrate_config_t *config);
    // configure UART mode with optional transfer config (SW reset shall be set)
    //  - mode:
    //      UCMODE_0 (UART mode)
    //      UCMODE_1 (idle-line multiprocessor mode)
    //      UCMODE_2 (address-bit multiprocessor mode)
    //      UCMODE_3 (UART mode with automatic baud-rate detection)
    uint8_t (*set_transfer_config)(UART_driver_t *_this, uint16_t mode, UART_transfer_config_t *config);
    // configure UART loopback mode (SW reset shall be set)
    uint8_t (*set_loopback)(UART_driver_t *_this, bool enabled);
#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__
    // configure auto baudrate detection (SW reset shall be set)
    //  - enable only manipulates UCABDEN bit in corresponding ABCTL register, UART mode is not changed
    //  - delimiter:
    //      UCDELIM_0 (1 bit time)
    //      UCDELIM_1 (2 bit times)
    //      UCDELIM_2 (3 bit times)
    //      UCDELIM_3 (4 bit times)
    uint8_t (*set_auto_baudrate_detection)(UART_driver_t *_this, bool enabled, uint8_t delimiter);
#endif
#ifdef __UART_IrDA_CONTROL_ENABLE__
    // configure IrDA control register (SW reset shall be set)
    //  - enable / disable IrDA encoder / decoder with optional config
    uint8_t (*set_IrDA_control)(UART_driver_t *_this, bool enabled, UART_IrDA_config_t *config);
#endif

    // interrupt service handlers
    uart_event_handler_t _on_character_received;
    uart_event_handler_t _on_transmit_buffer_empty;
    uart_event_handler_t _on_start_bit_received;
    uart_event_handler_t _on_transmit_complete;

};

// -------------------------------------------------------------------------------------

void UART_driver_register(UART_driver_t *driver, uint16_t base, uint8_t vector_no);


#endif /* _DRIVER_EUSCI_UART_H_ */
