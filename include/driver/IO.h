/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  IO port driver for MSP430 F5xx_6xx, FR5xx_6xx, FR2xx_4xx, FR57xx and later devices
 *   - direct port register access compatible across all devices
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _DRIVER_IO_H_
#define _DRIVER_IO_H_

#include <msp430.h>
#include <stdint.h>
#include <driver/config.h>
#include <driver/cpu.h>
#include <driver/disposable.h>
#include <driver/vector.h>

// -------------------------------------------------------------------------------------

// 8-bit access
#define PORT_1      1
#define PORT_2      2
#define PORT_3      3
#define PORT_4      4
#define PORT_5      5
#define PORT_6      6
#define PORT_7      7
#define PORT_8      8
#define PORT_9      9
#define PORT_10     10
#define PORT_11     11
// 16-bit access
#define PORT_A      1
#define PORT_B      3
#define PORT_C      5
#define PORT_D      7
#define PORT_E      9
#define PORT_F      11

#define PIN_0       (0x0001)
#define PIN_1       (0x0002)
#define PIN_2       (0x0004)
#define PIN_3       (0x0008)
#define PIN_4       (0x0010)
#define PIN_5       (0x0020)
#define PIN_6       (0x0040)
#define PIN_7       (0x0080)
#define PIN_8       (0x0100)
#define PIN_9       (0x0200)
#define PIN_10      (0x0400)
#define PIN_11      (0x0800)
#define PIN_12      (0x1000)
#define PIN_13      (0x2000)
#define PIN_14      (0x4000)
#define PIN_15      (0x8000)

// -------------------------------------------------------------------------------------

/**
 * Port base address
 */
#ifdef PA_BASE
#define PORT_A_BASE         (PA_BASE)
#endif
#ifdef PB_BASE
#define PORT_B_BASE         (PB_BASE)
#endif
#ifdef PC_BASE
#define PORT_C_BASE         (PC_BASE)
#endif
#ifdef PD_BASE
#define PORT_D_BASE         (PD_BASE)
#endif
#ifdef PE_BASE
#define PORT_E_BASE         (PE_BASE)
#endif
#ifdef PF_BASE
#define PORT_F_BASE         (PF_BASE)
#endif
#ifdef PJ_BASE
#define PORT_J_BASE         (PJ_BASE)
#endif
#ifdef P1_BASE
#define PORT_1_BASE         (P1_BASE)
#endif
#ifdef P2_BASE
#define PORT_2_BASE         (P2_BASE)
#endif
#ifdef P3_BASE
#define PORT_3_BASE         (P3_BASE)
#endif
#ifdef P4_BASE
#define PORT_4_BASE         (P4_BASE)
#endif
#ifdef P5_BASE
#define PORT_5_BASE         (P5_BASE)
#endif
#ifdef P6_BASE
#define PORT_6_BASE         (P6_BASE)
#endif
#ifdef P7_BASE
#define PORT_7_BASE         (P7_BASE)
#endif
#ifdef P8_BASE
#define PORT_8_BASE         (P8_BASE)
#endif
#ifdef P9_BASE
#define PORT_9_BASE         (P9_BASE)
#endif
#ifdef P10_BASE
#define PORT_10_BASE         (P10_BASE)
#endif
#ifdef P11_BASE
#define PORT_11_BASE         (P11_BASE)
#endif
#ifdef P12_BASE
#define PORT_12_BASE         (P12_BASE)
#endif

/**
 * Even port base shift (8-bit access)
 */
#ifndef __IO_PORT_LEGACY_SUPPORT__
#if defined(PORT_1_BASE) && defined(PORT_2_BASE)
#undef PORT_2_BASE
#define PORT_2_BASE         (PORT_1_BASE + 1)
#endif
#if defined(PORT_3_BASE) && defined(PORT_4_BASE)
#undef PORT_4_BASE
#define PORT_4_BASE         (PORT_3_BASE + 1)
#endif
#if defined(PORT_5_BASE) && defined(PORT_6_BASE)
#undef PORT_6_BASE
#define PORT_6_BASE         (PORT_5_BASE + 1)
#endif
#if defined(PORT_7_BASE) && defined(PORT_8_BASE)
#undef PORT_8_BASE
#define PORT_8_BASE         (PORT_7_BASE + 1)
#endif
#if defined(PORT_9_BASE) && defined(PORT_10_BASE)
#undef PORT_10_BASE
#define PORT_10_BASE         (PORT_9_BASE + 1)
#endif
#if defined(PORT_11_BASE) && defined(PORT_12_BASE)
#undef PORT_12_BASE
#define PORT_12_BASE         (PORT_11_BASE + 1)
#endif
#endif /* __IO_PORT_LEGACY_SUPPORT__ */

/**
 * Address of port base address
 *  - PORT_BASE(2) -> PORT_2_BASE
 *  - PORT_BASE(PORT_NO) -> PORT_1_BASE when PORT_NO defined as '1'
 */
#define PORT_BASE(NO)       _PORT_BASE_EX_(NO)
// concatenation of expanded parameter
#define _PORT_BASE_EX_(NO)  PORT_## NO ##_BASE

/**
 * Port register offsets from base address
 */
// models F5xx_6xx, FR5xx_6xx, FR2xx_4xx and FR57xx have predefined offsets
#if defined(OFS_P1IN)
#define OFS_PxIN            OFS_P1IN
#define OFS_PxOUT           OFS_P1OUT
#define OFS_PxDIR           OFS_P1DIR
#define OFS_PxREN           OFS_P1REN
#if defined(OFS_P1DS)
// f5xx_6xx family only
#define OFS_PxDS            OFS_P1DS
#endif
#if defined(OFS_P1SEL0) && defined(OFS_P1SEL1) && defined(OFS_P1SELC)
#define OFS_PxSEL0          OFS_P1SEL0
#define OFS_PxSEL1          OFS_P1SEL1
#define OFS_PxSELC          OFS_P1SELC
#elif defined(OFS_P1SEL)
// f5xx_6xx family
#define OFS_PxSEL0          OFS_P1SEL
#endif
#define OFS_PxIES           OFS_P1IES
#define OFS_PxIE            OFS_P1IE
#define OFS_PxIFG           OFS_P1IFG
// 1xx, 2xx, 3xx and 4xx models (no 16-bit access)
#elif defined(__IO_PORT_LEGACY_SUPPORT__)
#define OFS_PxIN            (0x0000)
#define OFS_PxOUT           (0x0001)
#define OFS_PxDIR           (0x0002)
#define OFS_PxIFG           (0x0003)
#define OFS_PxIES           (0x0004)
#define OFS_PxIE            (0x0005)
#define OFS_PxSEL0          (0x0006)
// not supported for 1xx and 4xx models
#define OFS_PxSEL1          (0x0021)
// not supported for 1xx and 3xx models
#define OFS_PxREN           (0x0007)
#endif /* __IO_PORT_LEGACY_SUPPORT__ */

/**
 * Address of port interrupt vector generator
 *  - odd port number: base address + 0x0e
 *  - even port number: base address - 1 + 0x1E
 *  - undefined for PORT_A - PORT_F
 */
#ifndef __IO_PORT_LEGACY_SUPPORT__
#define PORT_IV(NO)         hw_register_16(((PORT_BASE(NO)) + (_PORT_IV_EX_(NO))) & 0xFFFE)
// expand parameter, odd port number offset from base +0x0E, even port number offset +0x1E
#define _PORT_IV_EX_(NO)    ((0x000E) + ((((NO) & 0x0001) ^ 0x0001) << 4))
#endif /* __IO_PORT_LEGACY_SUPPORT__ */

/**
 * __attribute__((interrupt(PORT_VECTOR(PORT_NO))))
 *   - PORT_VECTOR(2)       -> PORT2_VECTOR
 *   - PORT_VECTOR(PORT_NO) -> PORT1_VECTOR when PORT_NO defined as '1'
 */
#define PORT_VECTOR(port_no)   VECTOR(PORT, port_no)

// -------------------------------------------------------------------------------------

/**
 * Direct port 8-bit register access
 *  - PORT_1 - PORT_12
 */
#define PORT_REG(NO, OFFSET)        hw_register_8(PORT_BASE(NO) + (OFFSET))
#define PORT_REG_IN(NO)             PORT_REG(NO, OFS_PxIN)
#define PORT_REG_OUT(NO)            PORT_REG(NO, OFS_PxOUT)
#define PORT_REG_DIR(NO)            PORT_REG(NO, OFS_PxDIR)
#define PORT_REG_REN(NO)            PORT_REG(NO, OFS_PxREN)
#if defined(OFS_PxDS)
#define PORT_REG_DS(NO)             PORT_REG(NO, OFS_PxDS)
#endif
#define PORT_REG_SEL0(NO)           PORT_REG(NO, OFS_PxSEL0)
#if defined(OFS_PxSEL1) && defined(OFS_PxSELC)
#define PORT_REG_SEL1(NO)           PORT_REG(NO, OFS_PxSEL1)
#define PORT_REG_SELC(NO)           PORT_REG(NO, OFS_PxSELC)
#endif
#define PORT_REG_IES(NO)            PORT_REG(NO, OFS_PxIES)
#define PORT_REG_IE(NO)             PORT_REG(NO, OFS_PxIE)
#define PORT_REG_IFG(NO)            PORT_REG(NO, OFS_PxIFG)

/**
 * Direct port 16-bit register access
 *  - PORT_A - PORT_F
 */
#ifndef __IO_PORT_LEGACY_SUPPORT__
#define PORT_REG_WORD(NO, OFFSET)   hw_register_16(PORT_BASE(NO) + (OFFSET))
#define PORT_REG_WORD_IN(NO)        PORT_REG_WORD_WORD(NO, OFS_PxIN)
#define PORT_REG_WORD_OUT(NO)       PORT_REG_WORD(NO, OFS_PxOUT)
#define PORT_REG_WORD_DIR(NO)       PORT_REG_WORD(NO, OFS_PxDIR)
#define PORT_REG_WORD_REN(NO)       PORT_REG_WORD(NO, OFS_PxREN)
#if defined(OFS_PxDS)
#define PORT_REG_WORD_DS(NO)        PORT_REG_WORD(NO, OFS_PxDS)
#endif
#define PORT_REG_WORD_SEL0(NO)      PORT_REG_WORD(NO, OFS_PxSEL0)
#if defined(OFS_PxSEL1) && defined(OFS_PxSELC)
#define PORT_REG_WORD_SEL1(NO)      PORT_REG_WORD(NO, OFS_PxSEL1)
#define PORT_REG_WORD_SELC(NO)      PORT_REG_WORD(NO, OFS_PxSELC)
#endif
#define PORT_REG_WORD_IES(NO)       PORT_REG_WORD(NO, OFS_PxIES)
#define PORT_REG_WORD_IE(NO)        PORT_REG_WORD(NO, OFS_PxIE)
#define PORT_REG_WORD_IFG(NO)       PORT_REG_WORD(NO, OFS_PxIFG)
#endif /* __IO_PORT_LEGACY_SUPPORT__ */

// -------------------------------------------------------------------------------------

#define _IO_port_driver_(_driver)               ((IO_port_driver_t *) (_driver))
#define _IO_pin_handle_(_handle)                ((IO_pin_handle_t *) (_handle))

// -------------------------------------------------------------------------------------

/**
 * IO driver public API access
 */
#define IO_port_driver_create(...) _IO_PORT_DRIVER_CREATE_GET_MACRO(__VA_ARGS__, IO_port_driver_create_4, IO_port_driver_create_3, IO_port_driver_create_2)(__VA_ARGS__)
#define IO_port_handle_register(_driver, _handle, _pin_mask) \
    _IO_port_driver_(_driver)->pin_handle_register(_IO_port_driver_(_driver), _IO_pin_handle_(_handle), _pin_mask)

// reg IN|OUT|DIR|REN|DS|SEL0|SEL1|SELC|IES|IE|IFG
#define IO_port_reg(_port, _reg) _IO_port_reg_offset_(_port, OFS_Px ## _reg)
#define IO_port_reg_set(_port, _reg, _mask) _IO_port_reg_offset_(_port, OFS_Px ## _reg) |= ((uint8_t) (_mask))
#define IO_port_reg_reset(_port, _reg, _mask) _IO_port_reg_offset_(_port, OFS_Px ## _reg) &= ~((uint8_t) (_mask))
#define IO_port_reg_toggle(_port, _reg, _mask) _IO_port_reg_offset_(_port, OFS_Px ## _reg) ^= ((uint8_t) (_mask))
#define _IO_port_reg_offset_(_port, _offset) hw_register_8((_port)->_base_register + (_offset))

// reg IN|OUT|DIR|REN|DS|SEL0|SEL1|SELC|IES|IE|IFG
#define IO_pin_handle_reg(_handle, _reg) _IO_pin_handle_reg_offset_(_handle, OFS_Px ## _reg)
#define IO_pin_handle_reg_set(_handle, _reg) _IO_pin_handle_reg_offset_(_handle, OFS_Px ## _reg) |= (_handle)->_pin_mask
#define IO_pin_handle_reg_reset(_handle, _reg) _IO_pin_handle_reg_offset_(_handle, OFS_Px ## _reg) &= ~((_handle)->_pin_mask)
#define IO_pin_handle_reg_toggle(_handle, _reg) _IO_pin_handle_reg_offset_(_handle, OFS_Px ## _reg) ^= ((_handle)->_pin_mask)
#define _IO_pin_handle_reg_offset_(_handle, _offset) hw_register_8((_handle)->_base_register + (_offset))

//<editor-fold desc="variable-args - IO_port_driver_create()">
#define _IO_PORT_DRIVER_CREATE_GET_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define IO_port_driver_create_2(_driver, _port_no) \
    IO_port_driver_register(_driver, _port_no, PORT_BASE(_port_no), PORT_VECTOR(_port_no), NULL, 0x00)
#define IO_port_driver_create_3(_driver, _port_no, _port_init) \
    IO_port_driver_register(_driver, _port_no, PORT_BASE(_port_no), PORT_VECTOR(_port_no), _port_init, 0x00)
#define IO_port_driver_create_4(_driver, _port_no, _port_init, _low_power_mode_pin_reset_filter) \
    IO_port_driver_register(_driver, _port_no, PORT_BASE(_port_no), PORT_VECTOR(_port_no), _port_init, _low_power_mode_pin_reset_filter)
//</editor-fold>

/**
 * Disable the GPIO power-on default high-impedance mode
 */
#define IO_unlock() \
    PM5CTL0 &= ~LOCKLPM5;

/**
 * IO driver public API return codes
 */
#define IO_OK                               (0x00)
#define IO_UNSUPPORTED_OPERATION            (0x20)
#define IO_PIN_HANDLE_REGISTERED_ALREADY    (0x21)

// -------------------------------------------------------------------------------------

typedef struct IO_port_driver IO_port_driver_t;
typedef struct IO_pin_handle IO_pin_handle_t;

/**
 * Physical IO port control
 */
struct IO_port_driver {
    // enable dispose(IO_port_driver_t *)
    Disposable_t _disposable;
    // base of HW IO port registers, (address of corresponding PxIN register)
    uint16_t _base_register;
    // port interrupt vector number
    uint8_t _vector_no;
    // port number (1 - 12)
    uint8_t _port_no;
    // interrupt vector register
    uint16_t _IV_register;
    // pin mask the function of which shall not be reset in IO_low_power_mode_prepare()
    uint8_t _low_power_mode_pin_reset_filter;
#ifndef __IO_PORT_LEGACY_SUPPORT__
    // if set then port state can be recovered after power-on-reset
    void (*_port_init)(IO_port_driver_t *_this);
#endif
    // -------- state --------
    IO_pin_handle_t *_pin0_handle;
    IO_pin_handle_t *_pin1_handle;
    IO_pin_handle_t *_pin2_handle;
    IO_pin_handle_t *_pin3_handle;
    IO_pin_handle_t *_pin4_handle;
    IO_pin_handle_t *_pin5_handle;
    IO_pin_handle_t *_pin6_handle;
    IO_pin_handle_t *_pin7_handle;
    // shared vector slot
    Vector_slot_t *_slot;

    // -------- public --------
    // register handle for given pin mask
    uint8_t (*pin_handle_register)(IO_port_driver_t *_this, IO_pin_handle_t *handle, uint8_t pin_mask);

};

/**
 * IO pin mask wrapper
 */
struct IO_pin_handle {
    // vector wrapper, enable dispose(IO_pin_handle_t *)
    Vector_handle_t vector;
    // base of HW IO port registers - duplicated from driver to allow direct register access even after disposed
    uint16_t _base_register;
    // pin mask serviced by this handle
    uint8_t _pin_mask;
    // HW port driver reference
    IO_port_driver_t *_driver;
    // backup of original Vector_handle_t.register_handler
    Vector_slot_t *(*_register_handler_parent)(Vector_handle_t *_this, vector_slot_handler_t, void *, void *);

    // -------- state --------
    // vector interrupt service handler
    vector_slot_handler_t _handler;
    // vector interrupt service handler argument 1 (argument 2 is the interrupt source pin)
    void *_handler_arg;

};

/**
 * Initialize port driver
 *  - port_no, base and vector_no must match (or just use IO_port_driver_create(driver, port_no) macro instead)
 *  - on FRAM devices driver can survive LPMx.5 (device restart in general), in that case:
 *    - driver and all handles registered on it must be persistent (must be placed in FRAM in some noinit / persistent section)
 *    - driver must have the 'port_init' function pointer set
 *    - before entering LPMx.5 the IO_low_power_mode_prepare() should be called - this is optional and if this is not called
 * then original content of used interrupt vectors shall be lost, also pin function shall not be reset to general-purpose IO
 *      - if function of some pins should be persisted in LPMx.5 (e.g. LFXIN and LFXOUT) then set the low_power_mode_pin_reset_filter
 *    - after device wakeup the IO_wakeup_reinit() must be called, then state of all ports shall be reset and interrupts
 * shall be serviced via registered handlers, the order shall correspond to priorities of port interrupt vectors and pin interrupt flags
 */
void IO_port_driver_register(IO_port_driver_t *driver, uint8_t port_no, uint16_t base, uint8_t vector_no,
        void (*port_init)(IO_port_driver_t *), uint8_t low_power_mode_pin_reset_filter);

// -------------------------------------------------------------------------------------

/**
 * Reinitialize all registered IO drivers and handles after power-on-reset (typically after LPMx.5 wakeup)
 *  - all IO drivers and handles must be declared persistent (must be stored in non-volatile address space)
 *    - wakeup only supported on FRAM devices, otherwise just LOCKLPM5 in PM5CTL0 is cleared
 *  - initialization follows MSP430 user guide 'Exit and Wake up From LPMx.5':
 *   1. Initialize the port registers exactly the same way as they were configured before the device entered LPM4.5, but do not enable port interrupts.
 *   2. Clear the LOCKLPM5 bit in the PM5CTL0 register.
 *   3. Enable port interrupts as necessary. -> enable all interrupts that were enabled before device entered low-power mode
 *   4. After enabling the port interrupts the wake-up interrupt will be serviced as a normal interrupt.
 */
void IO_wakeup_reinit(void);

/**
 * Prepare all registered IO drivers and handles for low power mode
 *  - all pins are set to general-purpose IO except those that are in driver->low_power_mode_pin_reset_filter
 *  - interrupts should be disabled already before calling this function
 */
void IO_low_power_mode_prepare(void);


#endif /* _DRIVER_IO_H_ */
