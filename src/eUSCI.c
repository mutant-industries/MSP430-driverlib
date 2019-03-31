// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/eUSCI.h>
#include <stddef.h>


// EUSCI_driver_t constructor
void EUSCI_driver_register(EUSCI_driver_t *driver, uint16_t base, EUSCI_type type,
        uint8_t vector_no, dispose_function_t dispose_hook) {

    zerofill(driver);

    // private
    driver->_CTLW0_register = base;
#ifndef __USCI_LEGACY_SUPPORT__
    driver->_STATW_register = base + (type == A ? OFS_UCAxSTATW : OFS_UCBxSTATW);
    driver->_IV_register = base + (type == A ? OFS_UCAxICTL : OFS_UCBxICTL) + OFS_ICTL_UCxIV;
#else
    // USCI constant register offset
    driver->_STATW_register = base + OFS_UCAxSTATW;
    driver->_IV_register = base + OFS_UCAxICTL + OFS_ICTL_UCxIV;
#endif
    driver->_owner = driver;

    // implicit software reset enable
    EUSCI_reset_enable(driver);

    // vector_trigger(), and vector_clear_interrupt_flag() are not supported, vector_set_enabled() manipulates whole IE register
    vector_handle_register(&driver->vector, (dispose_function_t) dispose_hook, vector_no,
#ifndef __USCI_LEGACY_SUPPORT__
            driver->_IV_register - OFS_ICTL_UCxIV + OFS_ICTL_UCxIE, (uint16_t) ~0, NULL, NULL);
#else
            // USCI 8-bit IE register access
            driver->_IV_register - OFS_ICTL_UCxIV + OFS_ICTL_UCxIE, (uint16_t) 0xFF, NULL, NULL);
#endif
}
