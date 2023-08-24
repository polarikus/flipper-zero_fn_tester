//
// Created by Игорь Данилов on 24.08.2023.
//
#pragma once
#include "furi.h"

static uint32_t uint32TestVal[] = {0, 4294967295};
static uint16_t uint16TestVal[] = {0, 65535};
static uint8_t uint8TestVal[] = {0, 255};
static bool testBoolVal[] = {true, false};

#define BTV_UNUSED(suite_name) \
    do {                       \
        UNUSED(uint32TestVal); \
        UNUSED(uint16TestVal); \
        UNUSED(uint8TestVal);  \
        UNUSED(testBoolVal);   \
    } while(0)
