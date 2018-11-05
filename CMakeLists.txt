# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2018-2019 Mutant Industries ltd.
cmake_minimum_required(VERSION 3.2)

project(MSP430-driverlib VERSION 0.1 LANGUAGES C)

add_library(MSP430-driverlib
        src/disposable.c
        src/vector.c
        src/timer.c)

target_include_directories(MSP430-driverlib
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
        PRIVATE src)

export(TARGETS MSP430-driverlib FILE MSP430-driverlib.cmake)
