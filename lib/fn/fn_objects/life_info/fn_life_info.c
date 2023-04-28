//
// Created by Игорь Данилов on 13.03.2023.
//
#include "fn_life_info_i.h"

FNLifeInfo* fn_life_info_alloc(){
    FNLifeInfo* life_info = malloc(sizeof(FNLifeInfo));
    life_info->date_end.mouth = 0;
    life_info->date_end.year = 0;
    life_info->date_end.day = 0;
    life_info->days_to_end = 0;
    life_info->marking_notifications_resource = 0;
    life_info->five_year_data_resource = 0;
    life_info->thirty_year_data_resource = 0;
    return life_info;
}

void fn_life_info_free(FNLifeInfo* life_info){
    free(life_info);
}

//life_info->date_end.day , life_info->date_end.mouth, life_info->date_end.year

void fn_life_info_get_end_date(const FNLifeInfo* lifeInfo, FuriString* tmp_string){
    if(lifeInfo->date_end.year == 0){
        furi_string_cat_printf(tmp_string, "not registered");
    } else{
        furi_string_cat_printf(tmp_string, "%02d.%02d.20%02d",
                               lifeInfo->date_end.day , lifeInfo->date_end.mouth, lifeInfo->date_end.year);
    }
}

uint8_t fn_life_info_get_reg_report_ctn_remaining(const FNLifeInfo* lifeInfo){
    return (lifeInfo->reg_report_ctn_remaining);
}

uint8_t fn_life_info_get_reg_report_ctn(const FNLifeInfo* lifeInfo){
    return (lifeInfo->reg_report_ctn);
}

uint32_t fn_life_info_get_five_year_data_resource(const FNLifeInfo* lifeInfo){
    return (lifeInfo->five_year_data_resource);
}

uint32_t fn_life_info_get_thirty_year_data_resource(const FNLifeInfo* lifeInfo){
    return (lifeInfo->thirty_year_data_resource);
}

uint32_t fn_life_info_get_marking_notifications_resource(const FNLifeInfo* lifeInfo){
    return (lifeInfo->marking_notifications_resource);
}

uint16_t fn_life_info_get_days_to_end(const FNLifeInfo* lifeInfo){
    return (lifeInfo->days_to_end);
}