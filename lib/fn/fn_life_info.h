//
// Created by Игорь Данилов on 13.03.2023.
//
#include <furi.h>

typedef struct FNLifeInfo FNLifeInfo;

FNLifeInfo* fn_life_info_alloc();

void fn_life_info_free(FNLifeInfo* life_info);

void fn_life_info_get_end_date(const FNLifeInfo* lifeInfo, FuriString* tmp_string);

uint8_t fn_life_info_get_reg_report_ctn_remaining(const FNLifeInfo* lifeInfo);

uint8_t fn_life_info_get_reg_report_ctn(const FNLifeInfo* lifeInfo);

uint32_t fn_life_info_get_five_year_data_resource(const FNLifeInfo* lifeInfo);

uint32_t fn_life_info_get_thirty_year_data_resource(const FNLifeInfo* lifeInfo);

uint32_t fn_life_info_get_marking_notifications_resource(const FNLifeInfo* lifeInfo);

uint16_t fn_life_info_get_days_to_end(const FNLifeInfo* lifeInfo);