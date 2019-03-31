/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  eUSCI (enhanced Universal Serial Communication Interface) generic driver
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_EUSCI_H_
#define _DRIVER_EUSCI_H_

#include <msp430.h>
#include <stdint.h>
#include <driver/cpu.h>
#include <driver/disposable.h>
#include <driver/vector.h>

// -------------------------------------------------------------------------------------

/**
 * Control word, bit rate control and RX / TX buffers
 *  - constant offset from base for both type A and B
 */
#if defined(OFS_UCAxCTLW0) && defined(OFS_UCAxBRW) && defined(OFS_UCAxRXBUF) && defined(OFS_UCAxTXBUF)
#define OFS_UCxCTLW0    OFS_UCAxCTLW0
#define OFS_UCxBRW      OFS_UCAxBRW
#define OFS_UCxRXBUF    OFS_UCAxRXBUF
#define OFS_UCxTXBUF    OFS_UCAxTXBUF
#else
#define OFS_UCxCTLW0    (0x0000)
#define OFS_UCxBRW      (0x0006)
#define OFS_UCxRXBUF    (0x000C)
#define OFS_UCxTXBUF    (0x000E)
#endif

/**
 * Status word register
 *  - offset from base for specific type
 */
#if ! defined(OFS_UCAxSTATW) && ! defined(OFS_UCBxSTATW)
#define OFS_UCAxSTATW   (0x000A)
#define OFS_UCBxSTATW   (0x0008)
#endif

/**
 * Interrupt control base
 *  - offset of corresponding interrupt enable register from base for specific type
 */
#if defined(OFS_UCAxIE) && defined(OFS_UCBxIE)
#define OFS_UCAxICTL    OFS_UCAxIE
#define OFS_UCBxICTL    OFS_UCBxIE
#elif defined(__USCI_LEGACY_SUPPORT__)
#define OFS_UCAxICTL    (0x001C)
#else
#define OFS_UCAxICTL    (0x001A)
#define OFS_UCBxICTL    (0x002A)
#endif

#define OFS_ICTL_UCxIE  (0x0000)

/**
 * Interrupt flag register and interrupt vector generator
 *  - offset from interrupt control base
 */
#if defined(OFS_UCAxIFG) && defined(OFS_UCAxIV)
#define OFS_ICTL_UCxIFG (OFS_UCAxIFG - OFS_UCAxICTL)
#define OFS_ICTL_UCxIV  (OFS_UCAxIV - OFS_UCAxICTL)
#elif defined(__USCI_LEGACY_SUPPORT__)
#define OFS_ICTL_UCxIFG (0x0001)
#define OFS_ICTL_UCxIV  (0x0002)
#else
#define OFS_ICTL_UCxIFG (0x0002)
#define OFS_ICTL_UCxIV  (0x0004)
#endif

// -------------------------------------------------------------------------------------

/**
 * Vector no definition prefix
 *  - most devices define USCI_xx_VECTOR even when it is eUSCI device actually
 */
#if defined(EUSCI_A0_VECTOR)
#define _EUSCI_PREFIX_   EUSCI
#else
#define _EUSCI_PREFIX_   USCI
#endif

/**
 * EUSCI_BASE(A, 2)             -> EUSCI_A2_BASE
 * EUSCI_A_BASE(1)              -> EUSCI_A1_BASE
 * EUSCI_B_BASE(INTERFACE_NO)   -> EUSCI_B1_BASE when INTERFACE_NO defined as '1'
 */
#define EUSCI_BASE(type, no)    __EUSCI_BASE_EX__(_EUSCI_PREFIX_, type, no)
#define EUSCI_A_BASE(no)        __EUSCI_BASE_EX__(_EUSCI_PREFIX_, A, no)
#define EUSCI_B_BASE(no)        __EUSCI_BASE_EX__(_EUSCI_PREFIX_, B, no)
// concatenation of expanded parameters
#define __EUSCI_BASE_EX__(prefix, type, no) __EUSCI_BASE_EX_2__(prefix, type, no)
#define __EUSCI_BASE_EX_2__(prefix, type, no) prefix ## _ ## type ## no ## _BASE

/**
 * __attribute__((interrupt(EUSCI_VECTOR(type, no))))
 *   - EUSCI_VECTOR(A, 2)               -> EUSCI_A2_VECTOR
 *   - EUSCI_VECTOR(B, 0)               -> EUSCI_B0_VECTOR
 *   - EUSCI_VECTOR(B, INTERFACE_NO)    -> EUSCI_B1_VECTOR when INTERFACE_NO defined as '1'
 * __attribute__((interrupt(EUSCI_A_VECTOR(no))))
 *   - EUSCI_A_VECTOR(2)                -> EUSCI_A2_VECTOR
 *   - EUSCI_A_VECTOR(INTERFACE_NO)     -> EUSCI_A1_VECTOR when INTERFACE_NO defined as '1'
 * __attribute__((interrupt(EUSCI_B_VECTOR(no))))
 *   - EUSCI_B_VECTOR(2)                -> EUSCI_B2_VECTOR
 *   - EUSCI_B_VECTOR(INTERFACE_NO)     -> EUSCI_B1_VECTOR when INTERFACE_NO defined as '1'
 */
#define EUSCI_VECTOR(type, no)   VECTOR(_EUSCI_PREFIX_, __EUSCI_VECTOR_EX__(type, no), _)
#define EUSCI_A_VECTOR(no)       VECTOR(_EUSCI_PREFIX_, __EUSCI_VECTOR_EX__(A, no), _)
#define EUSCI_B_VECTOR(no)       VECTOR(_EUSCI_PREFIX_, __EUSCI_VECTOR_EX__(B, no), _)
// concatenation of expanded parameters
#define __EUSCI_VECTOR_EX__(type, no)                   type ## no

// -------------------------------------------------------------------------------------

#define _EUSCI_driver_(_driver)                 ((EUSCI_driver_t *) (_driver))
#define EUSCI_event_handler(_handler)           ((eusci_event_handler_t) (_handler))

/**
 * eUSCI driver public API access
 */
// CTLW0 (control word) register
#define EUSCI_control_reg(_driver) _EUSCI_base_offset_reg_16(_driver, 0)
// BRW register
#define EUSCI_bitrate_control_reg(_driver) _EUSCI_base_offset_reg_16(_driver, OFS_UCxBRW)
// STATW (status word) register
#define EUSCI_status_reg(_driver) hw_register_16(_EUSCI_driver_(_driver)->_STATW_register)
// RX / TX buffers
#define EUSCI_RX_buffer(_driver) hw_register_8(EUSCI_RX_buffer_address(_driver))
#define EUSCI_TX_buffer(_driver) hw_register_8(EUSCI_TX_buffer_address(_driver))
// interrupt control registers
#ifndef __USCI_LEGACY_SUPPORT__
#define EUSCI_IE_reg(_driver) hw_register_16(_EUSCI_driver_(_driver)->_IV_register - OFS_ICTL_UCxIV + OFS_ICTL_UCxIE)
#define EUSCI_IFG_reg(_driver) hw_register_16(_EUSCI_driver_(_driver)->_IV_register - OFS_ICTL_UCxIV + OFS_ICTL_UCxIFG)
#define EUSCI_IV_reg(_driver) hw_register_16(_EUSCI_driver_(_driver)->_IV_register)
#else
#define EUSCI_IE_reg(_driver) hw_register_8(_EUSCI_driver_(_driver)->_IV_register - OFS_ICTL_UCxIV + OFS_ICTL_UCxIE)
#define EUSCI_IFG_reg(_driver) hw_register_8(_EUSCI_driver_(_driver)->_IV_register - OFS_ICTL_UCxIV + OFS_ICTL_UCxIFG)
#define EUSCI_IV_reg(_driver) hw_register_8(_EUSCI_driver_(_driver)->_IV_register)
#endif
// RX / TX buffer address for DMA channel control
#define EUSCI_RX_buffer_address(_driver) ((void *) (_EUSCI_driver_(_driver)->_CTLW0_register + OFS_UCxRXBUF))
#define EUSCI_TX_buffer_address(_driver) ((void *) (_EUSCI_driver_(_driver)->_CTLW0_register + OFS_UCxTXBUF))

// software reset
#define EUSCI_reset_enable(_driver) EUSCI_control_reg(_driver) |= UCSWRST
#define EUSCI_reset_disable(_driver) EUSCI_control_reg(_driver) &= ~UCSWRST
// interrupt control
#define EUSCI_interrupt_enable(_driver, _mask) EUSCI_IE_reg(_driver) |= _mask
#define EUSCI_interrupt_disable(_driver, _mask) EUSCI_IE_reg(_driver) &= ~(_mask)

// general 16-bit register on given offset from eUSCI_base direct access
#define _EUSCI_base_offset_reg_16(_driver, _offset) hw_register_16(_EUSCI_driver_(_driver)->_CTLW0_register + _offset)

// getter, setter
#define EUSCI_owner(_driver) _EUSCI_driver_(_driver)->_owner
#define EUSCI_event_arg(_driver) _EUSCI_driver_(_driver)->_event_arg

/**
 * eUSCI driver public API return codes
 */
#define EUSCI_OK                        (0x00)
#define EUSCI_UNSUPPORTED_OPERATION     (0x20)

// -------------------------------------------------------------------------------------

typedef struct EUSCI_driver EUSCI_driver_t;
typedef void (*eusci_event_handler_t)(void *owner, void *event_arg);

typedef enum {
    A,  // eUSCI_A (UART and SPI)
    B   // eUSCI_B (SPI and I2C)
} EUSCI_type;

/**
 * eUSCI generic driver
 */
struct EUSCI_driver {
    // vector wrapper, enable dispose(EUSCI_driver_t *)
    Vector_handle_t vector;
    // base of eUSCI registers, (address of corresponding UCxCTLW0 register)
    uint16_t _CTLW0_register;
    // address of corresponding UCxSTAT register
    uint16_t _STATW_register;
    // interrupt vector register
    uint16_t _IV_register;

    // -------- state --------
    // interrupt service first argument, driver itself by default
    void *_owner;
    // interrupt service second argument
    void *_event_arg;

};

// -------------------------------------------------------------------------------------

void EUSCI_driver_register(EUSCI_driver_t *driver, uint16_t base, EUSCI_type type,
        uint8_t vector_no, dispose_function_t dispose_hook);


#endif /* _DRIVER_EUSCI_H_ */
