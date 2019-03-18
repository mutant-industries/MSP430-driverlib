// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/eUSCI/SPI.h>
#include <stddef.h>

// modulation is not used in SPI mode, and UCAxMCTL should be cleared when using SPI mode for eUSCI_A
#if ! defined(OFS_UCA0MCTLW)
#define OFS_UCA0MCTLW   (0x0008)
#endif

// -------------------------------------------------------------------------------------

static void _spi_vector_handler(SPI_driver_t *driver) {
    uint8_t interrupt_handler_index;
    uint16_t interrupt_source;
    spi_event_handler_t handler;

    if ( ! (interrupt_source = SPI_IV_reg(driver))) {
        return;
    }

    // IV -> event (0x00 - no interrupt, 0x02 - UCRXIFG, 0x04 - UCTXIFG)
    interrupt_handler_index = (uint8_t) (interrupt_source / 2 - 1);

    if ((handler = (&driver->_on_character_received)[interrupt_handler_index]) == NULL) {
        // clear interrupt enable bit since there is no handler registered
        SPI_interrupt_disable(driver, 1 << interrupt_handler_index);

        return;
    }

    // execute handler with given owner and handler argument
    handler(SPI_owner(driver), SPI_event_arg(driver));
}

// -------------------------------------------------------------------------------------

static uint8_t _unsupported_operation() {
    return SPI_UNSUPPORTED_OPERATION;
}

static uint8_t _set_bitrate_config(SPI_driver_t *_this, SPI_bitrate_config_t *config) {
    SPI_reset_enable(_this);

    // clock source
    SPI_control_reg(_this) = (SPI_control_reg(_this) & ~(UCSSEL)) | config->clock_source;
    // clock pre-scaler (BRW register)
    SPI_bitrate_control_reg(_this) = config->clock_prescaler;

    return SPI_OK;
}

static uint8_t _set_transfer_config(SPI_driver_t *_this, uint16_t mode, SPI_transfer_config_t *config) {
    uint16_t control;

    SPI_reset_enable(_this);

    // set requested mode
    control = (SPI_control_reg(_this) & ~(UCMODE)) | mode;

    if (config) {
        // clear configurable bits if config set
        control &= ~(UCCKPH | UCCKPL | UCMSB | UC7BIT | UCMST | UCSTEM);
        // set transfer control
        control |= config->clock_phase | config->cock_polarity | config->receive_direction
                    | config->character_length | config->master_mode | config->STE_mode;
    }

    SPI_control_reg(_this) = control;

    return SPI_OK;
}

static uint8_t _set_loopback(SPI_driver_t *_this, bool enabled) {
    SPI_reset_enable(_this);

    // set / reset STATW.UCLISTEN bit
    SPI_status_reg(_this) = (SPI_status_reg(_this) & ~UCLISTEN) | (enabled ? UCLISTEN_1 : UCLISTEN_0);

    return SPI_OK;
}

// -------------------------------------------------------------------------------------

// SPI_driver_t destructor
static dispose_function_t _spi_driver_dispose(SPI_driver_t *_this) {
    // SPI software reset
    SPI_halt(_this);

    _this->set_bitrate_config = (uint8_t (*)(SPI_driver_t *, SPI_bitrate_config_t *)) _unsupported_operation;
    _this->set_transfer_config = (uint8_t (*)(SPI_driver_t *, uint16_t, SPI_transfer_config_t *)) _unsupported_operation;
    _this->set_loopback = (uint8_t (*)(SPI_driver_t *, bool)) _unsupported_operation;

    return NULL;
}

// SPI_driver_t constructor
void SPI_driver_register(SPI_driver_t *driver, uint16_t base, EUSCI_type type, uint8_t vector_no) {

    // parent constructor
    EUSCI_driver_register(_EUSCI_driver_(driver), base, type, vector_no, (dispose_function_t) _spi_driver_dispose);

    // default 3-pin SPI
    SPI_control_reg(driver) = UCMODE_0 | UCSYNC_1;
    // disable loopback if set
    SPI_status_reg(driver) &= ~UCLISTEN;

    // clear modulation control register on eUSCI_A
    if (type == A) {
        _EUSCI_base_offset_reg_16(driver, OFS_UCA0MCTLW) = 0;
    }

    // register vector service handler
    vector_register_handler(driver, _spi_vector_handler, driver, NULL);

    // public
    driver->set_bitrate_config = _set_bitrate_config;
    driver->set_transfer_config = _set_transfer_config;
    driver->set_loopback = _set_loopback;

    driver->_on_character_received = NULL;
    driver->_on_transmit_buffer_empty = NULL;
}
