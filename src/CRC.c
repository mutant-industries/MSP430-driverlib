// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <driver/CRC.h>
#include <compiler.h>
#include <driver/cpu.h>


#ifdef __CRC_16_HW_SUPPORT__


static void _seed(CRC_driver_t *_, crc_16_t seed) {
    hw_register_16(CRC_BASE + OFS_CRCINIRES) = seed;
}

static void _consume_byte(CRC_driver_t *_, uint8_t input) {
    hw_register_8(CRC_BASE + OFS_CRCDIRB) = input;
}

static void _consume_word(CRC_driver_t *_, uint16_t input) {
    hw_register_16(CRC_BASE + OFS_CRCDIRB) = input;
}

static crc_16_t _result(CRC_driver_t *_) {
    return hw_register_16(CRC_BASE + OFS_CRCINIRES);
}

#endif

// -------------------------------------------------------------------------------------

static bool _software_fallback_initialized;

/**
 * CRC_CCITT lookup table (case when SW fallback is used)
 */
__noinit static uint16_t _ccitt_crc_table[256];

/**
 * Based on 'A Painless Guide To CRC Error Detection Algorithms'
 *  - {@see http://www.ross.net/crc/download/crc_v3.txt}
 */
static void _generate_ccitt_crc_table() {
    uint16_t i, j, high_bit_set, crc;

    for (i = 0; i < 256; i++) {
        crc = i << 8;

        for (j = 0; j < 8; j++) {
            high_bit_set = crc & 0x8000;
            crc <<= 1;

            if (high_bit_set) {
                crc ^= 0x1021;
            }
        }

        _ccitt_crc_table[i] = crc;
    }
}

static void _seed_fallback(CRC_driver_t *_this, crc_16_t seed) {
    _this->_state = seed;
}

static void _consume_byte_fallback(CRC_driver_t *_this, uint8_t input) {
    _this->_state = _ccitt_crc_table[(_this->_state >> 8 ^ input) & 0xffU] ^ (_this->_state << 8);
}

static void _consume_word_fallback(CRC_driver_t *_this, uint16_t input) {
    _this->_state = _ccitt_crc_table[(_this->_state >> 8 ^ (uint8_t) input) & 0xffU] ^ (_this->_state << 8);
    _this->_state = _ccitt_crc_table[(_this->_state >> 8 ^ (uint8_t) (input >> 8)) & 0xffU] ^ (_this->_state << 8);
}

static crc_16_t _result_fallback(CRC_driver_t *_this) {
    return _this->_state;
}

// -------------------------------------------------------------------------------------

static uint16_t _calculate(CRC_driver_t *_this, void *address, uint16_t size, crc_16_t seed) {
    uintptr_t i = (uintptr_t) address;

    // nothing to be done
    if ( ! size) {
        return seed;
    }

    CRC_seed(_this, seed);

    // odd address
    if (i & 0x01) {
        CRC_consume_byte(_this, *((uint8_t *) i++));
    }

    // even addresses, word read
    for (; i < (((uintptr_t) address + size) & ~0x01); i += 2) {
        CRC_consume_word(_this, *((uint16_t *) i));
    }

    // odd address
    if (((uintptr_t) address + size) & 0x01) {
        CRC_consume_byte(_this, *((uint8_t *) i));
    }

    return CRC_result(_this);
}

// -------------------------------------------------------------------------------------

void CRC_driver_register(CRC_driver_t *driver, bool software_fallback) {
#ifdef __CRC_16_HW_SUPPORT__
    driver->seed = software_fallback ? _seed_fallback : _seed;
    driver->consume_byte = software_fallback ? _consume_byte_fallback : _consume_byte;
    driver->consume_word  =software_fallback ? _consume_word_fallback : _consume_word;
    driver->result = software_fallback ? _result_fallback : _result;
#else
    software_fallback = true;

    driver->seed = _seed_fallback;
    driver->consume_byte = _consume_byte_fallback;
    driver->consume_word  =_consume_word_fallback;
    driver->result = _result_fallback;
#endif

    if (software_fallback && ! _software_fallback_initialized) {
        _generate_ccitt_crc_table();
    }

    driver->calculate = _calculate;
}
