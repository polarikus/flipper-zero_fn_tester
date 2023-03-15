//
// Created by Игорь Данилов on 13.03.2023.
//

#pragma once
#include "furi.h"

typedef struct Date {
    uint8_t year;
    uint8_t mouth;
    uint8_t day;
} Date;

typedef enum {
    FNStage1 = 1,
    FNStage2 = 3,
    FNStage3 = 7,
    FNStage4 = 15
} FNState;