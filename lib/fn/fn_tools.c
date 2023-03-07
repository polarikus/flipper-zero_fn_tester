//
// Created by Игорь Данилов on 05.03.2023.
//

#include "fn_tools.h"
#include "fn_uart_i.h"
#include "fn_helpers.h"
#include "fn_uart_i.h"

typedef struct FNAnswer {
    FNError error;
    uint16_t data_len;
    uint8_t* data;
} FNAnswer;



