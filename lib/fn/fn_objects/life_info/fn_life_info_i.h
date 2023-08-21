//
// Created by Игорь Данилов on 13.03.2023.
//

#pragma once
#include "fn_life_info.h"
#include "data_types/fn_data_types.h"

struct FNLifeInfo {
    Date date_end;
    uint16_t days_to_end;
    uint8_t reg_report_ctn_remaining;
    uint8_t reg_report_ctn;
    //FN-M ONLY
    uint32_t five_year_data_resource;
    uint32_t thirty_year_data_resource;
    uint32_t marking_notifications_resource;//FFD 1.2 only
};
