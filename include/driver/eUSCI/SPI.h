/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  eUSCI (enhanced Universal Serial Communication Interface) SPI driver
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_EUSCI_SPI_H_
#define _DRIVER_EUSCI_SPI_H_

#include <driver/eUSCI.h>
#include <stdbool.h>

// -------------------------------------------------------------------------------------

/**
 * SPI_BASE(A, 2)           -> EUSCI_A2_BASE
 * SPI_A_BASE(1)            -> EUSCI_A1_BASE
 * SPI_B_BASE(INTERFACE_NO) -> EUSCI_B1_BASE when INTERFACE_NO defined as '1'
 */
#define SPI_BASE(type, no)      EUSCI_BASE(type, no)
#define SPI_A_BASE(no)          EUSCI_A_BASE(no)
#define SPI_B_BASE(no)          EUSCI_B_BASE(no)

/**
 * __attribute__((interrupt(SPI_VECTOR(type, no))))
 *   - SPI_VECTOR(A, 2)                 -> EUSCI_A2_VECTOR
 *   - SPI_VECTOR(B, 0)                 -> EUSCI_B0_VECTOR
 *   - SPI_VECTOR(B, INTERFACE_NO)      -> EUSCI_B1_VECTOR when INTERFACE_NO defined as '1'
 * __attribute__((interrupt(SPI_A_VECTOR(no))))
 *   - SPI_A_VECTOR(2)                  -> EUSCI_A2_VECTOR
 *   - SPI_A_VECTOR(INTERFACE_NO)       -> EUSCI_A1_VECTOR when INTERFACE_NO defined as '1'
 * __attribute__((interrupt(SPI_B_VECTOR(no))))
 *   - SPI_B_VECTOR(2)                  -> EUSCI_B2_VECTOR
 *   - SPI_B_VECTOR(INTERFACE_NO)       -> EUSCI_B1_VECTOR when INTERFACE_NO defined as '1'
 */
#define SPI_VECTOR(type, no)    EUSCI_VECTOR(type, no)
#define SPI_A_VECTOR(no)  EUSCI_A_VECTOR(no)
#define SPI_B_VECTOR(no)  EUSCI_B_VECTOR(no)

// -------------------------------------------------------------------------------------

#define _SPI_driver_(_driver)               ((SPI_driver_t *) (_driver))
#define SPI_event_handler(_handler)         ((spi_event_handler_t) (_handler))

/**
 * SPI driver public API access
 */
#define SPI_set_bitrate_config(_driver, _config) \
    _SPI_driver_(_driver)->set_bitrate_config(_SPI_driver_(_driver), _config)
#define SPI_set_transfer_config(_driver, _mode, _config) \
    _SPI_driver_(_driver)->set_transfer_config(_SPI_driver_(_driver), _mode, _config)
#define SPI_set_loopback(_driver, _enabled) \
    _SPI_driver_(_driver)->set_loopback(_SPI_driver_(_driver), _enabled)

// direct control register access
#define SPI_control_reg(_driver) EUSCI_control_reg(_driver)
#define SPI_bitrate_control_reg(_driver) EUSCI_bitrate_control_reg(_driver)
#define SPI_status_reg(_driver) EUSCI_status_reg(_driver)
#define SPI_RX_buffer(_driver) EUSCI_RX_buffer(_driver)
#define SPI_TX_buffer(_driver) EUSCI_TX_buffer(_driver)
#define SPI_IE_reg(_driver) EUSCI_IE_reg(_driver)
#define SPI_IFG_reg(_driver) EUSCI_IFG_reg(_driver)
#define SPI_IV_reg(_driver) EUSCI_IV_reg(_driver)
// RX / TX buffer address for DMA channel control
#define SPI_RX_buffer_address(_driver) EUSCI_RX_buffer_address(_driver)
#define SPI_TX_buffer_address(_driver) EUSCI_TX_buffer_address(_driver)

// software reset
#define SPI_halt(_driver) EUSCI_reset_enable(_driver)
#define SPI_reset_enable(_driver) EUSCI_reset_enable(_driver)
#define SPI_reset_disable(_driver) EUSCI_reset_disable(_driver)
// status flags
#define SPI_is_busy(_driver) ((bool) (SPI_status_reg(_driver) & UCBUSY))
// status flags to be used when interrupts are not used
#define SPI_is_RX_buffer_full(_driver) ((bool) (SPI_IFG_reg(_driver) & UCRXIFG))
#define SPI_is_TX_buffer_empty(_driver) ((bool) (SPI_IFG_reg(_driver) & UCTXIFG))
// interrupt control
#define SPI_interrupt_enable(_driver, _mask) EUSCI_interrupt_enable(_driver, _mask)
#define SPI_interrupt_disable(_driver, _mask) EUSCI_interrupt_disable(_driver, _mask)

// getter, setter
#define SPI_on_character_received(_driver) _SPI_driver_(_driver)->_on_character_received
#define SPI_on_transmit_buffer_empty(_driver) _SPI_driver_(_driver)->_on_transmit_buffer_empty
#define SPI_owner(_driver) EUSCI_owner(_driver)
#define SPI_event_arg(_driver) EUSCI_event_arg(_driver)

/**
 * SPI driver public API return codes
 */
#define SPI_OK                          EUSCI_OK
#define SPI_UNSUPPORTED_OPERATION       EUSCI_UNSUPPORTED_OPERATION

// -------------------------------------------------------------------------------------

/**
 * SPI clock select and bitrate config
 */
typedef struct SPI_bitrate_config {
    // UCSSEL__UCLK|UCSSEL__ACLK|UCSSEL__SMCLK
    uint8_t clock_source;
    // clock prescaler setting of the bitrate generator
    uint16_t clock_prescaler;

} SPI_bitrate_config_t;

/**
 * SPI transfer mode config
 */
typedef struct SPI_transfer_config {
    // UCCKPH_0 (data is changed on the first UCLK edge and captured on the following edge)
    // UCCKPH_1 (data is captured on the first UCLK edge and changed on the following edge)
    uint16_t clock_phase;
    // UCCKPL__LOW (the inactive state is low)|UCCKPL__HIGH (the inactive state is high)
    uint16_t cock_polarity;
    // UCMSB_0 (LSB first)|UCMSB_1 (MSB first)
    uint16_t receive_direction;
    // UC7BIT__8BIT|UC7BIT__7BIT
    uint16_t character_length;
    // UCMST_0 (slave mode)|UCMST_1 (master mode)
    uint16_t master_mode;
    // UCSTEM_0 (STE pin is used to prevent conflicts with other masters)
    // UCSTEM_1 (STE pin is used to generate the enable signal for a 4-wire slave)
    uint16_t STE_mode;

} SPI_transfer_config_t;

// -------------------------------------------------------------------------------------

typedef struct SPI_driver SPI_driver_t;
typedef eusci_event_handler_t spi_event_handler_t;

struct SPI_driver {
    // eUSCI driver inherit, enable dispose(SPI_driver_t *)
    EUSCI_driver_t eusci;
    // configure input clock and baudrate (SW reset shall be set)
    uint8_t (*set_bitrate_config)(SPI_driver_t *_this, SPI_bitrate_config_t *config);
    // configure SPI mode with optional transfer config (SW reset shall be set)
    //  - mode:
    //      UCMODE_0 (3-pin SPI)
    //      UCMODE_1 (4-pin SPI with UCxSTE active high: Slave enabled when UCxSTE = 1)
    //      UCMODE_2 (4-pin SPI with UCxSTE active low: Slave enabled when UCxSTE = 0)
    uint8_t (*set_transfer_config)(SPI_driver_t *_this, uint16_t mode, SPI_transfer_config_t *config);
    // configure SPI loopback mode (SW reset shall be set)
    uint8_t (*set_loopback)(SPI_driver_t *_this, bool enabled);

    // interrupt service handlers
    spi_event_handler_t _on_character_received;
    spi_event_handler_t _on_transmit_buffer_empty;

};

// -------------------------------------------------------------------------------------

void SPI_driver_register(SPI_driver_t *driver, uint16_t base, EUSCI_type type, uint8_t vector_no);


#endif /* _DRIVER_EUSCI_SPI_H_ */
