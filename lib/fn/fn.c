//
// Created by Игорь Данилов on 23.02.2023.
//
#include "fn_i.h"

#define FFD_1_STR "1"
#define FFD_1_05_STR "1.05"
#define FFD_1_1_STR "1.1"
#define FFD_1_2_STR "1.2"


uint32_t fn_get_last_document_number(const FNInfo* fn){
    return (fn->last_doc_number);
}

char* fn_get_fn_state(const FNInfo* fn)
{
    switch(fn->fn_state) {
    case FNStage1: return "Ready registration (1)";
    case FNStage2: return "Registered (2)";
    case FNStage3: return "The closed FN archive is awaiting transfer to the OFD (3)";
    case FNStage4: return "FN archive is closed (4)";
    default: return "Unknown";
    }
}

void fn_get_last_document_datetime(const FNInfo* fn, FuriString* string){
    furi_string_cat_printf(string, "%02d.%02d.20%02d %02d:%02d",
    fn->date_time.date, fn->date_time.mouth, fn->date_time.year,
                       fn->date_time.hour, fn->date_time.minute);
};

void fn_info_copy(FNInfo* dest, const FNInfo* src)
{
    memcpy(dest, src, sizeof(FNInfo));
}

bool fn_is_mgm(const FNInfo* fn)
{
    return !(fn->fw_version.fw_mode);
}

bool fn_is_session_open(const FNInfo* fn)
{
    return (fn->is_session_open);
}

const char* fn_get_fw_version(const FNInfo* fn)
{
    return fn->fw_version.fw_version;
}

const char* fn_get_sn(const FNInfo* fn)
{
    FURI_LOG_D("FN", "SN: %s", fn->serial_number);
    return fn->serial_number;
}

const char* fn_get_revision(const FNInfo* fn)
{
    return fn->revision_name;
}

const char* fn_get_ffd_version_string(const FNInfo* fn)
{
    switch(fn->ffd_enum) {
    case FFD_1:
        return FFD_1_STR;
    case FFD_1_05:
        return FFD_1_05_STR;
    case FFD_1_1:
        return FFD_1_1_STR;
    case FFD_1_2:
        return FFD_1_2_STR;
    default:
        return "unknown";
    }
}

uint8_t fn_get_ffd_enum(const FNInfo* fn){
    return fn->ffd_enum;
}