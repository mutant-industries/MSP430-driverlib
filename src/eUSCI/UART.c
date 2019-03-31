// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/eUSCI/UART.h>
#include <stddef.h>

// -------------------------------------------------------------------------------------

/**
 * compatibility defines
 */
#if ! defined(UCSSEL)
#define UCSSEL          (0x00c0)        /* eUSCI_A clock source select */
#endif
#if ! defined(UCMODE)
#define UCMODE          (0x0600)        /* eUSCI_A mode */
#endif
#if ! defined(UCPEN_1)
#define UCPEN_1         (0x8000)        /* Parity enabled */
#endif
#if ! defined(UCPAR__EVEN)
#define UCPAR__EVEN     (0x4000)        /* Even parity */
#endif
#if ! defined(UC7BIT__8BIT)
#define UC7BIT__8BIT    (0x0000)        /* 8-bit data */
#endif

// -------------------------------------------------------------------------------------

static void _uart_vector_handler(UART_driver_t *driver) {
    uint8_t interrupt_handler_index;
    uint16_t interrupt_source;
    uart_event_handler_t handler;

    if ( ! (interrupt_source = UART_IV_reg(driver))) {
        return;
    }

    // IV -> event (0x00 - no interrupt, 0x02 - UCRXIFG, 0x04 - UCTXIFG, 0x06 - UCSTTIFG, 0x08 - UCTXCPTIFG)
    interrupt_handler_index = (uint8_t) (interrupt_source / 2 - 1);

    if ((handler = (&driver->_on_character_received)[interrupt_handler_index]) == NULL) {
        // clear interrupt enable bit since there is no handler registered
        UART_interrupt_disable(driver, 1 << interrupt_handler_index);

        return;
    }

    // execute handler with given owner and handler argument
    handler(UART_owner(driver), UART_event_arg(driver));
}

// -------------------------------------------------------------------------------------

static uint8_t _unsupported_operation() {
    return UART_UNSUPPORTED_OPERATION;
}

static uint8_t _set_baudrate_config(UART_driver_t *_this, UART_baudrate_config_t *config) {
    UART_reset_enable(_this);

    // clock source
    UART_control_reg(_this) = (UART_control_reg(_this) & ~(UCSSEL)) | config->clock_source;
    // clock pre-scaler (BRW register)
    UART_baudrate_control_reg(_this) = config->clock_prescaler;
    // modulation and oversampling (MCTLW register)
    UART_modulation_control_reg(_this) =
            ((config->second_modulation_stage << 8) | (config->first_modulation_stage << 4) | ((uint8_t) config->oversampling));

    return UART_OK;
}

static uint8_t _set_transfer_config(UART_driver_t *_this, uint16_t mode, UART_transfer_config_t *config) {
    uint16_t control;

    UART_reset_enable(_this);

    // set requested mode
    control = (UART_control_reg(_this) & ~(UCMODE)) | mode;

    if (config) {
        // clear configurable bits if config set
        control &= ~(UCPEN | UCPAR | UCMSB | UC7BIT | UCSPB);
        // set transfer control
        control |= config->parity_enable | config->parity_select | config->receive_direction | config->character_length | config->stop_bit_select;
    }

    UART_control_reg(_this) = control;

    return UART_OK;
}

static uint8_t _set_loopback(UART_driver_t *_this, bool enabled) {
    UART_reset_enable(_this);

    // set / reset STATW.UCLISTEN bit
    UART_status_reg(_this) = (UART_status_reg(_this) & ~UCLISTEN) | (enabled ? UCLISTEN : 0);

    return UART_OK;
}

#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__

static uint8_t _set_auto_baudrate_detection(UART_driver_t *_this, bool enabled, uint8_t delimiter) {
    UART_reset_enable(_this);

    // set / reset enable bit, set break / sync delimiter length (ABCTL register)
    UART_auto_baudrate_control_reg(_this) = (UART_auto_baudrate_control_reg(_this) & ~(UCABDEN | UCDELIM))
            | (enabled ? UCABDEN_1 : UCABDEN_0) | delimiter;

    return UART_OK;
}

#endif

#ifdef __UART_IrDA_CONTROL_ENABLE__

static uint8_t _set_irda_control(UART_driver_t *_this, bool enabled, UART_IrDA_config_t *config) {
    uint16_t control;

    UART_reset_enable(_this);

    // set / reset enable bit
    control = (UART_IrDA_control_reg(_this) & ~(UCIREN)) | (enabled ? UCIREN_1 : UCIREN_0);

    if (config) {
        // clear configurable bits if config set
        control &= ~(UCIRTXCLK | UCIRTXPL | UCIRRXFE | UCIRRXPL | UCIRRXFL);
        // set IrDA control
        control |= config->transmit_pulse_clock | config->transmit_pulse_length | config->receive_filter_enabled
                | config->receive_input_polarity | config->receive_filter_length;
    }

    UART_IrDA_control_reg(_this) = control;

    return UART_OK;
}

#endif

// -------------------------------------------------------------------------------------

// UART_driver_t destructor
static dispose_function_t _uart_driver_dispose(UART_driver_t *_this) {
    // UART software reset
    UART_halt(_this);

    _this->set_baudrate_config = (uint8_t (*)(UART_driver_t *, UART_baudrate_config_t *)) _unsupported_operation;
    _this->set_transfer_config = (uint8_t (*)(UART_driver_t *, uint16_t, UART_transfer_config_t *)) _unsupported_operation;
    _this->set_loopback = (uint8_t (*)(UART_driver_t *, bool)) _unsupported_operation;
#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__
    _this->set_auto_baudrate_detection = (uint8_t (*)(UART_driver_t *, bool, uint8_t)) _unsupported_operation;
#endif
#ifdef __UART_IrDA_CONTROL_ENABLE__
    _this->set_IrDA_control = (uint8_t (*)(UART_driver_t *, bool, UART_IrDA_config_t *)) _unsupported_operation;
#endif

    return NULL;
}

// UART_driver_t constructor
void UART_driver_register(UART_driver_t *driver, uint16_t base, uint8_t vector_no) {

    // parent constructor
    EUSCI_driver_register(_EUSCI_driver_(driver), base , A, vector_no, (dispose_function_t) _uart_driver_dispose);

    // default UART mode, even parity, LSB first, 8-bit data, one stop bit
    UART_control_reg(driver) = UCPEN_1 | UCPAR__EVEN | UC7BIT__8BIT;
    // disable loopback if set
    UART_status_reg(driver) &= ~UCLISTEN;

    // register vector service handler
    vector_register_handler(driver, _uart_vector_handler, driver, NULL);

    // public
    driver->set_baudrate_config = _set_baudrate_config;
    driver->set_transfer_config = _set_transfer_config;
    driver->set_loopback = _set_loopback;
#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__
    driver->set_auto_baudrate_detection = _set_auto_baudrate_detection;
#endif
#ifdef __UART_IrDA_CONTROL_ENABLE__
    driver->set_IrDA_control = _set_irda_control;
#endif

    driver->_on_character_received = NULL;
    driver->_on_transmit_buffer_empty = NULL;
    driver->_on_start_bit_received = NULL;
    driver->_on_transmit_complete = NULL;
}
